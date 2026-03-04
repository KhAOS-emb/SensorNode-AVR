#pragma once
#include <avr/io.h>
#include <cstdint>

/**
 * @brief Hardware Abstraction Layer für den ADC (Analog-Digital-Wandler)
 *
 * Der ATmega328P hat einen 10-Bit ADC: Wandelt Spannungen 0-5V
 * in Werte 0-1023 um.
 *
 * Pins: A0=PC0, A1=PC1, A2=PC2, A3=PC3, A4=PC4, A5=PC5
 */
class ADC_HAL {
public:
    /**
     * @brief ADC initialisieren — muss einmal am Anfang aufgerufen werden
     * @param prescaler Taktteiler: 128 ergibt ADC-Takt von 125kHz bei 16MHz
     *                  (Empfohlen: ADPS2|ADPS1|ADPS0 für Prescaler 128)
     */
    static void init() {
        // Referenzspannung: AVCC (5V) = REFS0 setzen
        ADMUX = (1 << REFS0);

        // ADC aktivieren, Prescaler auf 128 setzen
        // Prescaler 128: 16MHz / 128 = 125kHz ADC-Takt (optimal: 50-200kHz)
        ADCSRA = (1 << ADEN)   // ADC Enable
               | (1 << ADPS2)  // Prescaler Bit 2
               | (1 << ADPS1)  // Prescaler Bit 1
               | (1 << ADPS0); // Prescaler Bit 0
    }

    /**
     * @brief Liest einen ADC-Kanal (blockierend)
     * @param channel ADC-Kanal 0-7 (0=A0, 1=A1, ...)
     * @return        10-Bit Wert (0-1023)
     */
    static uint16_t read(uint8_t channel) {
        // Kanal wählen (nur untere 3 Bits von channel nutzen)
        ADMUX = (ADMUX & 0xF8) | (channel & 0x07);

        // Konvertierung starten
        ADCSRA |= (1 << ADSC);

        // Warten bis Konvertierung fertig (ADSC geht auf 0)
        while (ADCSRA & (1 << ADSC)) { /* warten */ }

        // ADC-Wert lesen (ADCL muss VOR ADCH gelesen werden!)
        uint16_t result = ADCL;
        result |= (ADCH << 8);
        return result;
    }

    /**
     * @brief Konvertiert ADC-Wert in Spannung
     * @param adcValue 10-Bit ADC-Wert (0-1023)
     * @param vRef     Referenzspannung in Volt (typisch 5.0f)
     * @return         Spannung in Volt
     */
    static float toVoltage(uint16_t adcValue, float vRef = 5.0f) {
        return (adcValue / 1023.0f) * vRef;
    }
};