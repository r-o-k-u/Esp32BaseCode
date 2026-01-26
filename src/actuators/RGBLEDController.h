/**
 * @file RGBLEDController.h
 * @brief RGB LED control with color mixing and effects
 * @author Your Name
 * @version 2.0
 */

#ifndef RGB_LED_CONTROLLER_H
#define RGB_LED_CONTROLLER_H

#include "../config.h"
#include <Arduino.h>

class RGBLEDController
{
private:
    uint8_t redPin;
    uint8_t greenPin;
    uint8_t bluePin;

    bool initialized;
    bool state;

    // Current color values
    int redValue;
    int greenValue;
    int blueValue;
    int brightness;

    // Color transition
    bool transitioning;
    unsigned long transitionStart;
    unsigned long transitionDuration;
    int targetRed;
    int targetGreen;
    int targetBlue;

    // Animation effects
    int effectType;
    unsigned long effectTimer;
    int effectSpeed;
    int effectIntensity;

    // Color wheel
    int hue;
    int saturation;
    int value;

public:
    RGBLEDController(uint8_t rPin, uint8_t gPin, uint8_t bPin);

    bool begin();

    // Basic color control
    void setColor(int red, int green, int blue);
    void setColorHex(const String &hexColor);
    void setColorHSV(int hue, int saturation, int value);

    // Brightness control
    void setBrightness(int brightness);
    int getBrightness();

    // State control
    void setState(bool state);
    bool getState();

    // Color transitions
    void transitionToColor(int red, int green, int blue, unsigned long duration);
    void transitionToColorHex(const String &hexColor, unsigned long duration);
    void updateTransition();

    // Animation effects
    void startEffect(int effectType, int speed = 100, int intensity = 255);
    void stopEffect();
    void updateEffect();

    // Preset effects
    void rainbowCycle(int wait = 20);
    void colorWipe(int red, int green, int blue, int wait = 50);
    void theaterChase(int red, int green, int blue, int wait = 50);
    void theaterChaseRainbow(int wait = 50);
    void twinkle(int red, int green, int blue, int count = 10, int speed = 100);
    void twinkleRandom(int count = 10, int speed = 100);
    void sparkles(int red, int green, int blue, int count = 20);
    void fire(int cooling = 55, int sparking = 120, int speedDelay = 15);
    void lightning(int red, int green, int blue, int strikes = 3, int strikeDelay = 500, int flashDelay = 50);

    // Color utilities
    void fadeToBlack(int fadeRate = 10);
    void pulse(int red, int green, int blue, int duration = 1000);
    void breathe(int red, int green, int blue, int cycleTime = 2000);

    // Status and information
    String getColorStatus();
    int getRed();
    int getGreen();
    int getBlue();
    int getHue();
    int getSaturation();
    int getValue();

    // Configuration
    void setEffectSpeed(int speed);
    void setEffectIntensity(int intensity);

private:
    void applyColor();
    void setPinValue(uint8_t pin, int value);
    void rgbToHsv(int r, int g, int b, int &h, int &s, int &v);
    void hsvToRgb(int h, int s, int v, int &r, int &g, int &b);
    int easeInOutQuad(float t);
    void updateFireEffect();
    void updateLightningEffect();
};

// Effect type constants
#define EFFECT_NONE 0
#define EFFECT_RAINBOW 1
#define EFFECT_COLOR_WIPE 2
#define EFFECT_THEATER_CHASE 3
#define EFFECT_TWINKLE 4
#define EFFECT_SPARKLES 5
#define EFFECT_FIRE 6
#define EFFECT_LIGHTNING 7
#define EFFECT_BREATHE 8

#endif // RGB_LED_CONTROLLER_H
