/**
 * @file ServoController.cpp
 * @brief Servo motor implementation
 */

#include "ServoController.h"

ServoController::ServoController()
{
    servo1Attached = false;
    servo2Attached = false;
    currentAngle1 = 90;
    currentAngle2 = 90;
}

bool ServoController::begin()
{
    DEBUG_PRINTLN("Initializing servos...");

    // Configure servo timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);

    // Attach servo 1
    servo1.setPeriodHertz(50);            // Standard 50Hz servo
    servo1.attach(SERVO1_PIN, 500, 2400); // Min/Max pulse width
    servo1.write(90);                     // Center position
    servo1Attached = true;

    // Attach servo 2
    servo2.setPeriodHertz(50);
    servo2.attach(SERVO2_PIN, 500, 2400);
    servo2.write(90);
    servo2Attached = true;

    delay(500); // Let servos move to position

    DEBUG_PRINTLN("Servos ready!");
    return true;
}

void ServoController::setAngle(uint8_t servoNum, int angle)
{
    // Constrain angle to valid range
    angle = constrain(angle, 0, 180);

    if (servoNum == 1 && servo1Attached)
    {
        servo1.write(angle);
        currentAngle1 = angle;

#if DEBUG_ACTUATORS
        DEBUG_PRINTF("Servo 1 set to %d°\n", angle);
#endif
    }
    else if (servoNum == 2 && servo2Attached)
    {
        servo2.write(angle);
        currentAngle2 = angle;

#if DEBUG_ACTUATORS
        DEBUG_PRINTF("Servo 2 set to %d°\n", angle);
#endif
    }
}

void ServoController::sweep(uint8_t servoNum, int minAngle, int maxAngle, int delayMs)
{
    // Sweep from min to max
    for (int angle = minAngle; angle <= maxAngle; angle++)
    {
        setAngle(servoNum, angle);
        delay(delayMs);
    }

    // Sweep back from max to min
    for (int angle = maxAngle; angle >= minAngle; angle--)
    {
        setAngle(servoNum, angle);
        delay(delayMs);
    }
}

int ServoController::getAngle(uint8_t servoNum)
{
    return (servoNum == 1) ? currentAngle1 : currentAngle2;
}

void ServoController::detach(uint8_t servoNum)
{
    if (servoNum == 1 && servo1Attached)
    {
        servo1.detach();
        servo1Attached = false;
    }
    else if (servoNum == 2 && servo2Attached)
    {
        servo2.detach();
        servo2Attached = false;
    }
}