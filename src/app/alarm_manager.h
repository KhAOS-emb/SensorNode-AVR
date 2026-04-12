#pragma once
#include "../drivers/interfaces.h"
#include "moving_average.h"
#ifdef __AVR__
    #include <stdint.h>
#else
    #include <cstdint>
#endif

/**
 * @brief Verwaltet Alarm-Zustände basierend auf Sensor-Schwellwerten
 *
 * Implementiert einen Zustandsautomat:
 *
 *  NORMAL ──(überschritten)──► ALARM_PENDING ──(bestätigt)──► ALARM_ACTIVE
 *    ▲                              │                               │
 *    └──────(unter Schwelle)────────┘                               │
 *    └──────(manuell quittiert)─────────────────────────────────────┘
 *
 * "Pending" verhindert Fehlalarme durch kurze Spitzen (Debouncing für Sensoren!)
 */
class AlarmManager {
public:
    enum class State { NORMAL, ALARM_PENDING, ALARM_ACTIVE };

    struct Config {
        float tempHighThreshold  = 30.0f;  // °C
        float tempLowThreshold   = 5.0f;   // °C
        float humidHighThreshold = 80.0f;  // %
        uint16_t confirmMs       = 2000;   // 2 Sekunden bestätigen

        // Expliziter Konstruktor für AVR:
        Config() {}
        Config(float th, float tl, float hh, uint16_t cm)
        : tempHighThreshold(th), tempLowThreshold(tl),
          humidHighThreshold(hh), confirmMs(cm) {}
    };

    AlarmManager(ITemperatureSensor* tempSensor,
                 IHumiditySensor*    humSensor,
                 IMotionSensor*      motionSensor,
                 ISwitch*            relay,
                 IBuzzer*            buzzer,
                 Config              config = {})
        : tempSensor_(tempSensor), humSensor_(humSensor),
          motionSensor_(motionSensor), relay_(relay),
          buzzer_(buzzer), config_(config) {}

    /**
     * @brief Muss regelmäßig in der Hauptschleife aufgerufen werden
     * @param nowMs Aktuelle Systemzeit in Millisekunden
     */
    void update(uint32_t nowMs) {
        // Sensor-Werte lesen
        float temp = tempSensor_->getTemperature();
        float hum  = humSensor_->getHumidity();
        bool motion = motionSensor_->isMotionDetected();

        // Bedingung prüfen
        bool conditionMet = (temp > config_.tempHighThreshold) ||
                            (temp < config_.tempLowThreshold)  ||
                            (hum  > config_.humidHighThreshold)||
                            motion;

        // Zustandsautomat
        switch (state_) {
            case State::NORMAL:
                if (conditionMet) {
                    state_ = State::ALARM_PENDING;
                    pendingStartMs_ = nowMs;
                }
                break;

            case State::ALARM_PENDING:
                if (!conditionMet) {
                    state_ = State::NORMAL;  // Kurze Spitze — kein echter Alarm
                } else if (nowMs - pendingStartMs_ >= config_.confirmMs) {
                    state_ = State::ALARM_ACTIVE;
                    activateAlarm();
                }
                break;

            case State::ALARM_ACTIVE:
                if (!conditionMet) {
                    state_ = State::NORMAL;
                    deactivateAlarm();
                }
                break;
        }
    }

    /** @brief Alarm manuell quittieren (z.B. durch Button-Druck) */
    void acknowledge() {
        if (state_ == State::ALARM_ACTIVE) {
            state_ = State::NORMAL;
            deactivateAlarm();
        }
    }

    State getState() const { return state_; }
    bool  isAlarming() const { return state_ == State::ALARM_ACTIVE; }

private:
    ITemperatureSensor* tempSensor_;
    IHumiditySensor*    humSensor_;
    IMotionSensor*      motionSensor_;
    ISwitch*            relay_;
    IBuzzer*            buzzer_;
    Config              config_;

    State    state_{State::NORMAL};
    uint32_t pendingStartMs_{0};

    void activateAlarm() {
        relay_->setState(true);
        buzzer_->beep(880, 200);
    }

    void deactivateAlarm() {
        relay_->setState(false);
        buzzer_->stop();
    }
};