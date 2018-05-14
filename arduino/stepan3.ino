#define KP_POSITION		0.04
#define KD_POSITION		0.20

#define KP_SPEED		0.08
#define KI_SPEED		0.1

#define KP_STAB			0.19
#define KD_STAB			0.05

#define KP_STAB_RISEUP		0.03
#define KD_STAB_RISEUP		0.08

#define MAX_SPEED_TARGET	70
#define MAX_TARGET_ANGLE	35
#define MAX_CONTROL_OUTPUT	200
#define MAX_STEERING		50

#define ANGLE_READY		75
#define MAX_ACCEL_NORM		25
#define MAX_ACCEL_RISEUP	6
#define ROUTE_PAUSE		10000	// time between routes in ms (if standalone)
#define RISEUP_TIMEOUT		1500	// in ms

#define COMMAND_CENTER			// comment if standalone

#define WIFI_EXTERNAL_AP
// #define WIFI_INTERNAL_AP


#include <util/delay.h>
#include "dc_motors.h"
#include "pid.h"
#include "remote_cc.h"
#include "route.h"
#include "wifi.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;

Quaternion q;				// [w, x, y, z]         quaternion container
VectorFloat gravity;			// [x, y, z]            gravity vector
float ypr[3];				// [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

double angle_actual;			// near zero if vertical, positive if tilt forward
double angle_target_raw;
double angle_target;

double speed_actual;			// avg. of speed_M1, speed_M2
double speed_actual_filtered = 0.0;
double speed_target_raw;
double speed_target;			// positive if going to go forward

double angular_velocity;
double dt_f;

double Kp_speed = KP_SPEED;
double Ki_speed = KI_SPEED;

double Kp_stab = KP_STAB;
double Kd_stab = KD_STAB;

double Kp_pos = KP_POSITION;
double Kd_pos = KD_POSITION;

double control_output_raw = 0.0;
double control_output;			// positive when going forward

double steering_raw;
double steering;

double M1_pos_ctl;
double M2_pos_ctl;

int32_t step_M1;			// increases when forward
int32_t step_M2;
int32_t target_step_M1;			// 1 round = 300 mm = 3200 steps (1/16 microstepping)
int32_t target_step_M2;
uint32_t dt;
uint32_t now_us;
uint32_t now_ms;
uint32_t timer_pid;
uint32_t timer_riseup;
uint32_t timer_route;
uint32_t timer_batt;

int16_t max_accel = MAX_ACCEL_NORM;
int16_t control_M1;
int16_t control_M2;
int16_t speed_M1;			// Actual speed of M1 (positive when going forward)
int16_t speed_M2;			// Actual speed of M2 (positive when going forward)
int16_t mpu_fifo_count = 0;		// bytes currently in FIFO
uint16_t mpu_packet_size;		// expected DMP packet size (default is 42 bytes)

int8_t dir_M1;				// Actual direction of M1
int8_t dir_M2;				// Actual direction of M2
uint8_t route_point = 0;		// Current route segment
uint8_t mpu_status;			// (0 = success, !0 = error)
uint8_t mpu_fifo_buffer[64];		// FIFO storage buffer

bool mpu_ready = false;
bool position_up;
bool on_route = false;
bool on_riseup = false;
#if defined(WIFI_EXTERNAL_AP) || defined(WIFI_INTERNAL_AP)
#ifdef COMMAND_CENTER
bool on_control = false;
#endif
bool syslog_is_online = false;
bool node_connected = false;
#endif


int16_t my_round(double x)
{
	return (int)(x + (x < 0 ? -0.5 : 0.5));
}


void set_ready_to_riseup()
{
	Kp_stab = KP_STAB_RISEUP;	// preparing to riseup
	Kd_stab = KD_STAB_RISEUP;
	max_accel = MAX_ACCEL_RISEUP;	// 6
	angle_target = 0;
	control_output_raw = 0;
	step_M1 = 0;
	step_M2 = 0;
	PORTH &= ~_BV(5);		// enable motors
	timer_riseup = millis();	// starting to riseup
}


void setup()
{
	Serial.begin(115200);
	motor_pins_init();

#ifdef WIFI_EXTERNAL_AP
	Serial3.begin(115200);
	esp8266_connect_to_ext_ap();

	while (!node_connected)
		node_connected = esp8266_ipservices_init();
#endif

#ifdef WIFI_INTERNAL_AP
	Serial3.begin(115200);
	esp8266_init_local_ap();

	while (!syslog_is_online) {					// waiting for syslog server..
		syslog_is_online = esp8266_check_node();
		delay(1000);
	}

	while (!node_connected)
		node_connected = esp8266_ipservices_init();
#endif

	// MPU6050 init using I2Cdev libs
	Wire.begin();
	Wire.setClock(400000);

	Serial.println(F("Starting MPU6050 init.."));
	mpu.initialize();

	while (!mpu_ready) {
		mpu_ready = mpu.testConnection();
	}

	Serial.println(F("MPU6050 is active"));
	delay(500);
	Serial.println(F("Starting MPU6050 DMP init.."));
	mpu_status = mpu.dmpInitialize();

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
	}
	Serial.print(F("MPU6050 DMP is now activated. packetSize: "));
	Serial.println(mpu_packet_size);

	delay(3000);
	mpu.resetFIFO();

	// obtaining current angle from MPU6050
	while (mpu_fifo_count < mpu_packet_size) {		// wait till DMP data is ready
		mpu_fifo_count = mpu.getFIFOCount();
	}

	mpu.getFIFOBytes(mpu_fifo_buffer, mpu_packet_size);
	mpu_fifo_count -= mpu_packet_size;

	mpu.dmpGetQuaternion(&q, mpu_fifo_buffer);
	mpu.dmpGetGravity(&gravity, &q);
	mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

	angle_actual = (ypr[2] + 0.0) * RAD_TO_DEG;
//	angle_Y      = (ypr[1] + 0.0) * RAD_TO_DEG;

	timer_pid = micros();

	if (fabs(angle_actual) < ANGLE_READY) {			// position is UP
		position_up = true;
#ifndef COMMAND_CENTER
		timer_route = millis();
#endif
	} else {						// position is DOWN
		position_up = false;
#ifdef COMMAND_CENTER
		Kp_stab = 0;
		Kd_stab = 0;
#else
		set_ready_to_riseup();
		on_riseup = true;
#endif
	}
}


void loop()
{
#ifdef COMMAND_CENTER
	apply_remote_cmd();
#endif

	while (mpu_fifo_count < mpu_packet_size) {		// wait for DMP data
		mpu_fifo_count = mpu.getFIFOCount();
	}

	mpu.getFIFOBytes(mpu_fifo_buffer, mpu_packet_size);
	mpu_fifo_count -= mpu_packet_size;

	if (mpu_fifo_count > 0) {
		mpu.resetFIFO();
		mpu_fifo_count = 0;
	}

	mpu.dmpGetQuaternion(&q, mpu_fifo_buffer);
	mpu.dmpGetGravity(&gravity, &q);
	mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

	angle_actual = (ypr[2] + 0.0) * RAD_TO_DEG;
//	angle_Y      = (ypr[1] + 0.0) * RAD_TO_DEG;

	if (fabs(angle_actual) < ANGLE_READY) {			// we're VERTICAL (within working sector)
		if (!position_up) {				// we've just got UP, need to set normal params
			Kp_stab = KP_STAB;
			Kd_stab = KD_STAB;
			max_accel = MAX_ACCEL_NORM;
			step_M1 = 0;
			step_M2 = 0;
			position_up = true;
			on_riseup = false;
#ifndef COMMAND_CENTER
			timer_route = millis();
#endif
		} else {					// we were VERTICAL before
#ifndef COMMAND_CENTER
			if (!on_route) {
				dt = millis() - timer_route;
				if (dt > ROUTE_PAUSE) {
					on_route = true;
					route_point = 0;
				}
			}
#endif
		}
	} else {						// we're HORIZONTAL (out of working sector)
		on_route = false;
#ifdef COMMAND_CENTER
		on_control = false;
#endif
		if (position_up) {				// we've just fell down
			position_up = false;
			stall();				// motors off, wait for 2s
#ifdef COMMAND_CENTER
			Kp_stab = 0;
			Kd_stab = 0;
#else
			set_ready_to_riseup();			// preparing to riseup..
			on_riseup = true;
#endif
		} else  {					// we were DOWN before (we're still out of working sector)
#ifdef COMMAND_CENTER
			if (on_riseup && Kp_stab == 0) {
				set_ready_to_riseup();		// preparing to riseup..
			}
#endif
			if (on_riseup) {
				dt = millis() - timer_riseup;		// .. here we're still trying to riseup
				if (dt > RISEUP_TIMEOUT) {		// we have not managed to get ANGLE_READY in time, resetting..
					position_up = true;		// going to off) & reset, and then another attempt to riseup
				}
			}
		}
	}

	if (on_route) {
		follow_route();
		M1_pos_ctl = position_PD_control(step_M1, target_step_M1, Kp_pos, Kd_pos, speed_M1);
		M2_pos_ctl = position_PD_control(step_M2, target_step_M2, Kp_pos, Kd_pos, speed_M2);

		speed_target_raw = (M1_pos_ctl + M2_pos_ctl) / 2;
		speed_target     = constrain(speed_target_raw, -MAX_SPEED_TARGET, MAX_SPEED_TARGET);
	} else {
#ifdef COMMAND_CENTER
		if (!on_control)
			speed_target = 0;
#else
		speed_target = 0;
#endif
	}

	now_us = micros();
	dt_f = (double)(now_us - timer_pid) / 1000000;
	timer_pid = now_us;

	if (position_up) {
		speed_actual = (speed_M1 + speed_M2) / 2;
//		speed_actual_filtered = speed_actual_filtered * 0.9 + speed_actual * 0.1;
		speed_actual_filtered = speed_actual;

		angle_target_raw = speed_PI_control(dt_f, speed_actual_filtered, speed_target, Kp_speed, Ki_speed);
		angle_target     = constrain(angle_target_raw, -MAX_TARGET_ANGLE, MAX_TARGET_ANGLE);
	}

	control_output_raw += stability_PD_control(dt_f, angle_actual, angle_target, Kp_stab, Kd_stab);
	control_output      = constrain(control_output_raw, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT);

	if (on_route) {
		steering = get_steering();
	} else {
#ifdef COMMAND_CENTER
		if (!on_control)
			steering = 0;
#else
		steering = 0;
#endif
	}

	control_M1 = my_round(control_output + steering);
	control_M2 = my_round(control_output - steering);

	set_speed_M1(control_M1);
	set_speed_M2(control_M2);

	now_ms = millis();

#if defined(WIFI_EXTERNAL_AP) || defined(WIFI_INTERNAL_AP)
//	Serial3.println();
//	Serial3.print(F(", step_M1: "));		Serial3.print(step_M1);
//	Serial3.print(F(", step_M2: "));		Serial3.print(step_M2);
//	Serial3.print(F(", angle_target_raw: "));	Serial3.print(angle_target_raw);

	Serial3.print(F("UPT: "));			Serial3.print(now_ms);
	Serial3.print(F("|FIFO: "));			Serial3.print(mpu_fifo_count);
	Serial3.print(F("|ATR: "));			Serial3.print(angle_target_raw);
	Serial3.print(F("|A: "));			Serial3.print(angle_actual);
//	Serial3.print(F("|PUP: "));			Serial3.print(position_up);
//	Serial3.print(F(", on_riseup: "));		Serial3.print(on_riseup);
//	Serial3.print(F(", on_route: "));		Serial3.print(on_route);
	Serial3.print(F("|ONCT: "));			Serial3.print(on_control);
	Serial3.print(F("|STR: "));			Serial3.print(speed_target_raw);
	Serial3.print(F("|COR: "));			Serial3.print(control_output_raw);
	Serial3.print(F("|STR: "));			Serial3.println(steering);

//	Serial3.print(F("|dir_M1: "));			Serial3.print(dir_M1);
//	Serial3.print(F("|dir_M2: "));			Serial3.print(dir_M2);
//	Serial3.print(F("|M1_p_ct: "));			Serial3.print(M1_pos_ctl);
//	Serial3.print(F("|M2_p_ct: "));			Serial3.print(M2_pos_ctl);
//	Serial3.print(F(" sp_M1: "));			Serial3.print(speed_M1);
//	Serial3.print(F(" sp_M2: "));			Serial3.print(speed_M2);

#endif
}
