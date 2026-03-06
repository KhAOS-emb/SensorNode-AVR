#pragma once
#include "interfaces.h"
#include <avr/io.h>
#include <util/delay.h>

/**
 * @brief DHT11 Temperatur- und Feuchtigkeitssensor Treiber
 *
 * Implementiert das DHT11 Single-Wire Protokoll:
 * 1. Host sendet Start-Signal (LOW 18ms, HIGH 20-40µs)
 * 2. DHT11 antwortet (LOW 80µs, HIGH 80µs)
 * 3. DHT11 sendet 40 Datenbits
 *
 * @note DHT11 hat eine minimale Abtastrate von 1 Messung/Sekunde!
 * Häufigere Aufrufe liefern gecachte Werte.
 */
class DHT11Driver : public ITemperatureSensor, public IHumiditySensor {
public:
    /**
     * @brief Konstruktor
     * @param ddr   Zeiger auf DDR-Register des Pins (z.B. &DDRD)
     * @param port  Zeiger auf PORT-Register (z.B. &PORTD)
     * @param pin   Zeiger auf PIN-Register (z.B. &PIND)
     * @param bit   Bit-Nummer (z.B. PD4 = 4)
     */
    DHT11Driver(volatile uint8_t* ddr,
                volatile uint8_t* port,
                volatile uint8_t* pin,
                uint8_t bit)
        : ddr_(ddr), port_(port), pin_(pin), bit_(bit) {}

    /**
     * @brief Messung vom DHT11 anfordern und lesen
     * @return true bei Erfolg, false bei Kommunikationsfehler
     */
    bool update() override {
        if (!sendStartSignal()) return false;
        if (!waitForResponse()) return false;

        uint8_t data[5] = {0};
        for (uint8_t i = 0; i < 5; i++) {
            data[i] = readByte();
        }

        // Prüfsumme validieren
        uint8_t checksum = data[0] + data[1] + data[2] + data[3];
        if (checksum != data[4]) {
            return false;  // Übertragungsfehler
        }

        // Werte speichern
        humidity_ = static_cast<float>(data[0]) + data[1] / 10.0f;
        temperature_ = static_cast<float>(data[2]) + data[3] / 10.0f;
        lastReadOk_ = true;
        return true;
    }

    float getTemperature() override { return temperature_; }
    float getHumidity() override { return humidity_; }
    bool isValid() const { return lastReadOk_; }

private:
    volatile uint8_t* ddr_;
    volatile uint8_t* port_;
    volatile uint8_t* pin_;
    uint8_t bit_;
    float temperature_{0.0f};
    float humidity_{0.0f};
    bool lastReadOk_{false};

    void setPinOutput() { *ddr_ |= (1 << bit_); }
    void setPinInput()  { *ddr_ &= ~(1 << bit_); }
    void setPinHigh()   { *port_ |= (1 << bit_); }
    void setPinLow()    { *port_ &= ~(1 << bit_); }
    bool readPin()      { return (*pin_ & (1 << bit_)) != 0; }

    bool sendStartSignal() {
        setPinOutput();
        setPinLow();
        _delay_ms(18);     // Mind. 18ms LOW
        setPinHigh();
        _delay_us(30);     // 20-40µs HIGH
        setPinInput();
        return true;
    }

    bool waitForResponse() {
        // Warte auf LOW (DHT11 Antwort)
        uint8_t timeout = 100;
        while (readPin() && timeout--) { _delay_us(1); }
        if (!timeout) return false;

        // Warte auf HIGH
        timeout = 100;
        while (!readPin() && timeout--) { _delay_us(1); }
        if (!timeout) return false;

        // Warte bis HIGH endet (Response fertig)
        timeout = 100;
        while (readPin() && timeout--) { _delay_us(1); }
        return timeout > 0;
    }

    uint8_t readByte() {
        uint8_t byte = 0;
        for (uint8_t i = 0; i < 8; i++) {
            // Warte auf LOW-zu-HIGH Flanke (Start eines Bits)
            while (!readPin()) {}

            _delay_us(40);  // Nach 40µs lesen: <40µs = "0", >40µs = "1"
            byte <<= 1;
            if (readPin()) byte |= 0x01;

            // Warte bis HIGH endet
            while (readPin()) {}
        }
        return byte;
    }
};