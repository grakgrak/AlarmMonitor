#ifndef MAINSCREEN_H
#define MAINSCREEN_H
#include <Arduino.h>
#include "KeyPad.h"
#include "StateMachine.h"

#define UNLOCK_CODE 99

class TMainScreen
{
  private:
    unsigned long _lastKeyTime;
    int _lastKey = -1;
    int _keyIdx = 0;
    char _keyCode[5];
    bool _validKeyCode;
    String _lastMessage;
    const char *_lastState;
  public:
    TMainScreen();
    bool getTouch();
    void render(const String &action);
    void stateMessage(const char *msg);
    void message(const String &msg);
    int checkKeys();
    bool checkForUnlockCode(int key);
    void cancelEntry();
};

#endif //MAINSCREEN_H