#if GY_87
/* if you want to use DMP just switch to GY_521 */

#define FILTER_MADGWICK			1
#define FILTER_MAHONY			0

#define FILTER_AHRS			1
#define FILTER_IMU			0

#include "src/i2cdevlib/MPU6050.h"
#include "src/i2cdevlib/HMC5883L.h"

#if FILTER_MADGWICK
#define FILTER_BETA_NORMAL		0.02f
#define FILTER_BETA_STAB		4.0f
#include "src/MadgwickAHRS/MadgwickAHRS.h"		// pls note, the .h has been modified in order to set custom 'beta' and get q-vectors
static Madgwick filter;
#endif

#if FILTER_MAHONY
#define FILTER_KP_NORMAL		1.0f
#define FILTER_KP_STAB			6.0f
#include "src/MahonyAHRS/MahonyAHRS.h"			// pls note, the .h has been modified in order to set custom 'Kp' and get q-vectors
static Mahony filter;
#endif

// at least 8g-1000d/s or more is required for self-balancing physics..
static uint8_t acc_scale = MPU6050_ACCEL_FS_8;
static uint8_t gyro_scale = MPU6050_GYRO_FS_1000;

static uint8_t mag_rate = HMC5883L_RATE_30;			// Output Data Rate (Hz)
static uint8_t mag_gain = HMC5883L_GAIN_1370;			// Default is _1090
static uint8_t mag_avg = HMC5883L_AVERAGING_4;
static uint8_t mag_bias = HMC5883L_BIAS_NORMAL;
static uint8_t mag_mode = HMC5883L_MODE_CONTINUOUS;

static float acc_res;						// converter LSB to g (9.8 m/s^2)
static float gyro_res;						// converter LSB to deg/sec
static float mag_res;						// converter LSB to mGauss

// Put here actual offsets manually (are obtained by DEBUG_IMU_CALIBRATION):
static int16_t acc_offsets[3] = { -1048, 1969, 1374 };
static int16_t gyro_offsets[3] = { 91, -18, 9 };
// See calibration_hmc5883l/HOWTO.txt
static float mag_hard_iron_bias[3] = { -391.5f, -164.5f, 505.5f };		// in LSB
static float mag_soft_iron_bias[3] = { 1.001281f, 0.962450f, 1.039216f };	// in LSB

static MPU6050 mpu;
static HMC5883L mag;


void imu_init()
{
	bool mpu_ready = false;
	bool mag_ready = false;

	Wire.begin();
	Wire.setClock(400000);

	Serial.println(F("is MPU6050 online?"));
	while (!mpu_ready)
		mpu_ready = mpu.testConnection();
	Serial.println(F("MPU6050 is online"));

	Serial.println(F("MPU6050 init.."));
	mpu.initialize();				// i2cdevlib: GYRO_FS_250 (deg/sec), ACCEL_FS_2 (g)
	delay(100);

#if DEBUG_IMU_CALIBRATION				// now ACCEL_FS_2, GYRO_FS_250
	mpu.PrintActiveOffsets();
	Serial.println(F("MPU6050 calibrating (6 x 100 readings).."));
	mpu.CalibrateAccel(6);
	mpu.CalibrateGyro(6);
	mpu.PrintActiveOffsets();
	Serial.println();

	Serial.print(F("int16_t acc_offsets[3] = { "));
	Serial.print(mpu.getXAccelOffset());	Serial.print(F(", "));
	Serial.print(mpu.getYAccelOffset());	Serial.print(F(", "));
	Serial.print(mpu.getZAccelOffset());	Serial.println(F(" };"));

	Serial.print(F("int16_t gyro_offsets[3] = { "));
	Serial.print(mpu.getXGyroOffset());	Serial.print(F(", "));
	Serial.print(mpu.getYGyroOffset());	Serial.print(F(", "));
	Serial.print(mpu.getZGyroOffset());	Serial.println(F(" };"));
#else
	mpu.setXAccelOffset(acc_offsets[0]);
	mpu.setYAccelOffset(acc_offsets[1]);
	mpu.setZAccelOffset(acc_offsets[2]);

	mpu.setXGyroOffset(gyro_offsets[0]);
	mpu.setYGyroOffset(gyro_offsets[1]);
	mpu.setZGyroOffset(gyro_offsets[2]);

	mpu.setFullScaleAccelRange(acc_scale);
	mpu.setFullScaleGyroRange(gyro_scale);
	delay(100);

	mpu.setI2CBypassEnabled(true);

	Serial.println(F("is HMC5883L online?"));
	while (!mag_ready)
		mag_ready = mag.testConnection();
	Serial.println(F("HMC5883L is online"));

	Serial.println(F("HMC5883L init.."));
	mag.initialize();				// REG_A: Averaging=8 (1-default), rate=15Hz, Normal bias mode (00)
							// REG_B: Gain=1090 [001 00000]
							// REG_MODE: Single Mode 01 (Default)
	delay(100);

	mag.setDataRate(mag_rate);
	mag.setGain(mag_gain);
	mag.setSampleAveraging(mag_avg);
	mag.setMeasurementBias(mag_bias);
	mag.setMode(mag_mode);
	delay(100);

	get_acc_res();
	get_gyro_res();
	get_mag_res();

	__imu_stab();
#endif	// not CALIBRATION
}


/*
 This procedure speeds up and controls convergence of initial YPR received from the filter
 to actual Acc-Gyro-Mag values on early start.
*/
void __imu_stab()
{
	float x_pre, y_pre, z_pre;

	bool x_stab = false;
	bool y_stab = false;
	bool z_stab = false;

	int8_t x_delta_sign_ref, y_delta_sign_ref, z_delta_sign_ref;		// -1, 0, 1
	int8_t x_delta_sign_now, y_delta_sign_now, z_delta_sign_now;		// -1, 0, 1

	uint16_t counter = 0;
	uint32_t start_stab_ms = millis();

#if FILTER_MADGWICK
	filter.beta = FILTER_BETA_STAB;
#ifdef SYSLOG
	append_float_to_str(syslog_msg, "GY87: Stabilizing Madgwick filter params on start with beta = ", filter.beta, 4);
	logger_common(syslog_msg);
#endif
#endif

#if FILTER_MAHONY
	filter.twoKp = FILTER_KP_STAB;
#ifdef SYSLOG
	append_float_to_str(syslog_msg, "GY87: Stabilizing Mahony filter params on start with Kp = ", filter.twoKp, 4);
	logger_common(syslog_msg);
#endif
#endif
	for (uint8_t i = 0; i < 5; i++)
		imu_read_data();

	x_pre = axis_x;
	y_pre = axis_y;
	z_pre = axis_z;

	imu_read_data();

	if (x_pre != axis_x)
		x_delta_sign_ref = fabs(axis_x - x_pre) / (axis_x - x_pre);
	else
		x_delta_sign_ref = 0;

	if (y_pre != axis_y)
		y_delta_sign_ref = fabs(axis_y - y_pre) / (axis_y - y_pre);
	else
		y_delta_sign_ref = 0;

	if (z_pre != axis_z)
		z_delta_sign_ref = fabs(axis_z - z_pre) / (axis_z - z_pre);
	else
		z_delta_sign_ref = 0;

	while (!(x_stab && y_stab && z_stab)) {
		x_pre = axis_x;
		y_pre = axis_y;
		z_pre = axis_z;

		imu_read_data();

#if TILT_AXIS_X
		if (x_pre != axis_x) {
			x_delta_sign_now = fabs(axis_x - x_pre) / (axis_x - x_pre);
			if (x_delta_sign_now != x_delta_sign_ref && y_stab && z_stab)
				x_stab = true;
			else
				x_delta_sign_ref = x_delta_sign_now;
		} else if (y_stab && z_stab)
			x_stab = true;

		if (y_pre != axis_y) {
			y_delta_sign_now = fabs(axis_y - y_pre) / (axis_y - y_pre);
			if (y_delta_sign_now != y_delta_sign_ref)
				y_stab = true;
		} else
			y_stab = true;
#elif TILT_AXIS_Y
		if (x_pre != axis_x) {
			x_delta_sign_now = fabs(axis_x - x_pre) / (axis_x - x_pre);
			if (x_delta_sign_now != x_delta_sign_ref)
				x_stab = true;
		} else
			x_stab = true;

		if (y_pre != axis_y) {
			y_delta_sign_now = fabs(axis_y - y_pre) / (axis_y - y_pre);
			if (y_delta_sign_now != y_delta_sign_ref && x_stab && z_stab)
				y_stab = true;
			else
				y_delta_sign_ref = y_delta_sign_now;
		} else if (x_stab && z_stab)
			y_stab = true;
#endif

		if (z_pre != axis_z) {
			z_delta_sign_now = fabs(axis_z - z_pre) / (axis_z - z_pre);
			if (z_delta_sign_now != z_delta_sign_ref)
				z_stab = true;
		} else
			z_stab = true;
#ifdef SYSLOG
		sprintf(syslog_msg, " STAB: (%d)", counter);
		if (x_stab)
			sprintf(syslog_msg + strlen(syslog_msg), "\tX:True  ");
		else
			sprintf(syslog_msg + strlen(syslog_msg), "\tX:False ");
		dtostrf(axis_x, 7, 2, syslog_msg + strlen(syslog_msg));

		if (y_stab)
			sprintf(syslog_msg + strlen(syslog_msg), "\t\tY:True  ");
		else
			sprintf(syslog_msg + strlen(syslog_msg), "\t\tY:False ");
		dtostrf(axis_y, 7, 2, syslog_msg + strlen(syslog_msg));

		if (z_stab)
			sprintf(syslog_msg + strlen(syslog_msg), "\t\tZ:True  ");
		else
			sprintf(syslog_msg + strlen(syslog_msg), "\t\tZ:False ");
		dtostrf(axis_z, 7, 2, syslog_msg + strlen(syslog_msg));

		logger_telemetry(syslog_msg);
#endif
		counter++;
	}
	delay(20);

#if FILTER_MADGWICK
	filter.beta = FILTER_BETA_NORMAL;
#endif

#if FILTER_MAHONY
	filter.twoKp = FILTER_KP_NORMAL;
#endif
#ifdef SYSLOG
	sprintf(syslog_msg, " STAB: All YPR have stabilized in %lu ms (%d cycles)", millis() - start_stab_ms, counter);
	logger_common(syslog_msg);
#endif
}


void imu_read_data()
{
	int16_t acc_raw_data[3];
	int16_t gyro_raw_data[3];
	int16_t mag_raw_data[3];
	int16_t temp_raw_data;

	uint32_t now_us;
	uint32_t filter_dt_us;
	static uint32_t last_us = 0;

	float ax, ay, az;
	float gx, gy, gz;
	float mx, my, mz;
	float sample_freq;					// freq to update filter (in Hz)

	mpu.getAcceleration(&acc_raw_data[0], &acc_raw_data[1], &acc_raw_data[2]);
	ax = (float)acc_raw_data[0] * acc_res;			// converting to g (9.8 m/s^2)
	ay = (float)acc_raw_data[1] * acc_res;
	az = (float)acc_raw_data[2] * acc_res;

	mpu.getRotation(&gyro_raw_data[0], &gyro_raw_data[1], &gyro_raw_data[2]);
	gx = (float)gyro_raw_data[0] * gyro_res;		// converting to Degrees/sec
	gy = (float)gyro_raw_data[1] * gyro_res;
	gz = (float)gyro_raw_data[2] * gyro_res;

	mag.getHeading(&mag_raw_data[0], &mag_raw_data[1], &mag_raw_data[2]);
	// Applying hard- and soft-iron error compensation
	mx = ((float)mag_raw_data[0] - mag_hard_iron_bias[0]) * mag_soft_iron_bias[0] * mag_res;
	my = ((float)mag_raw_data[1] - mag_hard_iron_bias[1]) * mag_soft_iron_bias[1] * mag_res;
	mz = ((float)mag_raw_data[2] - mag_hard_iron_bias[2]) * mag_soft_iron_bias[2] * mag_res;

	now_us = micros();
	filter_dt_us = now_us - last_us;
	sample_freq = 1000000.0f / filter_dt_us;		// in Hz
	last_us = now_us;

	filter.begin(sample_freq);
#if FILTER_AHRS
	filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);	// 9-ODF
#elif FILTER_IMU
	filter.updateIMU(gx, gy, gz, ax, ay, az);		// 6-ODF
#endif

	axis_x = filter.getRoll();				// roll
	axis_y = filter.getPitch();				// pitch
	axis_z = filter.getYaw();				// yaw

#if DEBUG_IMU
	float dt_ms = (float)filter_dt_us / 1000.f;

#if DEBUG_IMU_RAW_DATA
	print_raw_ahrs_data(ax, ay, az, gx, gy, gz, mx, my, mz, axis_x, axis_y, axis_z, dt_ms);
#endif

#if DEBUG_IMU_PROCESSING_CUBE
	float temperature = (float)mpu.getTemperature() / 340.f + 36.53f;		// converting to Cels. deg
	float heading = atan2(my, mx) * 180 / M_PI;
	if (heading < 0)   heading += 360.0f;
	if (heading > 360) heading -= 360.0f;

	print_processing_cube_data(filter.q0, filter.q1, filter.q2, filter.q3, ax, ay, az, gx, gy, gz, mx, my, mz, temperature, dt_ms, heading);
#endif

#if DEBUG_IMU_PROCESSING_BUNNY
	print_processing_bunnyrotate_data(millis(), loop_number, dt_ms, axis_x, axis_y, axis_z);
#endif

#endif		// DEBUG_IMU
}


void get_acc_res()
{
	switch (acc_scale) {
		case MPU6050_ACCEL_FS_2:
			acc_res = 2.0f / 32768.0f;
			break;
		case MPU6050_ACCEL_FS_4:
			acc_res = 4.0f / 32768.0f;
			break;
		case MPU6050_ACCEL_FS_8:
			acc_res = 8.0f / 32768.0f;
			break;
		case MPU6050_ACCEL_FS_16:
			acc_res = 16.0f / 32768.0f;
			break;
	}
}


void get_gyro_res()
{
	switch (gyro_scale) {
		case MPU6050_GYRO_FS_250:
			gyro_res = 250.0f / 32768.0f;
			break;
		case MPU6050_GYRO_FS_500:
			gyro_res = 500.0f / 32768.0f;
			break;
		case MPU6050_GYRO_FS_1000:
			gyro_res = 1000.0f / 32768.0f;
			break;
		case MPU6050_GYRO_FS_2000:
			gyro_res = 2000.0f / 32768.0f;
			break;
	}
}


void get_mag_res()
{
	switch (mag_gain) {
		case HMC5883L_GAIN_1370:
			mag_res = 0.73f;
			break;
		case HMC5883L_GAIN_1090:
			mag_res = 0.92f;
			break;
		case HMC5883L_GAIN_820:
			mag_res = 1.22f;
			break;
		case HMC5883L_GAIN_660:
			mag_res = 1.52f;
			break;
		case HMC5883L_GAIN_440:
			mag_res = 2.27f;
			break;
		case HMC5883L_GAIN_390:
			mag_res = 2.56f;
			break;
		case HMC5883L_GAIN_330:
			mag_res = 3.03f;
			break;
		case HMC5883L_GAIN_220:
			mag_res = 4.35f;
			break;
	}
}
#endif	// GY_87
