#include <gtest/gtest.h>
#include "app/alarm_manager.h"
#include "tests/mocks/mock_sensor.h"

class AlarmManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<AlarmManager>(
            &tempSensor, &humSensor, &motionSensor,
            &relay, &buzzer,
            AlarmManager::Config(30.0f, 5.0f, 80.0f, 2000)
        );
    }

    MockTemperatureSensor tempSensor;
    MockHumiditySensor    humSensor;
    MockMotionSensor      motionSensor;
    MockRelay             relay;
    MockBuzzer            buzzer;
    std::unique_ptr<AlarmManager> manager;
};

TEST_F(AlarmManagerTest, StartsInNormalState) {
    EXPECT_EQ(manager->getState(), AlarmManager::State::NORMAL);
    EXPECT_FALSE(manager->isAlarming());
}

TEST_F(AlarmManagerTest, TransitionsToPendingWhenThresholdExceeded) {
    tempSensor.setTemperature(35.0f);  // Über Schwelle (30°C)
    manager->update(0);
    EXPECT_EQ(manager->getState(), AlarmManager::State::ALARM_PENDING);
    EXPECT_FALSE(relay.getState());    // Relay noch aus!
}

TEST_F(AlarmManagerTest, DoesNotAlarmOnBriefSpike) {
    tempSensor.setTemperature(35.0f);
    manager->update(0);       // Condition true → PENDING
    tempSensor.setTemperature(20.0f);
    manager->update(100);     // Condition false nach 100ms → zurück zu NORMAL
    EXPECT_EQ(manager->getState(), AlarmManager::State::NORMAL);
}

TEST_F(AlarmManagerTest, ActivatesAlarmAfterConfirmTime) {
    tempSensor.setTemperature(35.0f);
    manager->update(0);       // → PENDING
    manager->update(1000);    // Noch PENDING (1000ms < 2000ms)
    EXPECT_EQ(manager->getState(), AlarmManager::State::ALARM_PENDING);
    manager->update(2001);    // → ALARM_ACTIVE!
    EXPECT_EQ(manager->getState(), AlarmManager::State::ALARM_ACTIVE);
    EXPECT_TRUE(relay.getState());   // Relay ist jetzt AN
}

TEST_F(AlarmManagerTest, AcknowledgeResetsAlarm) {
    tempSensor.setTemperature(35.0f);
    manager->update(0);
    manager->update(2001);   // → ALARM_ACTIVE
    manager->acknowledge();   // Quittieren
    EXPECT_EQ(manager->getState(), AlarmManager::State::NORMAL);
    EXPECT_FALSE(relay.getState());  // Relay aus
}