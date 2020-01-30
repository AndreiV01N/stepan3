#define SONAR_TRIG_PIN			19
#define SONAR_ECHO_PIN			18
#define SONAR_TRIG_TIMEOUT_uS		1000
#define SONAR_PING_TIMEOUT_uS		23000	// i.e. max distance is abt. 4 m
#define SONAR_BUSY_TIMEOUT_uS		50000	// until next ping is allowed
#define SONAR_TIMER_RESET_AT		100	// 0..65535
#define SONAR_INDEFINITE_DIST_CM	6666.6f	// no response on ping in time, object probably is too far away

#include <avr/io.h>

static uint32_t sonar_trig_timer;
static uint32_t sonar_ping_timer;
static uint32_t sonar_busy_timer;
static bool sonar_echo_high_waiting = false;		// waiting for  __--  once TRIG is bounced
static bool sonar_echo_low_waiting = false;		// waiting for  --__


void sonar_pins_init()
{
	Serial.println("Ultrasonic HC-SR04 initialization...");
// http://www.atmel.com/Images/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf
// TRIG pin: D19 (PD2) as output
// ECHO pin: D18 (PD3) as input

	pinMode(SONAR_TRIG_PIN, OUTPUT);
	pinMode(SONAR_ECHO_PIN, INPUT);

	TCCR4A = 0;				// OC4A, OC4B, OC4C pins are disconnected
	TCCR4B = _BV(WGM42) | _BV(CS41) ;	// Timer4 CTC mode (4), Prescaler=8 (clk_io/8 = 2Mhz = 0.5us = 0.1715mm)
	OCR4A = SONAR_TIMER_RESET_AT;		// max error ~17 mm
	TCNT4 = 0;

	TIMSK4 |= _BV(OCIE4A);			// Enable Timer4 interrupt

	return;
}


ISR(TIMER4_COMPA_vect)
{
	if (sonar_echo_high_waiting) {
		if (digitalRead(SONAR_ECHO_PIN)) {				// got HIGH on ECHO_PIN
			sonar_echo_high_waiting = false;
			sonar_echo_low_waiting = true;
			sonar_ping_timer = micros();
		}
		if (micros() - sonar_trig_timer > SONAR_TRIG_TIMEOUT_uS) {	// timed out
			sonar_echo_high_waiting = false;
			sonar_busy_timer = micros();
		}
		return;
	}

	if (sonar_echo_low_waiting) {
		if (!digitalRead(SONAR_ECHO_PIN)) {				// got LOW on ECHO_PIN - the delay can be calculated
			sonar_echo_low_waiting = false;
			sonar_delay_us = micros() - sonar_ping_timer;
			sonar_dist_cm = float(sonar_delay_us) * 0.01715f;	// (0.0343 cm/us / 2)
			sonar_busy_timer = micros();
		}
		if (micros() - sonar_ping_timer > SONAR_PING_TIMEOUT_uS) {	// timed out, maybe an object is too far away
			sonar_echo_low_waiting = false;
			sonar_dist_cm = SONAR_INDEFINITE_DIST_CM;
			sonar_busy_timer = micros();
		}
		return;
	}

	if (!sonar_ready && (micros() - sonar_busy_timer) > SONAR_BUSY_TIMEOUT_uS)
		sonar_ready = true;
}


void sonar_send_ping()
{
	digitalWrite(SONAR_TRIG_PIN, LOW);
	delayMicroseconds(2);
	digitalWrite(SONAR_TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(SONAR_TRIG_PIN, LOW);
	sonar_trig_timer = micros();
	sonar_ready = false;
	sonar_echo_high_waiting = true;
	sonar_echo_low_waiting = false;
}
