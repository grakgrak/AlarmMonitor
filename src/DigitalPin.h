#ifndef DIGITALPIN_H
#define DIGITALPIN_H
#include <Arduino.h>
//--------------------------------------------------------------------
class TDigitalPin
{
  private:
    int _pin;
    bool _triggered;
    bool _beepOnTrigger;
    String _name;

  public:
    TDigitalPin( const String &name, uint8_t pin, uint8_t mode);

    void setBeepOnTrigger(bool flag)    {_beepOnTrigger = flag;}
    void update();
    bool IsTriggered() { return _triggered; }
    String name()   { return _name;}
    String state()  { return String(_triggered ? "T" : "Q");}
    void PublishStatus();
};


#endif//DIGITALPIN_H