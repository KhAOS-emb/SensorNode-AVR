#include "uart_hal.h"

/**
 * @brief Initialisiert den UART mit der angegebenen Baudrate
 * @param baudrate Übertragungsrate in Baud (z.B. 115200)
 */
void UART_HAL::init(uint32_t baudrate) {
    Serial.begin(baudrate);
}

/**
 * @brief Sendet ein einzelnes Zeichen über UART
 * @param c Das zu sendende Zeichen
 */
void UART_HAL::sendChar(char c) {
    Serial.write(c);
}

/**
 * @brief Sendet einen nullterminierten String über UART
 * @param str Zeiger auf den zu sendenden String
 */
void UART_HAL::sendString(const char* str) {
    while (*str) {
        sendChar(*str++);
    }
}

/**
 * @brief Sendet einen String mit anschließendem Zeilenumbruch (CRLF)
 * @param str Zeiger auf den zu sendenden String
 */
void UART_HAL::sendLine(const char* str) {
    sendString(str);
    sendChar('\r');
    sendChar('\n');
}

/**
 * @brief Sendet einen Integer-Wert als Dezimalzahl über UART
 * @param value Der zu sendende Wert
 */
void UART_HAL::sendInt(int32_t value) {
    Serial.print(value);
}

/**
 * @brief Prüft ob Daten im Empfangspuffer verfügbar sind
 * @return true wenn mindestens ein Byte empfangen wurde
 */
bool UART_HAL::dataAvailable() {
    return Serial.available() > 0;
}

/**
 * @brief Liest ein Zeichen aus dem Empfangspuffer
 * @return Das gelesene Zeichen, oder -1 wenn kein Zeichen verfügbar
 * @note Vor dem Aufruf mit dataAvailable() prüfen
 */
char UART_HAL::readChar() {
    return static_cast<char>(Serial.read());
}

/**
 * @brief ISR-Handler für UART-Empfang
 * @note Wird vom Arduino Framework intern gehandelt (HardwareSerial)
 */
void UART_HAL::rxInterruptHandler() {
}