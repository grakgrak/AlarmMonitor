#ifndef BEEPER_H
#define BEEPER_H
#include <Arduino.h>

//--------------------------------------------------------------------
class TBeeper
{
  private:
    enum TState {IDLE,START,ON,OFF};

    // tone vars and state machine
    int _freq;
    int _duration;
    bool _toneStateChanged;
    TState _toneState;

    TState _TONE_ON(bool stateChanged);
    TState _TONE_OFF(bool stateChanged);
    void setToneState(TState state);
    void toneLoop();

    // beep vars and state machine
    int _beepOn;
    int _beepOff;
    int _beepRepeat;
    bool _beepStateChanged;
    TState _beepState;

    TState _BEEP_ON(bool stateChanged);
    TState _BEEP_OFF(bool stateChanged);
    void setBeepState(TState state);
    void beepLoop();
  public:
    TBeeper();

    void Beep(int onMillis, int offMillis, int repeat);
    void Tone(int freq, int duration);
    void loop();
};

extern TBeeper Beeper;

#endif