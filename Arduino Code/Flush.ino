/*
This sketch can be used to drain reservoir and flush/clean lines and solenoid

Opens solenoid for 100 seconds and closes it for 1 second continuously

To clean lines/solenoid run 10% bleach through then flush with clean water a few times
MAN
*/
int solenoid = 13;

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

// Set the LCD address to  for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);//0x27  0x3F

void setup() {
  pinMode(solenoid, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);//set corresponding baud rate in Ardunio Serial Monitor
  lcd.backlight();
  lcd.begin();
}

void loop() {
  digitalWrite(solenoid, HIGH); lcd.clear(); lcd.print("open");
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("open");
  delay(100000);
  digitalWrite(solenoid, LOW); lcd.clear(); Serial.println("closed");
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("closed");
  delay(1000);
}
