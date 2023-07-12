/*
 * pwm.c
 *
 * Created: 30.06.2022 17:56:58
 * Author : sshmykov
 */ 
// #define F_CPU 16000000L
// #include <avr/io.h>
// #include <string.h>
// #include <util/delay.h>
//#include <avr/interrupt.h>
#include "glob.h"
#include "pwm.h"

#define PWM_PIN 6
#define COUNT_PIN 3
#define TOP 255
#define ONE_SEC 1000

volatile extern struct global_variables *glob_parametrs;

static uint8_t interrupt_flag = 0;
static uint16_t tick_counter = 0;

void mon_timer_init(void) {// 100.16025641025641 Hz (16000000/((155+1)*1024))
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
	if ((*glob_parametrs).flag_rev == 0) {
		(*glob_parametrs).flag_rev = 1;
	}
}

void timer_init(void) {
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  // 50000 Hz (16000000/((4+1)*64))
  OCR2A = 4;
  // CTC
  TCCR2A |= (1 << WGM21);
  // Prescaler 64
  TCCR2B |= (1 << CS22);
  // Output Compare Match A Interrupt Enable
  TIMSK2 |= (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect) {//couunt ticks from pulse to pulse
	cli();
	if (interrupt_flag) {
		tick_counter++;		
	}
	sei();
}

void counter_init(void) {
	DDRD &= ~(1 << COUNT_PIN);//INT1 Pin 3 of the arduino uno(PD3)
	PORTD |= (1 << COUNT_PIN); 
	EIMSK = (1 << INT1);
	//The rising edge of INT1 generates an interrupt request.
	EICRA |= (1 << ISC10);
	EICRA |= (1 << ISC11);
}

ISR(INT1_vect) {//pulse_capture
	//cli();
	interrupt_flag = !interrupt_flag;
	if (interrupt_flag == 0) {
		if ((*glob_parametrs).pulse_capture_flag == 0) {
			(*glob_parametrs).time_fixation = tick_counter;
			tick_counter = 0;
			(*glob_parametrs).pulse_capture_flag = 1;
		}
		interrupt_flag = 1;
	}
	//sei();
}

void pwm_init(void) {
	DDRD |= (1 << PWM_PIN);//Pin 6 of the arduino uno (PD6)
	TCCR0A |= (1 << WGM00) | (1 << COM0A1);
	TCCR0B |= (1 << WGM01) | (1 << CS00);
	TCNT0 = 0;

}

void setPwmDuty(uint8_t duty) {
	OCR0A = (uint16_t) (duty * TOP) / 100;
}
