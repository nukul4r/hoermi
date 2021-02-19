#include <Arduino.h>

// DISPLAY
// https://github.com/greiman/SSD1306Ascii/blob/master/examples/HelloWorldWire/HelloWorldWire.ino
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiWire oled;


// TEMPERATURE
// https://lastminuteengineers.com/multiple-ds18b20-arduino-tutorial/
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// RTC
//
#include <RTClib.h>
RTC_DS1307 rtc;

// RC
// https://daniel-ziegler.com/arduino/mikrocontroller/2017/06/16/Funksteckdose-arduino/
#include <RCSwitch.h>
RCSwitch sender = RCSwitch();

// declarations for setup
void setup();
void setupTempSensors();
void setupDisplay();
void setupTempSensors();
void setupRtc();
void setupRc();

// declarations for loop
void loop();
void stepOrReset();
void checkAndSwitchHeating();
bool shouldSwitchOnDay();
bool shouldSwitchOnNight();
bool needsHeatingDay();
bool needsHeatingNight();
bool isDay();
void heating(int id, bool state);
void nl();

void setup() {
  Serial.begin(9600);

  setupDisplay();
  setupTempSensors();
  setupRtc();
  setupRc();
}

void setupDisplay() {
  Wire.begin();
  Wire.setClock(400000L);

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);
}

void setupTempSensors() {
  sensors.begin();

  oled.clear();
  oled.print("Found ");
  oled.print(sensors.getDeviceCount(), DEC);
  oled.print(" temp. sensors");
  
  delay(1000);
  oled.clear();
}

void setupRtc() {
#ifndef ESP8266
  while (!Serial);
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void setupRc() {
  sender.enableTransmit(7);
  sender.setProtocol(1);
  sender.setPulseLength(415);
}

float dayTemp;
float nightTemp;
int currentStep = 0;
// 1 step = 2.3 sec -> 10 min = 260 steps
const int SWITCH_INTERVAL_STEPS = 260;

void loop() {
  DateTime time = rtc.now();
  int startTime = time.unixtime();
  
  oled.setCursor(0,0);

  oled.print(String(time.timestamp(DateTime::TIMESTAMP_DATE)));
  oled.print("   ");
  oled.print(String(time.timestamp(DateTime::TIMESTAMP_TIME)));
  nl();

  int remaining = SWITCH_INTERVAL_STEPS - currentStep;
  if (remaining != 0) {
    oled.print("Next switch: ");
    oled.print(remaining);
    oled.print("    ");
  } else {
    oled.print("Sending switch now...");    
  }

  nl();

  sensors.requestTemperatures();
  dayTemp = sensors.getTempCByIndex(0);
  nightTemp = sensors.getTempCByIndex(1);

  //head
  oled.print("      Day    Night");
  nl();
  oled.print("---------------------");
  nl();

  //Line 1: Current Temp
  oled.print("Tmp   ");

  oled.print(dayTemp);
  oled.print((char)9);
  oled.print("C");

  if (dayTemp < 10) {
    oled.print("  ");  
  } else {
    oled.print(" ");
  }
  

  oled.print(nightTemp);
  oled.print((char)9);
  oled.print("C");
  nl();

  //Line 2: Day/Night Mode
  oled.print("Prfl  ");
  oled.print(isDay() ? "true   " : "false  ");
  oled.print(!isDay() ? "true " : "false");
  nl();

  //Line 3: Temp Limit
  oled.print("Heat  ");
  oled.print(needsHeatingDay() ? "true   " : "false  ");
  oled.print(needsHeatingNight() ? "true " : "false");
  nl();

  //Line 4: Switch status

  oled.print("Swtch ");
  oled.print(shouldSwitchOnDay() ? "true   " : "false  ");
  oled.print(shouldSwitchOnNight() ? "true " : "false");
  nl();

  if (currentStep == 0) {
    checkAndSwitchHeating();  
  }
  
  delay(500);
  stepOrReset();
}

void stepOrReset() {
  if (currentStep >= SWITCH_INTERVAL_STEPS) {
    currentStep = 0;
  } else {
    currentStep = currentStep + 1;
  }
}

void checkAndSwitchHeating() {
  if (shouldSwitchOnDay()) {
    heating(1, true);
  } else {
    heating(1, false);
  }

  if (shouldSwitchOnNight()) {
    heating(2, true);
  } else {
    heating(2, false);
  }
}

bool shouldSwitchOnDay() {
  return (isDay() && needsHeatingDay());
}

bool shouldSwitchOnNight() {
  return (!isDay() && needsHeatingNight());
}

bool needsHeatingDay() {
  return dayTemp < 38;
}

bool needsHeatingNight() {
  return nightTemp < 15;
}

bool isDay() {
  DateTime time = rtc.now();
  int hour = time.hour();
  return (hour >= 9 && hour < 15);
}

void heating(int id, bool state) {
    if (id == 1) {
      if (state) {
        sender.send("000101010001010101010101");
      } else {
        sender.send("000101010001010101010100");
      }
    }
  
    if (id == 2) {
      if (state) {
        sender.send("000101010100010101010101");
      } else {
        sender.send("000101010100010101010100");
      }
    }
}

void nl() {
  oled.println("");
}
