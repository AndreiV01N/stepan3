#define MICROSTEPPING	16
#define ZERO_SPEED	65535

#include <avr/io.h>


void motor_pins_init()
{
	Serial.println("Stepper motors initialization...");
// http://www.atmel.com/Images/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf
// STEP_M1: D2 (PE4), DIR_M1: D5 (PE3) as outputs (step controlled by TIMER-1) CNC SHIELD-X
// STEP_M2: D3 (PE5), DIR_M2: D6 (PH3) as outputs (step controlled by TIMER-3) CNC SHIELD-Y

	DDRE |= _BV(3) | _BV(4) | _BV(5);	// DIR_M1, STEP_M1, STEP_M2 are outputs
	DDRH |= _BV(3) | _BV(5);		// DIR_M2, CNC_ENABLE       are outputs

	TCCR1A = 0;				// Timer1 CTC mode 4, OCxA,B outputs disconnected
	TCCR1B = _BV(WGM12) | _BV(CS11);	// Prescaler=8 (clk_io/8 = 2Mhz)
	OCR1A = ZERO_SPEED;			// Motor stopped
	speed_M1 = 0;
	step_M1 = 0;
	dir_M1 = 0;
	TCNT1 = 0;

	TCCR3A = 0;				// Timer3 CTC mode 4, OCxA,B outputs disconnected
	TCCR3B = _BV(WGM32) | _BV(CS31);	// Prescaler=8 (clk_io/8 = 2Mhz)
	OCR3A = ZERO_SPEED;			// Motor stopped
	speed_M2 = 0;
	step_M2 = 0;
	dir_M2 = 0;
	TCNT3 = 0;

	PORTH &= ~_BV(5);			// PH5 is LOW (ENABLE MOTORS)
	TIMSK1 |= _BV(OCIE1A);			// Enable Timer1 interrupt
	TIMSK3 |= _BV(OCIE1A);			// Enable Timer3 interrupt

	return;
}


ISR(TIMER1_COMPA_vect)		// M1
{
	if (dir_M1 == 0)
		return;

	PORTE |= _BV(4);	// HIGH

	if (dir_M1 > 0)
		step_M1++;
	else
		step_M1--;

	PORTE &= ~_BV(4);	// LOW
}


ISR(TIMER3_COMPA_vect)		// M2
{
	if (dir_M2 == 0)
		return;

	PORTE |= _BV(5);	// HIGH

	if (dir_M2 > 0)
		step_M2++;
	else
		step_M2--;

	PORTE &= ~_BV(5);	// LOW
}


void set_speed_M1(int16_t tspeed)
{
	int32_t timer_period;
	int16_t speed;

	if (tspeed == speed_M1)
		return;

	if (tspeed > 0) {
		if (tspeed < speed_M1 || (tspeed - speed_M1) <= max_accel)	// slow down or accel reasonably
			speed_M1 = tspeed;
		else
			speed_M1 += max_accel;					// accel too fast
	} else if (tspeed < 0) {
		if (speed_M1 < tspeed || (tspeed - speed_M1) >= -max_accel)	// slow down or accel reasonably
			speed_M1 = tspeed;
		else
			speed_M1 -= max_accel;					// accel too fast
	} else
		speed_M1 = tspeed;

#if MICROSTEPPING==16
	speed = speed_M1 * 50;				// Adjust factor from control output speed to real motor speed in steps/second
#else
	speed = speed_M1 * 25;				// 1/8 Microstepping
#endif

	if (speed == 0) {
		timer_period = ZERO_SPEED;		// 0xFF
		dir_M1 = 0;
	} else if (speed > 0) {
		timer_period = 2000000 / speed;		// 2Mhz timer
		dir_M1 = 1;
		PORTE &= ~_BV(3);			// LOW - DIR M1 (Forward)
	} else {
		timer_period = 2000000 / -speed;
		dir_M1 = -1;
		PORTE |= _BV(3);			// HIGH - DIR M1 (Reverse)
	}

	if (timer_period > 65535)			// Check for minimun speed (maximun period without overflow)
		timer_period = ZERO_SPEED;

	OCR1A = timer_period;
	if (TCNT1 > OCR1A)				// Check if we need to reset the timer
		TCNT1 = 0;
}


void set_speed_M2(int16_t tspeed)
{
	int32_t timer_period;
	int16_t speed;

	if (tspeed == speed_M2)
		return;

	if (tspeed > 0) {
		if (tspeed < speed_M2 || (tspeed - speed_M2) <= max_accel)
			speed_M2 = tspeed;
		else
			speed_M2 += max_accel;
	} else if (tspeed < 0) {
		if (speed_M2 < tspeed || (tspeed - speed_M2) >= -max_accel)
			speed_M2 = tspeed;
		else
			speed_M2 -= max_accel;
	} else
		speed_M2 = tspeed;

#if MICROSTEPPING==16
	speed = speed_M2 * 50;				// Adjust factor from control output speed to real motor speed in steps/second
#else
	speed = speed_M2 * 25;				// 1/8 Microstepping
#endif

	if (speed == 0) {
		timer_period = ZERO_SPEED;		// 0xFF
		dir_M2 = 0;
	} else if (speed > 0) {
		timer_period = 2000000 / speed;		// 2Mhz timer
		dir_M2 = 1;
		PORTH |= _BV(3);			// HIGH - DIR M2 (Forward)
	} else {
		timer_period = 2000000 / -speed;
		dir_M2 = -1;
		PORTH &= ~_BV(3);			// LOW - DIR M2 (Reverse)
	}

	if (timer_period > 65535)			// Check for minimun speed (maximun period without overflow)
		timer_period = ZERO_SPEED;

	OCR3A = timer_period;
	if (TCNT3 > OCR3A)				// Check if we need to reset the timer
		TCNT3 = 0;
}


void stall()
{
	PORTH |= _BV(5);		// Disable motors
	speed_M1 = 0;
	speed_M2 = 0;
	set_speed_M1(0);
	set_speed_M2(0);
	delay(2000);			// Wait 2s
}
