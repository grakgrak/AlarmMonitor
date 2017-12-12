#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>

#define VOID_KEY -1
#define ARM_KEY 10
#define CANCEL_KEY 11

class TKeyPad
{
  private:
    int _x;
    int _y;
    int _w;
    int _h;
    int _id;
    uint32_t _background;
    String _text;
    bool _enabled;
    void render_pad(uint32_t colour) const;
  public:
    TKeyPad(int x, int y, int w, int h, uint32_t background, const String &text, int id);
    void enabled(bool flag);
    void setText(const String &text)    {_text = text;}
    bool isEnabled() const    { return _enabled;}
    bool hitTest(int x, int y) const;
    void render() const;
    void flash(uint32_t colour) const;
    int getID() const { return _id;}
};


#endif  //KEYPAD_H
