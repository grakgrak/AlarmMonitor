#include "MqttCallback.h"
#include "Shared.h"

const char *SYS_SYNC_TIMESTAMP = "sys/sync/timestamp";
const char *ALARM_SYNC_TIMESTAMP = "alarm/sync/timestamp";
const char *CMD_ALARM_STATUS = "cmd/alarm/status";
const char *CMD_ALARM_BACKLIGHT = "cmd/alarm/backlight";
const char *CMD_ALARM_BEEP = "cmd/alarm/beep";
const char *CMD_ALARM_TONE = "cmd/alarm/tone";
const char *CMD_ALARM_RESTART = "cmd/alarm/restart";
const char *CMD_ALARM_CODE_ADD = "cmd/alarm/code/add";
const char *CMD_ALARM_CODE_DEL = "cmd/alarm/code/del";
const char *CMD_ALARM_RFID_ADD = "cmd/alarm/rfid/add";
const char *CMD_ALARM_RFID_DEL = "cmd/alarm/rfid/del";
const char *CMD_ALARM_RFID_WIPE = "cmd/alarm/rfid/wipe";
const char *CMD_ALARM_RFID_MASTER = "cmd/alarm/rfid/master";

#define JSON_BUF_SIZE   200

//--------------------------------------------------------------------
bool UidFromJson(StaticJsonBuffer<JSON_BUF_SIZE> &jsonBuffer, byte uid[], const char *payload)
{
    JsonObject &root = jsonBuffer.parseObject(payload);

    // uid[0] = root["uid0"].as<byte>();
    // uid[1] = root["uid1"].as<byte>();
    // uid[2] = root["uid2"].as<byte>();
    // uid[3] = root["uid3"].as<byte>();

    for(int i = 0; i < 4; ++i)
        uid[i] = root["uid"][i].as<byte>();

    return true;
}
//--------------------------------------------------------------------
void mqttCallback(char *topic, byte *payload, unsigned len)
{
    if(strcmp(topic, ALARM_HEARTBEAT_UPTIME) == 0)    // filter out heatbeat messages
        return;

    StaticJsonBuffer<JSON_BUF_SIZE> jsonBuffer;

    char value[MQTT_MAX_PACKET_SIZE + 1];

    memcpy((void *)value, (const void *)payload, len);
    value[len] = '\0';
    //String value((const char *)payload, len);

    Debug.printf("< %s:%s\n", topic, value);

    if (strcmp(topic, SYS_SYNC_TIMESTAMP) == 0 || strcmp(topic, ALARM_SYNC_TIMESTAMP) == 0)
    {
        updateSysTime((unsigned long)(atoll(value) / 1000));    // convert ms to secs

        PublishAlarmStatus();   // on time sync - publish our state

        return;
    }

    if (strcmp(topic, CMD_ALARM_STATUS) == 0)
    {
        PublishAlarmStatus();
        return;
    }

    if (strcmp(topic, CMD_ALARM_BACKLIGHT) == 0)
    {
        if (value =="on")
            setBacklight(true);
        if (value == "off")
            setBacklight(false);
        return;
    }

    if (strcmp(topic, CMD_ALARM_BEEP) == 0)
    {
        JsonObject &root = jsonBuffer.parseObject(value);
        Beeper.Beep(root["on"], root["off"], root["repeat"]);
        return;
    }

    if (strcmp(topic, CMD_ALARM_TONE) == 0)
    {
        JsonObject &root = jsonBuffer.parseObject(value);
        Beeper.Tone(root["freq"], root["duration"]);
        return;
    }

    if (strcmp(topic, CMD_ALARM_CODE_ADD) == 0)
    {
        StateMachine.addCode(value);
        return;
    }

    if (strcmp(topic, CMD_ALARM_CODE_DEL) == 0)
    {
        StateMachine.deleteCode(value);
        return;
    }

    if (strcmp(topic, CMD_ALARM_RFID_ADD) == 0)
    {
        byte uid[4];

        if (UidFromJson(jsonBuffer, uid, value))
            RFID.AddCard(uid);
        return;
    }

    if (strcmp(topic, CMD_ALARM_RFID_DEL) == 0)
    {
        byte uid[4];

        if (UidFromJson(jsonBuffer, uid, value))
            RFID.DeleteCard(uid);
        return;
    }

    if (strcmp(topic, CMD_ALARM_RFID_MASTER) == 0)
    {
        byte uid[4];

        if (UidFromJson(jsonBuffer, uid, value))
            RFID.SetMasterCard(uid);
        return;
    }

    if (strcmp(topic, CMD_ALARM_RFID_WIPE) == 0)
    {
        RFID.WipeData();
        return;
    }

    if (strcmp(topic, CMD_ALARM_RESTART) == 0)
    {
        Debug.println("External Command Restart...");
        esp_restart();
    }
}
