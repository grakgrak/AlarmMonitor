#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H
#include <Arduino.h>
#include <PubSubClient.h>
#include "Shared.h"

// publish topics
extern const char *SYS_LOGIN_NAME;
extern const char *ALARM_CURRENT_STATE;
extern const char *ALARM_HEARTBEAT_UPTIME;
extern const char *ALARM_RFID_CARD;
extern const char *ALARM_RFID_COUNT;
extern const char *ALARM_RFID_UID;
extern const char *ALARM_STATUS_ENV;
extern const char *ALARM_STATUS_RESET;
extern const char *ALARM_LOG_TRACE;

extern const char *ALARM_SENSOR; // partial topic

//--------------------------------------------------------------------

class TMqtt
{
public:
  static bool isConnected(unsigned long timeout);

  static bool init(MQTT_CALLBACK_SIGNATURE);
  static bool loop();
  static void publish(const char *topic, const char *payload);
  static void publish(const String &topic, const String &payload);
  static void publish(const String &topic, const JsonObject &j);

  static const char *server();
};

extern TMqtt Mqtt;

#endif // MQTTCLIENT_H