//uart.cpp

#include <Arduino.h>
#include "uart.h"

void UartInit(void) {
	 Serial.begin(115200);
	 Serial.setTimeout(0);
}

void DebagUart(const char * data) {
	Serial.print(data);
}
