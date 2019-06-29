#include <Arduino.h>

#include "GeigerDetector.h"

const auto INTERRUPT_PIN        = 2;
const auto INTERRUPT_PIN_SECOND = 3;

GeigerDetector *geigerDetector;

void calibration_cb(const unsigned int pulsesCount, const unsigned long pulsesDuration) {
    Serial.print("Calibration: ");
    Serial.print(pulsesCount);
    Serial.print(" : ");
    Serial.println(pulsesDuration);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
    }
    Serial.println("go");
    geigerDetector = new GeigerDetectorDoubleIrq(INTERRUPT_PIN, INTERRUPT_PIN_SECOND);
    Serial.println("Calibrating");
    geigerDetector->calibrate(4, 30000, calibration_cb);
    if (! geigerDetector->begin()) {
        Serial.println("Problem during GeigerDetector begin()");
    };
    Serial.println("Calibration results:");
    Serial.print("\tmediumPulseDuration: ");
    Serial.println(geigerDetector->getMediumPulseDuration());
    Serial.print("\tdeadTime: ");
    Serial.println(geigerDetector->getDeadTime());
    Serial.print("\tmaxPulseDuration: ");
    Serial.println(geigerDetector->getMaxPulseDuration());
}

void loop() {
    static unsigned long counter{0};
    if (geigerDetector->isChanged()) {
        counter += geigerDetector->getCounter();
        Serial.print(geigerDetector->getLastPulseDuration());

        Serial.print(" ");
        Serial.print(counter);

        Serial.print(" ");
        Serial.print(geigerDetector->getErrorCounter());

        Serial.print(" ");
        Serial.print(geigerDetector->getLongPulsesCounter());

        Serial.println();
    }
}
