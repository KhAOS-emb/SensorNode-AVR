#pragma once
#include "interfaces.h"
#include "../hal/pwm_hal.h"
#include <util/delay.h>

/** @brief Treiber für passiven Buzzer via PWM (Timer1) */
class BuzzerDriver : public IBuzzer {
public:
    void init() {
        PWM_HAL::initTimer1CTC(440);  // Mit 440Hz initialisieren
        PWM_HAL::disableOutput();     // Noch stumm
    }

    void beep(uint16_t frequencyHz, uint16_t durationMs) override {
        PWM_HAL::setFrequency(frequencyHz);
        PWM_HAL::enableOutput();
        // Blockierende Wartezeit (für einfache Töne OK)
        // In FreeRTOS-Projekt: vTaskDelay nutzen!
        for (uint16_t i = 0; i < durationMs; i++) {
            _delay_ms(1);
        }
        PWM_HAL::disableOutput();
    }

    void stop() override { PWM_HAL::disableOutput(); }

    /** @brief Spielt eine Alarmsequenz */
    void alarmSequence() {
        for (uint8_t i = 0; i < 3; i++) {
            beep(880, 150);   // A5
            _delay_ms(50);
            beep(660, 150);   // E5
            _delay_ms(50);
        }
    }
};