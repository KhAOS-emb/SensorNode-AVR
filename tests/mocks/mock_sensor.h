#pragma once
#include "drivers/interfaces.h"

/**
 * @brief Gefälschter Temperatur-Sensor für Unit Tests
 *
 * Kein echtes Bauteil nötig! Du kannst den zurückgegebenen Wert
 * beliebig setzen und so verschiedene Szenarien testen.
 */
class MockTemperatureSensor : public ITemperatureSensor {
public:
    bool update() override { return updateResult_; }
    float getTemperature() override { return fakeTemperature_; }

    // Test-Hilfsmethoden:
    void setTemperature(float t) { fakeTemperature_ = t; }
    void setUpdateResult(bool ok) { updateResult_ = ok; }
    int getUpdateCallCount() const { return updateCalls_; }

private:
    float fakeTemperature_{20.0f};
    bool updateResult_{true};
    int updateCalls_{0};
};

class MockRelay : public ISwitch {
public:
    void setState(bool on) override { state_ = on; callCount_++; }
    bool getState() const override { return state_; }
    void toggle() override { state_ = !state_; }
    int getCallCount() const { return callCount_; }

private:
    bool state_{false};
    int callCount_{0};
};

class MockBuzzer : public IBuzzer {
public:
    void beep(uint16_t freq, uint16_t dur) override {
        lastFreq_ = freq; lastDur_ = dur; beepCount_++;
    }
    void stop() override { stopCount_++; }
    uint16_t getLastFrequency() const { return lastFreq_; }
    int getBeepCount() const { return beepCount_; }

private:
    uint16_t lastFreq_{0}, lastDur_{0};
    int beepCount_{0}, stopCount_{0};
};