#pragma once
#include "interfaces.h"
#include "../hal/gpio_hal.h"

/**
 * @brief Treiber für das 2-Kanal 5V Relay Modul
 *
 * WICHTIG: Das Relay-Modul ist Active-LOW!
 * LOW-Signal am Steuerpin → Relay zieht an (Kontakt geschlossen)
 * HIGH-Signal → Relay fällt ab (Kontakt offen)
 * activeHigh=false im Konstruktor berücksichtigt das automatisch.
 */
class RelayDriver : public ISwitch {
public:
    /**
     * @param port      PORT-Register des Steuerpins
     * @param ddr       DDR-Register
     * @param pin       Pin-Nummer
     * @param activeHigh false = Active-LOW Relay (typisch für dein Modul)
     */
    RelayDriver(volatile uint8_t& port,
                volatile uint8_t& ddr,
                uint8_t pin,
                bool activeHigh = false)
        : port_(port), pin_(pin), activeHigh_(activeHigh) {
        GPIO_HAL::setDirection(ddr, pin, GPIO_HAL::Direction::OUTPUT);
        setState(false);  // Sicher: Relay aus beim Start
    }

    void setState(bool on) override {
        state_ = on;
        // Bei Active-LOW: on=true → Signal LOW, on=false → Signal HIGH
        bool pinLevel = activeHigh_ ? on : !on;
        GPIO_HAL::write(port_, pin_, pinLevel);
    }

    bool getState() const override { return state_; }

    void toggle() override { setState(!state_); }

private:
    volatile uint8_t& port_;
    uint8_t pin_;
    bool activeHigh_;
    bool state_{false};
};