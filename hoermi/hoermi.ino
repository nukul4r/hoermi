// DISPLAY
//
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
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
#include "RTClib.h"
RTC_DS1307 rtc;

// FUNK
// https://daniel-ziegler.com/arduino/mikrocontroller/2017/06/16/Funksteckdose-arduino/

void setup() {
  Serial.begin(9600);
  
  setupDisplay();
  setupTempSensors();
  setupRtc();
}

void setupDisplay(void) {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); 
  }
  
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();
}

void setupTempSensors(void) {
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

void setupRtc(void) {
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

void loop() {
 DateTime time = rtc.now();
   
 Serial.print("Temperature is: "); 
 Serial.println(sensors.getTempCByIndex(0));
 
 display.clearDisplay();
 display.setTextSize(1);
 display.setTextColor(SSD1306_WHITE);
 
 display.setCursor(0,0);
 display.print(String(time.timestamp(DateTime::TIMESTAMP_DATE)));

 display.setCursor(0,12);
 display.print(String(time.timestamp(DateTime::TIMESTAMP_TIME)));

 sensors.requestTemperatures();

 display.setCursor(0,24);
 display.print("Temp 1: ");
 display.print(sensors.getTempCByIndex(0));
 display.print((char)9);
 display.print("C");

 display.setCursor(0,36);
 display.print("Temp 2: ");
 display.print(sensors.getTempCByIndex(1));
 display.print((char)9);
 display.print("C");
 
 display.display();
 
 delay(500);

}

String getDateTimeFull() {
  DateTime time = rtc.now();
  return String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL);
}
