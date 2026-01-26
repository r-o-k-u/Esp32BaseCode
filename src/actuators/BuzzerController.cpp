/**
 * @file BuzzerController.cpp
 * @brief Buzzer control module implementation
 * @author Your Name
 * @version 2.0
 */

#include "BuzzerController.h"
#include "../utils/Logger.h"

BuzzerController::BuzzerController(uint8_t buzzerPin)
    : pin(buzzerPin), state(false), currentFrequency(0),
      toneStartTime(0), toneDuration(0)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

bool BuzzerController::begin()
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;

    DEBUG_PRINTLN("[BUZZER] Buzzer controller initialized on pin " + String(pin));
    return true;
}

void BuzzerController::setState(bool newState)
{
    state = newState;
    if (state)
    {
        digitalWrite(pin, HIGH);
    }
    else
    {
        digitalWrite(pin, LOW);
        currentFrequency = 0;
    }
}

bool BuzzerController::getState()
{
    return state;
}

void BuzzerController::playTone(int frequency, int duration)
{
    if (frequency <= 0)
        return;

    currentFrequency = frequency;
    toneStartTime = millis();
    toneDuration = duration;
    state = true;

    if (duration > 0)
    {
        // Use tone() for timed tones
        tone(pin, frequency, duration);
    }
    else
    {
        // Use analogWrite for continuous tone
        setPWMFrequency(frequency);
        analogWrite(pin, 128); // 50% duty cycle
    }

    DEBUG_PRINT("[BUZZER] Playing tone: " + String(frequency) + "Hz");
    if (duration > 0)
    {
        DEBUG_PRINTLN(" for " + String(duration) + "ms");
    }
    else
    {
        DEBUG_PRINTLN(" (continuous)");
    }
}

void BuzzerController::playNote(int frequency, int duration)
{
    playTone(frequency, duration);
}

void BuzzerController::stopTone()
{
    noTone(pin);
    analogWrite(pin, 0);
    state = false;
    currentFrequency = 0;
    toneDuration = 0;

    DEBUG_PRINTLN("[BUZZER] Tone stopped");
}

void BuzzerController::playMelody(const int *notes, const int *durations, int length)
{
    if (!notes || !durations || length <= 0)
        return;

    DEBUG_PRINTLN("[BUZZER] Playing melody with " + String(length) + " notes");

    for (int i = 0; i < length; i++)
    {
        if (notes[i] == 0)
        {
            // Rest
            delay(durations[i]);
        }
        else
        {
            playNote(notes[i], durations[i]);
            delay(durations[i] + 50); // Small gap between notes
        }
    }
}

void BuzzerController::playBeep(int frequency, int duration)
{
    playTone(frequency, duration);
    delay(duration + 100);
}

void BuzzerController::playErrorSound()
{
    // Three short beeps
    for (int i = 0; i < 3; i++)
    {
        playBeep(500, 100);
        delay(100);
    }
}

void BuzzerController::playSuccessSound()
{
    // Ascending tone
    playBeep(800, 100);
    delay(50);
    playBeep(1000, 100);
    delay(50);
    playBeep(1200, 200);
}

void BuzzerController::playAlertSound()
{
    // Siren-like sound
    for (int i = 0; i < 5; i++)
    {
        playTone(800, 200);
        delay(50);
        playTone(1200, 200);
        delay(50);
    }
}

void BuzzerController::setVolume(int dutyCycle)
{
    // Limit duty cycle to 0-255
    dutyCycle = constrain(dutyCycle, 0, 255);

    if (currentFrequency > 0)
    {
        setPWMFrequency(currentFrequency);
        analogWrite(pin, dutyCycle);
    }

    DEBUG_PRINTLN("[BUZZER] Volume set to: " + String(dutyCycle));
}

void BuzzerController::setFrequency(int frequency)
{
    currentFrequency = frequency;
    if (state && frequency > 0)
    {
        setPWMFrequency(frequency);
        analogWrite(pin, 128);
    }

    DEBUG_PRINTLN("[BUZZER] Frequency set to: " + String(frequency) + "Hz");
}

void BuzzerController::playPattern(const char *pattern)
{
    if (!pattern)
        return;

    DEBUG_PRINTLN("[BUZZER] Playing pattern: " + String(pattern));

    // Simple pattern parser
    // B = beep, S = short pause, L = long pause
    for (int i = 0; pattern[i] != '\0'; i++)
    {
        switch (pattern[i])
        {
        case 'B':
        case 'b':
            playBeep();
            break;
        case 'S':
        case 's':
            delay(200);
            break;
        case 'L':
        case 'l':
            delay(500);
            break;
        case ' ':
            delay(100);
            break;
        }
    }
}

void BuzzerController::beepSequence(int count, int interval)
{
    DEBUG_PRINTLN("[BUZZER] Beeping sequence: " + String(count) + " times");

    for (int i = 0; i < count; i++)
    {
        playBeep();
        if (i < count - 1)
        {
            delay(interval);
        }
    }
}

void BuzzerController::sirenSound(int duration)
{
    unsigned long startTime = millis();
    int direction = 1;
    int frequency = 500;

    DEBUG_PRINTLN("[BUZZER] Siren sound for " + String(duration) + "ms");

    while (millis() - startTime < duration)
    {
        playTone(frequency, 50);
        delay(50);

        frequency += direction * 50;
        if (frequency >= 1500)
            direction = -1;
        if (frequency <= 500)
            direction = 1;
    }

    stopTone();
}

bool BuzzerController::isPlaying()
{
    if (toneDuration == 0)
        return state;

    return (millis() - toneStartTime) < toneDuration;
}

unsigned long BuzzerController::getPlayTime()
{
    if (toneStartTime == 0)
        return 0;
    return millis() - toneStartTime;
}

void BuzzerController::update()
{
    // Check if timed tone has expired
    if (toneDuration > 0 && (millis() - toneStartTime) >= toneDuration)
    {
        stopTone();
    }
}

void BuzzerController::setPWMFrequency(int frequency)
{
    // Configure PWM for buzzer
    // This is a simplified implementation
    // For precise frequency control, you might need to use specific timers

    // ESP32 PWM configuration
    ledcSetup(0, frequency, 8); // Channel 0, 8-bit resolution
    ledcAttachPin(pin, 0);
}

void BuzzerController::playNoteInternal(int frequency, int duration)
{
    playTone(frequency, duration);
}

void BuzzerController::parsePattern(const char *pattern)
{
    // Pattern parsing is handled in playPattern()
    // This method is kept for future expansion
}
