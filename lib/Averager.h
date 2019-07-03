#pragma once

#include <stdint.h>

template <typename T>
class Averager {
public:
    Averager() : samplesIncluded(0) { this->accValue = T(); }
    void addValue(const T& value) { this->accValue += value; ++this->samplesIncluded; }
    const T& getAccumulatorValue() const { return this->accValue; }
    T const getAverage() {
        if (this->samplesIncluded == 0) {
            // Don't divide by zero
            return this->accValue;
        }

        return this->accValue / static_cast<float>(this->samplesIncluded);
    }
    void reset() { this->accValue = T(); this->samplesIncluded = 0; }

private:
    // The number of samples included in the accumulator value
    size_t samplesIncluded;
    // The accumulated sum of samples
    T accValue;
};
