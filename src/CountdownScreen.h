#ifndef COUNTDOWNSCREEN_H
#define COUNTDOWNSCREEN_H
#include <Arduino.h>
#include "KeyPad.h"

class TCountdownScreen
{
  private:
    unsigned long _lastUpdate;
    int _seconds;
    String _message;
  public:
    void render(const String &msg, int secs);
    void update();
    int checkKeys();
};

#endif //COUNTDOWNSCREEN_H