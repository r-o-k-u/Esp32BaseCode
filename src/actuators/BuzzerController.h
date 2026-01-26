/**
 * @file BuzzerController.h
 * @brief Buzzer control module
 * @author Your Name
 * @version 2.0
 */

#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#include "../config.h"
#include <Arduino.h>

class BuzzerController
{
private:
    uint8_t pin;
    bool state;
    int currentFrequency;
    unsigned long toneStartTime;
    unsigned long toneDuration;

    // Melody definitions
    struct Note
    {
        int frequency;
        int duration;
    };

public:
    BuzzerController(uint8_t buzzerPin);

    bool begin();
    void setState(bool state);
    bool getState();

    // Tone control
    void playTone(int frequency, int duration = 0);
    void playNote(int frequency, int duration);
    void stopTone();

    // Melody control
    void playMelody(const int *notes, const int *durations, int length);
    void playBeep(int frequency = 1000, int duration = 500);
    void playErrorSound();
    void playSuccessSound();
    void playAlertSound();

    // PWM control for volume
    void setVolume(int dutyCycle);
    void setFrequency(int frequency);

    // Pattern control
    void playPattern(const char *pattern);
    void beepSequence(int count, int interval = 200);
    void sirenSound(int duration = 2000);

    // Status and timing
    bool isPlaying();
    unsigned long getPlayTime();
    void update();

private:
    void setPWMFrequency(int frequency);
    void playNoteInternal(int frequency, int duration);
    void parsePattern(const char *pattern);
};

#endif // BUZZER_CONTROLLER_H
