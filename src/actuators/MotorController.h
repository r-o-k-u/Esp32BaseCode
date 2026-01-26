/**
 * @file MotorController.h
 * @brief DC Motor control using L298N motor driver
 * @author Your Name
 * @version 2.0
 */

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "../config.h"
#include <Arduino.h>

class MotorController
{
private:
    uint8_t in1Pin;
    uint8_t in2Pin;
    uint8_t enablePin;

    bool initialized;
    bool isRunning;
    bool isForward;
    int currentSpeed;
    int maxSpeed;

    // Motor characteristics
    int minSpeed;
    int acceleration;
    unsigned long lastSpeedChange;

public:
    MotorController(uint8_t in1, uint8_t in2, uint8_t enable);

    bool begin();

    // Basic control
    void setSpeed(int speed);
    void setDirection(bool forward);
    void start();
    void stop();
    void brake();

    // Advanced control
    void accelerateTo(int targetSpeed, int accelerationRate = 10);
    void decelerateTo(int targetSpeed, int decelerationRate = 10);
    void setMaxSpeed(int maxSpeed);
    void setMinSpeed(int minSpeed);

    // Speed control with ramping
    void rampUp(int targetSpeed = -1, int rampTime = 1000);
    void rampDown(int rampTime = 1000);
    void smoothStop(int rampTime = 1000);

    // Status and monitoring
    bool isRunningState();
    bool isMovingForward();
    int getCurrentSpeed();
    int getMaxSpeed();
    int getMinSpeed();

    // Compatibility methods for ActuatorManager
    int getSpeed();
    bool getDirection();

    // Motor protection
    void setAcceleration(int rate);
    void emergencyStop();
    bool checkOvercurrent();

    // Utility functions
    void update();
    void calibrate();
    String getMotorStatus();

private:
    void applySpeed(int speed);
    void updateSpeedWithRamping();
    int constrainSpeed(int speed);
    void setMotorPins(bool in1State, bool in2State);
};

#endif // MOTOR_CONTROLLER_H
