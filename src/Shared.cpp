#include "Shared.h"

//--------------------------------------------------------------------

TDigitalPin Sensor1("pir1", SENSOR1_PIN, INPUT, true); // GPIO36 input only no PU/PD - using external PU resistor
TDigitalPin Sensor2("pir2", SENSOR2_PIN, INPUT, true); // GPIO39 input only no PU/PD - using external PU resistor
TDigitalPin Sensor3("pir3", SENSOR3_PIN, INPUT, true); // GPIO34 input only no PU/PD - using external PU resistor
TDigitalPin Sensor4("pir4", SENSOR4_PIN, INPUT, true); // GPIO35 input only no PU/PD - using external PU resistor
TDigitalPin Sensor5("pir5", SENSOR5_PIN, INPUT_PULLUP, true);
TDigitalPin Sensor6("pir6", SENSOR6_PIN, INPUT_PULLUP, false);
TDigitalPin Sensor7("pir7", SENSOR7_PIN, INPUT_PULLUP, true);
TDigitalPin SensorDoor("door", SENSOR8_PIN, INPUT_PULLUP, false);

TButton OkButton(BUTTON_OK_PIN, INPUT_PULLDOWN);
TButton CancelButton(BUTTON_CANCEL_PIN, INPUT_PULLDOWN);
bool _okButtonState = true;
bool _cancelButtonState = true;


unsigned long _sysTime = 0;      // time in secs since 1 Jan 1970
unsigned long _sysStartTime = 0; // time in secs since 1 Jan 1970 - when started
unsigned long _prevMillis = 0;

bool _backlight = true;

//--------------------------------------------------------------------
unsigned long Now()
{
  while (millis() - _prevMillis >= 1000) 
  {
    // millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
    _sysTime++;
    _prevMillis += 1000;	
  }
  return _sysTime;
}
//--------------------------------------------------------------------
unsigned long startTime()
{
    return _sysStartTime;
}
//--------------------------------------------------------------------
unsigned long upTime()
{
    return Now() - _sysStartTime;    
}
//--------------------------------------------------------------------
void updateSysTime(unsigned long dt)
{
    _prevMillis = millis();  // restart counting from now

    _sysTime = dt;  // current number of secs since 1 Jan 1970

    if( _sysStartTime == 0)
        _sysStartTime = dt - (_prevMillis / 1000); // time in secs when started - adjusted for current runtime

    Debug.println( secsToTime(Now()));
}

//--------------------------------------------------------------------
String secsToTime(unsigned long duration)
{
    String secs(duration % 60);
    duration /= 60; // mins
    String mins(duration % 60);
    duration /= 60; // hours
    String hours(duration % 24);
    duration /= 24; // days

    if (secs.length() == 1)
        secs = "0" + secs;
    if (mins.length() == 1)
        mins = "0" + mins;
    if (hours.length() == 1)
        hours = "0" + hours;

    return hours + ":" + mins + ":" + secs;
}
//--------------------------------------------------------------------
TButtonEvent CheckForButtonPress()
{
    if (OkButton.Check() != _okButtonState)
    {
        _okButtonState = !_okButtonState;
        if (_okButtonState == false)
        {
            Debug.println("OK Button pressed.");
            return ARM_BUTTON;
        }
    }
    if (CancelButton.Check() != _cancelButtonState)
    {
        _cancelButtonState = !_cancelButtonState;
        if (_cancelButtonState == false)
        {
            Debug.println("Cancel Button pressed.");
            return CANCEL_BUTTON;
        }
    }
    return VOID_BUTTON;
}

//--------------------------------------------------------------------
int checkForKeyPress(const TKeyPad keys[], int keyCount )
{
    static int _lastKey = 0;
    static unsigned long _lastKeyTime = 0;

    uint16_t t_x, t_y; // To store the touch coordinates

    if (tft.getTouch(&t_x, &t_y))
    {
        // check to see if a button was hit
        for (int i = 0; i < keyCount; ++i)
            if (keys[i].hitTest(t_x, t_y))
            {
                // check for repeat key presses
                if (millis() - _lastKeyTime > 333) // if a third of a second has passed allow a repeat key
                    _lastKey = VOID_KEY;

                if (i == _lastKey || keys[i].isEnabled() == false)
                    return VOID_KEY; // ignore duplicate key presses or disabled key pads

                _lastKey = i;
                _lastKeyTime = millis();

                Beeper.Tone(1200, 30);

                keys[i].flash((i < 10) ? TFT_BLACK : TFT_WHITE);
                return keys[i].getID();
            }
    }
    return VOID_KEY; // no key
}

//--------------------------------------------------------------------
bool isBacklightOn()
{
    return _backlight;
}
//--------------------------------------------------------------------
void setBacklight(bool state)
{
    if (state != _backlight)
    {
        digitalWrite(BACKLIGHT_PIN, state);
        digitalWrite(BUTTONLIGHT_PIN, state == false);
        _backlight = state;
    }
}
