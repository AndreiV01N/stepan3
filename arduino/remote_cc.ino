#define K_SPEED_AXIS		70
#define K_STEERING_AXIS		30

#ifdef COMMAND_CENTER

static String command;
static String value;
static bool new_cmd;


bool __read_next_remote_cmd()
{
	char ch = ' ';
	command = "";
	value = "";
	new_cmd = false;
	uint8_t stage = 0;

	while (Serial3.available()) {
		if (ch == '}')
			return false;					// one more command remained in Serial3
		ch = Serial3.read();
		switch (stage) {
			case 0:
				if (ch == '{')
					stage = 1;
				break;
			case 1:
				if (ch == '=')
					stage = 2;
				else
					command += ch;			// Saving command name
				break;
			case 2:
				if (ch == '}')
					new_cmd = true;
				else
					value += ch;			// Saving command value
				break;
		}
	}
	return true;							// Serial3 is empty here
}


void apply_remote_cmd()
{
	bool empty_buffer = false;
	while (!empty_buffer) {
		empty_buffer = __read_next_remote_cmd();

		if (new_cmd) {
			if (command == "a1" && on_control && position_up) {			// X-axis on gamepad (speed)
				float a1_value = value.toFloat();
				speed_target = a1_value * K_SPEED_AXIS;

			} else if (command == "a0" && on_control && position_up) {		// Y-axis on gamepad (steering)
				float a0_value = value.toFloat();
				steering = a0_value * K_STEERING_AXIS;

			} else if (command == "b1" && value.toInt() == 1) {			// BUTTON-2 (reset)
				on_route = false;
				on_control = false;
				on_riseup = false;

			} else if (command == "b2" && value.toInt() == 1 && position_up) {	// BUTTON-3 (control)
				on_route = false;
				on_control = true;

			} else if (command == "b3" && value.toInt() == 1 && !position_up) {	// BUTTON-4 (riseup)
				on_riseup = true;

			} else if (command == "b4" && value.toInt() == 1 && position_up) {	// BUTTON-4 (autoroute)
				on_route = true;
				route_point = 0;
				on_control = false;

			} else if (command == "b5" && value.toInt() == 1 && position_up) {	// BUTTON-6 (sleep)
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
}

#endif
