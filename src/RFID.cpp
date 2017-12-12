#include "Shared.h"
#include <SPI.h>
#include <MFRC522.h> // Library for Mifare RC522 Devices

#define UID_BYTES 4 // number of bytes in a UID
#define RFID_CHECK_MS   50
//--------------------------------------------------------------------
MFRC522 _mfrc522(RFID_CS_PIN, RFID_RST_PIN);

TRFID RFID; // Global Instance

nvs_handle rfid_nvs;
byte _readCard[UID_BYTES];   // Stores scanned ID read from RFID Module
byte _masterCard[UID_BYTES]; // Stores master card's ID read from EEPROM
unsigned long _cardSeenAt = 0;
unsigned long _lastLoopTime = 0;
bool _isValid = false;
bool _isMaster = false;
bool _initialised = false;

//--------------------------------------------------------------------
String uidToString(byte uid[])
{
    String UID;
    for (byte i = 0; i < UID_BYTES; i++)
    {
        byte b = uid[i];
        UID += " ";
        UID += "0123456789ABCDEF"[b >> 4];
        UID += "0123456789ABCDEF"[b & 0x0F];
    }
    return UID;
}

//--------------------------------------------------------------------
bool IsValidCard(byte uid[])
{
    String key = uidToString(uid);
    size_t len = 0;

    _isMaster = (uid[0] == _masterCard[0]) && (uid[1] == _masterCard[1]) && (uid[2] == _masterCard[2]) && (uid[3] == _masterCard[3]);
    _isValid = nvs_get_blob(rfid_nvs, key.c_str(), NULL, &len) == ESP_OK; // true if the card is known

    return _isValid || _isMaster;
}

//--------------------------------------------------------------------
void TRFID::DeleteCard(byte uid[])
{
    String key = uidToString(uid);

    nvs_erase_key(rfid_nvs, key.c_str());
    nvs_commit(rfid_nvs);
}
//--------------------------------------------------------------------
void TRFID::DeleteCard()
{
    DeleteCard(_readCard);
}
//--------------------------------------------------------------------
void TRFID::WipeData()
{
    nvs_erase_all(rfid_nvs);
    nvs_commit(rfid_nvs);
    Debug.println("Data Wiped.");
}
//--------------------------------------------------------------------
void TRFID::SetMasterCard(byte uid[])
{
    for(int i = 0; i < UID_BYTES; ++i)
        _masterCard[i] = uid[i];

    nvs_set_blob(rfid_nvs, "master", &_masterCard, sizeof(_masterCard));

    nvs_commit(rfid_nvs);

    //Mqtt.publish(ALARM_LOG_TRACE, "Set Master: " + uidToString(_masterCard));

    Debug.println("Set Master: " + uidToString(_masterCard));
}
//--------------------------------------------------------------------
void TRFID::AddCard(byte uid[])
{
    if (IsValidCard(uid) == false)
    {
        String key = uidToString(uid);
        nvs_set_blob(rfid_nvs, key.c_str(), uid, UID_BYTES); // save the binary for the uid
        nvs_commit(rfid_nvs);
        Debug.println("Add Card: " + uidToString(uid));
        //Mqtt.publish(ALARM_LOG_TRACE, "Add Card: " + uidToString(_masterCard));
    }
}
//--------------------------------------------------------------------
void TRFID::AddCard()
{
    AddCard(_readCard);
}
//--------------------------------------------------------------------
void TRFID::PublishStatus()
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &j = jsonBuffer.createObject();

    // if a card was seen in the last second
    if ((millis() - _cardSeenAt) <= 1000)
    {
        MFRC522::Uid *uid = &_mfrc522.uid;
        j["uid"] = uidToString(uid->uidByte);
        j["type"] = _mfrc522.PICC_GetTypeName( _mfrc522.PICC_GetType(uid->sak));
        if( _isValid || _isMaster)
            j["state"] = _isMaster  ? "master" : "valid";
        else
            j["state"] = "unknown";
    }
    else
    {
        j["state"] = "nocard";
    }
    Mqtt.publish(ALARM_RFID_CARD, j);
}

//--------------------------------------------------------------------
void TRFID::loop()
{
    if( _initialised == false)
        return;

    if ((millis() - _lastLoopTime) <= RFID_CHECK_MS) // only check for a card periodically
        return;

    _lastLoopTime = millis();

    if (_mfrc522.PICC_IsNewCardPresent())
    {
        int err = _mfrc522.PICC_Select(&_mfrc522.uid);
        if (err == MFRC522::STATUS_OK)
        {
            // capture the card uid
            for (int i = 0; i < UID_BYTES; ++i)
                _readCard[i] = _mfrc522.uid.uidByte[i];

            _mfrc522.PICC_HaltA(); // Stop reading

            _cardSeenAt = millis();

            if (IsValidCard(_readCard))
            {
                Beeper.Tone(1500, 200);
                Beeper.Tone(1700, 100);
            }
            else
                Beeper.Tone(1500, 500);

            PublishStatus();
        }
        else
        {
            Debug.printf("RFID Select Error: %d\n", err);
        }
    }
}

//--------------------------------------------------------------------
int TRFID::IsCardPresent()
{
    if( _initialised == false)
        return 0;

    // if a card was seen in the last second
    int state = 0;

    if ((millis() - _cardSeenAt) <= 1000)
    {
        if( _isValid || _isMaster)
            state = _isMaster ?  2 : 1;
        else
            state = -1; // unknown card
    }

    _cardSeenAt = 0; // reset so we dont get double hits

    return state;
}

//--------------------------------------------------------------------
void TRFID::init()
{
    _mfrc522.PCD_Init();
    _mfrc522.PCD_DumpVersionToSerial();

    _initialised = true;
    
    if (nvs_open("rfid", NVS_READWRITE, &rfid_nvs) != ESP_OK)
    {
        Debug.println("Failed to open the RFID nvs.");
        return;
    }

    size_t len = sizeof(_masterCard);
    if (nvs_get_blob(rfid_nvs, "master", &_masterCard, &len) == ESP_ERR_NVS_NOT_FOUND)
        Debug.println("RFID master not found.");
    else
        Mqtt.publish("alarm/debug/log", "Master: " + uidToString(_masterCard));
}
