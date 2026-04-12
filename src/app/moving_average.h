#pragma once
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Gleitender Mittelwert-Filter
 *
 * Speichert die letzten N Messwerte und gibt deren Durchschnitt zurück.
 * Nützlich um verrauschte Sensor-Signale zu glätten.
 *
 * @tparam T     Datentyp der Messwerte (z.B. uint16_t, float)
 * @tparam N     Fenstergröße (Anzahl der gespeicherten Werte)
 *
 * Beispiel:
 *   MovingAverage<uint16_t, 8> filter;
 *   filter.addSample(100);
 *   filter.addSample(120);
 *   float avg = filter.getAverage(); // 110.0
 */
template<typename T, size_t N>
class MovingAverage {
    static_assert(N > 0, "Fenstergröße muss > 0 sein");

public:
    /**
     * @brief Fügt einen neuen Messwert hinzu
     * Ältester Wert wird überschrieben (Ringpuffer-Prinzip)
     */
    void addSample(T value) {
        buffer_[index_] = value;
        index_ = (index_ + 1) % N;   // Ringpuffer: nach N zurück zu 0
        if (count_ < N) count_++;     // Zähle bis N, dann bleibt es N
    }

    /**
     * @brief Gibt den Durchschnitt der bisherigen Messwerte zurück
     */
    float getAverage() const {
        if (count_ == 0) return 0.0f;
        T sum = 0;
        for (size_t i = 0; i < count_; i++) {
            sum += buffer_[i];
        }
        return static_cast<float>(sum) / static_cast<float>(count_);
    }

    /** @brief Anzahl der bisher hinzugefügten Samples (max N) */
    size_t getCount() const { return count_; }

    /** @brief Setzt den Filter zurück */
    void reset() {
        for (size_t i = 0; i < N; i++) buffer_[i] = 0;
        index_ = 0;
        count_ = 0;
    }

private:
    T      buffer_[N]{};
    size_t index_{0};
    size_t count_{0};
};