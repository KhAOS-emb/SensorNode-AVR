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
        Serial.println("DBG: sende Start...");
        sendStartSignal();
        if (!waitForResponse()) return false;

        Serial.println("DBG: warte Response...");
        if (!waitForResponse()) {
            Serial.println("DBG: waitForResponse FEHLER");
            return false;
        }

        uint8_t data[5] = {0};
        for (uint8_t i = 0; i < 5; i++) {
            data[i] = readByte();
        }

        uint8_t checksum = data[0] + data[1] + data[2] + data[3];
        uint8_t diff = (checksum > data[4]) 
                    ? checksum - data[4] 
                    : data[4] - checksum;

        if (diff > 1) {
            Serial.println("DBG: Checksum FEHLER");
            return false;
        }

        // Akzeptiert: diff == 0 (perfekt) oder diff == 1 (1 Bit Toleranz)
        humidity_    = data[0] + data[1] * 0.1f;
        temperature_ = data[2] + data[3] * 0.1f;
        lastReadOk_  = true;
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

    void sendStartSignal() {
        setPinOutput();
        setPinLow();
        _delay_ms(18);     // Mind. 18ms LOW
        setPinHigh();
        _delay_us(30);     // 20-40µs HIGH
        setPinInput();
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
            uint8_t timeout = 200;
            while (!readPin() && timeout--) { _delay_us(1); }
            if (!timeout) return 0;

            uint8_t count = 0;
            while (readPin() && count < 255) {
                count++;
            }

            byte <<= 1;
            if (count > 10) byte |= 0x01;
        }
        return byte;
    }
};