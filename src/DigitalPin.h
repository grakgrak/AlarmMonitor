#ifndef DIGITALPIN_H
#define DIGITALPIN_H
#include <Arduino.h>
//--------------------------------------------------------------------
class TDigitalPin
{
  private:
    int _pin;
    bool _pinLevel;
    bool _beepOnTrigger;
    bool _invert;
    String _name;

  public:
    TDigitalPin(const String &name, uint8_t pin, uint8_t mode, bool invert);

    void setBeepOnTrigger(bool flag)    {_beepOnTrigger = flag;}
    void update();
    bool IsTriggered() { return _pinLevel != _invert; }
    String name()   { return _name;}
    String state()  { return String(IsTriggered() ? "T" : "Q");}
    void PublishStatus();
};


#endif//DIGITALPIN_H