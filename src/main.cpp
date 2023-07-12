#include "glob.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "pwm.h"

#define MAX_VALUE 65535
#define TICK_PWM 23//(1/F_PWM) = 1/43360 = 23/1 000 000

struct global_variables glob = {OFF, OFF, OFF, OFF , OFF, OFF, OFF, OFF, OFF, OFF, OFF, MAX_VALUE};
volatile struct global_variables *glob_parametrs = &glob;

static int buf_index = 0;
static char buffer[BUFF_SIZE] = {0};
char buff_to_send[BUFF_SIZE];

const char command_on[] = "On\r";
const char command_off[] = "Off\r";
const char command_pwm[] = "Power";

void setup() {
	pwm_init();
	timer_init();
	counter_init();
  UartInit();
  sei();

	char buff_to_send[BUFF_SIZE];
	sprintf((char*)buff_to_send, "mode:OFF [power: 0%%]\r\n");
	DebagUart(buff_to_send); 
}

void loop() {
  if (Serial.available() > 0) {
    buf_index += Serial.readBytes(buffer + buf_index, 1);
    if (buf_index > BUFF_SIZE){
      buf_index = 0;
    }
    if (buf_index != 0 && buffer[buf_index - 1] == '\n') {
      buf_index = 0;
			const char space[] = " ";
			char *until_space;
			char *after_space;
			until_space = strtok (buffer, space);
			after_space = strtok (NULL, space);
			if ( after_space != NULL){
				*(after_space + 3) = '\0';
				(*glob_parametrs).pwm = strtol(after_space,0, 10);
				if (errno == ERANGE) {
					(*glob_parametrs).pwm = 100;//overflow
					errno = 0;
				} else {
					if ((*glob_parametrs).pwm > 100) {
						(*glob_parametrs).pwm = 100;
					} else {
						if ((*glob_parametrs).pwm <= 0) {
						(*glob_parametrs).pwm = 0;
						}
					}
				}
			}
			else {//str havnt spaces
				until_space = buffer;
			}
			if (!strcmp((char *)until_space, command_off)){
				setPwmDuty(0);
				sprintf((char*)buff_to_send, "mode:OFF [power: 0%%]\r\n");
			} else {
				if (!strcmp((char *)until_space, command_on)){
					setPwmDuty((*glob_parametrs).pwm);
					sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", (*glob_parametrs).pwm);
				} else {
					if (!strcmp((char *)until_space, command_pwm)){
						if ((*glob_parametrs).pwm) {
							setPwmDuty((*glob_parametrs).pwm);
							sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", (*glob_parametrs).pwm);
						} else {
							setPwmDuty(0);
							sprintf((char*)buff_to_send, "mode:OFF [power: 0%%]\r\n");
						}
					} else {
						sprintf((char*)buff_to_send, "Error\r\n");
					}
				}
			}
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
		} 
		if ((*glob_parametrs).pulse_capture_flag) {
			if ((*glob_parametrs).time_fixation > (*glob_parametrs).max_ticks_between_pulses) {
				(*glob_parametrs).max_ticks_between_pulses = (*glob_parametrs).time_fixation;
			}
			if ((*glob_parametrs).time_fixation < (*glob_parametrs).min_ticks_between_pulses) {
				(*glob_parametrs).min_ticks_between_pulses = (*glob_parametrs).time_fixation;
			}
			(*glob_parametrs).pulse_capture_flag = 0;
		}
		if ((*glob_parametrs).flag_rev) { 
			if ((*glob_parametrs).max_ticks_between_pulses >= (*glob_parametrs).min_ticks_between_pulses) {
				(*glob_parametrs).mid_ticks_between_pulses = (((*glob_parametrs).max_ticks_between_pulses + (*glob_parametrs).min_ticks_between_pulses) >> 1);
				(*glob_parametrs).real_time_between_pulses = ((*glob_parametrs).mid_ticks_between_pulses * TICK_PWM);//real_time_between_pulses * 1 000 000
				(*glob_parametrs).real_rev_time = 2 * (*glob_parametrs).real_time_between_pulses;//real_rev_time (* 1000
				(*glob_parametrs).rev_per_sec = 1000000/(*glob_parametrs).real_rev_time;//RPS = 1sec/real_rev_time		
			} else {
				(*glob_parametrs).rev_per_sec = 0;
			}
			(*glob_parametrs).rev_per_min = (*glob_parametrs).rev_per_sec * 60;//RPM = RPS * 60
			sprintf((char*)buff_to_send, "rev: %d RPM\r\n", (*glob_parametrs).rev_per_min);
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			(*glob_parametrs).max_ticks_between_pulses = 0;
			(*glob_parametrs).min_ticks_between_pulses = MAX_VALUE;
      (*glob_parametrs).flag_rev = 0;
		}
  }
}
