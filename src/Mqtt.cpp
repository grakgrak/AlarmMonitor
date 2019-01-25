#include "Mqtt.h"
#include "Shared.h"
#include <WiFi.h>

// listening topics
const char *SYS_SYNC_HASH = "sys/sync/#";
const char *CMD_ALARM_HASH = "cmd/alarm/#";

// publish topics
const char *SYS_LOGIN_NAME = "sys/login/name";
const char *ALARM_STATUS = "alarm/status";
const char *ALARM_CURRENT_LUX = "alarm/current/lux";
const char *ALARM_CURRENT_STATE = "alarm/current/state";
const char *ALARM_STATUS_ENV = "alarm/status/env";
const char *ALARM_STATUS_RESET = "alarm/status/reset";
const char *ALARM_HEARTBEAT = "alarm/heartbeat";
const char *ALARM_HEARTBEAT_UPTIME = "alarm/heartbeat/uptime";
const char *ALARM_RFID_UID = "alarm/rfid/uid";
const char *ALARM_RFID_CARD = "alarm/rfid/card";
const char *ALARM_RFID_COUNT = "alarm/rfid/count";
const char *ALARM_LOG_TRACE = "alarm/log/trace";
const char *ALARM_SENSOR = "alarm/sensor/"; // partial topic

//--------------------------------------------------------------------
const char *_mqttServer = "192.168.1.210";
const int _mqttPort = 1883;
const char *_mqttUser = "";
const char *_mqttPassword = "";

WiFiClient _wifiClient;
PubSubClient _mqttClient(_wifiClient);

TMqtt Mqtt; // Global Instance

MQTT_CALLBACK_SIGNATURE;
//--------------------------------------------------------------------
const char *TMqtt::server()
{
    return _mqttServer;
}
//--------------------------------------------------------------------
bool TMqtt::init(MQTT_CALLBACK_SIGNATURE)
{
    _mqttClient.setServer(_mqttServer, _mqttPort);
    ::callback = callback;
    _mqttClient.setCallback(callback);
    return true;
}
//--------------------------------------------------------------------
void TMqtt::publish(const char *topic, const char *payload)
{
    if(strcmp(topic, ALARM_CURRENT_LUX) != 0)   // dont log the LUX
        if (strcmp(topic, ALARM_HEARTBEAT_UPTIME) != 0) // dont log the heatbeat
        {
            Debug.printf("> %s:%s\n", topic, payload);
        }

    if (_mqttClient.connected())
        if (_mqttClient.publish(topic, payload) == false)
        {
            Debug.printf("Mqtt failed to publish %s:%s\n", topic, payload);
            Debug.printf("Mqtt State is: %d\n", _mqttClient.state());
            Debug.printf("Mqtt Connected: %d\n", _mqttClient.connected());
            _mqttClient.disconnect();
            Debug.printf("Mqtt Client disconnected. %d\n", _mqttClient.state());
        }
}

//--------------------------------------------------------------------
void TMqtt::publish(const String &topic, const String &payload)
{
    publish(topic.c_str(), payload.c_str());
}

//--------------------------------------------------------------------
void TMqtt::publish(const String &topic, const JsonObject &j)
{
    String payload;
    j.printTo(payload);
    publish(topic.c_str(), payload.c_str());
}

//--------------------------------------------------------------------
bool TMqtt::isConnected(unsigned long timeout)
{
    static unsigned long _lastAttempt = 0;

    // check we are connected to MQTT broker
    if (_mqttClient.state() != MQTT_CONNECTED)
    {
        setBacklight(true); // switch on the display

        //  dont re-try this connection too often
        if ((millis() - _lastAttempt) > timeout)
        {
            Debug.println("Connecting to MQTT Server...");
            Debug.printf("MQTTClient State = %d\n", _mqttClient.state());

            _mqttClient.setCallback(callback);

            // connect with a LWT message
            if (_mqttClient.connect("Alarm", _mqttUser, _mqttPassword, ALARM_STATUS, 0, true, "offline"))
            {
                // subscribe to
                _mqttClient.subscribe(SYS_SYNC_HASH);
                _mqttClient.subscribe(ALARM_SYNC_TIMESTAMP);
                _mqttClient.subscribe(CMD_ALARM_HASH);

                //login to mqtt
                _mqttClient.publish(ALARM_STATUS, "online", true);  // publish to retained topic
                _mqttClient.publish(SYS_LOGIN_NAME, "alarm");
            }
            else
            {
                Debug.println("re-trying connection to MQTT Server...");
                _lastAttempt = millis();
            }
        }
    }

    return _mqttClient.connected();
}
//--------------------------------------------------------------------
bool TMqtt::loop()
{
    // check we have a WIFi connection
    if (WiFi.status() == WL_CONNECTED)
        if (Mqtt.isConnected(5000)) // check we are connected to mqtt
            return _mqttClient.loop();

    return false;
}
