#ifndef BUTTON_H
#define BUTTON_H
#include <Arduino.h>

enum TButtonEvent
{
    VOID_BUTTON,
    ARM_BUTTON,
    CANCEL_BUTTON
};


//--------------------------------------------------------------------
class TButton
{
  private:
    int _pin;
    bool _state;
    bool _lastState;
    unsigned long _lastDebounceTime; // the last time the output pin was toggled
  public:
    TButton(uint8_t pin, uint8_t mode);

    bool Check();
};

#endif //BUTTON_H