#include "uart_hal.h"

void UART_HAL::init(uint32_t baudrate) {
    Serial.begin(baudrate);
}

void UART_HAL::sendChar(char c) {
    Serial.write(c);
}

void UART_HAL::sendString(const char* str) {
    while (*str) {
        sendChar(*str++);
    }
}

void UART_HAL::sendLine(const char* str) {
    sendString(str);
    sendChar('\r');
    sendChar('\n');
}

void UART_HAL::sendInt(int32_t value) {
    Serial.print(value);
}

bool UART_HAL::dataAvailable() {
    return Serial.available() > 0;
}

char UART_HAL::readChar() {
    return static_cast<char>(Serial.read());
}

void UART_HAL::rxInterruptHandler() {
}