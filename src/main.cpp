#include "Wire.h"
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <rom/rtc.h>
#include <WiFiUdp.h>    // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h
#include <ArduinoOTA.h> // https://github.com/esp8266/Arduino/blob/master/libraries/ArduinoOTA/ArduinoOTA.h
#include "Max44009.h"
#include "../../Credentials.h"
#include "Shared.h"

#define VERSION "1.1"

#define WIFI_CONNECT_TIMEOUT 12000
#define MQTT_CONNECT_TIMEOUT (WIFI_CONNECT_TIMEOUT + 10000)
#define TASK_STACK_SIZE 3000
//--------------------------------------------------------------------
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *hostname = "Terrorem";


#define RESET_REASON_COUNT  17
const char *_ResetReasons[RESET_REASON_COUNT] = {
    "",
    "POWERON_RESET", /**<1, Vbat power on reset*/
    "EXT_RESET", /**<2, External reset*/
    "SW_RESET", /**<3, Software reset digital core*/
    "OWDT_RESET", /**<4, Legacy watch dog reset digital core*/
    "DEEPSLEEP_RESET", /**<5, Deep Sleep reset digital core*/
    "SDIO_RESET", /**<6, Reset by SLC module, reset digital core*/
    "TG0WDT_SYS_RESET", /**<7, Timer Group0 Watch dog reset digital core*/
    "TG1WDT_SYS_RESET", /**<8, Timer Group1 Watch dog reset digital core*/
    "RTCWDT_SYS_RESET", /**<9, RTC Watch dog Reset digital core*/
    "INTRUSION_RESET", /**<10, Instrusion tested to reset CPU*/
    "TGWDT_CPU_RESET", /**<11, Time Group reset CPU*/
    "SW_CPU_RESET", /**<12, Software reset CPU*/
    "RTCWDT_CPU_RESET", /**<13, RTC Watch dog Reset CPU*/
    "EXT_CPU_RESET", /**<14, for APP CPU, reseted by PRO CPU*/
    "RTCWDT_BROWN_OUT_RESET", /**<15, Reset when the vdd voltage is not stable*/
    "RTCWDT_RTC_RESET" /**<16, RTC Watch dog reset digital core and rtc module*/
};

//--------------------------------------------------------------------
// Global Instances
//--------------------------------------------------------------------
Max44009 *myLux = NULL;
TFT_eSPI tft;
TaskHandle_t _beepHandle;

//--------------------------------------------------------------------
// Code to run a screen calibration, not needed when calibration values set in setup()
void touch_calibrate()
{
  uint16_t calData[5];
  //uint8_t calDataOK = 0;

  // Calibrate
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  tft.setTextFont(1);
  tft.println();

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  Serial.println(); Serial.println();
  Serial.println("// Use this calibration code in setup():");
  Serial.print("    uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("    tft.setTouch(calData);");
  Serial.println(); Serial.println();

  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Calibration complete!");
  tft.println("Calibration code sent to Serial port.");
  for (uint8_t i = 0; i < 5; i++)
  {
    tft.print(calData[i]);
    if (i < 4) tft.print(", ");
  }

  delay(10000);
}

//--------------------------------------------------------------------
void touch_calibrate_loop()
{
  uint16_t x = 0, y = 0; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  boolean pressed = getTouch(&x, &y);

  // Draw a white spot at the detected coordinates
  if (pressed) {
    tft.fillCircle(x, y, 2, TFT_WHITE);
    Serial.print("x,y = ");
    Serial.print(x);
    Serial.print(",");
    Serial.println(y);
  }
}

//--------------------------------------------------------------------
const char *resetReason(int reason)
{
    if( reason < RESET_REASON_COUNT)
        return _ResetReasons[reason + 1];
    return "Unknown";
}
//--------------------------------------------------------------------
void PublishAllSensors()
{
    SensorDoor.PublishStatus();
    Sensor1.PublishStatus();
    Sensor2.PublishStatus();
    Sensor3.PublishStatus();
    Sensor4.PublishStatus();
    Sensor5.PublishStatus();
    Sensor6.PublishStatus();
    Sensor7.PublishStatus();
}
//--------------------------------------------------------------------
void PublishEnvironment()
{
    StaticJsonDocument<200> jsonBuffer;

    jsonBuffer["Version"] = VERSION;
    jsonBuffer["UpTime"] = upTime();
    jsonBuffer["StartTime"] = startTime();
    //j["IP"] = WiFi.localIP().toString();
    //j["MQTT-IP"] = String(Mqtt.server());
    jsonBuffer["AlarmState"] = StateMachine.stateName();
    
    //j["Stack"] = uxTaskGetStackHighWaterMark(NULL);
    //j["Heap"] = ESP.getFreeHeap();
    //j["BeepStack"] = uxTaskGetStackHighWaterMark(_beepHandle);
    //j["FlashSize"] = ESP.getFlashChipSize();

    Mqtt.publish(ALARM_STATUS_ENV, jsonBuffer);
}
//--------------------------------------------------------------------
void PublishResetReason()
{
    StaticJsonDocument<200> jsonBuffer;

    jsonBuffer["Reset-CPU0:"] = resetReason(0);
    jsonBuffer["Reset-CPU1:"] = resetReason(1);

    Mqtt.publish(ALARM_STATUS_RESET, jsonBuffer);
}
//--------------------------------------------------------------------
void PublishAlarmStatus()
{
    PublishEnvironment();
    PublishResetReason();
    PublishAllSensors();
}

//--------------------------------------------------------------------
void beepTask(void *pvParameters)
{
    for (;;)
    {
        Beeper.loop();
        vTaskDelay(1); // 1 ms delay
    }
    vTaskDelete(NULL); // should never get here
}

//--------------------------------------------------------------------
bool isWiFiConnected(int timeout)
{
    static unsigned long lastConnectedTime = 0;

    if (WiFi.status() != WL_CONNECTED)
    {
        setBacklight(true); // switch on the display

        if ((millis() - lastConnectedTime) > WIFI_CONNECT_TIMEOUT) // allow time to connect to WiFi - if not then restart
        {
            Debug.println("Wifi Connect Timeout Restart...");
            esp_restart();
        }
        delay(timeout);
    }
    else
        lastConnectedTime = millis();

    return WiFi.status() == WL_CONNECTED;
}

//--------------------------------------------------------------------
void init_nvs()
{
    Debug.println("Init nvs.");
    // initialise the NVS
    switch (nvs_flash_init())
    {
    case ESP_OK:
        break;
    case ESP_ERR_NVS_NO_FREE_PAGES:
        Debug.println("nvs has been erased.");
        nvs_flash_erase();
        nvs_flash_init();
        break;
    default:
        Debug.println("Failed to init the nvs");
        break;
    }
}

//--------------------------------------------------------------------
void init_tft()
{
    Debug.println("Init TFT.");

    // Setup the TFT display
    tft.init();
    delay(50);
    tft.setRotation(3);

    // Calibrate the touch screen and retrieve the scaling factors
    uint16_t calData[5] = {247, 3541, 168, 3566, 7};
    tft.setTouch(calData);

    tft.setTextWrap(false);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 0);
}

//--------------------------------------------------------------------
void init_wifi()
{
    Debug.println("Init Wifi.");

    // Wifi setup
    WiFi.begin(ssid, password);
    WiFi.setHostname("Terrorem");

    while (isWiFiConnected(750) == false)
        Debug.println("Connecting to WiFi...");
    Debug.println("WiFi Connected - " + WiFi.localIP().toString());
}

//--------------------------------------------------------------------
void init_mqtt()
{
    Debug.println("Init MQTT.");

    // MQTT setup
    Mqtt.init(mqttCallback);
    while (Mqtt.isConnected(500) == false)
    {
        Debug.println("Connecting to MQTT...");
        delay(1000);

        if (millis() > MQTT_CONNECT_TIMEOUT) // allow time to connect to mqtt - if not then restart
        {
            Debug.println("MQTT Connect Timeout Restart...");
            esp_restart();
        }
    }
    Debug.println("MQTT Connected.");
}
//--------------------------------------------------------------------
void init_beeper()
{
    // start the beeper task - with the same priority as this task
    Debug.println("Init Beeper Task.");
    xTaskCreate(beepTask, "Beeper", TASK_STACK_SIZE, NULL, uxTaskPriorityGet(NULL), &_beepHandle);
}
//--------------------------------------------------------------------
void init_rfid()
{
    // init the RFID
    Debug.println("Init RFID.");
    RFID.init();
}
//--------------------------------------------------------------------
void init_OTA()
{
    // ArduinoOTA callback functions
    ArduinoOTA.onStart([]() {
        Debug.println("OTA starting...");
    });

    ArduinoOTA.onEnd([]() {
        Debug.println("OTA done.Reboot...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static unsigned int prevPcnt = 100;
        unsigned int pcnt = (progress / (total / 100));
        unsigned int roundPcnt = 5 * (int)(pcnt / 5);
        if (roundPcnt != prevPcnt)
        {
            prevPcnt = roundPcnt;
            Debug.println("OTA upload " + String(roundPcnt) + "%");
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Debug.print("OTA Error " + String(error) + ":");
        const char *line2 = "";
        switch (error)
        {
        case OTA_AUTH_ERROR:
            line2 = "Auth Failed";
            break;
        case OTA_BEGIN_ERROR:
            line2 = "Begin Failed";
            break;
        case OTA_CONNECT_ERROR:
            line2 = "Connect Failed";
            break;
        case OTA_RECEIVE_ERROR:
            line2 = "Receive Failed";
            break;
        case OTA_END_ERROR:
            line2 = "End Failed";
            break;
        }
        Debug.println(line2);
    });

    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPassword("terrorem");

    ArduinoOTA.begin();
}
//--------------------------------------------------------------------
void setup()
{
    Serial.begin(115200); // for debug
    delay(10);
    Debug.println("Alarm Starting");

    // setup the backlight pin
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, true);

    // setup the button light
    pinMode(BUTTONLIGHT_PIN, OUTPUT);
    digitalWrite(BUTTONLIGHT_PIN, false); // inverse of backlight state

    init_nvs();
    init_tft();
    init_wifi();
    init_mqtt();
    init_beeper();
    init_rfid();
    init_OTA();

    StateMachine.init();

    myLux = new Max44009(0x4A, SDA_PIN, CLK_PIN);

    Debug.println("Setup complete.");
    Debug.ShowOnTFT(false);

    // Calibrate the touch screen and retrieve the scaling factors
    // touch_calibrate();

    // touch screen calibration data
    uint16_t calData[5] = { 416, 3509, 267, 3531, 1 };
    tft.setTouch(calData);

    tft.fillScreen(TFT_BLACK);
}

//--------------------------------------------------------------------
void PeriodicAction(unsigned long now)
{
    static unsigned long lastHeartbeat = 0;

    if ((now - lastHeartbeat) >= 1000) // check for 1 second passing
    {
        if (lastHeartbeat != 0)
        {
            StaticJsonDocument<200> jsonBuffer;

            // send out the ambient light level
            if( myLux != NULL && (myLux->getError() == 0))
                jsonBuffer["lux"] = myLux->getLux();
            jsonBuffer["UpTime"] = upTime();

            Mqtt.publish(ALARM_HEARTBEAT, jsonBuffer);

            // send out the ambient light level
            // if( myLux != NULL)
            // {
            //     if (myLux->getError() == 0)
            //         Mqtt.publish(ALARM_CURRENT_LUX, String(myLux->getLux()));   // publish the light level
            // }

            // Mqtt.publish(ALARM_HEARTBEAT_UPTIME, String(upTime())); // tell the world we are still alive
        }
        lastHeartbeat = now;
    }
}

//--------------------------------------------------------------------
void loop()
{
    // touch_calibrate_loop();

    if (isWiFiConnected(750))
    {
        ArduinoOTA.handle();

        Mqtt.loop(); // checks connection and processes incomming messages

        StateMachine.loop();

        PeriodicAction(millis());
    }
    delay(1);
}
