#ifndef STATEMACHINE_H
#define STATEMACHINE_H
#include <Arduino.h>
#include <nvs.h>

namespace TMain
{
  enum State { SLEEPING, WAIT_FOR_CMD, EXIT_HOUSE, ENTER_HOUSE, WAIT_FOR_DOOR, ARMED, WALK_TEST, CONFIG, TRIGGERED };
  extern const char *Names[]; // defined in the .cpp file
}

class TStateMachine
{
private:
  nvs_handle _alarm_nvs;
  TMain::State _state;
  bool _stateChanged;
  char _code[5];

  TMain::State _SLEEPING(bool stateChanged);
  TMain::State _WAIT_FOR_CMD(bool stateChanged);
  TMain::State _WAIT_FOR_DOOR(bool stateChanged);
  TMain::State _EXIT_HOUSE(bool stateChanged);
  TMain::State _ARMED(bool stateChanged);
  TMain::State _ENTER_HOUSE(bool stateChanged);
  TMain::State _WALK_TEST(bool stateChanged);
  TMain::State _CONFIG(bool stateChanged);
  TMain::State _TRIGGERED(bool stateChanged);

  void setState(TMain::State state);
  void arm();
  void disarm();
  void updateSensors();
  int anySensorTriggered();
  void publishState(const char *stateName);

public:
  TStateMachine();
  void init();
  void loop();
  bool isArmed() { return _state == TMain::ARMED; }

  bool validateCode(const char *code);
  void addCode(const char *code);
  void deleteCode(const char *code);

  const char *stateName();
};

extern TStateMachine StateMachine;

#endif // STATEMACHINE_H