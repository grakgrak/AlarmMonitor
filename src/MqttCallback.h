#ifndef MQTTCALLBACK_H
#define MQTTCALLBACK_H
#include <Arduino.h>

extern void mqttCallback(char *topic, byte *payload, unsigned len);

extern const char *ALARM_SYNC_TIMESTAMP;


#endif // MQTTCALLBACK_H