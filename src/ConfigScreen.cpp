#include "ConfigScreen.h"
#include "Shared.h"

#define CONFIGSCREEN_KEYCOUNT 5
TKeyPad configKeys[] =
    {
        TKeyPad(10, 10, 180, 50, TFT_GREENYELLOW, "Add RFID Tag", ADDCARD_KEY),
        TKeyPad(10, 70, 180, 50, TFT_GREENYELLOW, "Del RFID Tag", DELCARD_KEY),
        TKeyPad(10, 130, 120, 50, TFT_GREENYELLOW, "Walk Test", WALKTEST_KEY),
        TKeyPad(10, 190, 100, 50, TFT_RED, "Reboot", REBOOT_KEY),
        TKeyPad(160, 190, 95, 50, TFT_ORANGE, "Cancel", CANCEL_KEY),
    };

//--------------------------------------------------------------------
void TConfigScreen::message(const String &msg)
{
    tft.fillRect(10, 190, 320, 240 - 190, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(msg, 130, 205, FONT_NORMAL);
}
//--------------------------------------------------------------------
void TConfigScreen::render()
{
    tft.fillScreen(TFT_BLACK);

    for (int i = 0; i < CONFIGSCREEN_KEYCOUNT; ++i)
        configKeys[i].render();
}

//--------------------------------------------------------------------
int TConfigScreen::checkKeys()
{
    return checkForKeyPress(configKeys, CONFIGSCREEN_KEYCOUNT);
}

//--------------------------------------------------------------------
TConfig::State TConfigScreen::_WAIT_FOR_CMD(bool newState)
{
    switch( checkKeys())
    {
        case CANCEL_KEY:
            return TConfig::FINISHED;
        case ADDCARD_KEY:
            message("Present Card");
            return TConfig::WAIT_FOR_ADD;
        case DELCARD_KEY:
            message("Present Card");
            return TConfig::WAIT_FOR_DELETE;
        case REBOOT_KEY:
            esp_restart();
            //esp_sleep_enable_timer_wakeup(4 * 1000 * 1000);   // sleep for 2 seconds
            //esp_deep_sleep_start();
            break;
        case WALKTEST_KEY:
            return TConfig::WALKTEST;
    }
    return TConfig::WAIT_FOR_CMD;
}
//--------------------------------------------------------------------
TConfig::State TConfigScreen::_WAIT_FOR_ADD(bool newState)
{
    if( checkKeys() == CANCEL_KEY)
        return TConfig::FINISHED;

    int card = RFID.IsCardPresent();
    if (card > 0)
    {
        message("Card exists");
        return TConfig::WAIT_FOR_CMD;
    }
    if (card < 0)
    {
        RFID.AddCard();
        message("Card Added");
        return TConfig::WAIT_FOR_CMD;
    }

    return TConfig::WAIT_FOR_ADD;
}
//--------------------------------------------------------------------
TConfig::State TConfigScreen::_WAIT_FOR_DELETE(bool newState)
{
    if( checkKeys() == CANCEL_KEY)
        return TConfig::FINISHED;

    int card = RFID.IsCardPresent();
    if (card > 0)
    {
        RFID.DeleteCard();
        message("Card Deleted");
        return TConfig::WAIT_FOR_CMD;
    }
    if (card < 0)
    {
        message("Unknown Card");
        return TConfig::WAIT_FOR_CMD;
    }

    return TConfig::WAIT_FOR_DELETE;
}

//--------------------------------------------------------------------
bool TConfigScreen::loop()
{
    static TConfig::State _state = TConfig::START; // The current state
    static bool _newState;                         // true the first time we change to a state

    TConfig::State nextState = _state;
    switch (_state)
    {
    case TConfig::START:
        _walkTest = false;
        nextState = TConfig::WAIT_FOR_CMD;
        break;
    case TConfig::WAIT_FOR_CMD:
        nextState = _WAIT_FOR_CMD(_newState);
        break;
    case TConfig::WAIT_FOR_ADD:
        nextState = _WAIT_FOR_ADD(_newState);
        break;
    case TConfig::WAIT_FOR_DELETE:
        nextState = _WAIT_FOR_DELETE(_newState);
        break;
    case TConfig::WALKTEST:
        _walkTest =  true;
        // drop through
    case TConfig::FINISHED:
        _state = TConfig::START;
        return false;
    }

    // update to the new state
    _newState = nextState != _state;
    _state = nextState;

    return true;
}
