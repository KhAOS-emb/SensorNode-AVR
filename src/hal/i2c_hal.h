#pragma once
#include <avr/io.h>
#include <util/twi.h>
#include <cstdint>

/**
 * @brief Hardware Abstraction Layer für I²C (TWI) Master-Kommunikation
 *
 * Kommuniziert direkt mit dem TWI-Hardware-Modul des ATmega328P.
 * Kein Wire.h — direkte Register-Programmierung.
 *
 * SDA = A4 (PC4), SCL = A5 (PC5) beim ATmega328P / Arduino Uno
 */
class I2C_HAL {
public:
    enum class Result {
        OK,
        ERROR_ARBITRATION_LOST,
        ERROR_NACK_ADDRESS,
        ERROR_NACK_DATA,
        ERROR_TIMEOUT
    };

    /**
     * @brief I²C Bus initialisieren
     * @param frequencyHz Taktfrequenz in Hz (Standard: 100000 = 100kHz)
     *
     * Formel für TWBR: TWBR = (F_CPU / frequencyHz - 16) / 2
     * Bei 16MHz und 100kHz: TWBR = (16000000/100000 - 16) / 2 = 72
     */
    static void init(uint32_t frequencyHz = 100000UL) {
        // Prescaler auf 1 setzen (TWSR Bits 0-1 = 00)
        TWSR = 0x00;
        // Bitrate berechnen und setzen
        TWBR = static_cast<uint8_t>((F_CPU / frequencyHz - 16) / 2);
        // TWI aktivieren
        TWCR = (1 << TWEN);
    }

    /**
     * @brief START-Condition senden
     * Signalisiert dem Bus: "Ich, der Master, übernehme jetzt"
     */
    static Result start() {
        TWCR = (1 << TWINT)   // Interrupt-Flag löschen (startet Aktion)
             | (1 << TWSTA)   // START senden
             | (1 << TWEN);   // TWI aktiviert lassen

        waitForComplete();

        uint8_t status = TWSR & 0xF8;  // Status-Bits auslesen (Maske ignoriert Prescaler)
        if (status != TW_START && status != TW_REP_START) {
            return Result::ERROR_TIMEOUT;
        }
        return Result::OK;
    }

    /**
     * @brief Slave-Adresse + Richtung senden
     * @param address  7-Bit I²C Adresse (z.B. 0x3C für SSD1306)
     * @param write    true = Schreiben, false = Lesen
     */
    static Result sendAddress(uint8_t address, bool write) {
        // Adresse in TWDR schreiben: [7bit Adresse][R/W Bit]
        TWDR = (address << 1) | (write ? TW_WRITE : TW_READ);
        TWCR = (1 << TWINT) | (1 << TWEN);

        waitForComplete();

        uint8_t status = TWSR & 0xF8;
        if (write && status != TW_MT_SLA_ACK) {
            return Result::ERROR_NACK_ADDRESS;
        }
        if (!write && status != TW_MR_SLA_ACK) {
            return Result::ERROR_NACK_ADDRESS;
        }
        return Result::OK;
    }

    /**
     * @brief Ein Datenbyte senden (Master Transmit)
     */
    static Result writeByte(uint8_t data) {
        TWDR = data;
        TWCR = (1 << TWINT) | (1 << TWEN);
        waitForComplete();

        if ((TWSR & 0xF8) != TW_MT_DATA_ACK) {
            return Result::ERROR_NACK_DATA;
        }
        return Result::OK;
    }

    /**
     * @brief Ein Datenbyte empfangen (Master Receive)
     * @param ack  true = ACK senden (weitere Bytes folgen), false = NACK (letztes Byte)
     */
    static uint8_t readByte(bool ack = true) {
        TWCR = (1 << TWINT)
             | (1 << TWEN)
             | (ack ? (1 << TWEA) : 0);  // TWEA = ACK senden
        waitForComplete();
        return TWDR;
    }

    /**
     * @brief STOP-Condition senden — gibt den Bus frei
     */
    static void stop() {
        TWCR = (1 << TWINT)
             | (1 << TWSTO)  // STOP senden
             | (1 << TWEN);
        // Warte bis STOP ausgeführt (TWSTO geht auf 0)
        while (TWCR & (1 << TWSTO)) {}
    }

    /**
     * @brief Komfort-Funktion: Mehrere Bytes in einem Rutsch schreiben
     * @param address  I²C Slave-Adresse
     * @param data     Zeiger auf Datenpuffer
     * @param length   Anzahl der Bytes
     */
    static Result writeBuffer(uint8_t address,
                               const uint8_t* data,
                               uint8_t length) {
        auto res = start();
        if (res != Result::OK) { stop(); return res; }

        res = sendAddress(address, true);
        if (res != Result::OK) { stop(); return res; }

        for (uint8_t i = 0; i < length; i++) {
            res = writeByte(data[i]);
            if (res != Result::OK) { stop(); return res; }
        }

        stop();
        return Result::OK;
    }

private:
    static void waitForComplete() {
        // Warte bis TWINT (Interrupt-Flag) gesetzt wird
        // Das bedeutet: Operation abgeschlossen
        while (!(TWCR & (1 << TWINT))) {}
    }
};