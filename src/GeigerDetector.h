#pragma once

#include <Arduino.h>
#include <limits.h>

#include "CppExternalInterrupt.h"

///////////////////////////////////////////////

class GeigerDetector {
public:
    typedef void (*CalibrationCallback)(unsigned int pulsesCount, unsigned long pulsesDuration);
protected:
    volatile bool          changed{false};
    volatile unsigned long counter{};
    volatile unsigned int  lastPulseDuration{};

    volatile unsigned long errorsCounter{};

    static constexpr int   DEFAULT_PULSE_DURATION = 300;
    volatile unsigned long longPulsesCounter{DEFAULT_PULSE_DURATION};

    static constexpr int  DEFAULT_DEAD_TIME = 50;
    volatile unsigned int deadTime{DEFAULT_DEAD_TIME};

    static constexpr int  DEFAULT_MAX_PULSE_TIME = 5000;
    volatile unsigned int maxPulseDuration{DEFAULT_MAX_PULSE_TIME};

    static constexpr int  DEFAULT_MEDIUM_PULSE_TIME = 5000;
    volatile unsigned int mediumPulseDuration{DEFAULT_MEDIUM_PULSE_TIME};

    void classifyPulseLength(unsigned long pulseDuration);

    void presetDefaultCalibration();
    bool defaultCalibrate(unsigned int maxPulses, unsigned long maxTime, CalibrationCallback callback);

    bool active{false};

public:
    virtual ~GeigerDetector() = default;

    bool          isChanged() const;
    unsigned long getCounter();
    unsigned int  getLastPulseDuration() const;
    unsigned long getErrorCounter() const;
    unsigned long getLongPulsesCounter() const;
    virtual void  clearErrors();
    unsigned int  getDeadTime() const;
    unsigned int  getMaxPulseDuration() const;
    unsigned int  getMediumPulseDuration() const;

    bool         isActive() const;
    virtual bool begin();
    virtual void end();

    virtual bool calibrate(unsigned int maxPulses = 4, unsigned long maxTime = ULONG_MAX, CalibrationCallback callback = nullptr);
};

///////////////////////////////////////////////

class GeigerDetectorSimple;

class GeigerDetectorSimpleInterrupt : public CppExternalInterrupt {
    GeigerDetectorSimple *owner;

public:
    explicit GeigerDetectorSimpleInterrupt(GeigerDetectorSimple *owner);
    ~GeigerDetectorSimpleInterrupt();

    void isrHandler() override;
};

///////////////////////////////////////////////

class GeigerDetectorSimple : public GeigerDetector {
    friend class GeigerDetectorSimpleInterrupt;

    GeigerDetectorSimpleInterrupt interruptPtr;
    virtual void                  fallingIrq();

protected:
    uint8_t pinNumber = 255;

public:
    GeigerDetectorSimple();
    explicit GeigerDetectorSimple(uint8_t pinNumber);
    ~GeigerDetectorSimple();

    bool calibrate(unsigned int maxPulses = 4, unsigned long maxTime = ULONG_MAX, CalibrationCallback callback = nullptr) override;
    bool begin() override;
    void end() override;
};

///////////////////////////////////////////////

class GeigerDetectorTiming : public GeigerDetectorSimple {
    unsigned long lastIsrMicros{};

    using GeigerDetectorSimple::GeigerDetectorSimple;

    void fallingIrq() override;
public:
    bool calibrate(unsigned int maxPulses = 4, unsigned long maxTime = ULONG_MAX, CalibrationCallback callback = nullptr) override;

};

///////////////////////////////////////////////

class GeigerDetectorDoubleIrq;

class GeigerDetectorDoubleIrqFallingInterrupt : public CppExternalInterrupt {
    GeigerDetectorDoubleIrq *owner;

public:
    explicit GeigerDetectorDoubleIrqFallingInterrupt(GeigerDetectorDoubleIrq *owner);
    ~GeigerDetectorDoubleIrqFallingInterrupt();

    void isrHandler() override;
};

///////////////////////////////////////////////

class GeigerDetectorDoubleIrq;

class GeigerDetectorDoubleIrqRisingInterrupt final : public CppExternalInterrupt {
    GeigerDetectorDoubleIrq *owner;

public:
    explicit GeigerDetectorDoubleIrqRisingInterrupt(GeigerDetectorDoubleIrq *owner);
    ~GeigerDetectorDoubleIrqRisingInterrupt();

    void isrHandler() override;
};

///////////////////////////////////////////////

class GeigerDetectorDoubleIrq : public GeigerDetector {
    friend class GeigerDetectorDoubleIrqFallingInterrupt;
    friend class GeigerDetectorDoubleIrqRisingInterrupt;

    GeigerDetectorDoubleIrqFallingInterrupt fallingInterruptPtr;
    GeigerDetectorDoubleIrqRisingInterrupt  risingInterruptPtr;

    unsigned long lastIsrMicros{};

protected:
    uint8_t pinNumberFalling = 255;
    uint8_t pinNumberRising  = 255;

public:
    GeigerDetectorDoubleIrq();
    explicit GeigerDetectorDoubleIrq(uint8_t pinNumberFalling, uint8_t pinNumberRising);
    ~GeigerDetectorDoubleIrq();

    bool begin() override;
    void end() override;

private:
    virtual void fallingIrq();
    virtual void risingIrq();
};

///////////////////////////////////////////////
