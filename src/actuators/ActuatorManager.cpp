/**
 * @file ActuatorManager.cpp
 * @brief Actuator management and control system implementation
 * @author Your Name
 * @version 2.0
 */

#include "ActuatorManager.h"
#include "../utils/JSONHelper.h"

// Global instance
ActuatorManager actuatorManager;

ActuatorManager::ActuatorManager()
    : ledController(nullptr), buzzerController(nullptr), motorController(nullptr),
      rgbController(nullptr), relayController(nullptr), servoController(nullptr),
      initialized(false)
{
}

ActuatorManager::~ActuatorManager()
{
    if (ledController)
        delete ledController;
    if (buzzerController)
        delete buzzerController;
    if (motorController)
        delete motorController;
    if (rgbController)
        delete rgbController;
    if (relayController)
        delete relayController;
    if (servoController)
        delete servoController;
}

bool ActuatorManager::begin()
{
    DEBUG_PRINTLN("[ACTUATOR] Initializing Actuator Manager...");

    initializeActuators();
    loadDefaultConfiguration();

    initialized = true;
    DEBUG_PRINTLN("[ACTUATOR] Actuator Manager initialized successfully");

    return true;
}

void ActuatorManager::initializeActuators()
{
    // Initialize LED Controller
    ledController = new LEDController(LED_PIN);
    if (ledController->begin())
    {
        DEBUG_PRINTLN("[ACTUATOR] LED Controller initialized");
    }

    // Initialize Buzzer Controller
    buzzerController = new BuzzerController(BUZZER_PIN);
    if (buzzerController->begin())
    {
        DEBUG_PRINTLN("[ACTUATOR] Buzzer Controller initialized");
    }

    // Initialize Motor Controller
    motorController = new MotorController(MOTOR1_IN1, MOTOR1_IN2, MOTOR1_EN);
    if (motorController->begin())
    {
        DEBUG_PRINTLN("[ACTUATOR] Motor Controller initialized");
    }

    // Initialize RGB LED Controller
    rgbController = new RGBLEDController(RGB_R_PIN, RGB_G_PIN, RGB_B_PIN);
    if (rgbController->begin())
    {
        DEBUG_PRINTLN("[ACTUATOR] RGB LED Controller initialized");
    }

    // Initialize Relay Controller
    relayController = new RelayController();
    if (relayController->begin())
    {
        DEBUG_PRINTLN("[ACTUATOR] Relay Controller initialized");
    }

    // Initialize Servo Controller
    servoController = new ServoController();
    if (servoController->begin())
    {
        DEBUG_PRINTLN("[ACTUATOR] Servo Controller initialized");
    }
}

// LED Control
void ActuatorManager::setLED(bool state)
{
    if (ledController)
    {
        ledController->setState(state);
    }
}

bool ActuatorManager::getLED()
{
    if (ledController)
    {
        return ledController->getState();
    }
    return false;
}

// Buzzer Control
void ActuatorManager::setBuzzer(bool state)
{
    if (buzzerController)
    {
        buzzerController->setState(state);
    }
}

void ActuatorManager::playTone(int frequency, int duration)
{
    if (buzzerController)
    {
        buzzerController->playTone(frequency, duration);
    }
}

void ActuatorManager::playMelody(const int *notes, const int *durations, int length)
{
    if (buzzerController)
    {
        buzzerController->playMelody(notes, durations, length);
    }
}

// Motor Control
void ActuatorManager::setMotorSpeed(int speed)
{
    if (motorController)
    {
        motorController->setSpeed(speed);
    }
}

void ActuatorManager::setMotorDirection(bool forward)
{
    if (motorController)
    {
        motorController->setDirection(forward);
    }
}

void ActuatorManager::stopMotor()
{
    if (motorController)
    {
        motorController->stop();
    }
}

int ActuatorManager::getSpeed()
{
    if (motorController)
    {
        return motorController->getSpeed();
    }
    return 0;
}

bool ActuatorManager::getDirection()
{
    if (motorController)
    {
        return motorController->getDirection();
    }
    return false;
}

// RGB LED Control
void ActuatorManager::setRGBColor(int red, int green, int blue)
{
    if (rgbController)
    {
        rgbController->setColor(red, green, blue);
    }
}

void ActuatorManager::setRGBColorHex(const String &hexColor)
{
    if (rgbController)
    {
        rgbController->setColorHex(hexColor);
    }
}

void ActuatorManager::setRGBBrightness(int brightness)
{
    if (rgbController)
    {
        rgbController->setBrightness(brightness);
    }
}

void ActuatorManager::rainbowCycle(int wait)
{
    if (rgbController)
    {
        rgbController->rainbowCycle(wait);
    }
}

// Relay Control
void ActuatorManager::setRelay(int relay, bool state)
{
    if (relayController)
    {
        relayController->setState(relay, state);
    }
}

bool ActuatorManager::getRelay(int relay)
{
    if (relayController)
    {
        return relayController->getState(relay);
    }
    return false;
}

void ActuatorManager::toggleRelay(int relay)
{
    if (relayController)
    {
        relayController->toggle(relay);
    }
}

void ActuatorManager::pulseRelay(int relay, int duration)
{
    if (relayController)
    {
        relayController->pulse(relay, duration);
    }
}

// Servo Control
void ActuatorManager::setServoAngle(int servo, int angle)
{
    if (servoController)
    {
        servoController->setAngle(servo, angle);
    }
}

void ActuatorManager::setServo(int servo, int angle)
{
    if (servoController)
    {
        servoController->setAngle(servo, angle);
    }
}

int ActuatorManager::getServoAngle(int servo)
{
    if (servoController)
    {
        return servoController->getAngle(servo);
    }
    return 0;
}

void ActuatorManager::sweepServo(int servo, int startAngle, int endAngle, int speed)
{
    if (servoController)
    {
        servoController->sweep(servo, startAngle, endAngle, speed);
    }
}

// Scene Management
void ActuatorManager::executeScene(const String &sceneName)
{
    executeSceneInternal(sceneName);
}

void ActuatorManager::emergencyStop()
{
    DEBUG_PRINTLN("[ACTUATOR] Emergency stop triggered");

    if (ledController)
        ledController->setState(false);
    if (buzzerController)
        buzzerController->setState(false);
    if (motorController)
        motorController->stop();
    if (rgbController)
        rgbController->setColor(0, 0, 0);
    if (relayController)
    {
        relayController->allOff();
    }
    if (servoController)
        servoController->setAngle(1, 90);
    servoController->setAngle(2, 90);
}

void ActuatorManager::triggerAlert()
{
    DEBUG_PRINTLN("[ACTUATOR] Triggering alert");

    // Flash red lights and sound buzzer
    setRGBColor(255, 0, 0);
    setBuzzer(true);
    delay(1000);
    setRGBColor(0, 0, 0);
    setBuzzer(false);
}

void ActuatorManager::update()
{
    // Update any running animations or effects
    if (rgbController)
    {
        rgbController->updateEffect();
    }
}

void ActuatorManager::setActuator(const String &actuatorName, int value)
{
    if (actuatorName == "led" || actuatorName == "LED")
    {
        setLED(value > 0);
    }
    else if (actuatorName == "buzzer")
    {
        setBuzzer(value > 0);
    }
    else if (actuatorName == "motor")
    {
        setMotorSpeed(value);
    }
    else if (actuatorName == "relay1" || actuatorName == "relay")
    {
        setRelay(1, value > 0);
    }
    else if (actuatorName == "relay2")
    {
        setRelay(2, value > 0);
    }
    else if (actuatorName == "relay3")
    {
        setRelay(3, value > 0);
    }
    else if (actuatorName == "servo1" || actuatorName == "servo")
    {
        setServoAngle(1, value);
    }
    else if (actuatorName == "servo2")
    {
        setServoAngle(2, value);
    }
    else if (actuatorName == "rgb")
    {
        // Simple RGB control - map value 0-255 to colors
        if (value == 0)
            setRGBColor(0, 0, 0); // Off
        else if (value < 85)
            setRGBColor(255, 0, 0); // Red
        else if (value < 170)
            setRGBColor(0, 255, 0); // Green
        else
            setRGBColor(0, 0, 255); // Blue
    }
}

// Status and Configuration
String ActuatorManager::getStatus()
{
    String status = "{\"actuators\":{";

    if (ledController)
    {
        status += "\"led\":" + String(ledController->getState() ? "true" : "false") + ",";
    }

    if (buzzerController)
    {
        status += "\"buzzer\":" + String(buzzerController->getState() ? "true" : "false") + ",";
    }

    if (motorController)
    {
        status += "\"motor\":{\"speed\":" + String(motorController->getSpeed()) +
                  ",\"direction\":" + String(motorController->getDirection() ? "true" : "false") + "},";
    }

    if (rgbController)
    {
        status += "\"rgb\":{\"r\":" + String(rgbController->getRed()) +
                  ",\"g\":" + String(rgbController->getGreen()) +
                  ",\"b\":" + String(rgbController->getBlue()) +
                  ",\"brightness\":" + String(rgbController->getBrightness()) + "},";
    }

    if (relayController)
    {
        status += "\"relays\":[";
        for (int i = 1; i <= 3; i++)
        {
            status += String(relayController->getState(i) ? "true" : "false");
            if (i < 3)
                status += ",";
        }
        status += "],";
    }

    if (servoController)
    {
        status += "\"servo\":" + String(servoController->getAngle(1)) + ",";
    }

    // Remove trailing comma if present
    if (status.endsWith(","))
    {
        status = status.substring(0, status.length() - 1);
    }

    status += "}}";
    return status;
}

bool ActuatorManager::saveConfiguration()
{
    if (!initialized)
        return false;

    String config = getStatus();
    DEBUG_PRINTLN("[ACTUATOR] Configuration saved: " + config);
    return true;
}

bool ActuatorManager::loadConfiguration()
{
    if (!initialized)
        return false;

    DEBUG_PRINTLN("[ACTUATOR] Configuration loaded (placeholder)");
    return true;
}

void ActuatorManager::loadDefaultConfiguration()
{
    // Set default states
    if (ledController)
        ledController->setState(false);
    if (buzzerController)
        buzzerController->setState(false);
    if (motorController)
        motorController->stop();
    if (rgbController)
        rgbController->setColor(0, 0, 0);
    if (relayController)
        relayController->allOff();
    if (servoController)
        servoController->setAngle(1, 90);
    servoController->setAngle(2, 90);

    DEBUG_PRINTLN("[ACTUATOR] Default configuration loaded");
}

void ActuatorManager::executeSceneInternal(const String &sceneName)
{
    if (sceneName == "welcome")
    {
        // Welcome sequence
        setLED(true);
        setRGBColor(0, 255, 0); // Green
        playTone(1000, 500);
        delay(1000);
        setLED(false);
        setRGBColor(0, 0, 0);
    }
    else if (sceneName == "alert")
    {
        // Alert sequence
        setRGBColor(255, 0, 0); // Red
        setBuzzer(true);
        pulseRelay(1, 1000);
        delay(2000);
        setRGBColor(0, 0, 0);
        setBuzzer(false);
    }
    else if (sceneName == "rainbow")
    {
        // Rainbow effect
        rainbowCycle(20);
    }
    else
    {
        DEBUG_PRINTLN("[ACTUATOR] Unknown scene: " + sceneName);
    }
}
