#ifndef DEBUG_H
#define DEBUG_H
#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library

#define SCREEN_HEIGHT 240

extern TFT_eSPI tft;

class TDebug : public Print
{
  private:
    int _voffset;
    bool _showOnTFT;

    void addLine()
    {
        _voffset += 16;
        if (_voffset >= SCREEN_HEIGHT)
            _voffset = 0;
        tft.setCursor(0, _voffset);
        tft.setTextFont(2);
    }

  public:
    TDebug()
    {
        _voffset = 0;
        _showOnTFT = true;
        tft.setTextFont(2);
    }

    void ShowOnTFT(bool flag)   {_showOnTFT = flag;}

    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual size_t write(uint8_t chr)
    {
        Serial.write(chr);

        if (_showOnTFT)
        {
            if (chr == '\n')
            {
                addLine();
                tft.print("-->                                                ");
                tft.setCursor(0, _voffset);
            }
            else
                tft.write(chr);
        }
        return 1;
    }
};

extern TDebug Debug;

#endif // DEBUG_H