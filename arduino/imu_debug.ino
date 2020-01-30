#if DEBUG_IMU
void print_raw_ahrs_data(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float axis_x, float axis_y, float axis_z, float dt_ms)
{
	Serial.print(F("Raw: "));
	Serial.print(ax, 3);		Serial.print(F("\t"));	// accel
	Serial.print(ay, 3);		Serial.print(F("\t"));
	Serial.print(az, 3);		Serial.print(F("\t"));
	Serial.print(gx, 3);		Serial.print(F("\t"));	// gyro
	Serial.print(gy, 3);		Serial.print(F("\t"));
	Serial.print(gx, 3);		Serial.print(F("\t"));
	Serial.print(mx, 3);		Serial.print(F("\t"));	// mag
	Serial.print(my, 3);		Serial.print(F("\t"));
	Serial.print(mz, 3);		Serial.print(F("\t"));
	Serial.print(F("RPY: "));
	Serial.print(axis_x, 3);	Serial.print(F("\t"));	// roll
	Serial.print(axis_y, 3);	Serial.print(F("\t"));	// pitch
	Serial.print(axis_z, 3);	Serial.print(F("\t"));	// yaw
	Serial.print(F("LOOP: "));
	Serial.println(dt_ms, 2);				// loop duration
}

void print_processing_cube_data(float q0, float q1, float q2, float q3, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float temp, float dt_ms, float heading)
{
	Serial.print(q0, 8);		Serial.print(F(","));
	Serial.print(q1, 8);		Serial.print(F(","));
	Serial.print(q2, 8);		Serial.print(F(","));
	Serial.print(q3, 8);		Serial.print(F(","));
	Serial.print(ax, 6);		Serial.print(F(","));
	Serial.print(ay, 6);		Serial.print(F(","));
	Serial.print(az, 6);		Serial.print(F(","));
	Serial.print(gx, 6);		Serial.print(F(","));
	Serial.print(gy, 6);		Serial.print(F(","));
	Serial.print(gz, 6);		Serial.print(F(","));
	Serial.print(mx, 6);		Serial.print(F(","));
	Serial.print(my, 6);		Serial.print(F(","));
	Serial.print(mz, 6);		Serial.print(F(","));
	Serial.print(temp, 2);		Serial.print(F(","));
	Serial.print(0.0f);		Serial.print(F(","));	// pressure stub
	Serial.print(dt_ms, 2);		Serial.print(F(","));
	Serial.print(heading, 2);	Serial.print(F(","));
	Serial.println(0.0f);					// altitude stub
}

void print_processing_bunnyrotate_data(uint32_t now_ms, uint32_t loop_num, float dt_ms, float axis_x, float axis_y, float axis_z)
{
	Serial.print(F("Orientation: "));
	Serial.print(axis_x, 2);	Serial.print(F(" "));
	Serial.print(axis_y, 2);	Serial.print(F(" "));
	Serial.print(axis_z, 2);	Serial.print(F(" uptime,loop,delta: "));
	Serial.print(now_ms);		Serial.print(F(" "));
	Serial.print(loop_num);		Serial.print(F(" "));
	Serial.println(dt_ms, 2);
}
#endif
