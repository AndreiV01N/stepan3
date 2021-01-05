#if GY_85

#define FILTER_MADGWICK			1
#define FILTER_MAHONY			0

#define FILTER_AHRS			0
#define FILTER_IMU			1

#define ADXL345_CALIBRATION_CYCLES	200
#define ADXL345_OFFSET_REGS_SCALE	64.0f		// 64 LSB/g for offset 8-bit regs as per datasheet
#define ITG3200_CALIBRATION_CYCLES	200

#include "src/i2cdevlib/ADXL345.h"
#include "src/i2cdevlib/ITG3200.h"
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

static ADXL345 accel;
static ITG3200 gyro;
static HMC5883L mag;

// at least 8g-1000d/s or more are required for self-balancing physics..
static uint8_t acc_range = ADXL345_RANGE_16G;			// 13-bit output in full_res
static bool acc_full_res = true;

static uint8_t gyro_scale = ITG3200_FULLSCALE_2000;		// do not change this
static uint8_t gyro_clk_src = ITG3200_CLOCK_PLL_XGYRO;		// ITG3200_CLOCK_INTERNAL
static uint8_t gyro_lpf = ITG3200_DLPF_BW_256;			// _5, _10, _20, _42, _98, _188, _256 - ("_256" eq 0 is default)
static uint8_t gyro_rate = 0;					// 1kHz / (3 + 1) = 250Hz = 4ms (default is 0)

static uint8_t mag_rate = HMC5883L_RATE_30;			// Output Data Rate (Hz)
static uint8_t mag_gain = HMC5883L_GAIN_1370;			// Default is _1090
static uint8_t mag_avg = HMC5883L_AVERAGING_4;
static uint8_t mag_bias = HMC5883L_BIAS_NORMAL;
static uint8_t mag_mode = HMC5883L_MODE_CONTINUOUS;

static float acc_res;						// converter LSB to g (9.8 m/s^2)
static float gyro_res;						// converter LSB to deg/sec
static float mag_res;						// converter LSB to mGauss

// Put here calculated offsets manually (obtained by DEBUG_IMU_CALIBRATION):
static float acc_offsets[3] = { 4.22, 10.66, -12.66 };
static float gyro_offsets[3] = { 28.4f, 13.2f, -1.3f };

// See ./calibration_hmc5883l/HOWTO.txt
static float mag_hard_iron_bias[3] = { 81.0f, -174.5f, -178.0f };
static float mag_soft_iron_bias[3] = { 0.990623f, 0.972098f, 1.039682f };


void imu_init()
{
	int16_t i;
	int16_t acc_raw_data[3];
	int16_t gyro_raw_data[3];

	Serial.println(F("Starting GY-85 init.."));
	Wire.begin();

/* Accelerometer init */
	if (accel.testConnection())
		Serial.println(F("ADXL345 (Accel) test connection OK"));
	else {
		Serial.println(F("ERROR: ADXL345 (Accel) test connection failed"));
		while(1);
	}
	accel.initialize();		// setAutoSleepEnabled(true), setMeasureEnabled(true)

	if (acc_full_res)
		accel.setFullResolution(ADXL345_FORMAT_FULL_RES_BIT);

	accel.setRange(acc_range);
	delay(100);
	get_acc_res();

#if DEBUG_IMU_CALIBRATION
	float acc_offsets_tmp[3] = { 0.f, 0.f, 0.f };
	float gyro_offsets_tmp[3] = { 0.f, 0.f, 0.f };

	Serial.println(F("INFO: ADXL345 (Accel) calibration is in progress.."));

	for (i = 0; i < ADXL345_CALIBRATION_CYCLES; i++) {			// Keep the IMU motionless while calibrating
		accel.getAcceleration(&acc_raw_data[0], &acc_raw_data[1], &acc_raw_data[2]);
		acc_offsets_tmp[0] += (float)acc_raw_data[0];
		acc_offsets_tmp[1] += (float)acc_raw_data[1];
		acc_offsets_tmp[2] += (float)acc_raw_data[2];
		acc_offsets_tmp[2] -= acc_res;					// remove gravity component (256.0 LSB/g in full-res)
		delay(100);
	}

	acc_offsets_tmp[0] /= (float)ADXL345_CALIBRATION_CYCLES;
	acc_offsets_tmp[1] /= (float)ADXL345_CALIBRATION_CYCLES;
	acc_offsets_tmp[2] /= (float)ADXL345_CALIBRATION_CYCLES;

	Serial.print(F("static float acc_offsets[3] = { "));
	Serial.print(acc_offsets_tmp[0]);		Serial.print(F(", "));
	Serial.print(acc_offsets_tmp[1]);		Serial.print(F(", "));
	Serial.print(acc_offsets_tmp[2]);		Serial.println(F(" };"));
#endif

/* Gyroscope init */
	if (gyro.testConnection())
		Serial.println(F("ITG3205 (Gyro) test connection OK"));
	else {
		Serial.println(F("ERROR: ITG3205 (Gyro) test connection failed"));
		while(1);
	}
	gyro.initialize();							// FULLSCALE_2000, CLOCK_PLL_XGYRO
	delay(100);

	gyro.setFullScaleRange(gyro_scale);
	gyro.setClockSource(gyro_clk_src);
	gyro.setDLPFBandwidth(gyro_lpf);
	gyro.setRate(gyro_rate);						// 0 - default; [Fclck/(div + 1)]; Fclk is either 1kHz or 8kHz
	delay(100);

	get_gyro_res();

#if DEBUG_IMU_CALIBRATION
	Serial.println(F("INFO: ITG3200 (Gyro) calibration in progress.."));

	for (i = 0; i < ITG3200_CALIBRATION_CYCLES; i++) {			// Do not move IMU while calibrating !
		gyro.getRotation(&gyro_raw_data[0], &gyro_raw_data[1], &gyro_raw_data[2]);
		gyro_offsets_tmp[0] += gyro_raw_data[0];
		gyro_offsets_tmp[1] += gyro_raw_data[1];
		gyro_offsets_tmp[2] += gyro_raw_data[2];
		delay(100);
	}

	gyro_offsets_tmp[0] /= ITG3200_CALIBRATION_CYCLES;
	gyro_offsets_tmp[1] /= ITG3200_CALIBRATION_CYCLES;
	gyro_offsets_tmp[2] /= ITG3200_CALIBRATION_CYCLES;

	Serial.print(F("static float gyro_offsets[3] = { "));
	Serial.print(gyro_offsets_tmp[0]);		Serial.print(F("f, "));
	Serial.print(gyro_offsets_tmp[1]);		Serial.print(F("f, "));
	Serial.print(gyro_offsets_tmp[2]);		Serial.println(F("f };"));
#endif
/* Magnetometer init */
	if (mag.testConnection()) {
		Serial.println(F("HMC5883L (Magnetometer) test connection OK"));
	} else {
		Serial.println(F("ERROR: HMC5883L (Magnetometer) test connection failed"));
		while(1);
	}
	mag.initialize();							// REG_A: Averaging=8 (1-default), rate=15Hz, Normal bias mode (00)
										// REG_B: Gain=1090 [001 00000]
										// REG_MODE: Single Mode 01 (Default)
	mag.setSampleAveraging(mag_avg);					// number of samples averaged per measurement output (1*, 2, 4, 8)
	mag.setDataRate(mag_rate);						// default is 15 Hz
	mag.setMeasurementBias(mag_bias);					// _NORMAL, _POSITIVE, _NEGATIVE
	mag.setGain(mag_gain);							// default 1090 (max: 1370)
	mag.setMode(mag_mode);							// _SINGLE (default), _IDLE
	delay(100);

	get_mag_res();

	__imu_stab();
}


/*
 This procedure speeds up and controls convergence of initial YPR received from the filter
 to actual Acc-Gyro-Mag values on early start. Takes up to 5s for GY-85
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
	append_float_to_str(syslog_msg, "GY85: Stabilizing Madgwick filter params on start with beta = ", filter.beta, 4);
	logger_common(syslog_msg);
#endif
#endif

#if FILTER_MAHONY
	filter.twoKp = FILTER_KP_STAB;
#ifdef SYSLOG
	append_float_to_str(syslog_msg, "GY85: Stabilizing Mahony filter params on start with Kp = ", filter.twoKp, 4);
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

	uint32_t now_us;
	uint32_t filter_dt_us;
	static uint32_t last_us = 0;

	float ax, ay, az;
	float gx, gy, gz;
	float mx, my, mz;
	float sample_freq;

	// ADXL345 hardware OFFSET registers are not used to get more accuracy
	accel.getAcceleration(&acc_raw_data[0], &acc_raw_data[1], &acc_raw_data[2]);
	ax = ((float)acc_raw_data[0] - acc_offsets[0]) / acc_res;				// in g (x 9.8 m/s^2)
	ay = ((float)acc_raw_data[1] - acc_offsets[1]) / acc_res;
	az = ((float)acc_raw_data[2] - acc_offsets[2]) / acc_res;

	gyro.getRotation(&gyro_raw_data[0], &gyro_raw_data[1], &gyro_raw_data[2]);
	gx = ((float)gyro_raw_data[0] - gyro_offsets[0]) / gyro_res;				// in Degrees/sec
	gy = ((float)gyro_raw_data[1] - gyro_offsets[1]) / gyro_res;
	gz = ((float)gyro_raw_data[2] - gyro_offsets[2]) / gyro_res;

	mag.getHeading(&mag_raw_data[0], &mag_raw_data[1], &mag_raw_data[2]);
	// Applying hard- and soft iron error compensation, converting to mGauss
	mx = ((float)mag_raw_data[0] - mag_hard_iron_bias[0]) * mag_soft_iron_bias[0] * mag_res;
	my = ((float)mag_raw_data[1] - mag_hard_iron_bias[1]) * mag_soft_iron_bias[1] * mag_res;
	mz = ((float)mag_raw_data[2] - mag_hard_iron_bias[2]) * mag_soft_iron_bias[2] * mag_res;

	now_us = micros();
	filter_dt_us = now_us - last_us;
	sample_freq = 1000000.0f / filter_dt_us;						// in Hz
	last_us = now_us;

	filter.begin(sample_freq);
#if FILTER_AHRS
	filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);					// 9-ODF
#elif FILTER_IMU
	filter.updateIMU(gx, gy, gz, ax, ay, az);						// 6-ODF
#endif

	axis_x = filter.getRoll();		// roll
	axis_y = filter.getPitch();		// pitch
	axis_z = filter.getYaw();		// yaw

#if DEBUG_IMU
	float dt_ms = (float)filter_dt_us / 1000.f;

#if DEBUG_IMU_RAW_DATA
	print_raw_ahrs_data(ax, ay, az, gx, gy, gz, mx, my, mz, axis_x, axis_y, axis_z, dt_ms);
#endif

#if DEBUG_IMU_PROCESSING_CUBE
	float temperature = 35.0f + ((float)(gyro.getTemperature() + 13200)) / 280.0f;
	float heading = atan2(my, mx) * 180 / M_PI;
	if (heading < 0)   heading += 360.0f;
	if (heading > 360) heading -= 360.0f;

	print_processing_cube_data(filter.q0, filter.q1, filter.q2, filter.q3, ax, ay, az, gx, gy, gz, mx, my, mz, temperature, dt_ms, heading);
#endif

#if DEBUG_IMU_PROCESSING_BUNNY
	print_processing_bunnyrotate_data(millis(), loop_number, dt_ms, axis_x, axis_y, axis_z);
#endif

#endif	// DEBUG_IMU
}

void get_acc_res()
{
	if (acc_full_res)
		acc_res = 256.f;			// in LSB/g
	else {
		switch (acc_range) {
			case ADXL345_RANGE_2G:
				acc_res = 256.f;
				break;
			case ADXL345_RANGE_4G:
				acc_res = 128.f;
				break;
			case ADXL345_RANGE_8G:
				acc_res = 64.f;
				break;
			case ADXL345_RANGE_16G:
				acc_res = 32.f;
				break;
		}
	}
}


void get_gyro_res()
{
	switch (gyro_scale) {
		case ITG3200_FULLSCALE_2000:
			gyro_res = 14.375f;
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
#endif	// GY_85
