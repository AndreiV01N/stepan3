#define POSITION_PTERM_MAX	40.0f
#define SPEED_ERROR_MAX		30.0f
#define SPEED_ITERM_MAX		10000.0f
#define STAB_DELTA_ERROR_MAX	8.0f

static float speed_i_error = 0.f;
static float angle_error_last = 0.f;

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
float speed_PI_control(float dt, float input, float setpoint, float Kp, float Ki)
{
	float speed_error = setpoint - input;
	float pTerm = Kp * speed_error;

	speed_i_error += constrain(speed_error, -SPEED_ERROR_MAX, SPEED_ERROR_MAX);
	speed_i_error  = constrain(speed_i_error, -SPEED_ITERM_MAX, SPEED_ITERM_MAX);
	float iTerm = Ki * speed_i_error * dt;

#if defined(WIFI_EXTERNAL_AP) || defined(WIFI_INTERNAL_AP)
	Serial3.print(F(" spP: "));	Serial3.print(pTerm);
	Serial3.print(F(" spI: "));	Serial3.print(iTerm);
#endif
	return pTerm + iTerm;
}

/*
 Input: tilt_error
 Output: delta of control_output
*/
float stability_PD_control(float dt, float input, float setpoint, float Kp, float Kd)
{
	float angle_error = input - setpoint;
	float pTerm = Kp * angle_error;

	float angle_error_delta = constrain(angle_error - angle_error_last, -STAB_DELTA_ERROR_MAX, STAB_DELTA_ERROR_MAX);
	float dTerm = Kd * angle_error_delta / dt;
	angle_error_last = angle_error;

#if defined(WIFI_EXTERNAL_AP) || defined(WIFI_INTERNAL_AP)
	Serial3.print(F(" coP: "));	Serial3.print(pTerm);
	Serial3.print(F(" coD: "));	Serial3.print(dTerm);
#endif
	return pTerm + dTerm;
}


void pid_reset_errors()
{
	speed_i_error = 0.f;
	angle_error_last = 0.f;
	return;
}
