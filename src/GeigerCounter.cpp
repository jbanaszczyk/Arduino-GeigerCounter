#include <Arduino.h>
#include <util/atomic.h>

#include "CppExternalInterrupt.h"

#include "GeigerCounter.h"

///////////////////////////////////////////////

bool GeigerCounter::isChanged() const {
    return changed;
}

unsigned long GeigerCounter::getCounter() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        const auto result = counter;
        counter           = 0;
        changed           = false;
        return result;
    }
}

unsigned int GeigerCounter::getLastPulseDuration() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        return lastPulseDuration;
    }
}

unsigned long GeigerCounter::getErrorCounter() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        return errorsCounter;
    }
}

unsigned long GeigerCounter::getLongPulsesCounter() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        return longPulsesCounter;
    }
}

///////////////////////////////////////////////

GeigerCounterSimpleInterrupt::GeigerCounterSimpleInterrupt(GeigerCounterSimple *owner) : owner(owner) {
}

GeigerCounterSimpleInterrupt::~GeigerCounterSimpleInterrupt() {
    end();
}

void GeigerCounterSimpleInterrupt::isrHandler() {
    owner->fallingIrq();
}

///////////////////////////////////////////////

bool GeigerCounterSimple::begin() {
    return interruptPtr.begin(pinNumber, FALLING);
}

GeigerCounterSimple::GeigerCounterSimple() : interruptPtr(this) {
}

void GeigerCounterSimple::end() {
    interruptPtr.end();
}

GeigerCounterSimple::GeigerCounterSimple(const uint8_t pinNumber) : GeigerCounterSimple() {
    this->pinNumber = pinNumber;
#if defined GEIGER_COUNTER_DYNAMIC
    GeigerCounterSimple::begin();
#endif
}

GeigerCounterSimple::~GeigerCounterSimple() {
#if defined GEIGER_COUNTER_DYNAMIC
    GeigerCounterSimple::end();
#endif
}

void GeigerCounterSimple::fallingIrq() {
    changed = true;
    ++counter;
    lastPulseDuration = DEFAULT_PULSE_DURATION;
}

///////////////////////////////////////////////

void GeigerCounterTiming::fallingIrq() {
    const auto pulseBegin = micros();
    lastIsrMicros         = pulseBegin;
    while (digitalRead(pinNumber) == 0) {
        delayMicroseconds(5);
    }
    lastPulseDuration = micros() - pulseBegin;

    changed = true;

    auto result = 1;
    if (lastPulseDuration < deadTime || lastPulseDuration > maxPulseTime) {
        ++errorsCounter;
        result = 0;
    }

    if (lastPulseDuration > mediumPulseTime_OneAndAHalf) {
        ++longPulsesCounter;
        result = 2;
    }
    counter += result;
}

///////////////////////////////////////////////

GeigerCounterDoubleIrqFallingInterrupt::GeigerCounterDoubleIrqFallingInterrupt(GeigerCounterDoubleIrq *owner) : owner(owner) {
}

GeigerCounterDoubleIrqFallingInterrupt::~GeigerCounterDoubleIrqFallingInterrupt() {
    end();
}

void GeigerCounterDoubleIrqFallingInterrupt::isrHandler() {
    owner->fallingIrq();
}

///////////////////////////////////////////////

GeigerCounterDoubleIrqRisingInterrupt::GeigerCounterDoubleIrqRisingInterrupt(GeigerCounterDoubleIrq *owner) : owner(owner) {
}

GeigerCounterDoubleIrqRisingInterrupt::~GeigerCounterDoubleIrqRisingInterrupt() {
    end();
}

void GeigerCounterDoubleIrqRisingInterrupt::isrHandler() {
    owner->risingIrq();
}

///////////////////////////////////////////////

bool GeigerCounterDoubleIrq::begin() {
    const auto result =
        fallingInterruptPtr.begin(pinNumberFalling, FALLING) &&
        risingInterruptPtr.begin(pinNumberRising, RISING);
    if (!result) {
        end();
    }
    return result;
}

GeigerCounterDoubleIrq::GeigerCounterDoubleIrq() : fallingInterruptPtr(this), risingInterruptPtr(this) {
}

void GeigerCounterDoubleIrq::end() {
    fallingInterruptPtr.end();
    risingInterruptPtr.end();
}

GeigerCounterDoubleIrq::GeigerCounterDoubleIrq(const uint8_t pinNumberFalling, const uint8_t pinNumberRising) : GeigerCounterDoubleIrq() {
    this->pinNumberFalling = pinNumberFalling;
    this->pinNumberRising  = pinNumberRising;
#if defined GEIGER_COUNTER_DYNAMIC
    GeigerCounterDoubleIrq::begin();
#endif
}

GeigerCounterDoubleIrq::~GeigerCounterDoubleIrq() {
#if defined GEIGER_COUNTER_DYNAMIC
    GeigerCounterDoubleIrq::end();
#endif
}

void GeigerCounterDoubleIrq::fallingIrq() {
    lastIsrMicros = micros();
}

void GeigerCounterDoubleIrq::risingIrq() {
    const auto pulseEnd = micros();
    lastPulseDuration = pulseEnd - lastIsrMicros;

    changed             = true;

    auto result = 1;
    if (lastPulseDuration < deadTime || lastPulseDuration > maxPulseTime) {
        ++errorsCounter;
        result = 0;
    }

    if (lastPulseDuration > mediumPulseTime_OneAndAHalf) {
        ++longPulsesCounter;
        result = 2;
    }
    counter += result;
}

///////////////////////////////////////////////
