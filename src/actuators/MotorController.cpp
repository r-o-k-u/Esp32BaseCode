/**
 * @file MotorController.cpp
 * @brief L298N Dual H-Bridge Motor Driver Controller Implementation
 * @author Your Name
 * @version 2.0
 *
 * Enhanced motor controller for L298N dual H-bridge motor driver.
 * Supports up to 2 DC motors with comprehensive control, logging,
 * and advanced features including acceleration curves, braking,
 * and detailed status monitoring.
 */

#include "MotorController.h"
#include "../utils/Logger.h"
#include <ArduinoJson.h>

// Constructor for dual motor control
MotorController::MotorController(int motor1Enable, int motor1Input1, int motor1Input2,
                                 int motor2Enable, int motor2Input1, int motor2Input2)
    : motor1EnablePin(motor1Enable), motor1Input1Pin(motor1Input1), motor1Input2Pin(motor1Input2),
      motor2EnablePin(motor2Enable), motor2Input1Pin(motor2Input1), motor2Input2Pin(motor2Input2),
      maxSpeed(255), minSpeed(50), enableLogging(true), accelerationTimer(50)
{
    // Initialize motor states
    motor1State.speed = 0;
    motor1State.direction = STOP;
    motor1State.lastChange = 0;
    motor1State.isRunning = false;
    motor1State.lastCommand = "initialized";
    motor1State.motorName = "Motor 1";

    motor2State.speed = 0;
    motor2State.direction = STOP;
    motor2State.lastChange = 0;
    motor2State.isRunning = false;
    motor2State.lastCommand = "initialized";
    motor2State.motorName = "Motor 2";

    targetSpeed1 = 0;
    targetSpeed2 = 0;
    currentSpeed1 = 0;
    currentSpeed2 = 0;

    DEBUG_PRINTLN("[MOTOR] Dual L298N Motor Controller initialized");
    DEBUG_PRINTF("[MOTOR] Motor 1: EN=%d, IN1=%d, IN2=%d\n", motor1EnablePin, motor1Input1Pin, motor1Input2Pin);
    DEBUG_PRINTF("[MOTOR] Motor 2: EN=%d, IN1=%d, IN2=%d\n", motor2EnablePin, motor2Input1Pin, motor2Input2Pin);
}

bool MotorController::begin()
{
    // Set pin modes
    pinMode(motor1EnablePin, OUTPUT);
    pinMode(motor1Input1Pin, OUTPUT);
    pinMode(motor1Input2Pin, OUTPUT);
    pinMode(motor2EnablePin, OUTPUT);
    pinMode(motor2Input1Pin, OUTPUT);
    pinMode(motor2Input2Pin, OUTPUT);

    // Initialize motors in stopped state
    setMotorPins(motor1EnablePin, motor1Input1Pin, motor1Input2Pin, 0, STOP, motor1State);
    setMotorPins(motor2EnablePin, motor2Input1Pin, motor2Input2Pin, 0, STOP, motor2State);

    logMotorAction(1, "initialized", 0, STOP);
    logMotorAction(2, "initialized", 0, STOP);

    DEBUG_PRINTLN("[MOTOR] Dual L298N Motor Controller started successfully");
    return true;
}

// ─────────────────────────────────────────────────────────────────────
// BASIC MOTOR CONTROL
// ─────────────────────────────────────────────────────────────────────

void MotorController::setMotor1(int speed, MotorDirection direction)
{
    validateSpeed(speed);

    motor1State.lastCommand = "setMotor1";
    motor1State.lastChange = millis();

    setMotorPins(motor1EnablePin, motor1Input1Pin, motor1Input2Pin, speed, direction, motor1State);

    logMotorAction(1, "setMotor1", speed, direction);
}

void MotorController::setMotor2(int speed, MotorDirection direction)
{
    validateSpeed(speed);

    motor2State.lastCommand = "setMotor2";
    motor2State.lastChange = millis();

    setMotorPins(motor2EnablePin, motor2Input1Pin, motor2Input2Pin, speed, direction, motor2State);

    logMotorAction(2, "setMotor2", speed, direction);
}

void MotorController::setMotors(int speed1, MotorDirection direction1,
                                int speed2, MotorDirection direction2)
{
    validateSpeed(speed1);
    validateSpeed(speed2);

    motor1State.lastCommand = "setMotors";
    motor1State.lastChange = millis();
    motor2State.lastCommand = "setMotors";
    motor2State.lastChange = millis();

    setMotorPins(motor1EnablePin, motor1Input1Pin, motor1Input2Pin, speed1, direction1, motor1State);
    setMotorPins(motor2EnablePin, motor2Input1Pin, motor2Input2Pin, speed2, direction2, motor2State);

    logMotorAction(1, "setMotors", speed1, direction1);
    logMotorAction(2, "setMotors", speed2, direction2);
}

void MotorController::stopMotor1()
{
    setMotor1(0, STOP);
    motor1State.isRunning = false;
}

void MotorController::stopMotor2()
{
    setMotor2(0, STOP);
    motor2State.isRunning = false;
}

void MotorController::stopAll()
{
    stopMotor1();
    stopMotor2();
    DEBUG_PRINTLN("[MOTOR] All motors stopped");
}

void MotorController::brakeMotor1()
{
    setMotor1(0, BRAKE);
    motor1State.isRunning = false;
    logMotorAction(1, "brake");
}

void MotorController::brakeMotor2()
{
    setMotor2(0, BRAKE);
    motor2State.isRunning = false;
    logMotorAction(2, "brake");
}

void MotorController::brakeAll()
{
    brakeMotor1();
    brakeMotor2();
    DEBUG_PRINTLN("[MOTOR] All motors braked");
}

// ─────────────────────────────────────────────────────────────────────
// ADVANCED MOTOR CONTROL
// ─────────────────────────────────────────────────────────────────────

void MotorController::setMotor1WithAcceleration(int targetSpeed, int accelerationRate)
{
    validateSpeed(targetSpeed);
    targetSpeed1 = targetSpeed;

    motor1State.lastCommand = "setMotor1WithAcceleration";
    motor1State.lastChange = millis();

    logMotorAction(1, "setMotor1WithAcceleration", targetSpeed);
}

void MotorController::setMotor2WithAcceleration(int targetSpeed, int accelerationRate)
{
    validateSpeed(targetSpeed);
    targetSpeed2 = targetSpeed;

    motor2State.lastCommand = "setMotor2WithAcceleration";
    motor2State.lastChange = millis();

    logMotorAction(2, "setMotor2WithAcceleration", targetSpeed);
}

void MotorController::forward(int speed)
{
    setMotors(speed, FORWARD, speed, FORWARD);
    logMotorAction(1, "forward", speed, FORWARD);
    logMotorAction(2, "forward", speed, FORWARD);
}

void MotorController::backward(int speed)
{
    setMotors(speed, BACKWARD, speed, BACKWARD);
    logMotorAction(1, "backward", speed, BACKWARD);
    logMotorAction(2, "backward", speed, BACKWARD);
}

void MotorController::turnLeft(int speed)
{
    setMotors(speed, BACKWARD, speed, FORWARD);
    logMotorAction(1, "turnLeft", speed, BACKWARD);
    logMotorAction(2, "turnLeft", speed, FORWARD);
}

void MotorController::turnRight(int speed)
{
    setMotors(speed, FORWARD, speed, BACKWARD);
    logMotorAction(1, "turnRight", speed, FORWARD);
    logMotorAction(2, "turnRight", speed, BACKWARD);
}

void MotorController::spinLeft(int speed1, int speed2)
{
    setMotors(speed1, BACKWARD, speed2, FORWARD);
    logMotorAction(1, "spinLeft", speed1, BACKWARD);
    logMotorAction(2, "spinLeft", speed2, FORWARD);
}

void MotorController::spinRight(int speed1, int speed2)
{
    setMotors(speed1, FORWARD, speed2, BACKWARD);
    logMotorAction(1, "spinRight", speed1, FORWARD);
    logMotorAction(2, "spinRight", speed2, BACKWARD);
}

// ─────────────────────────────────────────────────────────────────────
// STATUS AND MONITORING
// ─────────────────────────────────────────────────────────────────────

MotorState MotorController::getMotor1State() const
{
    return motor1State;
}

MotorState MotorController::getMotor2State() const
{
    return motor2State;
}

String MotorController::getMotorStateJSON(int motorNum) const
{
    StaticJsonDocument<256> doc;
    JsonObject motor = doc.createNestedObject("motor");

    if (motorNum == 1)
    {
        motor["name"] = motor1State.motorName;
        motor["speed"] = motor1State.speed;
        motor["direction"] = directionToString(motor1State.direction);
        motor["running"] = motor1State.isRunning;
        motor["lastChange"] = motor1State.lastChange;
        motor["lastCommand"] = motor1State.lastCommand;
    }
    else if (motorNum == 2)
    {
        motor["name"] = motor2State.motorName;
        motor["speed"] = motor2State.speed;
        motor["direction"] = directionToString(motor2State.direction);
        motor["running"] = motor2State.isRunning;
        motor["lastChange"] = motor2State.lastChange;
        motor["lastCommand"] = motor2State.lastCommand;
    }

    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

String MotorController::getStatusJSON() const
{
    StaticJsonDocument<512> doc;

    // Motor 1 status
    JsonObject motor1 = doc.createNestedObject("motor1");
    motor1["name"] = motor1State.motorName;
    motor1["speed"] = motor1State.speed;
    motor1["direction"] = directionToString(motor1State.direction);
    motor1["running"] = motor1State.isRunning;
    motor1["lastChange"] = motor1State.lastChange;
    motor1["lastCommand"] = motor1State.lastCommand;

    // Motor 2 status
    JsonObject motor2 = doc.createNestedObject("motor2");
    motor2["name"] = motor2State.motorName;
    motor2["speed"] = motor2State.speed;
    motor2["direction"] = directionToString(motor2State.direction);
    motor2["running"] = motor2State.isRunning;
    motor2["lastChange"] = motor2State.lastChange;
    motor2["lastCommand"] = motor2State.lastCommand;

    // System status
    doc["maxSpeed"] = maxSpeed;
    doc["minSpeed"] = minSpeed;
    doc["loggingEnabled"] = enableLogging;

    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

void MotorController::update()
{
    // Handle smooth acceleration for motor 1
    if (currentSpeed1 != targetSpeed1)
    {
        if (accelerationTimer.isReady())
        {
            updateMotorSpeed(1, targetSpeed1, motor1State);
        }
    }

    // Handle smooth acceleration for motor 2
    if (currentSpeed2 != targetSpeed2)
    {
        if (accelerationTimer.isReady())
        {
            updateMotorSpeed(2, targetSpeed2, motor2State);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// CONFIGURATION AND LOGGING
// ─────────────────────────────────────────────────────────────────────

void MotorController::setLogging(bool enable)
{
    enableLogging = enable;
    DEBUG_PRINTLN("[MOTOR] Logging " + String(enable ? "enabled" : "disabled"));
}

void MotorController::setMaxSpeed(int speed)
{
    maxSpeed = constrain(speed, minSpeed, 255);
    if (currentSpeed1 > maxSpeed)
    {
        currentSpeed1 = maxSpeed;
        setMotor1(currentSpeed1, motor1State.direction);
    }
    if (currentSpeed2 > maxSpeed)
    {
        currentSpeed2 = maxSpeed;
        setMotor2(currentSpeed2, motor2State.direction);
    }
    logMotorAction(1, "setMaxSpeed", maxSpeed);
    logMotorAction(2, "setMaxSpeed", maxSpeed);
}

void MotorController::setMinSpeed(int speed)
{
    minSpeed = constrain(speed, 0, maxSpeed);
    logMotorAction(1, "setMinSpeed", minSpeed);
    logMotorAction(2, "setMinSpeed", minSpeed);
}

void MotorController::testMotors()
{
    DEBUG_PRINTLN("[MOTOR] Starting motor test sequence...");

    // Test motor 1
    DEBUG_PRINTLN("[MOTOR] Testing Motor 1...");
    setMotor1(minSpeed, FORWARD);
    delay(1000);
    setMotor1(minSpeed, BACKWARD);
    delay(1000);
    stopMotor1();
    delay(500);

    // Test motor 2
    DEBUG_PRINTLN("[MOTOR] Testing Motor 2...");
    setMotor2(minSpeed, FORWARD);
    delay(1000);
    setMotor2(minSpeed, BACKWARD);
    delay(1000);
    stopMotor2();
    delay(500);

    // Test combined movements
    DEBUG_PRINTLN("[MOTOR] Testing combined movements...");
    forward(minSpeed);
    delay(1000);
    backward(minSpeed);
    delay(1000);
    turnLeft(minSpeed);
    delay(500);
    turnRight(minSpeed);
    delay(500);
    stopAll();

    DEBUG_PRINTLN("[MOTOR] Motor test sequence complete");
}

// ─────────────────────────────────────────────────────────────────────
// COMPATIBILITY METHODS FOR ACTUATOR MANAGER
// ─────────────────────────────────────────────────────────────────────

int MotorController::getSpeed()
{
    return motor1State.speed;
}

bool MotorController::getDirection()
{
    return motor1State.direction == FORWARD;
}

MotorDirection MotorController::stringToDirection(const String &directionStr)
{
    String dir = directionStr;
    dir.toLowerCase();
    if (dir == "forward" || dir == "fwd")
        return FORWARD;
    else if (dir == "backward" || dir == "reverse" || dir == "rev" || dir == "back")
        return BACKWARD;
    else if (dir == "stop" || dir == "off")
        return STOP;
    else if (dir == "brake")
        return BRAKE;
    else
        return STOP;
}

// ─────────────────────────────────────────────────────────────────────
// PRIVATE HELPER METHODS
// ─────────────────────────────────────────────────────────────────────

void MotorController::setMotorPins(int enablePin, int input1Pin, int input2Pin,
                                   int speed, MotorDirection direction, MotorState &state)
{
    // Update state
    state.speed = speed;
    state.direction = direction;
    state.isRunning = (speed > 0 && direction != STOP);
    state.lastChange = millis();

    // Set motor pins based on direction
    switch (direction)
    {
    case FORWARD:
        digitalWrite(input1Pin, HIGH);
        digitalWrite(input2Pin, LOW);
        break;
    case BACKWARD:
        digitalWrite(input1Pin, LOW);
        digitalWrite(input2Pin, HIGH);
        break;
    case STOP:
        digitalWrite(input1Pin, LOW);
        digitalWrite(input2Pin, LOW);
        break;
    case BRAKE:
        digitalWrite(input1Pin, HIGH);
        digitalWrite(input2Pin, HIGH);
        break;
    }

    // Apply PWM for speed control (only if not stopped or braked)
    if (direction == FORWARD || direction == BACKWARD)
    {
        analogWrite(enablePin, speed);
    }
    else
    {
        analogWrite(enablePin, 0);
    }
}

void MotorController::updateMotorSpeed(int motorNum, int targetSpeed, MotorState &state)
{
    int &currentSpeed = (motorNum == 1) ? currentSpeed1 : currentSpeed2;

    if (currentSpeed < targetSpeed)
    {
        currentSpeed += 5; // Acceleration step
        if (currentSpeed > targetSpeed)
            currentSpeed = targetSpeed;
    }
    else if (currentSpeed > targetSpeed)
    {
        currentSpeed -= 5; // Deceleration step
        if (currentSpeed < targetSpeed)
            currentSpeed = targetSpeed;
    }

    // Apply the updated speed
    setMotorPins((motorNum == 1) ? motor1EnablePin : motor2EnablePin,
                 (motorNum == 1) ? motor1Input1Pin : motor2Input1Pin,
                 (motorNum == 1) ? motor1Input2Pin : motor2Input2Pin,
                 currentSpeed, state.direction, state);
}

void MotorController::logMotorAction(int motorNum, const String &action,
                                     int speed, MotorDirection direction)
{
    if (!enableLogging)
        return;

    String logMessage = "Motor " + String(motorNum) + " " + action;
    if (speed >= 0)
        logMessage += " (speed: " + String(speed) + ")";
    if (direction != STOP)
        logMessage += " (dir: " + directionToString(direction) + ")";

    dataLogger.logEvent(logMessage.c_str());
}

String MotorController::directionToString(MotorDirection direction) const
{
    switch (direction)
    {
    case FORWARD:
        return "FORWARD";
    case BACKWARD:
        return "BACKWARD";
    case STOP:
        return "STOP";
    case BRAKE:
        return "BRAKE";
    default:
        return "UNKNOWN";
    }
}

void MotorController::validateSpeed(int &speed)
{
    speed = constrain(speed, 0, maxSpeed);
    if (speed > 0 && speed < minSpeed)
        speed = minSpeed;
}

// ─────────────────────────────────────────────────────────────────────
// COMPATIBILITY METHODS FOR ACTUATOR MANAGER
// ─────────────────────────────────────────────────────────────────────

void MotorController::setSpeed(int speed)
{
    setMotor1(speed, motor1State.direction);
    logMotorAction(1, "setSpeed", speed);
}

void MotorController::setDirection(bool forward)
{
    MotorDirection direction = forward ? FORWARD : BACKWARD;
    setMotor1(motor1State.speed, direction);
    logMotorAction(1, "setDirection", forward ? 1 : 0);
}

void MotorController::stop()
{
    stopMotor1();
    logMotorAction(1, "stop");
}