#include <gtest/gtest.h>
#include "app/moving_average.h"

// Test-Suite: Alle Tests für MovingAverage
// Syntax: TEST(TestSuiteName, TestName)

TEST(MovingAverage, StartsWithZeroCount) {
    MovingAverage<uint16_t, 4> filter;
    EXPECT_EQ(filter.getCount(), 0u);
}

TEST(MovingAverage, AverageOfOneSample) {
    MovingAverage<uint16_t, 4> filter;
    filter.addSample(100);
    EXPECT_FLOAT_EQ(filter.getAverage(), 100.0f);
}

TEST(MovingAverage, AverageOfThreeSamples) {
    MovingAverage<uint16_t, 4> filter;
    filter.addSample(100);
    filter.addSample(200);
    filter.addSample(300);
    // Durchschnitt: (100+200+300)/3 = 200
    EXPECT_FLOAT_EQ(filter.getAverage(), 200.0f);
}

TEST(MovingAverage, OverwritesOldestWhenFull) {
    MovingAverage<uint16_t, 3> filter;  // Fenstergröße 3
    filter.addSample(100);
    filter.addSample(200);
    filter.addSample(300);
    filter.addSample(400);  // Überschreibt 100 (ältester Wert)
    // Jetzt: 200, 300, 400 → Durchschnitt = 300
    EXPECT_FLOAT_EQ(filter.getAverage(), 300.0f);
    EXPECT_EQ(filter.getCount(), 3u);
}

TEST(MovingAverage, ResetClearsAllValues) {
    MovingAverage<uint16_t, 4> filter;
    filter.addSample(500);
    filter.addSample(600);
    filter.reset();
    EXPECT_EQ(filter.getCount(), 0u);
    EXPECT_FLOAT_EQ(filter.getAverage(), 0.0f);
}

TEST(MovingAverage, WorksWithFloatType) {
    MovingAverage<float, 2> filter;
    filter.addSample(1.5f);
    filter.addSample(2.5f);
    EXPECT_NEAR(filter.getAverage(), 2.0f, 0.001f);
}