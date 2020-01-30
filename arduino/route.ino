#define MAX_STEERING		100.0f

static int32_t check_step_M1;
static int32_t check_step_M2;
static int32_t rest_of_steps_M1;		// how far the end of current route segment
static int32_t rest_of_steps_M2;
static int32_t rest_of_steps_SUM;
static int8_t direction_M1;			// -1: reverse, +1: forward
static int8_t direction_M2;

// looks like a numeral "eight" in size of ~1m
static int32_t route_M1[] = {
	5168,
	20000,
	5168,
	-5168,
	-20000,
	-5168
};

static int32_t route_M2[] = {
	10000,
	10336,
	10000,
	-10000,
	-10336,
	-10000
};

static uint8_t route_size = sizeof(route_M1) / sizeof(route_M1[0]);;			// number of route segments


void __update_route()
{
	check_step_M1 = step_M1 + route_M1[route_point];
	if (route_M1[route_point] >= 0) {
		target_step_M1 = check_step_M1 + (abs(check_step_M1 >> 4));		// 1 + .0625
		direction_M1 = 1;
	} else {
		target_step_M1 = check_step_M1 - (abs(check_step_M1 >> 4));
		direction_M1 = -1;
	}

	check_step_M2 = step_M2 + route_M2[route_point];
	if (route_M2[route_point] >= 0) {
		target_step_M2 = check_step_M2 + (abs(check_step_M2 >> 4));
		direction_M2 = 1;
	} else {
		target_step_M2 = check_step_M2 - (abs(check_step_M2 >> 4));
		direction_M2 = -1;
	}

	route_point++;									// pointing to next segment
	return;
}


void follow_route()
{
	if (route_point == 0)								// just started a new route
		__update_route();							// update with first segment
	else {
		rest_of_steps_M1 = check_step_M1 - step_M1;
		rest_of_steps_M1 = direction_M1 > 0 ? rest_of_steps_M1 : -rest_of_steps_M1;
		rest_of_steps_M2 = check_step_M2 - step_M2;
		rest_of_steps_M2 = direction_M2 > 0 ? rest_of_steps_M2 : -rest_of_steps_M2;

		if (rest_of_steps_M1 <= 0 && rest_of_steps_M2 <= 0) {			// we've reached end of segment
			if (route_point < route_size)
				__update_route();					// update with next segment
			else {								// end of the last segment, route is over
				target_step_M1 = step_M1;
				target_step_M2 = step_M2;
				route_point = 0;
#ifdef STANDALONE
				on_route = false;
				timer_route_ms = millis();
#endif
			}
		}
	}
	return;
}


float get_steering()									// is called only when (on_route == true)
{
	rest_of_steps_M1 = check_step_M1 - step_M1;
	rest_of_steps_M2 = check_step_M2 - step_M2;
	rest_of_steps_SUM = rest_of_steps_M1 + rest_of_steps_M2;

	rest_of_steps_SUM = rest_of_steps_SUM == 0 ? 1 : rest_of_steps_SUM;		// prevent div by zero

	float control_output_M1 = control_output * (float)rest_of_steps_M1 / (float)rest_of_steps_SUM;
	float control_output_M2 = control_output * (float)rest_of_steps_M2 / (float)rest_of_steps_SUM;

	float steering_raw = control_output_M1 - control_output_M2;
//	float steering     = constrain(steering_raw, -MAX_STEERING, MAX_STEERING);

	return steering_raw;
}
