#include "glob.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "pwm.h"

#define MAX_VALUE 65535
//#define TICK_PWM 23//(1/F_PWM) = 1/43360 = 23/1 000 000
#define TICK_PWM 20//(1/F_PWM) = 1/50000 = 20/1 000 000

struct global_variables glob = {OFF, OFF, OFF, OFF, OFF, OFF , OFF, OFF, OFF, OFF, OFF, OFF, OFF, MAX_VALUE};
volatile struct global_variables *glob_parametrs = &glob;

static int buf_index = 0;
static char buffer[BUFF_SIZE] = {0};
//char buff_to_send[BUFF_SIZE];

const char welcome[] = 
"\r\nUart boadrate - 115200. Add (NL & CR) to the end of the commands to use them.\r\n\
Type \"power\" - for fan control\r\n\
Type \"mon\"   - to set the monitoring period\r\n";

void setup() {
	pwm_init();
	timer_init();
	counter_init();
  UartInit();
  mon_timer_init();
  //sei();
	DebagUart(welcome); 
}



void loop() {
  if (Serial.available() > 0) {
    buf_index += Serial.readBytes(buffer + buf_index, 1);
    if (buf_index > BUFF_SIZE){
      buf_index = 0;
    }
    if (buf_index != 0 && buffer[buf_index - 1] == '\n') {
      buf_index = 0;
      switch (cmd_parser(buffer)) {
        case MS_POWER:
          pwm_parser(buffer);
          break;
        case MS_MON:
          mon_parser(buffer);
          break;
      }
    }
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

  if ((*glob_parametrs).flag_rev) { //1 sec
    // char buff_to_send[BUFF_SIZE];
    // sprintf((char*)buff_to_send, "min: %ld max %ld\r\n", (*glob_parametrs).min_ticks_between_pulses, (*glob_parametrs).max_ticks_between_pulses);
    // DebagUart(buff_to_send);
    if ((*glob_parametrs).max_ticks_between_pulses >= (*glob_parametrs).min_ticks_between_pulses) {
      (*glob_parametrs).mid_ticks_between_pulses = (((*glob_parametrs).max_ticks_between_pulses + (*glob_parametrs).min_ticks_between_pulses) >> 1);
      (*glob_parametrs).real_time_between_pulses = ((*glob_parametrs).mid_ticks_between_pulses * TICK_PWM);//real_time_between_pulses * 1 000 000
      (*glob_parametrs).real_rev_time = 2 * (*glob_parametrs).real_time_between_pulses;//real_rev_time
      (*glob_parametrs).rev_per_sec = 1000000/(*glob_parametrs).real_rev_time;//RPS = 1sec/real_rev_time		
    } else {
      (*glob_parametrs).rev_per_sec = 0;
    }
    (*glob_parametrs).rev_per_min = (*glob_parametrs).rev_per_sec * 60;//RPM = RPS * 60
    static uint16_t counter = 0;
    counter++;
    if (counter >= (*glob_parametrs).output_period && (*glob_parametrs).mon_fl) {
      char buff_to_send[BUFF_SIZE];
      sprintf((char*)buff_to_send, "rev: %d RPM\r\n", (*glob_parametrs).rev_per_min);
      DebagUart(buff_to_send);
      counter = 0;
    }
    (*glob_parametrs).max_ticks_between_pulses = 0;
    (*glob_parametrs).min_ticks_between_pulses = MAX_VALUE;
    (*glob_parametrs).flag_rev = 0;
  }
}
