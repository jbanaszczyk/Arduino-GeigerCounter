#include "CppExternalInterrupt.h"

// #define GEIGER_COUNTER_DYNAMIC


class GeigerCounterSimple_;

class GeigerCounterInterrupt final : public CppExternalInterrupt {
    GeigerCounterSimple_ *interruptOwnerPtr;
    
public:
    explicit GeigerCounterInterrupt(GeigerCounterSimple_ *owner) {
        interruptOwnerPtr = owner;
    }

    void isrHandler() override;

    ~GeigerCounterInterrupt() {
        end();
    }
};

class GeigerCounterSimple_ {
    friend class GeigerCounterInterrupt;

    GeigerCounterInterrupt interruptPtr;
    volatile int           count{};

public:
    GeigerCounterSimple_() : interruptPtr(this) {
    }

    void begin(const int8_t pinNumber, const int mode) {
        interruptPtr.begin(pinNumber, mode);
    }

    int getCount() const {
        return count;
    }

    void setCount() {
        ++count;
    }

};

void GeigerCounterInterrupt::isrHandler() {
    interruptOwnerPtr->setCount();
}


class GeigerCounter {
public:
    virtual                  ~GeigerCounter() = default;
    virtual bool             begin(const uint8_t pin) = 0;
    virtual bool             begin() = 0;
    virtual void             end() = 0;
    static unsigned long int getCounter();
};

class GeigerCounterSimple : GeigerCounter {
    static unsigned long int counter;
    int                      interruptNumber = 0;
    uint8_t                  pinNumber       = 255;
public:

    bool begin(const uint8_t pinNumber) override {
        this->pinNumber = pinNumber;
        return begin();
    }

    bool begin() override {
        interruptNumber = digitalPinToInterrupt(pinNumber);
        if (interruptNumber < 0) {
            return false;
        }
        attachInterrupt(interruptNumber, tube_isr_simple, FALLING);
        return true;
    }

    void end() override {
        if (interruptNumber >= 0) {
            detachInterrupt(interruptNumber);
        }
        interruptNumber = -1;
    }

    GeigerCounterSimple() = default;

    GeigerCounterSimple(const uint8_t pinNumber): pinNumber(pinNumber) {
#if defined GEIGER_COUNTER_DYNAMIC
        GeigerCounterSimple::begin();
#endif
    }

    static unsigned long int getCounter() {
        noInterrupts();
        const auto result = counter;
        counter           = 0;
        interrupts();
        return result;
    };

    ~GeigerCounterSimple() {
#if defined GEIGER_COUNTER_DYNAMIC
        GeigerCounterSimple::end();
#endif
    }

private:
    static void tube_isr_simple() {
        ++counter;
        // add_tube_event(false, 300);
    }
};

const auto INTERRUPT_PIN        = 2;
const auto INTERRUPT_PIN_SECOND = 3;




GeigerCounterSimple_ foo;

void setup() {
    Serial.begin(115200);
    Serial.println("go");
    foo.begin(2,FALLING);
}

void loop() {
    Serial.print(foo.getCount());
    Serial.print(" ");
    Serial.println();
    delay(100);
}















/*
unsigned long lastIsrMicros;

struct IsrTime {
    unsigned long duration;
    bool          falling;
};

const auto                      BUFFER_SIZE = 16;
volatile IsrTime                buffer [BUFFER_SIZE];
volatile decltype(+BUFFER_SIZE) bufferIndexWrite;
decltype(+BUFFER_SIZE)          bufferIndexRead;

void add_tube_event(bool falling, unsigned long duration) {
    if (!falling) {
        buffer[bufferIndexWrite].duration = duration;
        buffer[bufferIndexWrite].falling  = falling;
        bufferIndexWrite                  = (bufferIndexWrite + 1) % BUFFER_SIZE;
    }
}

void add_tube_event(bool falling) {
    const auto now      = micros();
    const auto duration = now - lastIsrMicros;
    lastIsrMicros       = now;
    add_tube_event(falling, duration);
}

void tube_isr_simple() {
    add_tube_event(false, 300);
}

void tube_isr_counting() {
    add_tube_event(true);

    while (digitalRead(INTERRUPT_PIN) == 0) {
        delayMicroseconds(5);
    }
    add_tube_event(false);
}

void tube_isr_falling() {
    add_tube_event(true);
}

void tube_isr_rising() {
    add_tube_event(false);
}

void setup1() {
    Serial.begin(115200);

    Serial.print("PIN: ");
    Serial.println(digitalPinToInterrupt(INTERRUPT_PIN));

    bufferIndexWrite = 0;
    bufferIndexRead  = 0;

    // pinMode(INTERRUPT_PIN, INPUT);
    // pinMode(INTERRUPT_PIN_SECOND, INPUT);
    //
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), tube_isr_falling, FALLING);
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN_SECOND), tube_isr_rising, RISING);

    // pinMode(INTERRUPT_PIN, INPUT);
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), tube_isr_simple, FALLING);
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), tube_isr_counting, FALLING);
}


void loop1() {
    if (bufferIndexWrite != bufferIndexRead) {

        Serial.print(buffer[bufferIndexRead].falling
                         ? "-"
                         : "+");

        Serial.print(buffer[bufferIndexRead].duration);
        Serial.print("  ");
        Serial.println();
        bufferIndexRead = (bufferIndexRead + 1) % BUFFER_SIZE;
    }
}

*/

/*
GeigerCounterSimple geigerCounter;

void setup() {
    Serial.begin(115200);

    auto result =     geigerCounter.begin(INTERRUPT_PIN);


    Serial.print("Geiger setup: ");
    Serial.println(result);

}

void loop() {

}

*/
