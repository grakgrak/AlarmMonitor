#ifndef RFID_H
#define RFID_H
#include <Arduino.h>

class TRFID
{
  public:
    static void loop();
    static void init();

    static int IsCardPresent();  // 0 if no card, -1 if unknown card, 1 if known card, 2 if master card
    static void WipeData();
    static void SetMasterCard(byte uid[]);
    static void AddCard(byte uid[]);
    static void AddCard();  // adds the last read card
    static void DeleteCard(byte uid[]);
    static void DeleteCard(); // deletes the last read card
    static void PublishStatus();
};

extern TRFID RFID;

#endif // RFID_H