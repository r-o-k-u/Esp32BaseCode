/**
 * @file RGBLEDController.cpp
 * @brief RGB LED control with color mixing and effects implementation
 * @author Your Name
 * @version 2.0
 */

#include "RGBLEDController.h"
#include "../utils/Logger.h"
#include <algorithm>

RGBLEDController::RGBLEDController(uint8_t rPin, uint8_t gPin, uint8_t bPin)
    : redPin(rPin), greenPin(gPin), bluePin(bPin), initialized(false), state(false),
      redValue(0), greenValue(0), blueValue(0), brightness(255), transitioning(false),
      transitionStart(0), transitionDuration(0), targetRed(0), targetGreen(0), targetBlue(0),
      effectType(EFFECT_NONE), effectTimer(0), effectSpeed(100), effectIntensity(255),
      hue(0), saturation(100), value(100)
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    // Initialize with all LEDs off
    setPinValue(redPin, 0);
    setPinValue(greenPin, 0);
    setPinValue(bluePin, 0);
}

bool RGBLEDController::begin()
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    // Initialize with all LEDs off
    setPinValue(redPin, 0);
    setPinValue(greenPin, 0);
    setPinValue(bluePin, 0);

    initialized = true;
    state = false;
    redValue = greenValue = blueValue = 0;
    brightness = 255;

    DEBUG_PRINTLN("[RGB] RGB LED controller initialized on pins R:" + String(redPin) +
                  ", G:" + String(greenPin) + ", B:" + String(bluePin));

    return true;
}

void RGBLEDController::setColor(int red, int green, int blue)
{
    if (!initialized)
        return;

    // Constrain values to 0-255
    redValue = constrain(red, 0, 255);
    greenValue = constrain(green, 0, 255);
    blueValue = constrain(blue, 0, 255);

    // Convert to HSV for internal representation
    rgbToHsv(redValue, greenValue, blueValue, hue, saturation, value);

    applyColor();
    state = (redValue > 0 || greenValue > 0 || blueValue > 0);

    DEBUG_PRINT("[RGB] Color set to RGB(" + String(redValue) + ", " + String(greenValue) + ", " + String(blueValue) + ")");
    DEBUG_PRINTLN(" HSV(" + String(hue) + ", " + String(saturation) + ", " + String(value) + ")");
}

void RGBLEDController::setColorHex(const String &hexColor)
{
    if (!initialized)
        return;

    String color = hexColor;
    if (color.startsWith("#"))
        color = color.substring(1);

    if (color.length() == 6)
    {
        int red = (int)strtol(color.substring(0, 2).c_str(), NULL, 16);
        int green = (int)strtol(color.substring(2, 4).c_str(), NULL, 16);
        int blue = (int)strtol(color.substring(4, 6).c_str(), NULL, 16);

        setColor(red, green, blue);
    }
    else
    {
        DEBUG_PRINTLN("[RGB] Invalid hex color format: " + hexColor);
    }
}

void RGBLEDController::setColorHSV(int h, int s, int v)
{
    if (!initialized)
        return;

    hue = constrain(h, 0, 360);
    saturation = constrain(s, 0, 100);
    value = constrain(v, 0, 100);

    hsvToRgb(hue, saturation, value, redValue, greenValue, blueValue);
    applyColor();
    state = (redValue > 0 || greenValue > 0 || blueValue > 0);

    DEBUG_PRINT("[RGB] Color set to HSV(" + String(hue) + ", " + String(saturation) + ", " + String(value) + ")");
    DEBUG_PRINTLN(" RGB(" + String(redValue) + ", " + String(greenValue) + ", " + String(blueValue) + ")");
}

void RGBLEDController::setBrightness(int brightness)
{
    if (!initialized)
        return;

    this->brightness = constrain(brightness, 0, 255);

    // Recalculate current color with new brightness
    hsvToRgb(hue, saturation, map(this->brightness, 0, 255, 0, 100), redValue, greenValue, blueValue);
    applyColor();

    DEBUG_PRINTLN("[RGB] Brightness set to: " + String(this->brightness) + " (" + String((this->brightness * 100) / 255) + "%)");
}

int RGBLEDController::getBrightness()
{
    return brightness;
}

void RGBLEDController::setState(bool newState)
{
    state = newState;

    if (state)
    {
        applyColor();
    }
    else
    {
        setPinValue(redPin, 0);
        setPinValue(greenPin, 0);
        setPinValue(bluePin, 0);
        redValue = greenValue = blueValue = 0;
    }

    DEBUG_PRINTLN("[RGB] State set to: " + String(state ? "ON" : "OFF"));
}

bool RGBLEDController::getState()
{
    return state;
}

void RGBLEDController::transitionToColor(int red, int green, int blue, unsigned long duration)
{
    if (!initialized)
        return;

    targetRed = constrain(red, 0, 255);
    targetGreen = constrain(green, 0, 255);
    targetBlue = constrain(blue, 0, 255);

    transitioning = true;
    transitionStart = millis();
    transitionDuration = duration;

    DEBUG_PRINTLN("[RGB] Starting transition to RGB(" + String(targetRed) + ", " + String(targetGreen) + ", " + String(targetBlue) + ") over " + String(duration) + "ms");
}

void RGBLEDController::transitionToColorHex(const String &hexColor, unsigned long duration)
{
    if (!initialized)
        return;

    String color = hexColor;
    if (color.startsWith("#"))
        color = color.substring(1);

    if (color.length() == 6)
    {
        int red = (int)strtol(color.substring(0, 2).c_str(), NULL, 16);
        int green = (int)strtol(color.substring(2, 4).c_str(), NULL, 16);
        int blue = (int)strtol(color.substring(4, 6).c_str(), NULL, 16);

        transitionToColor(red, green, blue, duration);
    }
}

void RGBLEDController::updateTransition()
{
    if (!transitioning || !initialized)
        return;

    unsigned long elapsed = millis() - transitionStart;

    if (elapsed >= transitionDuration)
    {
        // Transition complete
        setColor(targetRed, targetGreen, targetBlue);
        transitioning = false;
        return;
    }

    // Calculate transition progress
    float progress = (float)elapsed / transitionDuration;
    progress = easeInOutQuad(progress);

    // Interpolate colors
    int currentRed = redValue + (targetRed - redValue) * progress;
    int currentGreen = greenValue + (targetGreen - greenValue) * progress;
    int currentBlue = blueValue + (targetBlue - blueValue) * progress;

    // Apply intermediate color
    setPinValue(redPin, map(currentRed, 0, 255, 0, brightness));
    setPinValue(greenPin, map(currentGreen, 0, 255, 0, brightness));
    setPinValue(bluePin, map(currentBlue, 0, 255, 0, brightness));
}

void RGBLEDController::startEffect(int effectType, int speed, int intensity)
{
    this->effectType = effectType;
    this->effectSpeed = speed;
    this->effectIntensity = intensity;
    effectTimer = millis();

    DEBUG_PRINTLN("[RGB] Starting effect " + String(effectType) + " (speed: " + String(speed) + ", intensity: " + String(intensity) + ")");
}

void RGBLEDController::stopEffect()
{
    effectType = EFFECT_NONE;
    setColor(0, 0, 0);

    DEBUG_PRINTLN("[RGB] Effect stopped");
}

void RGBLEDController::updateEffect()
{
    if (effectType == EFFECT_NONE || !initialized)
        return;

    unsigned long currentTime = millis();

    switch (effectType)
    {
    case EFFECT_RAINBOW:
        if (currentTime - effectTimer >= effectSpeed)
        {
            hue = (hue + 1) % 360;
            setColorHSV(hue, effectIntensity, 100);
            effectTimer = currentTime;
        }
        break;

    case EFFECT_COLOR_WIPE:
        // Simple color wipe effect
        if (currentTime - effectTimer >= effectSpeed)
        {
            // Toggle between current color and black
            static bool on = true;
            if (on)
            {
                setColor(redValue, greenValue, blueValue);
            }
            else
            {
                setColor(0, 0, 0);
            }
            on = !on;
            effectTimer = currentTime;
        }
        break;

    case EFFECT_FIRE:
        updateFireEffect();
        break;

    case EFFECT_LIGHTNING:
        updateLightningEffect();
        break;
    }
}

void RGBLEDController::rainbowCycle(int wait)
{
    for (int j = 0; j < 256; j++)
    {
        setColorHSV(j, 255, effectIntensity);
        delay(wait);
    }
}

void RGBLEDController::colorWipe(int red, int green, int blue, int wait)
{
    setColor(red, green, blue);
    delay(wait);
    setColor(0, 0, 0);
    delay(wait);
}

void RGBLEDController::theaterChase(int red, int green, int blue, int wait)
{
    for (int j = 0; j < 10; j++)
    {
        for (int q = 0; q < 3; q++)
        {
            setColor(red, green, blue);
            delay(wait);
            setColor(0, 0, 0);
            delay(wait);
        }
    }
}

void RGBLEDController::theaterChaseRainbow(int wait)
{
    for (int j = 0; j < 256; j += 10)
    {
        theaterChase(((j >> 3) & 31) * 8, ((j >> 8) & 31) * 8, ((j >> 16) & 31) * 8, wait);
    }
}

void RGBLEDController::twinkle(int red, int green, int blue, int count, int speed)
{
    for (int i = 0; i < count; i++)
    {
        setColor(red, green, blue);
        delay(random(50, speed));
        setColor(0, 0, 0);
        delay(random(50, speed));
    }
}

void RGBLEDController::twinkleRandom(int count, int speed)
{
    for (int i = 0; i < count; i++)
    {
        int r = random(255);
        int g = random(255);
        int b = random(255);
        twinkle(r, g, b, 1, speed);
    }
}

void RGBLEDController::sparkles(int red, int green, int blue, int count)
{
    for (int i = 0; i < count; i++)
    {
        setColor(red, green, blue);
        delay(random(10, 100));
        setColor(0, 0, 0);
        delay(random(10, 100));
    }
}

void RGBLEDController::fire(int cooling, int sparking, int speedDelay)
{
    // Fire effect implementation
    static byte heat[3];

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < 3; i++)
    {
        int coolAmount = random(0, ((cooling * 10) / 3) + 2);
        heat[i] = max(0, (int)heat[i] - coolAmount);
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    heat[2] = (heat[2] + heat[1] + heat[1]) / 3;
    heat[1] = (heat[1] + heat[0] + heat[0]) / 3;

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random(255) < sparking)
    {
        int y = random(3);
        int sparkAmount = random(160, 255);
        heat[y] = min(255, (int)heat[y] + sparkAmount);
    }

    // Step 4.  Map from heat cells to LED colors
    byte colorindex = heat[2];
    if (colorindex < 64)
    {
        setColor(colorindex * 4, 0, 0); // Red
    }
    else if (colorindex < 128)
    {
        setColor(255, (colorindex - 64) * 4, 0); // Orange/Yellow
    }
    else
    {
        setColor(255, 255, (colorindex - 128) * 4); // White/Yellow
    }

    delay(speedDelay);
}

void RGBLEDController::lightning(int red, int green, int blue, int strikes, int strikeDelay, int flashDelay)
{
    for (int i = 0; i < strikes; i++)
    {
        // Random delay between strikes
        delay(random(strikeDelay / 2, strikeDelay));

        // Flash sequence
        int flashes = random(2, 5);
        for (int j = 0; j < flashes; j++)
        {
            setColor(red, green, blue);
            delay(random(flashDelay / 2, flashDelay));
            setColor(0, 0, 0);
            delay(random(flashDelay / 2, flashDelay));
        }
    }
}

void RGBLEDController::fadeToBlack(int fadeRate)
{
    redValue = max(0, redValue - fadeRate);
    greenValue = max(0, greenValue - fadeRate);
    blueValue = max(0, blueValue - fadeRate);

    applyColor();
}

void RGBLEDController::pulse(int red, int green, int blue, int duration)
{
    unsigned long startTime = millis();
    int startRed = redValue;
    int startGreen = greenValue;
    int startBlue = blueValue;

    while (millis() - startTime < duration)
    {
        unsigned long elapsed = millis() - startTime;
        float progress = (float)elapsed / duration;

        // Sinusoidal pulse
        float pulse = (sin(progress * PI * 2) + 1) / 2;

        int currentRed = startRed + (red - startRed) * pulse;
        int currentGreen = startGreen + (green - startGreen) * pulse;
        int currentBlue = startBlue + (blue - startBlue) * pulse;

        setColor(currentRed, currentGreen, currentBlue);
        delay(20);
    }
}

void RGBLEDController::breathe(int red, int green, int blue, int cycleTime)
{
    unsigned long startTime = millis();

    while (millis() - startTime < cycleTime)
    {
        unsigned long elapsed = millis() - startTime;
        float progress = (float)elapsed / cycleTime;

        // Smooth breathing effect
        float breath = (sin(progress * PI * 2 - PI / 2) + 1) / 2;

        int currentRed = red * breath;
        int currentGreen = green * breath;
        int currentBlue = blue * breath;

        setColor(currentRed, currentGreen, currentBlue);
        delay(20);
    }
}

String RGBLEDController::getColorStatus()
{
    String status = "{\"rgb\":{";
    status += "\"state\":" + String(state ? "true" : "false") + ",";
    status += "\"color\":{\"r\":" + String(redValue) + ",\"g\":" + String(greenValue) + ",\"b\":" + String(blueValue) + "},";
    status += "\"hsv\":{\"h\":" + String(hue) + ",\"s\":" + String(saturation) + ",\"v\":" + String(value) + "},";
    status += "\"brightness\":" + String(brightness) + ",";
    status += "\"effect\":" + String(effectType);
    status += "}}";
    return status;
}

int RGBLEDController::getRed()
{
    return redValue;
}

int RGBLEDController::getGreen()
{
    return greenValue;
}

int RGBLEDController::getBlue()
{
    return blueValue;
}

int RGBLEDController::getHue()
{
    return hue;
}

int RGBLEDController::getSaturation()
{
    return saturation;
}

int RGBLEDController::getValue()
{
    return value;
}

void RGBLEDController::setEffectSpeed(int speed)
{
    effectSpeed = constrain(speed, 10, 1000);
    DEBUG_PRINTLN("[RGB] Effect speed set to: " + String(effectSpeed));
}

void RGBLEDController::setEffectIntensity(int intensity)
{
    effectIntensity = constrain(intensity, 0, 255);
    DEBUG_PRINTLN("[RGB] Effect intensity set to: " + String(effectIntensity));
}

void RGBLEDController::applyColor()
{
    if (!state)
    {
        setPinValue(redPin, 0);
        setPinValue(greenPin, 0);
        setPinValue(bluePin, 0);
        return;
    }

    setPinValue(redPin, map(redValue, 0, 255, 0, brightness));
    setPinValue(greenPin, map(greenValue, 0, 255, 0, brightness));
    setPinValue(bluePin, map(blueValue, 0, 255, 0, brightness));
}

void RGBLEDController::setPinValue(uint8_t pin, int value)
{
    analogWrite(pin, value);
}

void RGBLEDController::rgbToHsv(int r, int g, int b, int &h, int &s, int &v)
{
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float maxVal = max(max(rf, gf), bf);
    float minVal = min(min(rf, gf), bf);
    float delta = maxVal - minVal;

    v = maxVal * 100;

    if (maxVal == 0)
    {
        s = 0;
    }
    else
    {
        s = (delta / maxVal) * 100;
    }

    if (delta == 0)
    {
        h = 0;
    }
    else
    {
        if (rf == maxVal)
        {
            h = 60 * fmod(((gf - bf) / delta), 6);
        }
        else if (gf == maxVal)
        {
            h = 60 * (((bf - rf) / delta) + 2);
        }
        else
        {
            h = 60 * (((rf - gf) / delta) + 4);
        }
    }

    h = constrain(h, 0, 360);
    s = constrain(s, 0, 100);
    v = constrain(v, 0, 100);
}

void RGBLEDController::hsvToRgb(int h, int s, int v, int &r, int &g, int &b)
{
    float hf = h / 360.0f;
    float sf = s / 100.0f;
    float vf = v / 100.0f;

    float c = vf * sf;
    float x = c * (1 - abs(fmod(hf * 6, 2) - 1));
    float m = vf - c;

    float rf, gf, bf;

    if (hf < 1.0 / 6)
    {
        rf = c;
        gf = x;
        bf = 0;
    }
    else if (hf < 2.0 / 6)
    {
        rf = x;
        gf = c;
        bf = 0;
    }
    else if (hf < 3.0 / 6)
    {
        rf = 0;
        gf = c;
        bf = x;
    }
    else if (hf < 4.0 / 6)
    {
        rf = 0;
        gf = x;
        bf = c;
    }
    else if (hf < 5.0 / 6)
    {
        rf = x;
        gf = 0;
        bf = c;
    }
    else
    {
        rf = c;
        gf = 0;
        bf = x;
    }

    r = (rf + m) * 255;
    g = (gf + m) * 255;
    b = (bf + m) * 255;

    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
}

int RGBLEDController::easeInOutQuad(float t)
{
    return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

void RGBLEDController::updateFireEffect()
{
    // Simplified fire effect update
    static int fireTimer = 0;

    if (millis() - fireTimer > 50)
    {
        fire(55, 120, 15);
        fireTimer = millis();
    }
}

void RGBLEDController::updateLightningEffect()
{
    // Lightning effect is handled in the main lightning() function
    // This is a placeholder for continuous lightning effects
}
