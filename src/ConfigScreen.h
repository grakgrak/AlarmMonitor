#ifndef CONFIGSCREEN_H
#define CONFIGSCREEN_H

#include <Arduino.h>
#include "KeyPad.h"

#define ADDCARD_KEY 1
#define DELCARD_KEY 2
#define WALKTEST_KEY 3

namespace TConfig
{
    enum State {START, WAIT_FOR_CMD, WAIT_FOR_ADD, WAIT_FOR_DELETE, WALKTEST, FINISHED};
}

class TConfigScreen
{
  private:
    TConfig::State _WAIT_FOR_CMD(bool newState);
    TConfig::State _WAIT_FOR_ADD(bool newState);
    TConfig::State _WAIT_FOR_DELETE(bool newState);
    int checkKeys();
    void message(const String &msg);
    bool _walkTest;
  public:
    void render();
    bool loop();
    bool performWalkTest()  { return _walkTest;}
};

#endif // CONFIGSCREEN_H