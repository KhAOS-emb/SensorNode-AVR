#pragma once
#include <avr/io.h>
#include <avr/interrupt.h>
#include <cstdint>
#include <string_view>

/**
 * @brief UART HAL mit Interrupt-getriebenem Empfang und Sende-Puffer
 *
 * Nutzt Ringpuffer für Empfang (RX) — kein verlorenes Byte bei langsamer
 * Verarbeitung. Senden ist blockierend (einfacher und für unsere Zwecke OK).
 *
 * TX = PD1, RX = PD0 beim ATmega328P
 */
class UART_HAL {
public:
    static constexpr uint8_t RX_BUFFER_SIZE = 64;  // Ringpuffer-Größe

    /**
     * @brief UART initialisieren
     * @param baudrate Übertragungsrate (z.B. 115200)
     *
     * UBRR-Formel: UBRR = (F_CPU / (16 * baudrate)) - 1
     */
    static void init(uint32_t baudrate);

    /** @brief Sendet ein einzelnes Zeichen (blockiert bis Sendepuffer frei) */
    static void sendChar(char c);

    /** @brief Sendet einen String */
    static void sendString(std::string_view str);

    /** @brief Sendet Integer als Dezimalzahl */
    static void sendInt(int32_t value);

    /** @brief Sendet Zahl + Zeilenumbruch */
    static void sendLine(std::string_view str);

    /** @brief Prüft ob Zeichen im Empfangspuffer verfügbar */
    static bool dataAvailable();

    /** @brief Liest ein Zeichen aus dem Empfangspuffer (non-blocking!) */
    static char readChar();

    /** @brief ISR-Handler — wird vom USART_RX_vect Interrupt aufgerufen */
    static void rxInterruptHandler();

private:
    // Ringpuffer für Empfang
    static volatile uint8_t rxBuffer_[RX_BUFFER_SIZE];
    static volatile uint8_t rxHead_;   // Schreib-Index (ISR schreibt hier)
    static volatile uint8_t rxTail_;   // Lese-Index (Hauptprogramm liest hier)
};