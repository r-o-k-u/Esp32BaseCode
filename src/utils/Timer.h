/**
 * @file Timer.h
 * @brief Simple non-blocking timer utility
 */

#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer
{
private:
    uint32_t interval;
    uint32_t lastTime;

public:
    Timer(uint32_t intervalMs) : interval(intervalMs), lastTime(0) {}

    bool isReady()
    {
        uint32_t currentTime = millis();
        if (currentTime - lastTime >= interval)
        {
            lastTime = currentTime;
            return true;
        }
        return false;
    }

    void reset()
    {
        lastTime = millis();
    }

    void setInterval(uint32_t intervalMs)
    {
        interval = intervalMs;
    }

    uint32_t getElapsed()
    {
        return millis() - lastTime;
    }
};

#endif
