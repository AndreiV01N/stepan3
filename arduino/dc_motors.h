#ifndef _DC_MOTORS_H
#define _DC_MOTORS_H

void motor_pins_init();
void set_speed_M1(int16_t tspeed);
void set_speed_M2(int16_t tspeed);
void stall();

#endif
