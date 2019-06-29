#include <Arduino.h>
#include <util/atomic.h>

#include "CppExternalInterrupt.h"

#include "GeigerDetector.h"

///////////////////////////////////////////////

bool GeigerDetector::isChanged() const {
    return changed;
}

bool GeigerDetector::begin() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        clearErrors();
        changed = false;
        counter = 0;
    }
    return true;
}

void GeigerDetector::end() {
}

unsigned long GeigerDetector::getCounter() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        const auto result = counter;
        counter           = 0;
        changed           = false;
        return result;
    }
}

unsigned int GeigerDetector::getLastPulseDuration() const {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        return lastPulseDuration;
    }
}

unsigned long GeigerDetector::getErrorCounter() const {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        return errorsCounter;
    }
}

unsigned long GeigerDetector::getLongPulsesCounter() const {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        return longPulsesCounter;
    }
}

void GeigerDetector::clearErrors() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        errorsCounter     = 0;
        longPulsesCounter = 0;
    }
}

bool GeigerDetector::isActive() const {
    return active;
}

unsigned int GeigerDetector::getDeadTime() const {
    return deadTime;
}

unsigned int GeigerDetector::getMaxPulseDuration() const {
    return maxPulseDuration;
}

unsigned int GeigerDetector::getMediumPulseDuration() const {
    return mediumPulseDuration;
}

void GeigerDetector::classifyPulseLength(const unsigned long pulseDuration) {

    lastPulseDuration = pulseDuration;

    if (pulseDuration < deadTime || pulseDuration > maxPulseDuration) {
        ++errorsCounter;
        return;
    }

    changed = true;

    if (pulseDuration > mediumPulseDuration * 3 / 2) {
        ++longPulsesCounter;
        counter = 2;
        return;
    }

    ++counter;
}

void GeigerDetector::presetDefaultCalibration() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        deadTime            = DEFAULT_DEAD_TIME;
        maxPulseDuration    = DEFAULT_MAX_PULSE_TIME;
        mediumPulseDuration = DEFAULT_MEDIUM_PULSE_TIME;
    }
}

bool GeigerDetector::defaultCalibrate(const unsigned int maxPulses, const unsigned long maxTime, const CalibrationCallback callback) {
    const auto startTime = millis();
    presetDefaultCalibration();
    const auto wasActive = isActive();
    auto       result    = false;
    if (begin()) {
        auto pulsesCount{decltype(maxPulses){0}};
        auto pulsesDuration{0UL};
        while (pulsesCount < maxPulses && millis() - startTime < maxTime) {
            delay(10);
            if (isChanged()) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    pulsesCount += getCounter();
                    pulsesDuration += getLastPulseDuration();
                }
                if (callback != nullptr) {
                    callback(pulsesCount, pulsesDuration);
                }
            }
        }
        end();
        if (pulsesCount != 0) {
            result              = true;
            mediumPulseDuration = pulsesDuration / pulsesCount;
            maxPulseDuration    = mediumPulseDuration * 2;
            deadTime            = mediumPulseDuration / 4;
        }
    }
    if (wasActive) {
        begin();
    }
    return result;
}

bool GeigerDetector::calibrate(const unsigned int maxPulses, const unsigned long maxTime, const CalibrationCallback callback) {
    return defaultCalibrate(maxPulses, maxTime, callback);
}

///////////////////////////////////////////////

GeigerDetectorSimpleInterrupt::GeigerDetectorSimpleInterrupt(GeigerDetectorSimple *owner) : owner(owner) {
}

GeigerDetectorSimpleInterrupt::~GeigerDetectorSimpleInterrupt() {
    end();
}

void GeigerDetectorSimpleInterrupt::isrHandler() {
    owner->fallingIrq();
}

///////////////////////////////////////////////

bool GeigerDetectorSimple::begin() {
    GeigerDetector::begin();
    return interruptPtr.begin(pinNumber, FALLING);
}

GeigerDetectorSimple::GeigerDetectorSimple() : interruptPtr(this) {
}

void GeigerDetectorSimple::end() {
    interruptPtr.end();
    GeigerDetector::end();
}

GeigerDetectorSimple::GeigerDetectorSimple(const uint8_t pinNumber) : GeigerDetectorSimple() {
    this->pinNumber = pinNumber;
}

GeigerDetectorSimple::~GeigerDetectorSimple() {
    GeigerDetectorSimple::end();
}

bool GeigerDetectorSimple::calibrate(unsigned, unsigned long, CalibrationCallback) {
    return true;
}

void GeigerDetectorSimple::fallingIrq() {
    changed = true;
    ++counter;
    lastPulseDuration = DEFAULT_PULSE_DURATION;
}

///////////////////////////////////////////////

void GeigerDetectorTiming::fallingIrq() {
    const auto pulseBegin = micros();
    lastIsrMicros         = pulseBegin;
    while (digitalRead(pinNumber) == 0) {
        delayMicroseconds(5);
    }
    classifyPulseLength(micros() - pulseBegin);
}

bool GeigerDetectorTiming::calibrate(const unsigned maxPulses, const unsigned long maxTime, const CalibrationCallback callback) {
    return defaultCalibrate(maxPulses, maxTime, callback);
}

///////////////////////////////////////////////

GeigerDetectorDoubleIrqFallingInterrupt::GeigerDetectorDoubleIrqFallingInterrupt(GeigerDetectorDoubleIrq *owner) : owner(owner) {
}

GeigerDetectorDoubleIrqFallingInterrupt::~GeigerDetectorDoubleIrqFallingInterrupt() {
    end();
}

void GeigerDetectorDoubleIrqFallingInterrupt::isrHandler() {
    owner->fallingIrq();
}

///////////////////////////////////////////////

GeigerDetectorDoubleIrqRisingInterrupt::GeigerDetectorDoubleIrqRisingInterrupt(GeigerDetectorDoubleIrq *owner) : owner(owner) {
}

GeigerDetectorDoubleIrqRisingInterrupt::~GeigerDetectorDoubleIrqRisingInterrupt() {
    end();
}

void GeigerDetectorDoubleIrqRisingInterrupt::isrHandler() {
    owner->risingIrq();
}

///////////////////////////////////////////////

bool GeigerDetectorDoubleIrq::begin() {
    GeigerDetector::begin();
    const auto result =
        fallingInterruptPtr.begin(pinNumberFalling, FALLING) &&
        risingInterruptPtr.begin(pinNumberRising, RISING);
    if (!result) {
        end();
    }
    return result;
}

GeigerDetectorDoubleIrq::GeigerDetectorDoubleIrq() : fallingInterruptPtr(this), risingInterruptPtr(this) {
}

GeigerDetectorDoubleIrq::GeigerDetectorDoubleIrq(const uint8_t pinNumberFalling, const uint8_t pinNumberRising) : GeigerDetectorDoubleIrq() {
    this->pinNumberFalling = pinNumberFalling;
    this->pinNumberRising  = pinNumberRising;
}

void GeigerDetectorDoubleIrq::end() {
    fallingInterruptPtr.end();
    risingInterruptPtr.end();
    GeigerDetector::end();
}

GeigerDetectorDoubleIrq::~GeigerDetectorDoubleIrq() {
    GeigerDetectorDoubleIrq::end();
}

void GeigerDetectorDoubleIrq::fallingIrq() {
    lastIsrMicros = micros();
}

void GeigerDetectorDoubleIrq::risingIrq() {
    classifyPulseLength(micros() - lastIsrMicros);
}

///////////////////////////////////////////////
