#pragma once
#include <stdint.h>

// ─── Sensor Interfaces ─────────────────────────────────────

/** Interface für Sensoren mit Temperatur-Messung */
class ITemperatureSensor {
public:
    virtual bool update() = 0;          // Messung auslösen, gibt true bei Erfolg
    virtual float getTemperature() = 0; // Letzte Temperatur in °C
    virtual ~ITemperatureSensor() = default;
};

/** Interface für Sensoren mit Feuchtigkeits-Messung */
class IHumiditySensor {
public:
    virtual float getHumidity() = 0;    // Relative Luftfeuchtigkeit in %
    virtual ~IHumiditySensor() = default;
};

/** Interface für Bewegungssensoren */
class IMotionSensor {
public:
    virtual bool isMotionDetected() = 0;
    virtual ~IMotionSensor() = default;
};

/** Interface für Helligkeitssensoren */
class ILightSensor {
public:
    virtual uint16_t getLightLevel() = 0;  // 0-1023 (ADC-Wert)
    virtual ~ILightSensor() = default;
};

// ─── Ausgabe Interfaces ────────────────────────────────────

/** Interface für Display-Ausgabe */
class IDisplay {
public:
    virtual void clear() = 0;
    virtual void print(uint8_t col, uint8_t row,
                       const char* text) = 0;
    virtual void update() = 0;  // Puffer auf Display übertragen
    virtual ~IDisplay() = default;
};

/** Interface für schaltbare Ausgänge (Relays, LEDs) */
class ISwitch {
public:
    virtual void setState(bool on) = 0;
    virtual bool getState() const = 0;
    virtual void toggle() = 0;
    virtual ~ISwitch() = default;
};

/** Interface für Buzzer-Ausgabe */
class IBuzzer {
public:
    virtual void beep(uint16_t frequencyHz, uint16_t durationMs) = 0;
    virtual void stop() = 0;
    virtual ~IBuzzer() = default;
};