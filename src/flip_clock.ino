// IRremote - Version: Latest
// AccelStepper - Version: Latest
// SevSeg - Version: Latest

// #define DEBUG_IR
#define BAUD 115200

#include <AccelStepper.h>
#include <SevSeg.h>

/***** TINY RECEIVER MACROS *****/
// These need to come before the TinyIRReceiver include
#define NO_LED_FEEDBACK_CODE
#define TINY_RECEIVER_USE_ARDUINO_ATTACH_INTERRUPT
#define IR_INPUT_PIN 18

#ifdef DEBUG_IR
    #define DEBUG
#endif
#include "TinyIRReceiver.hpp"
#undef DEBUG

/***** STEPPER CONSTANTS *****/

#define STEPS_PER_REV        2048
#define STEPPER_MAX_SPEED    (STEPS_PER_REV / 4)
#define STEPPER_ACCELERATION (STEPS_PER_REV / 2)

AccelStepper steppers[]
    = {AccelStepper(AccelStepper::FULL4WIRE, 22, 26, 24, 28), AccelStepper(AccelStepper::FULL4WIRE, 23, 27, 25, 29)};

/***** DISPLAY CONSTANTS *****/

const byte NUM_DIGITS = 4;
const byte DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
const byte SEGMENT_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13};

SevSeg sevSeg;

/***** IR INTERRUPT ISR *****/

struct IrData {
    uint16_t address;
    uint16_t command;
    bool isRepeat;
} irData = {0, 0, 0};
bool irReady = 0;

void handleReceivedTinyIRData(uint16_t address, uint8_t command, bool isRepeat) {
#ifdef DEBUG_IR
    Serial.print(F("A=0x"));
    Serial.print(address, HEX);
    Serial.print(F(" C=0x"));
    Serial.print(command, HEX);
    Serial.print(F(" R="));
    Serial.print(isRepeat);
    Serial.println();
#endif
    irData = {address, command, isRepeat};
    irReady = 1;
}

/***** MAIN FUNCTIONS *****/

void setup() {
    Serial.begin(BAUD);
    initPCIInterruptForTinyReceiver();

    // Set all SS pins to output
    sevSeg.begin(COMMON_CATHODE, NUM_DIGITS, DIGIT_PINS, SEGMENT_PINS);
    sevSeg.setNumber(3141, 3);

    for (auto& s : steppers) {
        s.setMaxSpeed(STEPPER_MAX_SPEED);
        s.setAcceleration(STEPPER_ACCELERATION);
        s.moveTo(STEPS_PER_REV * 2.5);
    }
}

void loop() {
    // Read IR remote
    if (irReady) {
        if (irData.address == 0 && irData.isRepeat == 0) {
            switch (irData.command) {
                case 0x16: /*0*/ sevSeg.setNumber(0); break;
                case 0x0C: /*1*/ sevSeg.setNumber(1); break;
                case 0x18: /*2*/ sevSeg.setNumber(2); break;
                case 0x5E: /*3*/ sevSeg.setNumber(3); break;
                case 0x08: /*4*/ sevSeg.setNumber(4); break;
                case 0x1C: /*5*/ sevSeg.setNumber(5); break;
                case 0x5A: /*6*/ sevSeg.setNumber(6); break;
                case 0x42: /*7*/ sevSeg.setNumber(7); break;
                case 0x52: /*8*/ sevSeg.setNumber(8); break;
                case 0x4A: /*9*/
                    sevSeg.setNumber(9);
                    break;
                    //     case 0x45: /*power*/ toggleDisplayPower(); break;
            }
        }
        irReady = 0;
    }

    // Run steppers
    for (auto& s : steppers) {
        if (s.distanceToGo() == 0) s.moveTo(-s.currentPosition());
        s.run();
    }

    // Refresh display
    sevSeg.refreshDisplay();
}
