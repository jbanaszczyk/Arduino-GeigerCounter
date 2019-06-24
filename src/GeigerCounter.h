#pragma once

#include <Arduino.h>

#include "CppExternalInterrupt.h"

#define GEIGER_COUNTER_DYNAMIC

///////////////////////////////////////////////

class GeigerCounter {
protected:
    volatile bool          changed{false};
    volatile unsigned long counter{};
    volatile unsigned int  lastPulseDuration{};

    volatile unsigned long errorsCounter{};
    volatile unsigned long longPulsesCounter{};

    static constexpr int  DEFAULT_DEAD_TIME = 50;
    volatile unsigned int deadTime{DEFAULT_DEAD_TIME};

    static constexpr int  DEFAULT_MAX_PULSE_TIME = 5000;
    volatile unsigned int maxPulseTime{DEFAULT_MAX_PULSE_TIME};

    static constexpr int  DEFAULT_MEDIUM_PULSE_TIME = 5000;
    volatile unsigned int mediumPulseTime_OneAndAHalf{DEFAULT_MEDIUM_PULSE_TIME * 3 / 2};

public:
    virtual ~GeigerCounter() = default;

    bool                  isChanged() const;
    unsigned long         getCounter();
    virtual unsigned int  getLastPulseDuration();
    virtual unsigned long getErrorCounter();
    virtual unsigned long getLongPulsesCounter();

    virtual bool begin() = 0;
    virtual void end() = 0;
};

///////////////////////////////////////////////

class GeigerCounterSimple;

class GeigerCounterSimpleInterrupt : public CppExternalInterrupt {
    GeigerCounterSimple *owner;

public:
    explicit GeigerCounterSimpleInterrupt(GeigerCounterSimple *owner);
    ~GeigerCounterSimpleInterrupt();

    void isrHandler() override;
};

///////////////////////////////////////////////

class GeigerCounterSimple : public GeigerCounter {
    friend class GeigerCounterSimpleInterrupt;

    GeigerCounterSimpleInterrupt interruptPtr;

protected:
    uint8_t pinNumber = 255;

public:
    GeigerCounterSimple();
    explicit GeigerCounterSimple(uint8_t pinNumber);
    ~GeigerCounterSimple();

    bool begin() override;
    void end() override;

private:
    static constexpr int DEFAULT_PULSE_DURATION = 300;
    virtual void         fallingIrq();
};

///////////////////////////////////////////////

class GeigerCounterTiming : public GeigerCounterSimple {
    unsigned long lastIsrMicros{};

    using GeigerCounterSimple::GeigerCounterSimple;

    void fallingIrq() override;
};

///////////////////////////////////////////////

class GeigerCounterDoubleIrq;

class GeigerCounterDoubleIrqFallingInterrupt : public CppExternalInterrupt {
    GeigerCounterDoubleIrq *owner;

public:
    explicit GeigerCounterDoubleIrqFallingInterrupt(GeigerCounterDoubleIrq *owner);
    ~GeigerCounterDoubleIrqFallingInterrupt();

    void isrHandler() override;
};

///////////////////////////////////////////////

class GeigerCounterDoubleIrq;

class GeigerCounterDoubleIrqRisingInterrupt final : public CppExternalInterrupt {
    GeigerCounterDoubleIrq *owner;

public:
    explicit GeigerCounterDoubleIrqRisingInterrupt(GeigerCounterDoubleIrq *owner);
    ~GeigerCounterDoubleIrqRisingInterrupt();

    void isrHandler() override;
};

///////////////////////////////////////////////

class GeigerCounterDoubleIrq : public GeigerCounter {
    friend class GeigerCounterDoubleIrqFallingInterrupt;
    friend class GeigerCounterDoubleIrqRisingInterrupt;

    GeigerCounterDoubleIrqFallingInterrupt fallingInterruptPtr;
    GeigerCounterDoubleIrqRisingInterrupt  risingInterruptPtr;

    unsigned long lastIsrMicros{};

protected:
    uint8_t pinNumberFalling = 255;
    uint8_t pinNumberRising  = 255;

public:
    GeigerCounterDoubleIrq();
    explicit GeigerCounterDoubleIrq(uint8_t pinNumberFalling, uint8_t pinNumberRising);
    ~GeigerCounterDoubleIrq();

    bool begin() override;
    void end() override;

private:
    virtual void fallingIrq();
    virtual void risingIrq();
};

///////////////////////////////////////////////
