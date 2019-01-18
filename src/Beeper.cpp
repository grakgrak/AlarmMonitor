#include "Shared.h"

#define CHANNEL 0
#define DUTY_CYCLE 125
#define RESOLUTION 8

//--------------------------------------------------------------------
TBeeper Beeper; // Global Instance

//--------------------------------------------------------------------
TBeeper::TBeeper()
{
    _beepState = IDLE;
    _toneState = IDLE;

    ledcSetup(CHANNEL, 2000, RESOLUTION);
    ledcAttachPin(TONE_PIN, CHANNEL);
    ledcWrite(CHANNEL, 0);
    ledcWriteTone(CHANNEL, 0); // turn tone off

    pinMode(BEEP_PIN, OUTPUT);
    digitalWrite(BEEP_PIN, LOW);
}

//--------------------------------------------------------------------
TBeeper::TState TBeeper::_TONE_ON(bool stateChanged)
{
    static unsigned long timeout;

    if (stateChanged)
    {
        ledcWrite(CHANNEL, DUTY_CYCLE);
        ledcWriteTone(CHANNEL, _freq); // turn tone on

        timeout = millis();
    }

    if ((millis() - timeout) >= _duration)
        return OFF;

    return ON;
}

//--------------------------------------------------------------------
TBeeper::TState TBeeper::_TONE_OFF(bool stateChanged)
{
    ledcWrite(CHANNEL, 0);
    ledcWriteTone(CHANNEL, 0); // turn tone off

    return IDLE;
}

//--------------------------------------------------------------------
void TBeeper::Tone(int freq, int duration)
{
    _freq = freq;
    _duration = duration;
    _toneState = IDLE;
    setToneState(duration == 0 ? OFF : ON);
}

//--------------------------------------------------------------------
void TBeeper::setToneState(TState state)
{
    _toneStateChanged = (state != _toneState);
    _toneState = state;
}

//--------------------------------------------------------------------
void TBeeper::toneLoop()
{
    switch ((int)_toneState)
    {
    case IDLE:
        break;
    case ON:
        setToneState(_TONE_ON(_toneStateChanged));
        break;
    case OFF:
        setToneState(_TONE_OFF(_toneStateChanged));
        break;
    }
}

//--------------------------------------------------------------------
TBeeper::TState TBeeper::_BEEP_ON(bool stateChanged)
{
    static unsigned long timeout;

    if (stateChanged)
    {
        digitalWrite(BEEP_PIN, HIGH);
        timeout = millis();
    }

    if ((millis() - timeout) >= _beepOn)
        return OFF;

    return ON;
}

//--------------------------------------------------------------------
TBeeper::TState TBeeper::_BEEP_OFF(bool stateChanged)
{
    static unsigned long timeout;

    if (stateChanged)
    {
        digitalWrite(BEEP_PIN, LOW);

        if (--_beepRepeat <= 0)
            return IDLE;

        timeout = millis();
    }

    if ((millis() - timeout) >= _beepOff)
        return ON;

    return OFF;
}

//--------------------------------------------------------------------
void TBeeper::setBeepState(TState state)
{
    _beepStateChanged = (state != _beepState);
    _beepState = state;
}

//--------------------------------------------------------------------
void TBeeper::beepLoop()
{
    switch ((int)_beepState)
    {
    case IDLE:
        break;
    case ON:
        setBeepState(_BEEP_ON(_beepStateChanged));
        break;
    case OFF:
        setBeepState(_BEEP_OFF(_beepStateChanged));
        break;
    }
}

//--------------------------------------------------------------------
void TBeeper::Beep(int onMillis, int offMillis, int repeat)
{
    _beepOn = onMillis;
    _beepOff = offMillis;
    _beepRepeat = repeat;
    _beepState = IDLE;
    setBeepState(repeat == 0 ? OFF : ON);
}

//--------------------------------------------------------------------
void TBeeper::loop()
{
    toneLoop();
    beepLoop();
}
