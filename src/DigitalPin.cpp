#include "Shared.h"

//--------------------------------------------------------------------
TDigitalPin::TDigitalPin(const String &name, uint8_t pin, uint8_t mode, bool invert)
{
    _pin = pin;
    _name = name;
    _beepOnTrigger = true;
    _invert = invert;
    
    pinMode(_pin, mode);
    _pinLevel = digitalRead(_pin);
}

//--------------------------------------------------------------------
void TDigitalPin::update()
{
    if (digitalRead(_pin) != _pinLevel)    // edge detect
    {
        _pinLevel = !_pinLevel;
        PublishStatus();
        if (IsTriggered() && _beepOnTrigger)
            Beeper.Tone(1000, 30);
    }
}

//--------------------------------------------------------------------
void TDigitalPin::PublishStatus() 
{
    Mqtt.publish(ALARM_SENSOR + _name, state()); 
}
