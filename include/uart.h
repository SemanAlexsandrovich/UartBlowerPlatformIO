//uart.h

#ifndef UART_H_
#define UART_H_

#define BUFF_SIZE 32

enum commands {
    MS_POWER,
    MS_MON
};

void mon_parser(char * buff);
void pwm_parser(char * buff);
int cmd_parser (char * cmd_buff);
void UartInit(void);
void DebagUart(const char*);

#endif /* UART_H_ */
