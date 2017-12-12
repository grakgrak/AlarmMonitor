#include "Button.h"
#include "Shared.h"

#define DEBOUNCE_DELAY  50  // increase to reduce sensitivity

//--------------------------------------------------------------------
TButton::TButton(uint8_t pin, uint8_t mode)
{
    _lastDebounceTime = 0;
    _pin = pin;

    pinMode(_pin, mode);
    _state = digitalRead(_pin);
    _lastState = _state;
}


//--------------------------------------------------------------------
bool TButton::Check()
{
    int reading = digitalRead(_pin);

    // check to see if you just pressed the button
    // If the switch changed, due to noise or pressing:
    if (reading != _lastState)
        _lastDebounceTime = millis();        // reset the debouncing timer

    if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY)
    {
        // if the button state has changed:
        if (reading != _state)
        {
            _state = reading;
            return _state;
        }
    }
    _lastState = reading;  // save the button state for next time 
    return _state;
}