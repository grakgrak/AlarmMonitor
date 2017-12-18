#include "CountdownScreen.h"
#include "Shared.h"

TKeyPad cancel(200, 130, 100, 50, TFT_ORANGE, "Cancel", CANCEL_KEY);


void TCountdownScreen::render(const String &msg, int secs)
{
    _message = msg;
    _seconds = secs;
    _lastUpdate = 0;
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(_message, 160, 10, FONT_NORMAL);

    cancel.render();
}

void TCountdownScreen::update()
{
    if(millis() - _lastUpdate > 1000)
    {
        _lastUpdate = millis();

        String secs(_seconds);
        if( _seconds > 0)
            --_seconds;

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.fillRect(30, (240 - 75) / 2, 100, 75, TFT_BLACK);   // blank out the number area
        tft.setTextDatum(CC_DATUM);
        tft.drawString(secs, 30 + 50, 120, FONT_LARGE);
    }
}

int TCountdownScreen::checkKeys()
{
    uint16_t t_x, t_y; // To store the touch coordinates

    if (tft.getTouch(&t_x, &t_y))
    {
        // check to see if cancel button was hit
            if (cancel.hitTest(t_x, t_y))
                return CANCEL_KEY;
    }
    return VOID_KEY;
}
