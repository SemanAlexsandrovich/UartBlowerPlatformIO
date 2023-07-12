//glob.h
#ifndef GLOB_H_
#define GLOB_H_

//#include <avr/io.h>
#include <Arduino.h>

enum status {
	OFF,
	ON
};

struct global_variables {
  uint8_t mon_fl;
  uint16_t output_period;
  uint8_t pwm;
  uint16_t time_fixation;
  uint8_t flag_rev;
  uint16_t analys_increment;
  uint8_t pulse_capture_flag;
  uint32_t max_ticks_between_pulses;
  uint16_t rev_per_min;
  uint16_t rev_per_sec;
  uint32_t mid_ticks_between_pulses;
  uint32_t real_time_between_pulses;
  uint32_t real_rev_time;
  uint32_t min_ticks_between_pulses;
};


#endif /* GLOB_H_ */