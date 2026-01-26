/**
 * @file LEDController.h
 * @brief Simple LED control
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "../config.h"

class LEDController
{
private:
    uint8_t pin;
    bool state;
    bool initialized;

public:
    LEDController(uint8_t ledPin);

    bool begin();
    void setState(bool on);
    bool getState();
};

#endif
