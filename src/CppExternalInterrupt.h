#pragma once

#include <Arduino.h>

class CppExternalInterrupt {
    static constexpr auto MAX_EXTERNAL_NUM_INTERRUPTS = 8;

    template <int N>
    static void   isrHandlerExt();

    static void (*handlers[MAX_EXTERNAL_NUM_INTERRUPTS])();

    static CppExternalInterrupt *instances[MAX_EXTERNAL_NUM_INTERRUPTS];

    static int registerIsr(CppExternalInterrupt *intThisPtr, int8_t pinNumber, int mode);

    static void deRegisterIsr(int interruptNumber);

    int interruptNumber{NOT_AN_INTERRUPT};

public:
    virtual ~CppExternalInterrupt() = default;

    bool begin(int8_t pinNumber, int mode);
    void end();

    virtual void isrHandler() = 0;
};
