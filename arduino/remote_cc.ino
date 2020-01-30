#ifdef COMMAND_CENTER

#define K_STEERING_AXIS		60

static String command;
static String value;


/* remote command format is: {command=value} */
bool __read_remote_cmd()
{
	char ch;
	uint8_t stage = 0;

	String command_tmp;
	String value_tmp;

	bool new_cmd = false;

	while (Serial3.available()) {

		ch = Serial3.read();

		switch (stage) {
			case 0:
				if (ch == '{') {
					command_tmp = "";
					stage = 1;
				}
				break;
			case 1:
				if (ch == '=') {
					value_tmp = "";
					stage = 2;
				} else
					command_tmp += ch;		// Saving command name
				break;
			case 2:
				if (ch == '}') {
					command = command_tmp;
					value = value_tmp;
					new_cmd = true;
					stage = 0;
				} else
					value_tmp += ch;		// Saving command value
				break;
		}
	}
	return new_cmd;							// Serial3 is empty here
}


void apply_remote_cmd()
{
	if (__read_remote_cmd()) {
		if (command == "a1"		&& on_control
						&& position_up) {			// X-axis on gamepad (speed)
			float a1_value = value.toFloat();
			speed_target = a1_value * MAX_SPEED_TARGET;

		} else if (command == "a0"	&& on_control
						&& position_up) {			// Y-axis on gamepad (steering)
			float a0_value = value.toFloat();
			steering = a0_value * K_STEERING_AXIS;

		} else if (command == "b1"	&& value.toInt() == 1) {		// BUTTON-2 (reset)
			on_route = false;
			on_control = false;
			on_riseup = false;

		} else if (command == "b2"	&& value.toInt() == 1
						&& position_up
						&& is_awake) {				// BUTTON-3 (control)
			on_route = false;
			on_control = true;

		} else if (command == "b3"	&& value.toInt() == 1
						&& !position_up
						&& is_awake) {				// BUTTON-4 (riseup)
			on_riseup = true;

		} else if (command == "b4"	&& value.toInt() == 1
						&& position_up
						&& is_awake) {				// BUTTON-5 (autoroute)
			on_route = true;
			route_point = 0;
			on_control = false;

		} else if (command == "b5"	&& value.toInt() == 1
						&& is_awake) {				// BUTTON-6 (sleep)
			stall();

		} else if (command == "pos_kp") {
			Kp_pos = value.toFloat();

		} else if (command == "pos_kd") {
			Kd_pos = value.toFloat();

		} else if (command == "spd_kp") {
			Kp_speed = value.toFloat();

		} else if (command == "spd_ki") {
			Ki_speed = value.toFloat();

		} else if (command == "stb_kp") {
			Kp_stab = value.toFloat();

		} else if (command == "stb_kd") {
			Kd_stab = value.toFloat();
		}
	}
}
#endif	// COMMAND_CENTER
