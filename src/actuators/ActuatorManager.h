/**
 * @file ActuatorManager.h
 * @brief Actuator management and control system
 * @author Your Name
 * @version 2.0
 */

#ifndef ACTUATOR_MANAGER_H
#define ACTUATOR_MANAGER_H

#include "../config.h"
#include "LEDController.h"
#include "BuzzerController.h"
#include "MotorController.h"
#include "RGBLEDController.h"
#include "RelayController.h"
#include "ServoController.h"
#include "../utils/JSONHelper.h"
#include "../utils/Logger.h"

class ActuatorManager
{
private:
    LEDController *ledController;
    BuzzerController *buzzerController;
    MotorController *motorController;
    RGBLEDController *rgbController;
    RelayController *relayController;
    ServoController *servoController;

    bool initialized;

public:
    ActuatorManager();
    ~ActuatorManager();

    bool begin();

    // LED Control
    void setLED(bool state);
    bool getLED();

    // Buzzer Control
    void setBuzzer(bool state);
    void playTone(int frequency, int duration = 0);
    void playMelody(const int *notes, const int *durations, int length);

    // Motor Control
    void setMotorSpeed(int speed);
    void setMotorDirection(bool forward);
    void stopMotor();
    int getSpeed();
    bool getDirection();

    // RGB LED Control
    void setRGBColor(int red, int green, int blue);
    void setRGBColorHex(const String &hexColor);
    void setRGBBrightness(int brightness);
    void rainbowCycle(int wait);

    // Relay Control
    void setRelay(int relay, bool state);
    bool getRelay(int relay);
    void toggleRelay(int relay);
    void pulseRelay(int relay, int duration);

    // Servo Control
    void setServoAngle(int servo, int angle);
    void setServo(int servo, int angle);
    int getServoAngle(int servo);
    void sweepServo(int servo, int startAngle, int endAngle, int speed);

    // Scene Management
    void executeScene(const String &sceneName);
    void emergencyStop();
    void triggerAlert();
    void setActuator(const String &actuatorName, int value);

    // Status and Configuration
    String getStatus();
    bool saveConfiguration();
    bool loadConfiguration();
    void loadDefaultConfiguration();
    void update();

private:
    void initializeActuators();
    void executeSceneInternal(const String &sceneName);
};

#endif // ACTUATOR_MANAGER_H
