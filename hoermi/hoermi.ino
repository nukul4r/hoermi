// DISPLAY
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4); // Change to (0x27,20,4) for 20x4 LCD.


// TEMPERATURE EXT
// https://lastminuteengineers.com/multiple-ds18b20-arduino-tutorial/
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4 //data = 4
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

void setup() {
  Serial.begin(9600);

  setupDisplay();
  setupTempSensors();
  setupRtc();
  setupRc();
}

void setupDisplay(void) {
  lcd.init();
  lcd.backlight();
}

void setupTempSensors() {
  sensors.begin();

  lcd.clear();
  lcd.print("Temp. sensors: ");
  lcd.print(sensors.getDeviceCount(), DEC);
   
  delay(1000);
  lcd.clear();
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
boolean lastDisplay = true;

void loop() {

  DateTime time = rtc.now();
  
  if ((time.second() / 10) % 2 == 0) {
    displayStatus();
  } else {
    displaySwitch();
  }
  
  if (currentStep == 0) {
    checkAndSwitchHeating();  
  }
  
  delay(500);
  stepOrReset();
}

void displayTemp() {
  sensors.requestTemperatures();
  dayTemp = sensors.getTempCByIndex(0);
  nightTemp = sensors.getTempCByIndex(1);

  lcd.setCursor(0,0);

  lcd.print("D ");
  lcd.print(dayTemp);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor(10,0);
  lcd.print("N ");
  lcd.print(nightTemp);
  lcd.print((char)223);
  lcd.print("C");
}

void displayStatus() {
  if (lastDisplay) {
    lcd.clear();
  }

  lastDisplay = false;

  //Line 1: Temperature
  displayTemp();
  
  DateTime time = rtc.now();
  //int startTime = time.unixtime();

  //Line 3: Current time
  lcd.setCursor(0,2);
  lcd.print(String(time.timestamp(DateTime::TIMESTAMP_DATE)));
  lcd.print(" ");
  lcd.print(String(time.timestamp(DateTime::TIMESTAMP_TIME)));

  //Line 4: Next switch
  lcd.setCursor(0,3);
  int remaining = SWITCH_INTERVAL_STEPS - currentStep;
  if (remaining != 0) {
    lcd.print("Next switch: ");
    lcd.print(remaining);
  } else {
    lcd.print("Switching...");    
  }
}

void displaySwitch() {
  if (!lastDisplay) {
    lcd.clear();
  }

  lastDisplay = true;

  //Line 1: Temperature
  displayTemp();

  //Line 2: Day/Night Mode
  lcd.setCursor(0,1);
  lcd.print("   Profile: ");
  lcd.print(isDay() ? "Day" : "Night");

  //Line 3: Temp Limit
  lcd.setCursor(0,2);
  lcd.print("Needs heat: ");
  if (isDay()) {
    lcd.print(needsHeatingDay() ? "yes" : "no");
  } else {
    lcd.print(needsHeatingNight() ? "yes" : "no");
  }

  //Line 4: Switch status
  lcd.setCursor(0,3);
  lcd.print(" Switch on: ");
  if (isDay()) {
    lcd.print(shouldSwitchOnDay() ? "yes" : "no");
  } else {
    lcd.print(shouldSwitchOnNight() ? "yes" : "no");
  }
}

void displayNight() {
  
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
  //return (!isDay() && needsHeatingNight());
  return needsHeatingNight();
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
  //oled.println("");
}
