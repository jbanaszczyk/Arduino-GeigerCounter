#include "CppExternalInterrupt.h"

#include <Arduino.h>

// Some compatibility stuff

#ifndef _countof
template <typename _CountofType, size_t _SizeOfArray>
char (*                                 __countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)
#endif

template <int N>
void          CppExternalInterrupt::isrHandlerExt() {
    if (instances[N] != nullptr)
        instances[N]->isrHandler();
}

int CppExternalInterrupt::registerIsr(CppExternalInterrupt *intThisPtr, const int8_t pinNumber, const int mode) {
    const auto interruptNumber = digitalPinToInterrupt(pinNumber);
    if (interruptNumber >= 0 && interruptNumber < sizeof(handlers) / sizeof(handlers[0])) {
        pinMode(pinNumber, INPUT);
        attachInterrupt(interruptNumber, handlers[interruptNumber], mode);
        instances[interruptNumber] = intThisPtr;
        return interruptNumber;
    }
    return NOT_AN_INTERRUPT;
}

void CppExternalInterrupt::deRegisterIsr(const int interruptNumber) {
    if (interruptNumber >= 0 && interruptNumber < sizeof(handlers) / sizeof(handlers[0])) {
        detachInterrupt(interruptNumber);
        instances[interruptNumber] = nullptr;
    }
}

bool CppExternalInterrupt::isActive() const {
    return interruptNumber != NOT_AN_INTERRUPT;
}

bool CppExternalInterrupt::begin(const int8_t pinNumber, const int mode) {
    interruptNumber = registerIsr(this, pinNumber, mode);
    return isActive();
}

void CppExternalInterrupt::end() {
    deRegisterIsr(interruptNumber);
    interruptNumber = NOT_AN_INTERRUPT;
}

CppExternalInterrupt *CppExternalInterrupt::instances[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

void (*CppExternalInterrupt::handlers[])() = {isrHandlerExt<0>, isrHandlerExt<1>, isrHandlerExt<2>, isrHandlerExt<3>, isrHandlerExt<4>, isrHandlerExt<5>, isrHandlerExt<6>, isrHandlerExt<7>};
