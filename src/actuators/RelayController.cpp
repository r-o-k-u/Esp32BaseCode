/**
 * @file RelayController.cpp
 * @brief Relay control implementation
 */

#include "RelayController.h"

RelayController::RelayController(bool activeL)
{
    activeLow = activeL;
    relay1State = false;
    relay2State = false;
    relay3State = false;
}

bool RelayController::begin()
{
    DEBUG_PRINTLN("Initializing relays...");

    // Configure relay pins as outputs
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    pinMode(RELAY3_PIN, OUTPUT);

    // Turn all relays off
    allOff();

    DEBUG_PRINTLN("Relays ready!");
    return true;
}

void RelayController::setState(uint8_t relay, bool state)
{
    uint8_t pin;
    bool *statePtr;

    // Select relay
    switch (relay)
    {
    case 1:
        pin = RELAY1_PIN;
        statePtr = &relay1State;
        break;
    case 2:
        pin = RELAY2_PIN;
        statePtr = &relay2State;
        break;
    case 3:
        pin = RELAY3_PIN;
        statePtr = &relay3State;
        break;
    default:
        return;
    }

    // Set state (invert if active low)
    bool outputState = activeLow ? !state : state;
    digitalWrite(pin, outputState);
    *statePtr = state;

#if DEBUG_ACTUATORS
    DEBUG_PRINTF("Relay %d: %s\n", relay, state ? "ON" : "OFF");
#endif
}

bool RelayController::getState(uint8_t relay)
{
    switch (relay)
    {
    case 1:
        return relay1State;
    case 2:
        return relay2State;
    case 3:
        return relay3State;
    default:
        return false;
    }
}

void RelayController::toggle(uint8_t relay)
{
    setState(relay, !getState(relay));
}

void RelayController::allOn()
{
    setState(1, true);
    setState(2, true);
    setState(3, true);
}

void RelayController::allOff()
{
    setState(1, false);
    setState(2, false);
    setState(3, false);
}

void RelayController::pulse(uint8_t relay, uint32_t durationMs)
{
    setState(relay, true);
    delay(durationMs);
    setState(relay, false);
}