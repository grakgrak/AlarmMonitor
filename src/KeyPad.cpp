#include "KeyPad.h"
#include "Shared.h"

#define TEXT_OFFSET 4   // to centre the text vertically

//--------------------------------------------------------------------
TKeyPad::TKeyPad(int x, int y, int w, int h, uint32_t background, const String &text, int id)
{
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _id = id;
    _background = background;
    _text = text;
    _enabled = true;
}
//--------------------------------------------------------------------
void TKeyPad::enabled(bool flag)
{
    if( flag != _enabled)
    {
        _enabled = flag;
        render();
    }
}
//--------------------------------------------------------------------
bool TKeyPad::hitTest(int x, int y) const
{
    return (x > _x) && (x < _x + _w) && (y > _y) && (y < _y + _h);
}

//--------------------------------------------------------------------
void TKeyPad::render_pad(uint32_t colour) const
{
    tft.fillRoundRect(_x, _y, _w, _h, 3, colour);
    tft.drawRoundRect(_x, _y, _w + 1, _h + 1, 3, TFT_LIGHTGREY);
    tft.drawRoundRect(_x, _y, _w, _h, 3, TFT_WHITE);
}

//--------------------------------------------------------------------
void TKeyPad::render() const
{
    render_pad(_enabled ? _background : TFT_DARKGREY);

    tft.setTextColor(TFT_BLACK, _enabled ? _background : TFT_DARKGREY);
    tft.setTextDatum(CC_DATUM);
    tft.drawString(_text, _x + _w / 2, _y + _h / 2 + TEXT_OFFSET, FONT_NORMAL);
    
    //int tw = tft.textWidth(_text);
    //tft.drawString(_text, _x + (_w - tw) / 2, _y + (_h - 22) / 2); // text height is 26 pixels but using 22 to ignore descenders
}

//--------------------------------------------------------------------
void TKeyPad::flash(uint32_t colour) const
{
    render_pad(colour);
    delay(100);
    render();
}
