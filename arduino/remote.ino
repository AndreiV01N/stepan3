#ifdef REMOTE_CTL

#define K_STEERING_AXIS			60


void apply_remote_cmd()
{
/*
	Serial.print(remote_cmd);
	Serial.print(" = ");
	Serial.println(remote_cmd_val);
*/
	if (strcmp(remote_cmd, "a0") == 0		&& on_control
							&& position_up) {		// X-axis on gamepad (steering)
		float a0_value = atof(remote_cmd_val);
		steering = a0_value * K_STEERING_AXIS;

	} else if (strcmp(remote_cmd, "a1") == 0	&& on_control
							&& position_up) {		// Y-axis on gamepad (speed)
		float a1_value = atof(remote_cmd_val);
		speed_target = a1_value * MAX_SPEED_TARGET;

	} else if (strcmp(remote_cmd, "b1") == 0	&& atoi(remote_cmd_val) == 1) {	// BUTTON-2 (reset)
		on_route = false;
		on_control = false;
		on_riseup = false;

	} else if (strcmp(remote_cmd, "b2") == 0	&& atoi(remote_cmd_val) == 1
							&& position_up
							&& is_awake) {			// BUTTON-3 (control)
		on_route = false;
		on_control = true;

	} else if (strcmp(remote_cmd, "b3") == 0	&& atoi(remote_cmd_val) == 1
							&& !position_up
							&& is_awake) {			// BUTTON-4 (riseup)
		on_riseup = true;

	} else if (strcmp(remote_cmd, "b4") == 0	&& atoi(remote_cmd_val) == 1
							&& position_up
							&& is_awake) {			// BUTTON-5 (autoroute)
		on_route = true;
		route_point = 0;
		on_control = false;

	} else if (strcmp(remote_cmd, "b5") == 0	&& atoi(remote_cmd_val) == 1
							&& is_awake) {			// BUTTON-6 (sleep)
		stall();

	} else if (strcmp(remote_cmd, "pos_kp") == 0) {
		Kp_pos = atof(remote_cmd_val);

	} else if (strcmp(remote_cmd, "pos_kd") == 0) {
		Kd_pos = atof(remote_cmd_val);

	} else if (strcmp(remote_cmd, "spd_kp") == 0) {
		Kp_speed = atof(remote_cmd_val);

	} else if (strcmp(remote_cmd, "spd_ki") == 0) {
		Ki_speed = atof(remote_cmd_val);

	} else if (strcmp(remote_cmd, "stb_kp") == 0) {
		Kp_stab = atof(remote_cmd_val);

	} else if (strcmp(remote_cmd, "stb_kd") == 0) {
		Kd_stab = atof(remote_cmd_val);
	}
}
#endif	// REMOTE_CTL
