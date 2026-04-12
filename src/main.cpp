#include <Arduino.h>
#include <avr/interrupt.h>

// HAL
#include "hal/i2c_hal.h"
#include "hal/gpio_hal.h"
#include "hal/adc_hal.h"
#include "hal/uart_hal.h"
#include "hal/pwm_hal.h"

// Treiber
#include "drivers/dht11_driver.h"
#include "drivers/ssd1306_driver.h"
#include "drivers/relay_driver.h"
#include "drivers/buzzer_driver.h"

// Applikation
#include "app/alarm_manager.h"
#include "app/moving_average.h"

// ─── Pin-Definitionen ──────────────────────────────
// (Passe diese an deine tatsächliche Verkabelung an!)
constexpr uint8_t DHT11_BIT  = PD4;   // Digital Pin 4
constexpr uint8_t PIR_BIT    = PD3;   // Digital Pin 3 (INT1)
constexpr uint8_t RELAY1_BIT = PD5;   // Digital Pin 5
constexpr uint8_t RELAY2_BIT = PD6;   // Digital Pin 6
constexpr uint8_t BUZZER_BIT = PB1;   // Digital Pin 9 (OC1A/PWM)
constexpr uint8_t BUTTON1_BIT = PD7;  // Digital Pin 7
constexpr uint8_t LED_RED_BIT = PB0;  // Digital Pin 8
constexpr uint8_t ADC_LIGHT  = 0;     // A0
constexpr uint8_t ADC_POT    = 1;     // A1

// ─── PIR Interrupt ────────────────────────────────
volatile bool g_motionDetected = false;

// Simple PIR wrapper für IMotionSensor Interface
class PIRSensor : public IMotionSensor {
public:
    bool isMotionDetected() override {
        bool val = g_motionDetected;
        g_motionDetected = false;  // Flag zurücksetzen nach Lesen
        return val;
    }
};

// Lichtsensor wrapper
class LightSensor : public ILightSensor {
public:
    uint16_t getLightLevel() override {
        return ADC_HAL::read(ADC_LIGHT);
    }
};

// ─── Globale Objekte ──────────────────────────────
DHT11Driver  dht11(&DDRD, &PORTD, &PIND, DHT11_BIT);
SSD1306Driver oled;
RelayDriver  relay1(PORTD, DDRD, RELAY1_BIT, false);  // Active-LOW
RelayDriver  relay2(PORTD, DDRD, RELAY2_BIT, false);
BuzzerDriver buzzer;
PIRSensor    pir;
LightSensor  light;

AlarmManager alarmMgr(
    &dht11,   // ITemperatureSensor
    &dht11,   // IHumiditySensor (DHT11 implementiert beide!)
    &pir,     // IMotionSensor
    &relay1,  // ISwitch
    &buzzer,  // IBuzzer
    AlarmManager::Config(30.0f, 5.0f, 80.0f, 2000)
);

MovingAverage<uint16_t, 8> lightFilter;

// ─── ISR (Interrupt Service Routinen) ─────────────
ISR(INT1_vect) {
    g_motionDetected = true;  // PIR: Bewegung erkannt!
}

// ─── Setup ────────────────────────────────────────
void setup() {
    // Initialisierung in der richtigen Reihenfolge!
    UART_HAL::init(115200);
    I2C_HAL::init();
    ADC_HAL::init();

    // PIR Interrupt konfigurieren (INT1, steigende Flanke)
    GPIO_HAL::setDirection(DDRD, PIR_BIT, GPIO_HAL::Direction::PIN_INPUT);
    EICRA |= (1 << ISC11) | (1 << ISC10);  // Steigende Flanke
    EIMSK |= (1 << INT1);                  // INT1 aktivieren
    sei();  // Globale Interrupts an

    // Button mit Pull-Up
    GPIO_HAL::setDirection(DDRD, BUTTON1_BIT, GPIO_HAL::Direction::PIN_INPUT);
    GPIO_HAL::setPullUp(PORTD, BUTTON1_BIT, GPIO_HAL::Pull::PULL_UP);

    // LED als Ausgang
    GPIO_HAL::setDirection(DDRB, LED_RED_BIT, GPIO_HAL::Direction::PIN_OUTPUT);

    // OLED initialisieren
    if (!oled.init()) {
        UART_HAL::sendLine("FEHLER: OLED nicht gefunden!");
    }

    buzzer.init();

    UART_HAL::sendLine("=== SensorNode-AVR gestartet ===");

    // Kurzer Startup-Ton
    buzzer.beep(440, 100);
}

// ─── Hauptschleife ────────────────────────────────
void loop() {
    static uint32_t lastSensorRead = 0;
    static uint32_t lastDisplay    = 0;
    static bool     lastButtonState = true;  // Pull-Up → HIGH wenn nicht gedrückt

    uint32_t now = millis();

    // Sensoren lesen (max. 1x pro Sekunde wegen DHT11)
    if (now - lastSensorRead >= 2000) {
        lastSensorRead = now;

        if (dht11.update()) {
            UART_HAL::sendString("Temp: ");
            UART_HAL::sendInt(static_cast<int>(dht11.getTemperature()));
            UART_HAL::sendString("C  Hum: ");
            UART_HAL::sendInt(static_cast<int>(dht11.getHumidity()));
            UART_HAL::sendLine("%");
        } else {
            UART_HAL::sendLine("DHT11: Lesefehler!");
        }

        // Licht mit Filter
        lightFilter.addSample(ADC_HAL::read(ADC_LIGHT));
    }

    // Alarm-Logik aktualisieren
    alarmMgr.update(now);

    // Status-LED: Blinkt im Alarm
    if (alarmMgr.isAlarming()) {
        GPIO_HAL::write(PORTB, LED_RED_BIT, (now / 250) % 2 == 0);
    } else {
        GPIO_HAL::write(PORTB, LED_RED_BIT, false);
    }

    // Button-Handling mit Debouncing
    bool buttonNow = GPIO_HAL::read(PIND, BUTTON1_BIT);
    if (!buttonNow && lastButtonState) {  // Fallende Flanke = gedrückt
        alarmMgr.acknowledge();
        UART_HAL::sendLine("Alarm quittiert.");
    }
    lastButtonState = buttonNow;

    // Display aktualisieren (100ms Refresh)
    if (now - lastDisplay >= 100) {
        lastDisplay = now;

        oled.clear();

        char buf[20];
        // Zeile 1: Temperatur
        snprintf(buf, sizeof(buf), "Temp: %dC",
                 dht11.isValid() ? static_cast<int>(dht11.getTemperature()) : 0);
        oled.print(0, 0, buf);

        // Zeile 2: Luftfeuchtigkeit
        snprintf(buf, sizeof(buf), "Hum:  %dC%%",
                 dht11.isValid() ? static_cast<int>(dht11.getHumidity()) : 0);
        oled.print(0, 1, buf);

        // Zeile 3: Licht
        snprintf(buf, sizeof(buf), "Licht: %d",
                 static_cast<int>(lightFilter.getAverage()));
        oled.print(0, 2, buf);

        // Zeile 4: Status
        oled.print(0, 3, alarmMgr.isAlarming() ? "!! ALARM !!" : "OK");

        oled.update();
    }
}