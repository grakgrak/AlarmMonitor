#include "MainScreen.h"
#include "Shared.h"

#define MAINSCREEN_KEYCOUNT 12
TKeyPad mainKeys[] =
    {
        TKeyPad(10, 10, 50, 50, TFT_GREENYELLOW, "1", 1),
        TKeyPad(70, 10, 50, 50, TFT_GREENYELLOW, "2", 2),
        TKeyPad(130, 10, 50, 50, TFT_GREENYELLOW, "3", 3),
        TKeyPad(10, 70, 50, 50, TFT_GREENYELLOW, "4", 4),
        TKeyPad(70, 70, 50, 50, TFT_GREENYELLOW, "5", 5),
        TKeyPad(130, 70, 50, 50, TFT_GREENYELLOW, "6", 6),
        TKeyPad(10, 130, 50, 50, TFT_GREENYELLOW, "7", 7),
        TKeyPad(70, 130, 50, 50, TFT_GREENYELLOW, "8", 8),
        TKeyPad(130, 130, 50, 50, TFT_GREENYELLOW, "9", 9),
        TKeyPad(70, 190, 50, 50, TFT_GREENYELLOW, "0", 0),
        TKeyPad(200, 10, 100, 100, TFT_BLUE, "Arm", ARM_KEY),
        TKeyPad(200, 130, 100, 50, TFT_ORANGE, "Clear", CANCEL_KEY)
        };

//--------------------------------------------------------------------
TMainScreen::TMainScreen()
{
    _lastKeyTime = 0;
    _lastState = "";
    _lastKey = VOID_KEY;
    _keyIdx = 0;
    _validKeyCode = false;
}
//--------------------------------------------------------------------
void TMainScreen::stateMessage(const char *msg)
{
    _lastState = msg;

    tft.fillRect(0, 190, 60, 50, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(CC_DATUM);
    tft.drawString(msg, 35, 220, FONT_NORMAL);
}
//--------------------------------------------------------------------
void TMainScreen::message(const String &msg)
{
    _lastMessage = msg;

    tft.fillRect(130, 190, 320 - 130, 240 - 190, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(msg, 130, 205, FONT_NORMAL);
}
//--------------------------------------------------------------------
bool TMainScreen::getTouch()
{
    uint16_t t_x, t_y; // To store the touch coordinates
    if (tft.getTouch(&t_x, &t_y))
    {
        Debug.printf("Touch at (%d,%d)\n", t_x, t_y);
        return true;
    }
    return false;
}
//--------------------------------------------------------------------
void TMainScreen::render(const String &action)
{
    _validKeyCode = false;
    _keyIdx = 0;

    mainKeys[ARM_KEY].setText(action);
    mainKeys[ARM_KEY].enabled(false);

    tft.fillScreen(TFT_BLACK);
    for (int i = 0; i < MAINSCREEN_KEYCOUNT; ++i)
        mainKeys[i].render();

    message(_lastMessage);
    stateMessage(_lastState);
}

//--------------------------------------------------------------------
void TMainScreen::cancelEntry()
{
    _validKeyCode = false;
    _keyIdx = 0;
    mainKeys[ARM_KEY].enabled(false);
    message("");
}
//--------------------------------------------------------------------
bool TMainScreen::checkForUnlockCode(int key)
{
    if (key == CANCEL_KEY)
        cancelEntry();

    if (key < 10) // handle number keys
    {
        _keyCode[_keyIdx] = '0' + key; // save the last keypress
        _keyIdx = ++_keyIdx % 5;       // increment and loop
        _keyCode[_keyIdx] = '\0';
        message(_keyCode);

        _validKeyCode = false;
        if (_keyIdx == 4)
        {
            _keyIdx = 0; // reset the index to the start

            _validKeyCode = StateMachine.validateCode(_keyCode);
        }
    }

    mainKeys[ARM_KEY].enabled(_validKeyCode);

    return _validKeyCode && (key == ARM_KEY);
}
//--------------------------------------------------------------------
int TMainScreen::checkKeys()
{
    int key = checkForKeyPress(mainKeys, MAINSCREEN_KEYCOUNT);

    if (key != VOID_KEY)
        if (checkForUnlockCode(key))
            return UNLOCK_CODE;

    return key; // no key
}
