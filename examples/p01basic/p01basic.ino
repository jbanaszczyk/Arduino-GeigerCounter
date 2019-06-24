#include <Arduino.h>

#include "GeigerCounter.h"

const auto INTERRUPT_PIN        = 2;
const auto INTERRUPT_PIN_SECOND = 3;

GeigerCounter *foo;

void setup() {
    Serial.begin(115200);
    Serial.println("go");
    foo = new GeigerCounterDoubleIrq(INTERRUPT_PIN, INTERRUPT_PIN_SECOND);
    foo->begin();
}

void loop() {
    static unsigned long counter{0};
    if (foo->isChanged()) {
        counter += foo->getCounter();
        Serial.print(foo->getLastPulseDuration());

        Serial.print(" ");
        Serial.print(counter);

        Serial.print(" ");
        Serial.print(foo->getErrorCounter());
        Serial.print(" ");
        Serial.print(foo->getLongPulsesCounter());

        Serial.println();
    }
}
