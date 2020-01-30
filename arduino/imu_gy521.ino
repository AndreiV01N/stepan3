#if GY_521

#define USE_DMP				0

#if USE_DMP

#include "src/i2cdevlib/MPU6050_6Axis_MotionApps20.h"
static int16_t mpu_fifo_count = 0;			// bytes currently in FIFO
static uint16_t mpu_packet_size;			// expected DMP packet size (default is 42 bytes)

#else	// not USE_DMP

#define FILTER_MADGWICK			1
#define FILTER_MAHONY			0

#include "src/i2cdevlib/MPU6050.h"

#if FILTER_MADGWICK
#define FILTER_BETA_NORMAL		0.02f
#define FILTER_BETA_STAB		0.5f
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

// Put here actual offsets manually (are obtained by DEBUG_IMU_CALIBRATION):
static int16_t acc_offsets[3] = { -1048, 1969, 1374 };
static int16_t gyro_offsets[3] = { 91, -18, 9 };

static float acc_res;					// converter LSB to g (9.8 m/s^2)
static float gyro_res;					// converter LSB to deg/sec

#endif	// not USE_DMP

static MPU6050 mpu;


void imu_init()
{
	bool mpu_ready = false;

	Wire.begin();
	Wire.setClock(400000);

	Serial.println(F("is MPU6050 online?"));
	while (!mpu_ready)
		mpu_ready = mpu.testConnection();
	Serial.println(F("MPU6050 is online"));

	Serial.println(F("MPU6050 init.."));
	mpu.initialize();				// i2cdevlib: GYRO_FS_250 (deg/sec), ACCEL_FS_2 (g)
	delay(100);

#if USE_DMP
	Serial.println(F("Starting MPU6050 DMP init.."));
	uint8_t mpu_status = mpu.dmpInitialize();	// (0 = success, !0 = error)

	if (mpu_status == 0) {
		Serial.println(F("Enabling MPU6050 DMP.."));
		mpu.setDMPEnabled(true);
		mpu_packet_size = mpu.dmpGetFIFOPacketSize();
	} else {
		// ERROR!
		// 1 = initial memory load failed
		// 2 = DMP configuration updates failed
		// (if it's going to break, usually the code will be 1)
		Serial.print(F("DMP Initialization failed. Code: "));
		Serial.println(mpu_status);
		while (1);
	}

	Serial.print(F("MPU6050 DMP is now activated. packet size: "));
	Serial.println(mpu_packet_size);

	delay(3000);
	mpu.resetFIFO();
	__imu_stab();

#else	// not USE_DMP

#if DEBUG_IMU_CALIBRATION				// now ACCEL_FS_2, GYRO_FS_250
	mpu.PrintActiveOffsets();
	Serial.println(F("MPU6050 calibrating (6 x 100 readings).."));
	mpu.CalibrateAccel(6);
	mpu.CalibrateGyro(6);
	mpu.PrintActiveOffsets();
	Serial.println();

	Serial.print(F("static int16_t acc_offsets[3] = { "));
	Serial.print(mpu.getXAccelOffset());	Serial.print(F(", "));
	Serial.print(mpu.getYAccelOffset());	Serial.print(F(", "));
	Serial.print(mpu.getZAccelOffset());	Serial.println(F(" };"));

	Serial.print(F("static int16_t gyro_offsets[3] = { "));
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

	get_acc_res();
	get_gyro_res();

	__imu_stab();
#endif	// not CALIBRATION
#endif	// not USE_DMP
}


/*
This procedure speeds up and controls convergence of initial YPR received from the filter
to actual Acc-Gyro values on early start. Upon my tests it never takes more than 2 sec.
*/
void __imu_stab()
{
	uint8_t i;
#if USE_DMP
	for (i = 0; i < 10; i++) {
		imu_read_data();
		delay(50);
	}
#else
	float x_pre, y_pre, z_pre;

	bool x_stab = false;
	bool y_stab = false;
	bool z_stab = false;

	int8_t x_delta_sign_ref, y_delta_sign_ref, z_delta_sign_ref;		// -1, 0, 1
	int8_t x_delta_sign_now, y_delta_sign_now, z_delta_sign_now;		// -1, 0, 1

	uint32_t start_stab_ms = millis();

#if FILTER_MADGWICK
	filter.beta = FILTER_BETA_STAB;
	Serial3.print(F("Stabilizing Madgwick filter params on start with beta = "));
	Serial3.println(filter.beta, 4);
#endif

#if FILTER_MAHONY
	filter.twoKp = FILTER_KP_STAB;
	Serial3.print(F("Stabilizing Mahony filter params on start with Kp = "));
	Serial3.println(filter.twoKp, 4);
#endif
	for (i = 0; i < 5; i++)
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

		if (x_pre != axis_x) {
			x_delta_sign_now = fabs(axis_x - x_pre) / (axis_x - x_pre);
			if (x_delta_sign_now != x_delta_sign_ref)
				x_stab = true;
		} else
			x_stab = true;

		if (y_pre != axis_y) {
			y_delta_sign_now = fabs(axis_y - y_pre) / (axis_y - y_pre);
			if (y_delta_sign_now != y_delta_sign_ref)
				y_stab = true;
		} else
			y_stab = true;

		if (z_pre != axis_z) {
			z_delta_sign_now = fabs(axis_z - z_pre) / (axis_z - z_pre);
			if (z_delta_sign_now != z_delta_sign_ref)
				z_stab = true;
		} else
			z_stab = true;


		Serial3.print(F("\nSTAB:\t"));
		if (x_stab)
			Serial3.print(F("X:True\t"));
		else
			Serial3.print(F("X:False\t"));

		if (y_stab)
			Serial3.print(F("Y:True\t"));
		else
			Serial3.print(F("Y:False\t"));

		if (z_stab)
			Serial3.println(F("Z:True"));
		else
			Serial3.println(F("Z:False"));

	}

#if FILTER_MADGWICK
	filter.beta = FILTER_BETA_NORMAL;
#endif

#if FILTER_MAHONY
	filter.twoKp = FILTER_KP_NORMAL;
#endif
	Serial3.print(F("\nSTAB: All YPR have stabilized in ms: "));
	Serial3.println(millis() - start_stab_ms);
#endif	// not USE_DMP
}


void imu_read_data()
{
#if USE_DMP
	Quaternion quat;		// [w, x, y, z]         quaternion container
	VectorFloat gravity;		// [x, y, z]            gravity vector
	uint8_t mpu_fifo_buffer[64];	// FIFO storage buffer
	float ypr[3];			// [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

//	if (mpu.getFIFOCount() > 0) mpu.resetFIFO();
//	mpu_fifo_count = 0;

	while (mpu_fifo_count < mpu_packet_size)		// wait till DMP data is ready (usually 2 or 3 checks)
		mpu_fifo_count = mpu.getFIFOCount();

	mpu.getFIFOBytes(mpu_fifo_buffer, mpu_packet_size);
	mpu_fifo_count -= mpu_packet_size;

	if (mpu_fifo_count > 0) {
		mpu.resetFIFO();
		mpu_fifo_count = 0;
	}

	mpu.dmpGetQuaternion(&quat, mpu_fifo_buffer);
	mpu.dmpGetGravity(&gravity, &quat);
	mpu.dmpGetYawPitchRoll(ypr, &quat, &gravity);

	axis_z = ypr[0] * RAD_TO_DEG;
	axis_y = ypr[1] * RAD_TO_DEG;
	axis_x = ypr[2] * RAD_TO_DEG;

#if DEBUG_IMU

#if DEBUG_IMU_PROCESSING_CUBE
	print_processing_cube_data(quat.w, quat.x, quat.y, quat.z, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
#endif

#if DEBUG_IMU_PROCESSING_BUNNY
	print_processing_bunnyrotate_data(millis(), loop_number, 0.f, axis_x, axis_y, axis_z);
#endif

#endif	// DEBUG_IMU

#else	// not USE_DMP
	int16_t acc_raw_data[3];
	int16_t gyro_raw_data[3];

	uint32_t now_us;
	uint32_t filter_dt_us;
	static uint32_t last_us = 0;

	float ax, ay, az;
	float gx, gy, gz;
	float sample_freq;

	mpu.getAcceleration(&acc_raw_data[0], &acc_raw_data[1], &acc_raw_data[2]);
	ax = (float)acc_raw_data[0] * acc_res;		// in g (9.8 m/s^2)
	ay = (float)acc_raw_data[1] * acc_res;
	az = (float)acc_raw_data[2] * acc_res;

	mpu.getRotation(&gyro_raw_data[0], &gyro_raw_data[1], &gyro_raw_data[2]);
	gx = (float)gyro_raw_data[0] * gyro_res;	// in Degrees/sec
	gy = (float)gyro_raw_data[1] * gyro_res;
	gz = (float)gyro_raw_data[2] * gyro_res;

	now_us = micros();
	filter_dt_us = now_us - last_us;
	sample_freq = 1000000.0f / filter_dt_us;	// in Hz
	last_us = now_us;

	filter.begin(sample_freq);
	filter.updateIMU(gx, gy, gz, ax, ay, az);	// 6-ODF

	axis_x = filter.getRoll();			// roll
	axis_y = filter.getPitch();			// pitch
	axis_z = filter.getYaw();			// yaw

#if DEBUG_IMU
	float dt_ms = (float)filter_dt_us / 1000.f;

#if DEBUG_IMU_RAW_DATA
	print_raw_ahrs_data(ax, ay, az, gx, gy, gz, 0.f, 0.f, 0.f, axis_x, axis_y, axis_z, dt_ms);
#endif

#if DEBUG_IMU_PROCESSING_CUBE
	float temperature = 35.0f + ((float)(mpu.getTemperature() + 521)) / 340.0f;
	print_processing_cube_data(filter.q0, filter.q1, filter.q2, filter.q3, ax, ay, az, gx, gy, gz, 0.f, 0.f, 0.f, temperature, dt_ms, 0.f);
#endif

#if DEBUG_IMU_PROCESSING_BUNNY
	print_processing_bunnyrotate_data(millis(), loop_number, dt_ms, axis_x, axis_y, axis_z);
#endif

#endif	// DEBUG_IMU
#endif	// not USE_DMP
}

#if !USE_DMP
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
#endif	// not USE_DMP
#endif	// GY_521
