#pragma once
#include <avr/io.h>
#include <stdint.h>

/**
 * @brief Hardware Abstraction Layer für GPIO (General Purpose I/O)
 *
 * Kapselt den direkten Register-Zugriff für digitale Ein-/Ausgangspins.
 *
 * Beispiel:
 *   GPIO_HAL::setDirection(DDRB, PB5, GPIO_HAL::Direction::OUTPUT);
 *   GPIO_HAL::write(PORTB, PB5, true);
 */
class GPIO_HAL {
public:
    enum class Direction { INPUT, OUTPUT };
    enum class Pull { NONE, PULLUP };

    /**
     * @brief Setzt die Richtung eines Pins (Eingang oder Ausgang)
     * @param ddr    Data Direction Register (z.B. DDRB, DDRC, DDRD)
     * @param pin    Pin-Nummer im Register (z.B. PB5, PC0, PD2)
     * @param dir    OUTPUT oder INPUT
     */
    static void setDirection(volatile uint8_t& ddr,
                             uint8_t pin,
                             Direction dir) {
        if (dir == Direction::OUTPUT) {
            ddr |= (1 << pin);   // Bit setzen → Ausgang
        } else {
            ddr &= ~(1 << pin);  // Bit löschen → Eingang
        }
    }

    /**
     * @brief Aktiviert Pull-Up Widerstand für Eingangs-Pin
     * Wichtig für Buttons (verhindert floating/undefinierter Zustand)
     */
    static void setPullUp(volatile uint8_t& port,
                          uint8_t pin,
                          Pull pull) {
        if (pull == Pull::PULLUP) {
            port |= (1 << pin);
        } else {
            port &= ~(1 << pin);
        }
    }

    /**
     * @brief Setzt einen Ausgangs-Pin HIGH oder LOW
     * @param port   Port-Register (z.B. PORTB, PORTC, PORTD)
     * @param pin    Pin-Nummer
     * @param value  true = HIGH, false = LOW
     */
    static void write(volatile uint8_t& port,
                      uint8_t pin,
                      bool value) {
        if (value) {
            port |= (1 << pin);
        } else {
            port &= ~(1 << pin);
        }
    }

    /**
     * @brief Liest den aktuellen Zustand eines Eingangs-Pins
     * @param pins   PIN-Register (z.B. PINB, PINC, PIND)
     * @param pin    Pin-Nummer
     * @return       true = HIGH, false = LOW
     */
    static bool read(volatile uint8_t& pins, uint8_t pin) {
        return (pins & (1 << pin)) != 0;
    }

    /**
     * @brief Invertiert den Zustand eines Ausgangs-Pins
     */
    static void toggle(volatile uint8_t& port, uint8_t pin) {
        port ^= (1 << pin);  // XOR mit Maske togglet das Bit
    }
};