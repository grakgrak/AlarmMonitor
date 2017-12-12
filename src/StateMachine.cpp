#include "Shared.h"
#include "MainScreen.h"
#include "CountdownScreen.h"
#include "ConfigScreen.h"

#define SCREEN_TIMEOUT (60 * 1000)    // Turn screen off if no activiy for this time
#define WAIT_FOR_DOOR_MS (20 * 1000)  // time to wait for the door to open or close
#define ALARM_SOUNDER_MS (5 * 60 * 1000)    //  How long to sound the alarm for when triggered

namespace TMain
{
  const char *Names[] = {"ZZZ", "CMD", "EXH", "ENH", "W4D", "ARM", "WT", "CFG", "TR" };
}
//--------------------------------------------------------------------
TMainScreen MainScreen; // Main display screen
TCountdownScreen CountdownScreen;
TConfigScreen ConfigScreen;
TStateMachine StateMachine; // Global Instance

//--------------------------------------------------------------------
TStateMachine::TStateMachine()
{
    _state = TMain::SLEEPING;
    setState(TMain::SLEEPING);
}
//--------------------------------------------------------------------
void TStateMachine::init()
{
    if (nvs_open("alarm", NVS_READWRITE, &_alarm_nvs) != ESP_OK)
    {
        Debug.println("Failed to open the Alarm nvs.");
        return;
    }

    setState(TMain::WAIT_FOR_CMD);

    int8_t flag;
    if (nvs_get_i8(_alarm_nvs, "isArmed", &flag) == ESP_OK)
        if (flag != 0)
            setState(TMain::ARMED);
}
//--------------------------------------------------------------------
void TStateMachine::setState(TMain::State state)
{
    _stateChanged = _state != state;
    _state = state;

    if (_stateChanged)
    {
        publishState();
        MainScreen.stateMessage(TMain::Names[_state]);
    }
}
//--------------------------------------------------------------------
bool TStateMachine::validateCode(const char *code)
{
    size_t len = 0;

    return nvs_get_blob(_alarm_nvs, code, NULL, &len) == ESP_OK; // true if the code is known
}
//--------------------------------------------------------------------
void TStateMachine::addCode(const char *code)
{
    if (validateCode(code) == false)
    {
        nvs_set_blob(_alarm_nvs, code, "", 1); // save the binary for the uid
        nvs_commit(_alarm_nvs);
        Debug.printf("Add Code: %s\n", code);
    }
}
//--------------------------------------------------------------------
void TStateMachine::deleteCode(const char *code)
{
    if (validateCode(code))
    {
        nvs_erase_key(_alarm_nvs, code); // save the binary for the uid
        nvs_commit(_alarm_nvs);
        Debug.printf("Delete Code: %s\n", code);
    }
}
//--------------------------------------------------------------------
void TStateMachine::arm()
{
    Beeper.Beep(2000, 0, 1);    // beep for 2 seconds

    nvs_set_i8(_alarm_nvs, "isArmed", 1);
    nvs_commit(_alarm_nvs);
}
//--------------------------------------------------------------------
void TStateMachine::disarm()
{
    Beeper.Tone(2000, 250);
    Beeper.Tone(1500, 250);

    nvs_set_i8(_alarm_nvs, "isArmed", 0);
    nvs_commit(_alarm_nvs);
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_SLEEPING(bool stateChanged)
{
    if (stateChanged)
    {
        Beeper.Beep(0, 0, 0); // turn beeper off
        setBacklight(false);
        MainScreen.cancelEntry();
    }

    if (MainScreen.getTouch())
        return TMain::WAIT_FOR_CMD;

    if (RFID.IsCardPresent() != 0)
        return TMain::WAIT_FOR_CMD;

    if (isBacklightOn())
        return TMain::WAIT_FOR_CMD;

    if (CheckForButtonPress() != VOID_BUTTON)
        return TMain::WAIT_FOR_CMD;

    if (SensorDoor.IsTriggered()) // if DOOR open goto EXIT_HOUSE
        return TMain::WAIT_FOR_CMD;

    return TMain::SLEEPING;
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_WAIT_FOR_CMD(bool stateChanged)
{
    static unsigned long timeout;
    if (stateChanged)
    {
        //RFID.Reset();
        Beeper.Beep(0, 0, 0); // turn beeper off
        timeout = millis();

        MainScreen.cancelEntry();
        MainScreen.render("Arm");
        setBacklight(true);
    }

    if ((millis() - timeout) > SCREEN_TIMEOUT) // if inactive timeout goto IDLE
        return TMain::SLEEPING;

    int key = MainScreen.checkKeys();
    if (key == UNLOCK_CODE)
        return TMain::WAIT_FOR_DOOR;
    if (key >= 0)
    {
        timeout = millis(); // extend screen timeout
        return TMain::WAIT_FOR_CMD;
    }

    switch (CheckForButtonPress())
    {
    case TButtonEvent::ARM_BUTTON:
        if (MainScreen.checkForUnlockCode(ARM_KEY))
            return TMain::WAIT_FOR_DOOR;
        break;
    case TButtonEvent::CANCEL_BUTTON:
        MainScreen.cancelEntry();
        break;
    }

    int card = RFID.IsCardPresent();
    if( card == 2)  // if master card seen then enter config mode
        return TMain::CONFIG;
    if (card > 0) // second show of rfid card arms system
        return TMain::WAIT_FOR_DOOR;
    if (card < 0)
    {
        MainScreen.message("Unknown Card");
        timeout = millis(); // reset the screen timeout
    }

    return TMain::WAIT_FOR_CMD; // stay in current state
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_WAIT_FOR_DOOR(bool stateChanged)
{
    static unsigned long timeout;
    if (stateChanged)
    {
        MainScreen.message("Exit Home");
        // TODO: Check all sensors - if bad goto SYSTEM_ERROR
        Beeper.Beep(200, 800, 100);
        setBacklight(true);
        timeout = millis();
    }

    if ((millis() - timeout) > WAIT_FOR_DOOR_MS) // time to open door or cancel command
        return TMain::WAIT_FOR_CMD;

    if (MainScreen.checkKeys() == CANCEL_KEY)
        return TMain::WAIT_FOR_CMD;

    if (CheckForButtonPress() == CANCEL_BUTTON)
        return TMain::WAIT_FOR_CMD;

    if (SensorDoor.IsTriggered()) // if DOOR open goto EXIT_HOUSE
        return TMain::EXIT_HOUSE;

    if (RFID.IsCardPresent() > 0)
        return TMain::WAIT_FOR_CMD;

    return TMain::WAIT_FOR_DOOR; // stay in current state
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_EXIT_HOUSE(bool stateChanged)
{
    static unsigned long timeout;
    if (stateChanged)
    {
        CountdownScreen.render("Waiting for door...", 20);
        Beeper.Beep(250, 750, 100);
        timeout = millis();
    }

    CountdownScreen.update();

    if (CountdownScreen.checkForKeyPress() == CANCEL_KEY)
        return TMain::WAIT_FOR_CMD;

    if (CheckForButtonPress() == CANCEL_BUTTON)
        return TMain::WAIT_FOR_CMD;

    if (RFID.IsCardPresent() > 0)
        return TMain::WAIT_FOR_CMD;

    // check the Door
    if (SensorDoor.IsTriggered())
        timeout = millis();
    else if (millis() - timeout > 2000) // wait 2 seconds with the door shut before ARMING
        return TMain::ARMED;

    return TMain::EXIT_HOUSE; // stay in current state
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_ARMED(bool stateChanged)
{
    if (stateChanged)
    {
        arm();
        setBacklight(false);
    }

    if (RFID.IsCardPresent() > 0)
    {
        disarm();
        return TMain::WAIT_FOR_CMD;
    }

    if (MainScreen.getTouch())
        return TMain::ENTER_HOUSE;

    if (SensorDoor.IsTriggered()) // check DOOR for open
        return TMain::ENTER_HOUSE;

    if (anySensorTriggered())
        return TMain::TRIGGERED;

    return TMain::ARMED; // stay in current state
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_ENTER_HOUSE(bool stateChanged)
{
    static unsigned long timeout;
    if (stateChanged)
    {
        MainScreen.render("Disarm");
        setBacklight(true);
        Beeper.Beep(250, 750, 100);
        // start 20 second timeout
        timeout = millis();
    }

    // if entry timeout
    if ((millis() - timeout) > WAIT_FOR_DOOR_MS)
        return TMain::TRIGGERED; // sound the alarm

    // if RFID hit goto WAIT_FOR_CMD
    if (RFID.IsCardPresent() > 0)
    {
        disarm();
        return TMain::WAIT_FOR_CMD;
    }

    if (MainScreen.checkKeys() == UNLOCK_CODE)
    {
        disarm();
        return TMain::WAIT_FOR_CMD;
    }

    return TMain::ENTER_HOUSE;
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_WALK_TEST(bool stateChanged)
{
    if (stateChanged)
    {
    }
    return TMain::WALK_TEST; // stay in current state
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_CONFIG(bool stateChanged)
{
    if (stateChanged)
    {
        ConfigScreen.render();
    }

    if( ConfigScreen.loop() == false)
        return TMain::WAIT_FOR_CMD;

    return TMain::CONFIG; // stay in current state
}
//--------------------------------------------------------------------
TMain::State TStateMachine::_TRIGGERED(bool stateChanged)
{
    static unsigned long timeout;
    if (stateChanged)
    {
        setBacklight(true);
        MainScreen.render("Disarm");
        Beeper.Beep(400, 100, 1000); // sound the alarm
        timeout = millis();
    }

    if (MainScreen.checkKeys() == UNLOCK_CODE)
    {
        disarm();
        return TMain::WAIT_FOR_CMD;
    }

    // On rfid hit - goto WAIT_FOR_CMD
    if (RFID.IsCardPresent() > 0)
    {
        disarm();
        return TMain::WAIT_FOR_CMD;
    }

    if (millis() - timeout > ALARM_SOUNDER_MS) // Only sound the alarm for a limited time
        return TMain::ARMED;

    return TMain::TRIGGERED; // stay in current state
}

//--------------------------------------------------------------------
void TStateMachine::publishState()
{
    Mqtt.publish(ALARM_CURRENT_STATE, TMain::Names[_state]);
}
//--------------------------------------------------------------------
bool TStateMachine::anySensorTriggered()
{
    return Sensor1.IsTriggered() || Sensor2.IsTriggered() || Sensor3.IsTriggered() || Sensor4.IsTriggered() || Sensor5.IsTriggered() || Sensor6.IsTriggered() || Sensor7.IsTriggered();
}
//--------------------------------------------------------------------
void TStateMachine::updateSensors()
{
    // update the sensor states to get the latest readings
    // also publishes a change in state to mqtt
    Sensor1.update();
    Sensor2.update();
    Sensor3.update();
    Sensor4.update();
    Sensor5.update();
    Sensor6.update();
    Sensor7.update();
    SensorDoor.update();
}
//--------------------------------------------------------------------
void TStateMachine::loop()
{
    RFID.loop();

    updateSensors();    // get all the current states for the sensors.

    switch (_state)
    {
    case TMain::SLEEPING:
        setState(_SLEEPING(_stateChanged));
        break;
    case TMain::WAIT_FOR_CMD:
        setState(_WAIT_FOR_CMD(_stateChanged));
        break;
    case TMain::WAIT_FOR_DOOR:
        setState(_WAIT_FOR_DOOR(_stateChanged));
        break;
    case TMain::EXIT_HOUSE:
        setState(_EXIT_HOUSE(_stateChanged));
        break;
    case TMain::ENTER_HOUSE:
        setState(_ENTER_HOUSE(_stateChanged));
        break;
    case TMain::ARMED:
        setState(_ARMED(_stateChanged));
        break;
    case TMain::WALK_TEST:
        setState(_WALK_TEST(_stateChanged));
        break;
    case TMain::CONFIG:
        setState(_CONFIG(_stateChanged));
        break;
    case TMain::TRIGGERED:
        setState(_TRIGGERED(_stateChanged));
        break;
    }
}