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
    void render(int secs, const String &msg);
    void update();
    int checkKeys();
};

#endif //COUNTDOWNSCREEN_H