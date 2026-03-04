#include "uart_hal.h"
#include <stdlib.h>  // für itoa

// Statische Member-Variablen initialisieren
volatile uint8_t UART_HAL::rxBuffer_[UART_HAL::RX_BUFFER_SIZE] = {};
volatile uint8_t UART_HAL::rxHead_ = 0;
volatile uint8_t UART_HAL::rxTail_ = 0;

// ISR für empfangenes UART-Byte
// Wird automatisch aufgerufen wenn ein Byte ankommt
ISR(USART_RX_vect) {
    UART_HAL::rxInterruptHandler();
}

void UART_HAL::init(uint32_t baudrate) {
    // UBRR berechnen
    uint16_t ubrr = (F_CPU / (16UL * baudrate)) - 1;

    // Baudrate setzen (High und Low Byte)
    UBRR0H = static_cast<uint8_t>(ubrr >> 8);
    UBRR0L = static_cast<uint8_t>(ubrr);

    // Empfang und Senden aktivieren + Empfangs-Interrupt aktivieren
    UCSR0B = (1 << RXEN0)    // Empfänger aktivieren
           | (1 << TXEN0)    // Sender aktivieren
           | (1 << RXCIE0);  // RX Complete Interrupt aktivieren

    // Format: 8 Datenbits, kein Paritätsbit, 1 Stoppbit (8N1 — Standard)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    sei();  // Globale Interrupts aktivieren (notwendig für ISR!)
}

void UART_HAL::rxInterruptHandler() {
    uint8_t byte = UDR0;  // Byte aus Hardware-Register lesen (MUSS sofort passieren!)

    uint8_t nextHead = (rxHead_ + 1) % RX_BUFFER_SIZE;

    if (nextHead != rxTail_) {  // Puffer noch nicht voll?
        rxBuffer_[rxHead_] = byte;
        rxHead_ = nextHead;
    }
    // Wenn Puffer voll: Byte wird verworfen (kein Überlauf)
}

void UART_HAL::sendChar(char c) {
    // Warte bis Sende-Puffer frei
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = static_cast<uint8_t>(c);
}

void UART_HAL::sendString(std::string_view str) {
    for (char c : str) {
        sendChar(c);
    }
}

void UART_HAL::sendLine(std::string_view str) {
    sendString(str);
    sendChar('\r');
    sendChar('\n');
}

void UART_HAL::sendInt(int32_t value) {
    char buf[12];
    itoa(value, buf, 10);
    sendString(buf);
}

bool UART_HAL::dataAvailable() {
    return rxHead_ != rxTail_;
}

char UART_HAL::readChar() {
    if (!dataAvailable()) return '\0';

    char c = static_cast<char>(rxBuffer_[rxTail_]);
    rxTail_ = (rxTail_ + 1) % RX_BUFFER_SIZE;
    return c;
}