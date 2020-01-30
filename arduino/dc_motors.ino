#define M1_STEP_PIN		2			// PE4
#define M2_STEP_PIN		3			// PE5
#define M1_DIR_PIN		5			// PE3
#define M2_DIR_PIN		6			// PH3
#define	CNC_ENABLED_PIN		8			// PH5

#define ZERO_SPEED		65535
#define RISEUP_SPEED		500
#define RISEUP_BRAKE		5
#define MICROSTEPPING		4			// power of 2 for 16, 8, 4, 2, 1 (i.e. 4 means 1/16, 0 means "full stepping")
							// must fit to jumpers' settings on CNC-shield
#define TIMER_CLOCK		250000			// (Hz) configured in TCCR1B, TCCR3B registers

/*
	2 x 17HS4401 - Rated current: 1.7A
	microstepping:		FULL	1/2	1/4	1/8	1/16
	steps/rotation:		200	400	800	1600	3200
	degree/step		1.8	0.9	0.45	0.225	0.1125

	2 x A4988, Rs = 0.1 Ohm, Vref has to be set near 0.95V
*/

#include <avr/io.h>


/* http://www.atmel.com/Images/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf
   STEP_M1: D2 (PE4), DIR_M1: D5 (PE3) as outputs (step controlled by TIMER-1) CNC SHIELD-X
   STEP_M2: D3 (PE5), DIR_M2: D6 (PH3) as outputs (step controlled by TIMER-3) CNC SHIELD-Y
   (both motors are driven by A4988)
*/
void motor_pins_init()
{
	Serial.println("Stepper motors initialization...");

	pinMode(M1_STEP_PIN, OUTPUT);			// PE4
	pinMode(M2_STEP_PIN, OUTPUT);			// PE5
	pinMode(M1_DIR_PIN, OUTPUT);			// PE3
	pinMode(M2_DIR_PIN, OUTPUT);			// PH3
	pinMode(CNC_ENABLED_PIN, OUTPUT);		// PH5

	TCCR1A = 0;					// OC1A, OC1B, OC1C pins are disconnected
	TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10);	// Timer1 mode 4 (CTC), Prescaler=64 (clk_io/64 = 250kHz)
	OCR1A = ZERO_SPEED;				// M1 stopped (16-bit)
	speed_M1 = 0;
	step_M1 = 0;
	dir_M1 = 0;
	TCNT1 = 0;

	TCCR3A = 0;					// OC3A, OC3B, OC3C pins are disconnected
	TCCR3B = _BV(WGM32) | _BV(CS31) | _BV(CS30);	// Timer3 mode 4 (CTC), Prescaler=64 (clk_io/64 = 250kHz)
	OCR3A = ZERO_SPEED;				// M2 stopped (16-bit)
	speed_M2 = 0;
	step_M2 = 0;
	dir_M2 = 0;
	TCNT3 = 0;

	PORTH &= ~_BV(5);				// PH5 is LOW (enable motors)
	TIMSK1 |= _BV(OCIE1A);				// Enable Timer1 interrupt
	TIMSK3 |= _BV(OCIE3A);				// Enable Timer3 interrupt

	return;
}


ISR(TIMER1_COMPA_vect)		// M1
{
	if (dir_M1 == 0)	// zero speed
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
	if (dir_M2 == 0)	// zero speed
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

	if (speed_M1 >= 0) {							// going fwd or stopped
		if (speed_M1 < tspeed) {					// speedup (accel) to fwd
			if ((tspeed - speed_M1) < max_accel)
				speed_M1 = tspeed;
			else
				speed_M1 += max_accel;
		} else {							// slow down (deccel)
			if ((speed_M1 - tspeed) < max_deccel)
				speed_M1 = tspeed;
			else
				speed_M1 -= max_deccel;
		}
	} else {								// going back
		if (tspeed < speed_M1) {					// speedup (accel) to bck
			if ((speed_M1 - tspeed) < max_accel )
				speed_M1 = tspeed;
			else
				speed_M1 -= max_accel;
		} else {							// slow down (deccel)
			if ((tspeed - speed_M1) < max_deccel)
				speed_M1 = tspeed;
			else
				speed_M1 += max_deccel;
		}
	}

	speed = speed_M1 << MICROSTEPPING;		/* cast to real motor speed in u-steps/sec
							TIMER_CLOCK = 250 kHz:

							uSTEPPING = 16 (*is used)
							min speed = 1 << 4      = 16    steps/sec, OCRnA = 250000/16    = 15625
							max speed = 700 << 4    = 11200 steps/sec, OCRnA = 250000/11200 = 22

							uSTEPPING = 8
							min speed = 1 << 3      = 8     steps/sec, OCRnA = 250000/8     = 31250
							max speed = 700 << 3    = 5600  steps/sec, OCRnA = 250000/5600  = 45

							uSTEPPING = 4
							min speed = 1 << 2      = 4     steps/sec, OCRnA = 250000/4     = 62500
							max speed = 700 << 2    = 2800  steps/sec, OCRnA = 250000/2800  = 89
							*/
	if (speed == 0) {
		timer_period = ZERO_SPEED;
		dir_M1 = 0;
	} else if (speed > 0) {
		timer_period = TIMER_CLOCK / speed;
		dir_M1 = 1;
		PORTE &= ~_BV(3);			// LOW - DIR M1 (Forward)
	} else {
		timer_period = TIMER_CLOCK / -speed;
		dir_M1 = -1;
		PORTE |= _BV(3);			// HIGH - DIR M1 (Reverse)
	}

	if (timer_period > ZERO_SPEED)			// Check for minimun speed (maximum period without overflow)
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

	if (speed_M2 >= 0) {							// going fwd or stopped
		if (speed_M2 < tspeed) {					// speedup (accel) to fwd
			if ((tspeed - speed_M2) < max_accel)
				speed_M2 = tspeed;
			else
				speed_M2 += max_accel;
		} else {							// slow down (deccel)
			if ((speed_M2 - tspeed) < max_deccel)
				speed_M2 = tspeed;
			else
				speed_M2 -= max_deccel;
		}
	} else {								// going back
		if (tspeed < speed_M2) {					// speedup (accel) to bck
			if ((speed_M2 - tspeed) < max_accel )
				speed_M2 = tspeed;
			else
				speed_M2 -= max_accel;
		} else {							// slow down (deccel)
			if ((tspeed - speed_M2) < max_deccel)
				speed_M2 = tspeed;
			else
				speed_M2 += max_deccel;
		}
	}

	speed = speed_M2 << MICROSTEPPING;		// cast to real motor speed in steps/sec

	if (speed == 0) {
		timer_period = ZERO_SPEED;
		dir_M2 = 0;
	} else if (speed > 0) {
		timer_period = TIMER_CLOCK / speed;
		dir_M2 = 1;
		PORTH |= _BV(3);			// HIGH - DIR M2 (Forward)
	} else {
		timer_period = TIMER_CLOCK / -speed;
		dir_M2 = -1;
		PORTH &= ~_BV(3);			// LOW - DIR M2 (Reverse)
	}

	if (timer_period > ZERO_SPEED)			// Check for minimun speed (maximum period without overflow)
		timer_period = ZERO_SPEED;

	OCR3A = timer_period;
	if (TCNT3 > OCR3A)				// Check if we need to reset the timer
		TCNT3 = 0;
}


void stall()
{
	set_speed_M1(0);
	set_speed_M2(0);
	on_riseup = false;
	is_awake = false;
	riseup_phase = 0;
	control_output = 0.f;
	control_output_raw = 0.f;
	timer_sleep_ms = millis();
//	PORTH |= _BV(5);				// PH5 is HIGH (disable motors)
}


void riseup_control()
{
	int16_t riseup_speed;
	int16_t riseup_brake;
	
	switch (riseup_phase) {
		case 0:
//			PORTH &= ~_BV(5);			// PH5 is LOW (enable motors)
			max_accel = MAX_ACCEL_RISEUP;
			max_deccel = MAX_DECCEL_RISEUP;

			if (tilt_now > TILT_READY) {
				riseup_speed = -RISEUP_SPEED;
				riseup_brake = -RISEUP_BRAKE;
			} else if (tilt_now < -TILT_READY) {
				riseup_speed = RISEUP_SPEED;
				riseup_brake = RISEUP_BRAKE;
			}

			timer_riseup_ms = millis();
			set_speed_M1(riseup_speed);
			set_speed_M2(riseup_speed);
			riseup_phase = 1;
			break;
		case 1:
			if (speed_M1 == riseup_speed && speed_M2 == riseup_speed)
				riseup_phase = 2;		// got the right speed, it's time to backpedal
			else {
				set_speed_M1(riseup_speed);
				set_speed_M2(riseup_speed);
			}
			break;
		case 2:
			if (speed_M1 == riseup_brake && speed_M2 == riseup_brake)
				riseup_phase = 3;
			else {
				set_speed_M1(riseup_brake);
			}	set_speed_M2(riseup_brake);
			break;
		case 3:
			// nothing else has to do, just hoping it's jumped in tilt working sector..
			break;
	}
}
