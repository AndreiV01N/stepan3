#define SPEED_ITERM_MAX_ERROR	30
#define SPEED_ITERM_MAX		10000
#define STAB_DELTA_ERROR_MAX	8
#define POSITION_PTERM_MAX	115

static double speed_i_error = 0;
static double setpoint_last = 0;
static double input_last = 0;


double position_PD_control(int32_t input, int32_t setpoint, double Kp, double Kd, int16_t speedM)
{
	double error = setpoint - input;								// target_step_Mx - step_Mx
	double pTerm = constrain(Kp * error, -POSITION_PTERM_MAX, POSITION_PTERM_MAX);
	double dTerm = Kd * double(speedM);

	return pTerm + dTerm;										// speed_target_raw
}


double speed_PI_control(double dt, double input, double setpoint, double Kp, double Ki)
{
	double speed_error = input - setpoint;								// speed_actual - speed_target
	double pTerm = Kp * speed_error;

	speed_i_error += constrain(speed_error, -SPEED_ITERM_MAX_ERROR, SPEED_ITERM_MAX_ERROR);		// -30...+30
	speed_i_error  = constrain(speed_i_error, -SPEED_ITERM_MAX, SPEED_ITERM_MAX);			// -10000...+10000
	double iTerm = Ki * speed_i_error * dt;

	return -(pTerm + iTerm);									// angle_target_raw
}


double stability_PD_control(double dt, double input, double setpoint, double Kp, double Kd)
{
	double angle_error = input - setpoint;								// angle_actual - angle_target
	double pTerm = Kp * angle_error;

	double Kd_setpoint = constrain((setpoint - setpoint_last), -STAB_DELTA_ERROR_MAX, STAB_DELTA_ERROR_MAX);
	double dTerm1 = Kd * Kd_setpoint;
	double dTerm2 = Kd * (input - input_last);
	input_last = input;
	setpoint_last = setpoint;

	return pTerm + (dTerm2 - dTerm1) / dt;								// control_output
}
