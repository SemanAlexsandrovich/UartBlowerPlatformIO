//uart.cpp

#include <Arduino.h>
#include "glob.h"
#include "uart.h"
#include "pwm.h"

#define DATA_PERIOD_LIMIT 60

volatile extern struct global_variables *glob_parametrs;

const char power[] = 
"\r\nwrite the command \"power\" as follows:\r\n\
\"power (power value)\"\r\n\
(power value) = [0;100]\r\n\
For example:\r\n\
\"power 100\" - turn on the fan at full power\r\n\
\"power 0\" - turn off fan\r\n";

const char mon[] = 
"\r\nTo configure the monitoring parameters, write the command \"non\" as follows:\r\n\
\"mon (period)\"\r\n\
(period) - period in seconds with which data will be received on the serial monitor\r\n\
For example:\r\n\
\"mon 5\" - output data every 5 seconds\r\n\
\"mon 0\" - turn off monitoring\r\n";

int cmd_parser (char * cmd_buff) {
	const char *cmd_list[] = {	
		"power",
		"mon"
	};
	for (int i=0; i < BUFF_SIZE; i++) {
		if (cmd_buff[i] == '\n' || cmd_buff[i] == '\r' || cmd_buff[i] == ' ') {
			cmd_buff[i] = '\0';
			break;
		}
	}
	for (uint8_t i = 0; i < (sizeof(cmd_list)/sizeof(cmd_list[0])); i++) {
		if (!strcmp(cmd_buff, cmd_list[i])) {
			return i;
		}
	}
	return -1;
}

void pwm_parser(char * buff) {
	char pwm_param[BUFF_SIZE];
	int j = 0;
	for (int i=6; i < BUFF_SIZE; i++) {
		if (buff[i] == '\n' || buff[i] == '\r') {
			pwm_param[j] = '\0';
			break;
		}
		pwm_param[j] = buff[i];
		j++;
	}
	char sep_sym[]=" ";
	char *pwm_choose = strtok (pwm_param,sep_sym);
	if (pwm_choose != NULL) {
		uint16_t pwm = atoi(pwm_choose);
		if (pwm >= 0 && pwm <= 100) {
			(*glob_parametrs).pwm = pwm;
			setPwmDuty((*glob_parametrs).pwm);
			char buff_to_send[BUFF_SIZE];
			sprintf((char*)buff_to_send, "[power: %d%%]\r\n", (*glob_parametrs).pwm);
			DebagUart(buff_to_send);
		} else {
			DebagUart("wrong value\r\n");
		}
	} else {
		DebagUart(power);
	}
}

void mon_parser(char * buff) {
	char mon_param[BUFF_SIZE];
	int j = 0;
	for (int i=4; i < BUFF_SIZE; i++) {
		if (buff[i] == '\n' || buff[i] == '\r') {
			mon_param[j] = '\0';
			break;
		}
		mon_param[j] = buff[i];
		j++;
	}
	char sep_sym[]=" ";
	char *period_choose = strtok (mon_param,sep_sym);
	if (period_choose != NULL) {
		uint16_t period_data = atoi(period_choose);
		if (period_data > 0 && period_data <= DATA_PERIOD_LIMIT) {
			(*glob_parametrs).output_period = period_data;
			DebagUart("start monitoring\r\n");
			(*glob_parametrs).mon_fl = ON;
		} else {
			DebagUart("stop monitoring\r\n");
			(*glob_parametrs).mon_fl = OFF;
			(*glob_parametrs).output_period = 0;
		}
	} else {
		DebagUart(mon);
	}
}

void UartInit(void) {
	 Serial.begin(115200);
	 Serial.setTimeout(0);
}

void DebagUart(const char * data) {
	Serial.print(data);
}
