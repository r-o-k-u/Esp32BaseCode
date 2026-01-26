/**
 * @file LEDController.cpp
 * @brief Simple LED control implementation
 */

#include "LEDController.h"

LEDController::LEDController(uint8_t ledPin)
    : pin(ledPin), state(false), initialized(false)
{
}

bool LEDController::begin()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;
    initialized = true;

    DEBUG_PRINTLN("[LED] LED controller initialized on pin " + String(pin));
    return true;
}

void LEDController::setState(bool on)
{
    if (!initialized)
        return;

    state = on;
    digitalWrite(pin, state ? HIGH : LOW);

    DEBUG_PRINTLN("[LED] LED " + String(state ? "ON" : "OFF"));
}

bool LEDController::getState()
{
    return state;
}
