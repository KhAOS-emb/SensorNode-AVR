#pragma once
#include "interfaces.h"
#include "../hal/i2c_hal.h"
#include <string.h>  // für memset

/**
 * @brief SSD1306 OLED Display Treiber (128x64 Pixel, I²C)
 *
 * Nutzt einen Frame-Buffer: Du zeichnest in den Puffer,
 * dann wird er mit update() auf das Display übertragen.
 *
 * I²C Adresse: 0x3C (typisch) oder 0x3D
 */
class SSD1306Driver : public IDisplay {
public:
    static constexpr uint8_t WIDTH  = 128;
    static constexpr uint8_t HEIGHT = 64;
    static constexpr uint8_t PAGES  = HEIGHT / 8;  // 8 Seiten à 8 Pixel
    static constexpr uint8_t I2C_ADDRESS = 0x3C;

    SSD1306Driver() = default;

    bool init() {
        // Initialisierungssequenz für SSD1306
        const uint8_t initCmds[] = {
            0xAE,        // Display OFF
            0xD5, 0x80,  // Display Clock Divide Ratio
            0xA8, 0x3F,  // Multiplex Ratio (64 Zeilen)
            0xD3, 0x00,  // Display Offset
            0x40,        // Start Line = 0
            0x8D, 0x14,  // Charge Pump ON
            0x20, 0x00,  // Horizontal Addressing Mode
            0xA1,        // Segment Remap
            0xC8,        // COM Output Scan Direction
            0xDA, 0x12,  // COM Pins Configuration
            0x81, 0xCF,  // Contrast
            0xD9, 0xF1,  // Pre-charge Period
            0xDB, 0x40,  // VCOMH Deselect Level
            0xA4,        // Display: Output folgt RAM
            0xA6,        // Normal Display (nicht invertiert)
            0xAF         // Display ON
        };

        for (uint8_t cmd : initCmds) {
            sendCommand(cmd);
        }

        clear();
        update();
        return true;
    }

    void clear() override {
        memset(buffer_, 0x00, sizeof(buffer_));
    }

    /** @brief Einfache 5x7 Zeichenausgabe an Position (col, row) */
    void print(uint8_t col, uint8_t row, const char* text) override {
        // Vereinfachte Implementierung: nutzt vordefinierten Font
        // In der vollständigen Implementierung: 5x7 oder 8x8 Font-Array
        while (*text) {
            drawChar(col, row, *text++);
            col += 6;  // 5 Pixel Breite + 1 Pixel Abstand
            if (col >= WIDTH) break;
        }
    }

    void update() override {
        // Cursor-Position auf 0,0 setzen
        sendCommand(0x21); sendCommand(0); sendCommand(127);  // Spalten 0-127
        sendCommand(0x22); sendCommand(0); sendCommand(7);    // Seiten 0-7

        // Frame-Buffer in einem I²C Transfer senden
        // SSD1306 erwartet: Control-Byte 0x40, dann Daten
        // Wir senden in Blöcken (I²C Buffer-Limit beachten)
        for (uint8_t page = 0; page < PAGES; page++) {
            uint8_t packet[17];  // 1 Control-Byte + 16 Daten-Bytes
            packet[0] = 0x40;    // Data Control Byte

            for (uint8_t chunk = 0; chunk < (WIDTH / 16); chunk++) {
                for (uint8_t i = 0; i < 16; i++) {
                    packet[1 + i] = buffer_[page * WIDTH + chunk * 16 + i];
                }
                I2C_HAL::writeBuffer(I2C_ADDRESS, packet, 17);
            }
        }
    }

    /** @brief Setzt/löscht einen einzelnen Pixel */
    void setPixel(uint8_t x, uint8_t y, bool on = true) {
        if (x >= WIDTH || y >= HEIGHT) return;
        uint8_t page = y / 8;
        uint8_t bit  = y % 8;
        if (on) {
            buffer_[page * WIDTH + x] |= (1 << bit);
        } else {
            buffer_[page * WIDTH + x] &= ~(1 << bit);
        }
    }

private:
    uint8_t buffer_[PAGES * WIDTH]{};  // Frame-Buffer: 8 Pages × 128 Pixel

    void sendCommand(uint8_t cmd) {
        uint8_t packet[2] = {0x00, cmd};  // 0x00 = Command Control Byte
        I2C_HAL::writeBuffer(I2C_ADDRESS, packet, 2);
    }

    // Minimaler 5x7 Font für Buchstaben (nur A-Z, 0-9 für den Anfang)
    // Vollständige Implementierung: Font-Array mit allen ASCII-Zeichen
    void drawChar(uint8_t col, uint8_t page, char c) {
        // Platzhalter — vollständige Font-Tabelle in separater font.h Datei
        (void)col; (void)page; (void)c;
        // TODO: Font-Array implementieren (gut als eigene Aufgabe!)
    }
};