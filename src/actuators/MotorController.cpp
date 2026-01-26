/**
 * @file MotorController.cpp
 * @brief DC Motor control using L298N motor driver implementation
 * @author Your Name
 * @version 2.0
 */

#include "MotorController.h"
#include "../utils/Logger.h"

MotorController::MotorController(uint8_t in1, uint8_t in2, uint8_t enable)
    : in1Pin(in1), in2Pin(in2), enablePin(enable), initialized(false),
      isRunning(false), isForward(true), currentSpeed(0), maxSpeed(255),
      minSpeed(50), acceleration(10), lastSpeedChange(0)
{
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(enablePin, OUTPUT);

    // Initialize motor in stopped state
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enablePin, 0);
}

bool MotorController::begin()
{
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(enablePin, OUTPUT);

    // Initialize motor in stopped state
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enablePin, 0);

    initialized = true;
    isRunning = false;
    isForward = true;
    currentSpeed = 0;

    DEBUG_PRINTLN("[MOTOR] Motor controller initialized on pins IN1:" + String(in1Pin) +
                  ", IN2:" + String(in2Pin) + ", EN:" + String(enablePin));

    return true;
}

void MotorController::setSpeed(int speed)
{
    if (!initialized)
        return;

    speed = constrainSpeed(speed);
    currentSpeed = speed;

    if (speed == 0)
    {
        stop();
    }
    else
    {
        applySpeed(speed);
        isRunning = true;
    }

    DEBUG_PRINT("[MOTOR] Speed set to: " + String(currentSpeed));
    DEBUG_PRINTLN(" (" + String((currentSpeed * 100) / 255) + "%)");
}

void MotorController::setDirection(bool forward)
{
    if (!initialized)
        return;

    isForward = forward;

    if (isRunning && currentSpeed > 0)
    {
        applySpeed(currentSpeed);
    }

    DEBUG_PRINTLN("[MOTOR] Direction set to: " + String(forward ? "Forward" : "Reverse"));
}

void MotorController::start()
{
    if (!initialized)
        return;

    if (currentSpeed == 0)
    {
        currentSpeed = minSpeed;
    }

    applySpeed(currentSpeed);
    isRunning = true;

    DEBUG_PRINTLN("[MOTOR] Motor started at speed: " + String(currentSpeed));
}

void MotorController::stop()
{
    if (!initialized)
        return;

    applySpeed(0);
    isRunning = false;
    currentSpeed = 0;

    DEBUG_PRINTLN("[MOTOR] Motor stopped");
}

void MotorController::brake()
{
    if (!initialized)
        return;

    // Hard brake by setting both inputs high
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, HIGH);
    analogWrite(enablePin, 0);

    isRunning = false;
    currentSpeed = 0;

    DEBUG_PRINTLN("[MOTOR] Motor braked");
}

void MotorController::accelerateTo(int targetSpeed, int accelerationRate)
{
    if (!initialized)
        return;

    targetSpeed = constrainSpeed(targetSpeed);
    acceleration = accelerationRate;

    if (targetSpeed > currentSpeed)
    {
        // Accelerating
        while (currentSpeed < targetSpeed && isRunning)
        {
            currentSpeed += acceleration;
            if (currentSpeed > targetSpeed)
                currentSpeed = targetSpeed;

            applySpeed(currentSpeed);
            delay(10); // Small delay for smooth acceleration
            update();
        }
    }
    else if (targetSpeed < currentSpeed)
    {
        // Decelerating
        while (currentSpeed > targetSpeed && isRunning)
        {
            currentSpeed -= acceleration;
            if (currentSpeed < targetSpeed)
                currentSpeed = targetSpeed;

            applySpeed(currentSpeed);
            delay(10); // Small delay for smooth deceleration
            update();
        }
    }

    DEBUG_PRINTLN("[MOTOR] Accelerated to: " + String(currentSpeed));
}

void MotorController::decelerateTo(int targetSpeed, int decelerationRate)
{
    accelerateTo(targetSpeed, decelerationRate);
}

void MotorController::setMaxSpeed(int maxSpeed)
{
    this->maxSpeed = constrain(maxSpeed, minSpeed, 255);
    if (currentSpeed > this->maxSpeed)
    {
        currentSpeed = this->maxSpeed;
        applySpeed(currentSpeed);
    }

    DEBUG_PRINTLN("[MOTOR] Max speed set to: " + String(this->maxSpeed));
}

void MotorController::setMinSpeed(int minSpeed)
{
    this->minSpeed = constrain(minSpeed, 0, maxSpeed);
    if (currentSpeed < this->minSpeed && currentSpeed > 0)
    {
        currentSpeed = this->minSpeed;
        applySpeed(currentSpeed);
    }

    DEBUG_PRINTLN("[MOTOR] Min speed set to: " + String(this->minSpeed));
}

void MotorController::rampUp(int targetSpeed, int rampTime)
{
    if (!initialized)
        return;

    if (targetSpeed == -1)
        targetSpeed = maxSpeed;
    targetSpeed = constrainSpeed(targetSpeed);

    int startSpeed = currentSpeed;
    unsigned long startTime = millis();
    unsigned long duration = rampTime;

    DEBUG_PRINTLN("[MOTOR] Ramping up from " + String(startSpeed) + " to " + String(targetSpeed) +
                  " over " + String(rampTime) + "ms");

    while (millis() - startTime < duration && isRunning)
    {
        unsigned long elapsed = millis() - startTime;
        float progress = (float)elapsed / duration;

        currentSpeed = startSpeed + (targetSpeed - startSpeed) * progress;
        applySpeed(currentSpeed);

        delay(10);
    }

    currentSpeed = targetSpeed;
    applySpeed(currentSpeed);
}

void MotorController::rampDown(int rampTime)
{
    rampUp(0, rampTime);
}

void MotorController::smoothStop(int rampTime)
{
    rampDown(rampTime);
    stop();
}

bool MotorController::isRunningState()
{
    return isRunning;
}

bool MotorController::isMovingForward()
{
    return isForward;
}

int MotorController::getCurrentSpeed()
{
    return currentSpeed;
}

int MotorController::getMaxSpeed()
{
    return maxSpeed;
}

int MotorController::getMinSpeed()
{
    return minSpeed;
}

void MotorController::setAcceleration(int rate)
{
    acceleration = constrain(rate, 1, 50);
    DEBUG_PRINTLN("[MOTOR] Acceleration rate set to: " + String(acceleration));
}

void MotorController::emergencyStop()
{
    brake();
    DEBUG_PRINTLN("[MOTOR] Emergency stop activated");
}

bool MotorController::checkOvercurrent()
{
    // Placeholder for overcurrent detection
    // In a real implementation, this would read from a current sensor
    return false;
}

void MotorController::update()
{
    // Update motor state if running
    if (isRunning && currentSpeed > 0)
    {
        applySpeed(currentSpeed);
    }
}

void MotorController::calibrate()
{
    // Motor calibration routine
    DEBUG_PRINTLN("[MOTOR] Starting motor calibration...");

    // Test forward direction
    setDirection(true);
    setSpeed(minSpeed);
    delay(1000);

    // Test reverse direction
    setDirection(false);
    setSpeed(minSpeed);
    delay(1000);

    // Stop motor
    stop();

    DEBUG_PRINTLN("[MOTOR] Motor calibration complete");
}

String MotorController::getMotorStatus()
{
    String status = "{\"motor\":{";
    status += "\"running\":" + String(isRunning ? "true" : "false") + ",";
    status += "\"direction\":\"" + String(isForward ? "forward" : "reverse") + "\",";
    status += "\"speed\":" + String(currentSpeed) + ",";
    status += "\"maxSpeed\":" + String(maxSpeed) + ",";
    status += "\"minSpeed\":" + String(minSpeed) + ",";
    status += "\"acceleration\":" + String(acceleration);
    status += "}}";
    return status;
}

void MotorController::applySpeed(int speed)
{
    if (speed == 0)
    {
        // Stop motor
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, LOW);
        analogWrite(enablePin, 0);
        isRunning = false;
    }
    else
    {
        // Set direction
        setMotorPins(isForward, !isForward);

        // Apply PWM
        analogWrite(enablePin, speed);
        isRunning = true;
    }
}

void MotorController::updateSpeedWithRamping()
{
    // This method can be used for continuous speed ramping
    // Implementation depends on specific requirements
}

int MotorController::constrainSpeed(int speed)
{
    return constrain(speed, 0, maxSpeed);
}

void MotorController::setMotorPins(bool in1State, bool in2State)
{
    digitalWrite(in1Pin, in1State ? HIGH : LOW);
    digitalWrite(in2Pin, in2State ? HIGH : LOW);
}

// Compatibility methods for ActuatorManager
int MotorController::getSpeed()
{
    return getCurrentSpeed();
}

bool MotorController::getDirection()
{
    return isMovingForward();
}
