#pragma once
#include <avr/io.h>
#include <cstdint>

/**
 * @brief PWM HAL für Timer1 (16-Bit) im CTC Mode
 *
 * CTC = Clear Timer on Compare Match
 * Erzeugt präzise Frequenzen auf OC1A (Pin 9) und OC1B (Pin 10)
 *
 * Nutzung für Buzzer:
 *   PWM_HAL::initTimer1CTC(440);  // Kammerton A4 = 440Hz
 *   PWM_HAL::enableOutput();
 *   delay_ms(500);
 *   PWM_HAL::disableOutput();     // Stille
 */
class PWM_HAL {
public:
    /**
     * @brief Timer1 im CTC-Modus initialisieren
     * @param frequencyHz Gewünschte Frequenz in Hz
     *
     * Formel: OCR1A = (F_CPU / (2 * Prescaler * Frequenz)) - 1
     * Bei 16MHz, Prescaler 8, 440Hz: OCR1A = (16000000 / (2*8*440)) - 1 = 2271
     */
    static void initTimer1CTC(uint16_t frequencyHz) {
        // CTC Modus: WGM12 setzen, Prescaler 8: CS11
        TCCR1A = 0;
        TCCR1B = (1 << WGM12) | (1 << CS11);  // CTC, Prescaler 8

        // Compare-Wert berechnen
        OCR1A = (F_CPU / (2UL * 8UL * frequencyHz)) - 1;

        TCNT1 = 0;  // Timer auf 0 zurücksetzen
    }

    /** @brief PWM-Ausgang auf Pin 9 aktivieren (OC1A) */
    static void enableOutput() {
        // Toggle OC1A bei Compare Match
        TCCR1A |= (1 << COM1A0);
        DDRB |= (1 << PB1);  // Pin 9 als Ausgang
    }

    /** @brief PWM-Ausgang deaktivieren (Stille) */
    static void disableOutput() {
        TCCR1A &= ~(1 << COM1A0);
        PORTB &= ~(1 << PB1);
    }

    /** @brief Frequenz ändern ohne Timer neu zu initialisieren */
    static void setFrequency(uint16_t frequencyHz) {
        OCR1A = (F_CPU / (2UL * 8UL * frequencyHz)) - 1;
    }

    /** @brief Timer stoppen */
    static void stop() {
        TCCR1B = 0;
        disableOutput();
    }
};