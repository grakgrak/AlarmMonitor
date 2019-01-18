#ifndef SHARED_H
#define SHARED_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <nvs_flash.h>
#include <nvs.h>

#include "Beeper.h"
#include "Button.h"
#include "Debug.h"
#include "DigitalPin.h"
#include "KeyPad.h"
#include "Mqtt.h"
#include "MqttCallback.h"
#include "RFID.h"
#include "StateMachine.h"

#define DOOR_PIN 26
#define SENSOR7_PIN 25
#define SENSOR6_PIN 32
#define SENSOR5_PIN 33
#define SENSOR4_PIN 35
#define SENSOR3_PIN 34
#define SENSOR2_PIN 39
#define SENSOR1_PIN 36

#define RFID_CS_PIN 17
#define RFID_RST_PIN 16

#define BACKLIGHT_PIN 2
#define TONE_PIN 4
#define BEEP_PIN 15

#define CLK_PIN 14
#define SDA_PIN 27

//#define BUTTON_OK_PIN 14
//#define BUTTON_CANCEL_PIN 12
#define BUTTONLIGHT_PIN 13

// these are actually set in User_Setup.h
// if the TFT_eSPI.h library is updated then copy these values back to User_Setup.h
//#define ILI9341_DRIVER
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS 5    // Chip select control pin
#define TFT_DC 22   // Data Command control pin
#define TOUCH_CS 21 // Chip select pin (T_CS) of touch screen

#define FONT_NORMAL 4
#define FONT_LARGE 8

extern TFT_eSPI tft;

extern TDigitalPin SensorDoor;
extern TDigitalPin Sensor1;
extern TDigitalPin Sensor2;
extern TDigitalPin Sensor3;
extern TDigitalPin Sensor4;
extern TDigitalPin Sensor5;
extern TDigitalPin Sensor6;
extern TDigitalPin Sensor7;

extern void updateSysTime(unsigned long dt);
extern unsigned long Now();
extern unsigned long upTime();
extern unsigned long startTime();

extern String secsToTime(unsigned long duration);

extern bool isBacklightOn();
extern void setBacklight(bool state);
//extern TButtonEvent CheckForButtonPress();
extern int checkForKeyPress(const TKeyPad keys[], int keyCount);
extern void PublishAlarmStatus();

#endif //SHARED_H
