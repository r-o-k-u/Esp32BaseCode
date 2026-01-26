/**
 * @file ServoController.h
 * @brief Servo motor control
 */

#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "../config.h"
#include <ESP32Servo.h>

class ServoController
{
private:
    Servo servo1;
    Servo servo2;
    bool servo1Attached;
    bool servo2Attached;
    int currentAngle1;
    int currentAngle2;

public:
    ServoController();

    bool begin();
    void setAngle(uint8_t servoNum, int angle);
    void sweep(uint8_t servoNum, int minAngle, int maxAngle, int delayMs);
    int getAngle(uint8_t servoNum);
    void detach(uint8_t servoNum);
};

#endif
