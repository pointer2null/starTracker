#include "GButton.h"

GButton::GButton(int pin, int pinmode)
{
    pinMode = pinMode;
    attachedPin = pin;
    callbackHandler = nullptr;
    firstThresholdMs = 800;
    secondThreasholdMs = 3000;
}

GButton::GButton(int pin, int pinmode, void (*handler)(uint8_t button_flags))
{
    pinMode = pinMode;
    attachedPin = pin;
    callbackHandler = handler;
    firstThresholdMs = 800;
    secondThreasholdMs = 3000;
}

GButton::GButton(int pin, int pinmode, void (*handler)(uint8_t button_flags), uint16_t firstThresholdMs, uint16_t secondThreasholdMs)
{
    pinMode = pinMode;
    attachedPin = pin;
    callbackHandler = handler;
    firstThresholdMs = firstThresholdMs;
    secondThreasholdMs = secondThreasholdMs;
}