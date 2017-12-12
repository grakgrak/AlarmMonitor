#include "Shared.h"

//--------------------------------------------------------------------
TDigitalPin::TDigitalPin(const String &name, uint8_t pin, uint8_t mode)
{
    _pin = pin;
    _name = name;
    _beepOnTrigger = true;
    pinMode(_pin, mode);
    _triggered = digitalRead(_pin);
}

//--------------------------------------------------------------------
void TDigitalPin::update()
{
    if (digitalRead(_pin) != _triggered)
    {
        _triggered = !_triggered;
        PublishStatus();
        if (_triggered && _beepOnTrigger)
            Beeper.Tone(1000, 30);
    }
}

//--------------------------------------------------------------------
void TDigitalPin::PublishStatus() 
{
    Mqtt.publish(ALARM_SENSOR + _name, state()); 
}
