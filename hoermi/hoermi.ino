// DISPLAY
//
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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
RCSwitch rcSender = RCSwitch();

void setup() {
  Serial.begin(9600);
  
  setupDisplay();
  setupTempSensors();
  setupRtc();
  setupRc();
}

void setupDisplay(void) {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
}

void setupTempSensors() {
  sensors.begin();

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Found ");
  display.print(sensors.getDeviceCount(), DEC);
  display.println(" devices.");
  display.println("");

   display.display();
   delay(1000);
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
  rcSender.enableTransmit(7);
  rcSender.setProtocol(1);
  rcSender.setPulseLength(415);
}

float dayTemp;
float nightTemp;

void loop() {
 DateTime time = rtc.now();
 
 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(SSD1306_WHITE);
 
 display.setCursor(0,0);
 display.print(String(time.timestamp(DateTime::TIMESTAMP_DATE)));

 display.setCursor(80,0);
 display.print(String(time.timestamp(DateTime::TIMESTAMP_TIME)));

 sensors.requestTemperatures();
 dayTemp = sensors.getTempCByIndex(0);
 nightTemp = sensors.getTempCByIndex(1);

 int colDay = 36;
 int colNight = 85;

 int lineHeight = 11;
 int head = 11;
 int line1 = 24;
 int line2 = lineHeight + line1;
 int line3 = lineHeight + line2;
 int line4 = lineHeight + line3;

 //head
 display.setCursor(colDay,head);
 display.print("Day");

 display.setCursor(colNight,head);
 display.print("Night");
 
 display.drawLine(0, 21,  display.width()-1, 21, SSD1306_WHITE);

 //Line 1: Current Temp
 display.setCursor(0,line1);
 display.print("Tmp");

 display.setCursor(colDay,line1);
 display.print(dayTemp);
 display.print((char)9);
 display.print("C");

 display.setCursor(colNight,line1);
 display.print(nightTemp);
 display.print((char)9);
 display.print("C");

 //Line 2: Day/Night Mode
 display.setCursor(0,line2);
 display.print("Prfl");

 display.setCursor(colDay,line2);
 display.print(isDay() ? "true" : "false");

 display.setCursor(colNight,line2);
 display.print(!isDay() ? "true" : "false");

 //Line 3: Temp Limit
 display.setCursor(0,line3);
 display.print("Heat");

 display.setCursor(colDay,line3);
 display.print(needsHeatingDay() ? "true" : "false");

 display.setCursor(colNight,line3);
 display.print(needsHeatingNight() ? "true" : "false");

 //Line 4: Switch status

 display.setCursor(0,line4);
 display.print("Swtch");

 display.setCursor(colDay,line4);
 display.print(shouldSwitchOnDay() ? "true" : "false");

 display.setCursor(colNight,line4);
 display.print(shouldSwitchOnNight() ? "true" : "false");

 display.display();

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
 
 delay(500);

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
      if (rtc.now().second() == 10) {
        rcSender.send("000101010001010101010100");  
      }
      
  
//  if (id == 1) {
//    if (state) {
//      sender.send("000101010001010101010101");
//    } else {
//      sender.send("000101010001010101010100");
//    }
//  }
//
//  if (id == 2) {
//    if (state) {
//      sender.send("000101010100010101010101");
//    } else {
//      sender.send("000101010100010101010100");
//    }
//  }
}
