#define POSITION_PTERM_MAX	40.0f
#define SPEED_ERROR_MAX		30.0f
#define SPEED_ITERM_MAX		10000.0f
#define STAB_DELTA_ERROR_MAX	8.0f

static float speed_i_error = 0.f;
static float tilt_error_last = 0.f;

/*
 Input: step error
 Output: speed_target
*/
float position_PD_control(int32_t input, int32_t setpoint, float Kp, float Kd, int16_t speedM)
{
	float position_error = setpoint - input;
	float pTerm = constrain(Kp * position_error, -POSITION_PTERM_MAX, POSITION_PTERM_MAX);

	float dTerm = Kd * float(speedM);

	return pTerm + dTerm;
}

/*
 Input: speed error
 Output: tilt_target
*/
float speed_PI_control(float dt_s, float input, float setpoint, float Kp, float Ki)
{
	float speed_error = setpoint - input;
	float pTerm = Kp * speed_error;

	speed_i_error += constrain(speed_error, -SPEED_ERROR_MAX, SPEED_ERROR_MAX);
	speed_i_error  = constrain(speed_i_error, -SPEED_ITERM_MAX, SPEED_ITERM_MAX);
	float iTerm = Ki * speed_i_error * dt_s;
#ifdef SYSLOG
	append_float_to_str(syslog_msg, " spP: ", pTerm, 2);
	append_float_to_str(syslog_msg, " spI: ", iTerm, 2);
#endif
	return pTerm + iTerm;
}

/*
 Input: tilt_error
 Output: delta of control_output
*/
float stability_PD_control(float dt_s, float input, float setpoint, float Kp, float Kd)
{
	float tilt_error = input - setpoint;
	float pTerm = Kp * tilt_error;

	float tilt_error_delta = constrain(tilt_error - tilt_error_last, -STAB_DELTA_ERROR_MAX, STAB_DELTA_ERROR_MAX);
	float dTerm = Kd * tilt_error_delta / dt_s;
	tilt_error_last = tilt_error;
#ifdef SYSLOG
	append_float_to_str(syslog_msg, " coP: ", pTerm, 2);
	append_float_to_str(syslog_msg, " coD: ", dTerm, 2);
#endif
	return pTerm + dTerm;
}


void pid_reset_errors()
{
	speed_i_error = 0.f;
	tilt_error_last = 0.f;
	return;
}
