#pragma once

#include <vector>
#include <Arduino.h>

#include "esphome/core/component.h"

#define START_INTERVAL 900
#define SHORT_INTERVAL 208
#define LONG_INTERVAL 1875

namespace esphome {
namespace hunter_roam {

class HunterRoam : public Component {
    public:
        byte stopZone(byte zone);
        byte startZone(byte zone, byte time);
        byte startProgram(byte num);
        String errorHint(byte error);
        void set_pin(int pin);

    private:
        int _pin;  // Store the pin
        void hunterBitfield(std::vector <byte> &bits, byte pos, byte val, byte len);
        void writeBus(std::vector<byte> buffer, bool extrabit);
        void sendLow(void);
        void sendHigh(void);
};

}
}