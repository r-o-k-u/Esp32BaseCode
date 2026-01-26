/**
 * @file RelayController.h
 * @brief Relay module control
 */

#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H
#include "../config.h"

class RelayController
{
private:
    bool relay1State;
    bool relay2State;
    bool relay3State;
    bool activeLow; // Most relay modules are active low

public:
    RelayController(bool activeLow = true);

    bool begin();
    void setState(uint8_t relay, bool state);
    bool getState(uint8_t relay);
    void toggle(uint8_t relay);
    void allOn();
    void allOff();
    void pulse(uint8_t relay, uint32_t durationMs);
};

#endif
