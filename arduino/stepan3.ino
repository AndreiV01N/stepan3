#define GY_521				0	// available IMU/AHRS boards: GY_521, GY_85, GY_87
#define GY_85				1
#define GY_87				0

#define TILT_AXIS_X			1	// depends on how the IMU/AHRS board is positioned inside
#define TILT_AXIS_Y			0

#define DEBUG_IMU			0	// if this is set to 1, one of 4 below should be set to 1 as well
#define DEBUG_IMU_RAW_DATA		0
#define DEBUG_IMU_PROCESSING_CUBE	0
#define DEBUG_IMU_PROCESSING_BUNNY	0
#define DEBUG_IMU_CALIBRATION		0	// if set to 1 - lock the robot vertically (approx.) and keep it motionless

#define KP_POSITION			0.08f
#define KD_POSITION			0.20f

#define KP_SPEED			0.06f
#define KI_SPEED			0.08f

#if GY_85
#define KP_STAB				0.50f
#define KD_STAB				0.20f
#else
#define KP_STAB				0.30f
#define KD_STAB				0.10f
#endif

#define LOOP_NUM_K_SOFT_START		50	// K*_stab are tuned on first N loops to start smoothly when abs(tilt) < TILT_COMFORT

#define MAX_SPEED_TARGET		140
#define MAX_TILT_TARGET			35
#define MAX_CONTROL_OUTPUT		700

#define MAX_ACCEL_NORMAL		50
#define MAX_DECCEL_NORMAL		200
#define MAX_ACCEL_RISEUP		10
#define MAX_DECCEL_RISEUP		50	// acts the same way as "ABS" in real vehecles

#define MAX_LEN_REMOTE_CMD		6
#define MAX_LEN_REMOTE_CMD_VAL		9

#define TILT_BIAS			-0.0f	// doesn't really matter (helps to prevent onetime short moving drift on start)
#define TILT_READY			60.0f	// robot's tilt is supposed as VERTICAL (within working sector)
#define TILT_COMFORT			15.0f	// relates to LOOP_NUM_K_SOFT_START
#define ROUTE_PAUSE			10000	// time between routes in ms (if standalone)
#define SLEEP_TIMEOUT			2000	// in ms
#define RISEUP_TIMEOUT			1000	// in ms

#define BATTERY_PIN			A3	// 10-bit voltage of 6xAA on BATTERY_PIN (Vbatt -> 10kOhm -> BATTERY_PIN -> 3.3kOhm -> GND)
						// 300 (totally exhausted AAs) ... 450 (new and fresh AAs)

#define ECHO				true
#define NO_ECHO				false

#define RET_ON_EMPTY_BUFF		true
#define NO_RET_ON_EMPTY_BUFF		false

#define MODE				6	// 0 - networkless, standalone
						// 1 - AP, syslog only
						// 2 - AP, remote control only
						// 3 - AP, remote control + syslog
						// 4 - STA, syslog only
						// 5 - STA, remote control only
						// 6 - STA, remote control + syslog

#if MODE == 1
#define WIFI_AP_MODE
#define SYSLOG					// the first connected STA is considered as a syslog-server
#endif

#if MODE == 2
#define WIFI_AP_MODE
#define REMOTE_CTL
#endif

#if MODE == 3
#define WIFI_AP_MODE
#define REMOTE_CTL
#define SYSLOG					// the first connected STA is considered as a syslog-server
#endif

#if MODE == 4
#define WIFI_STA_MODE
#define SYSLOG
#endif

#if MODE == 5
#define WIFI_STA_MODE
#define REMOTE_CTL
#endif

#if MODE == 6
#define WIFI_STA_MODE
#define REMOTE_CTL
#define SYSLOG
#endif

#ifdef SYSLOG
char syslog_msg[255];
#endif

#ifdef REMOTE_CTL
char remote_cmd[MAX_LEN_REMOTE_CMD + 1];		// 6 + 1
char remote_cmd_val[MAX_LEN_REMOTE_CMD_VAL + 1];	// 9 + 1
#endif

float axis_x;				// roll by default (in deg.)
float axis_y;				// pitch by default (in deg.)
float axis_z;				// yaw by default (in deg.)

float tilt_now;				// near zero if vertical, positive if tilt forward (in deg.)
float tilt_target_raw = 0.f;
float tilt_target = 0.f;

float speed_now;			// avg(speed_M1, speed_M2)
float speed_target_raw = 0.f;		// all of speed_* below are in "full steps"/sec (1 wheel rotation eq 200 full steps)
float speed_target = 0.f;		// e.g. speed_target=200 means 1 wheel rotation per second (approx 0.3 m/s)

float dt_f_s;				// loop duty - is used by PID regulators

float Kp_pos = KP_POSITION;
float Kd_pos = KD_POSITION;

float Kp_speed = KP_SPEED;
float Ki_speed = KI_SPEED;

float Kp_stab = KP_STAB;
float Kd_stab = KD_STAB;

float control_output_raw = 0.f;
float control_output = 0.f;		// positive when going forward

float steering_raw;
float steering;

float M1_pos_ctl;
float M2_pos_ctl;

float sonar_dist_cm = 0.f;

int32_t step_M1;			// increases when forward
int32_t step_M2;
int32_t target_step_M1;			// 1 wheel rotation = 300 mm (3200 steps for 1/16 microstepping or 200 steps for "full stepping")
int32_t target_step_M2;
uint32_t dt_ms;	
uint32_t now_us;
uint32_t now_ms;
uint32_t timer_pid_us;
uint32_t timer_loop_ms;
uint32_t timer_riseup_ms;
uint32_t timer_route_ms;
uint32_t timer_sleep_ms;
uint32_t loop_number = 0;

int16_t max_accel = MAX_ACCEL_NORMAL;
int16_t max_deccel = MAX_DECCEL_NORMAL;
int16_t control_M1;
int16_t control_M2;
int16_t speed_M1;			// actual speed of M1 (positive when going forward)
int16_t speed_M2;
uint16_t battery_raw;

int8_t dir_M1;				// actual direction of M1 (positive when rotates forward, zero if stops)
int8_t dir_M2;
uint8_t route_point = 0;		// current route segment (see route.ino)
uint8_t riseup_phase = 0;

bool is_awake = false;			// 'true' means the robot is not asleep and ready to any activities
bool position_up = false;		// 'true' means actual 'tilt_now' value < abs(TILT_READY) - i.e. we are UP
bool on_route = false;			// 'true' means the robot is now following predefined route path
bool on_riseup = false;			// 'true' means the robot is now trying to reach abs(TILT_READY) from laying position
bool on_control = false;		// 'true' means robot's movements are now controlled remotely
bool on_soft_start = false;		// 'true' means Kp_stab and Kd_stab are being corrected
bool is_sta_online = false;
bool is_sonar_ready = true;


int16_t my_round(float x)
{
	return (int)(x + (x < 0 ? -0.5f : 0.5f));
}


void append_float_to_str(char *str, const char *tag, float f, const uint8_t prec)
{
	strcat(str, tag);
	dtostrf(f, prec + 5, prec, str + strlen(str));
}


float get_tilt()
{
#if TILT_AXIS_X
	return (axis_x - TILT_BIAS);
#elif TILT_AXIS_Y
	return (axis_y - TILT_BIAS);
#endif
}


void setup()
{
	Serial.begin(115200);
	Serial3.begin(115200);

#if DEBUG_IMU
	imu_init();						// can take a few seconds
#else

#ifdef WIFI_STA_MODE
	esp8266_connect_to_ap();
	esp8266_ipservices_init();
#endif

#ifdef WIFI_AP_MODE
	esp8266_init_local_ap();

	while (!is_sta_online) {
		is_sta_online = esp8266_is_any_sta_online();
		delay(1000);
	}

	esp8266_ipservices_init();
#endif

	motor_pins_init();
	sonar_pins_init();
	delay(100);
	imu_init();
	imu_read_data();
	tilt_now = get_tilt();

	if (fabs(tilt_now) < TILT_READY) {			// an initial position is UP
		position_up = true;
		is_awake = true;
		timer_route_ms = millis();
		timer_pid_us = micros();
		if (fabs(tilt_now) < TILT_COMFORT)
			on_soft_start = true;
	} else {
		position_up = false;
		stall();
	}

	timer_loop_ms = millis();
#endif		// not DEBUG_IMU
}


void loop()
{
#if DEBUG_IMU

#if !DEBUG_IMU_CALIBRATION					// do nothing in the loop() while calibrating
	imu_read_data();
#endif

#else	// not DEBUG_IMU
	imu_read_data();
	tilt_now = get_tilt();

	if (fabs(tilt_now) < TILT_READY) {			// we are now UP (within tilt working sector)
		if (position_up) {				// last time we were UP as well
			if (is_awake) {
				// this helps a lot to avoid moving twitches on early start being UP
				if (on_soft_start && loop_number <= LOOP_NUM_K_SOFT_START) {
					Kp_stab = KP_STAB * loop_number / LOOP_NUM_K_SOFT_START;
					Kd_stab = KD_STAB * loop_number / LOOP_NUM_K_SOFT_START;
					if (loop_number == LOOP_NUM_K_SOFT_START)
						on_soft_start = false;
				}
#ifndef REMOTE_CTL
				if (!on_route) {
					dt_ms = millis() - timer_route_ms;
					if (dt_ms > ROUTE_PAUSE) {
						on_route = true;
						route_point = 0;
					}
				}
#endif
			} else {				// received 'Sleep' remote command earlier while being UP
				set_speed_M1(0);
				set_speed_M2(0);
				dt_ms = millis() - timer_sleep_ms;
				if (dt_ms > SLEEP_TIMEOUT)
					is_awake = true;
			}
		} else {					// we've just got UP, needs to set normal params
			step_M1 = 0;
			step_M2 = 0;
			position_up = true;
			is_awake = true;
			on_riseup = false;
			riseup_phase = 0;
			max_accel = MAX_ACCEL_NORMAL;
			max_deccel = MAX_DECCEL_NORMAL;
			timer_route_ms = millis();
			timer_pid_us = micros() - 10000;	// set pid timer 10ms behind
			pid_reset_errors();
		}
	} else {						// we are now DOWN (out of tilt working sector)
		if (position_up) {				// just fell over for any reasons (last time we were UP)
			on_route = false;
			on_control = false;
			position_up = false;
			on_riseup = false;
			if (is_awake)				// fell over on its own (it was not a 'Sleep' remote command)
				stall();
			else {					// we're already asleep - just keep falling down after receiving 'Sleep' cmd
				set_speed_M1(0);
				set_speed_M2(0);
				dt_ms = millis() - timer_sleep_ms;
				if (dt_ms > SLEEP_TIMEOUT)
					is_awake = true;
			}
		} else {					// last time we were DOWN too (we're still out of tilt working sector)
			if (is_awake) {
				if (on_riseup) {		// here we're keep trying to get UP
					riseup_control();
					dt_ms = millis() - timer_riseup_ms;
					if (dt_ms > RISEUP_TIMEOUT)		// we've not managed to reach tilt working sector in time..
						stall();
				}
			} else {				// in 'sleep' state due to stall() called or it's robot's initial pos
				set_speed_M1(0);
				set_speed_M2(0);
				dt_ms = millis() - timer_sleep_ms;
				if (dt_ms > SLEEP_TIMEOUT) {
					is_awake = true;
#ifndef REMOTE_CTL
					on_riseup = true;	// auto-riseup if standalone
#endif
				}
			}
		}
	}

	speed_now = float(speed_M1 + speed_M2) / 2.f;

	if (position_up && is_awake) {
		if (on_route) {
			follow_route();				// updates target_step_M*
			M1_pos_ctl = position_PD_control(step_M1, target_step_M1, Kp_pos, Kd_pos, speed_M1);
			M2_pos_ctl = position_PD_control(step_M2, target_step_M2, Kp_pos, Kd_pos, speed_M2);

			speed_target_raw = (M1_pos_ctl + M2_pos_ctl) / 2;
			speed_target     = constrain(speed_target_raw, -MAX_SPEED_TARGET, MAX_SPEED_TARGET);
		} else if (!on_control)
			speed_target = 0;

		now_us = micros();
		dt_f_s = (float)(now_us - timer_pid_us) / 1000000.f;
		timer_pid_us = now_us;

		tilt_target_raw = speed_PI_control(dt_f_s, speed_now, speed_target, Kp_speed, Ki_speed);
		tilt_target     = constrain(tilt_target_raw, -MAX_TILT_TARGET, MAX_TILT_TARGET);		// -35..+35

		control_output_raw += stability_PD_control(dt_f_s, tilt_now, tilt_target, Kp_stab, Kd_stab);
		control_output      = constrain(control_output_raw, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT);	// -700..+700

		if (on_route)
			steering = get_steering();
		else if (!on_control)
			steering = 0;

		control_M1 = my_round(control_output + steering);
		control_M2 = my_round(control_output - steering);

		set_speed_M1(control_M1);
		set_speed_M2(control_M2);

		if (is_sonar_ready)
			sonar_send_ping();
	}

	battery_raw = analogRead(BATTERY_PIN);

	loop_number++;
	now_ms = millis();
	dt_ms = now_ms - timer_loop_ms;
	timer_loop_ms = now_ms;

#ifdef SYSLOG
/*
Attention!
Too many params here increases loop duty time a lot.
Ideally dt_ms value should not exceed 16-17 ms.
Otherwise you'll face some tremors and other misbehaves..
*/

	sprintf(syslog_msg + strlen(syslog_msg), " LPN: %lu", loop_number);
	sprintf(syslog_msg + strlen(syslog_msg), " LPT: %lu", dt_ms);
//	sprintf(syslog_msg + strlen(syslog_msg), " BAT: %u", battery_raw);

//	append_float_to_str(syslog_msg, " DST: ", sonar_dist_cm, 2);
	append_float_to_str(syslog_msg, " SPD: ", speed_now, 2);
//	append_float_to_str(syslog_msg, " SPDt: ", speed_target, 2);
	append_float_to_str(syslog_msg, " TLT: ", tilt_now, 2);
	append_float_to_str(syslog_msg, " TLTt: ", tilt_target, 2);
	append_float_to_str(syslog_msg, " COUT: ", control_output, 2);
//	append_float_to_str(syslog_msg, " YAW: ", axis_z, 2);

	logger_telemetry(syslog_msg);
#elif defined REMOTE_CTL
	esp8266_rx_buff_parser_wrapper(8, remote_cmd, __parse_loop_data, RET_ON_EMPTY_BUFF, NO_ECHO);
#endif

#endif	// not DEBUG_IMU
}
