/**
 * @file MotorController.h
 * @brief L298N Dual H-Bridge Motor Driver Controller
 * @author Your Name
 * @version 2.0
 *
 * Enhanced motor controller for L298N dual H-bridge motor driver.
 * Supports up to 2 DC motors with comprehensive control, logging,
 * and advanced features including acceleration curves, braking,
 * and detailed status monitoring.
 */

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "../config.h"
#include "../utils/Logger.h"
#include "../utils/Timer.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../core/DataLogger.h"

/**
 * @brief Motor direction enumeration
 */
enum MotorDirection
{
    STOP = 0,
    FORWARD = 1,
    BACKWARD = 2,
    BRAKE = 3
};

/**
 * @brief Motor state structure for tracking
 */
struct MotorState
{
    int speed;                // Current speed (0-255)
    MotorDirection direction; // Current direction
    unsigned long lastChange; // Timestamp of last change
    bool isRunning;           // Whether motor is currently active
    String lastCommand;       // Last command executed
    String motorName;         // Motor identifier
};

/**
 * @brief L298N Motor Controller Class
 *
 * Controls L298N dual H-bridge motor driver for DC motors.
 * Supports PWM speed control, direction control, and advanced features.
 */
class MotorController
{
private:
    // Motor 1 pins
    int motor1EnablePin;
    int motor1Input1Pin;
    int motor1Input2Pin;
    DataLogger dataLogger; // Added data logger for logging events

    // Motor 2 pins
    int motor2EnablePin;
    int motor2Input1Pin;
    int motor2Input2Pin;

    // Configuration
    int maxSpeed;
    int minSpeed;
    bool enableLogging;

    // State tracking
    MotorState motor1State;
    MotorState motor2State;

    // Timing for smooth acceleration
    Timer accelerationTimer;
    int targetSpeed1;
    int targetSpeed2;
    int currentSpeed1;
    int currentSpeed2;

    // Private helper methods
    void setMotorPins(int enablePin, int input1Pin, int input2Pin,
                      int speed, MotorDirection direction, MotorState &state);
    void updateMotorSpeed(int motorNum, int targetSpeed, MotorState &state);
    void logMotorAction(int motorNum, const String &action,
                        int speed = -1, MotorDirection direction = STOP);
    void validateSpeed(int &speed);

public:
    /**
     * @brief Constructor for dual motor control
     * @param motor1Enable Enable pin for motor 1 (PWM capable)
     * @param motor1Input1 Input 1 pin for motor 1
     * @param motor1Input2 Input 2 pin for motor 1
     * @param motor2Enable Enable pin for motor 2 (PWM capable)
     * @param motor2Input1 Input 1 pin for motor 2
     * @param motor2Input2 Input 2 pin for motor 2
     */
    MotorController(int motor1Enable, int motor1Input1, int motor1Input2,
                    int motor2Enable, int motor2Input1, int motor2Input2);

    // ─────────────────────────────────────────────────────────────────────
    // COMPATIBILITY METHODS FOR ACTUATOR MANAGER
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Compatibility method - set motor speed
     * @param speed Speed value
     */
    void setSpeed(int speed);

    /**
     * @brief Compatibility method - set motor direction
     * @param forward True for forward, false for backward
     */
    void setDirection(bool forward);

    /**
     * @brief Compatibility method - stop motor
     */
    void stop();

    /**
     * @brief Initialize the motor controller
     */
    bool begin();

    // ─────────────────────────────────────────────────────────────────────
    // BASIC MOTOR CONTROL
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Set motor 1 speed and direction
     * @param speed Speed (0-255)
     * @param direction Motor direction
     */
    void setMotor1(int speed, MotorDirection direction = FORWARD);

    /**
     * @brief Set motor 2 speed and direction
     * @param speed Speed (0-255)
     * @param direction Motor direction
     */
    void setMotor2(int speed, MotorDirection direction = FORWARD);

    /**
     * @brief Set both motors simultaneously
     * @param speed1 Speed for motor 1
     * @param direction1 Direction for motor 1
     * @param speed2 Speed for motor 2
     * @param direction2 Direction for motor 2
     */
    void setMotors(int speed1, MotorDirection direction1,
                   int speed2, MotorDirection direction2);

    /**
     * @brief Stop motor 1
     */
    void stopMotor1();

    /**
     * @brief Stop motor 2
     */
    void stopMotor2();

    /**
     * @brief Stop both motors
     */
    void stopAll();

    /**
     * @brief Brake motor 1 (short brake)
     */
    void brakeMotor1();

    /**
     * @brief Brake motor 2 (short brake)
     */
    void brakeMotor2();

    /**
     * @brief Brake both motors
     */
    void brakeAll();

    // ─────────────────────────────────────────────────────────────────────
    // ADVANCED MOTOR CONTROL
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Set motor 1 speed with smooth acceleration
     * @param targetSpeed Target speed (0-255)
     * @param accelerationRate Rate of acceleration (steps per update)
     */
    void setMotor1WithAcceleration(int targetSpeed, int accelerationRate = 10);

    /**
     * @brief Set motor 2 speed with smooth acceleration
     * @param targetSpeed Target speed (0-255)
     * @param accelerationRate Rate of acceleration (steps per update)
     */
    void setMotor2WithAcceleration(int targetSpeed, int accelerationRate = 10);

    /**
     * @brief Drive forward (both motors forward)
     * @param speed Motor speed
     */
    void forward(int speed = 200);

    /**
     * @brief Drive backward (both motors backward)
     * @param speed Motor speed
     */
    void backward(int speed = 200);

    /**
     * @brief Turn left (motor 1 backward, motor 2 forward)
     * @param speed Motor speed
     */
    void turnLeft(int speed = 150);

    /**
     * @brief Turn right (motor 1 forward, motor 2 backward)
     * @param speed Motor speed
     */
    void turnRight(int speed = 150);

    /**
     * @brief Spin left (both motors backward with different speeds)
     * @param speed1 Speed for motor 1
     * @param speed2 Speed for motor 2
     */
    void spinLeft(int speed1 = 100, int speed2 = 100);

    /**
     * @brief Spin right (both motors forward with different speeds)
     * @param speed1 Speed for motor 1
     * @param speed2 Speed for motor 2
     */
    void spinRight(int speed1 = 100, int speed2 = 100);

    // ─────────────────────────────────────────────────────────────────────
    // STATUS AND MONITORING
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Get motor 1 state
     * @return Current motor state
     */
    MotorState getMotor1State() const;

    /**
     * @brief Get motor 2 state
     * @return Current motor state
     */
    MotorState getMotor2State() const;

    /**
     * @brief Get motor state as JSON
     * @param motorNum Motor number (1 or 2)
     * @return JSON string with motor state
     */
    String getMotorStateJSON(int motorNum) const;

    /**
     * @brief Get complete status as JSON
     * @return JSON string with all motor states
     */
    String getStatusJSON() const;

    /**
     * @brief Update motor speeds for smooth acceleration
     */
    void update();

    // ─────────────────────────────────────────────────────────────────────
    // CONFIGURATION AND LOGGING
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Enable or disable logging
     * @param enable True to enable logging
     */
    void setLogging(bool enable);

    /**
     * @brief Set maximum motor speed
     * @param speed Maximum speed (0-255)
     */
    void setMaxSpeed(int speed);

    /**
     * @brief Set minimum motor speed
     * @param speed Minimum speed (0-255)
     */
    void setMinSpeed(int speed);

    /**
     * @brief Perform motor test sequence
     */
    void testMotors();

    // ─────────────────────────────────────────────────────────────────────
    // COMPATIBILITY METHODS FOR ACTUATOR MANAGER
    // ─────────────────────────────────────────────────────────────────────

    /**
     * @brief Compatibility method - get motor 1 speed
     * @return Current speed of motor 1
     */
    int getSpeed();

    /**
     * @brief Compatibility method - get motor 1 direction
     * @return True if motor 1 is moving forward
     */
    bool getDirection();

    /**
     * @brief Get motor direction from string
     * @param directionStr Direction as string ("forward", "backward", "stop", "brake")
     * @return MotorDirection enum value
     */
    static MotorDirection stringToDirection(const String &directionStr);

    /**
     * @brief Convert motor direction to string
     * @param direction Motor direction enum
     * @return String representation of direction
     */
    String directionToString(MotorDirection direction) const;
};

#endif // MOTOR_CONTROLLER_H