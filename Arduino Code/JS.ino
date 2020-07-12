
#define LR_pin A0 //anolog pin for X
#define UD_pin A1 //anolog pin for Y

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library


//Variables to change
//////////////////////////////////////////////////////////////////////
// Set the LCD address to  for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);//0x27  0x3F


int threshold = 30;
int SolOpenDuration = 200;
int DelayToRew = 850; 
//////////////////////////////////////////////////////////////////////


int ITI = 3000; // ms. Intertrial Interval. SET

int solenoid = 13; //pin out to solenoid

int Y = 0;
int X = 0;
int baseY = 0;
int baseX = 0;

int TrialCt = 0;


unsigned long SolOpenTime = 0;
unsigned long SolCloseTime = 0;
unsigned long thresholdcrossTime = 0;
unsigned long ms = 0;
float pos;

int EM = 0;
/*Eventmarkers
  0 = not in trial
  1 = threshold met - delay
  2 = sol open
  3 = iti start
*/

void setup() {
  pinMode(solenoid, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200); //set corresponding baud rate in Ardunio Serial Monitor
  
  baseX = analogRead(LR_pin);
  baseY = analogRead(UD_pin);

  lcd.backlight();
  lcd.begin();
}

//strings to int
void loop() {
  lcd.clear();
  Y = analogRead(UD_pin) - baseY;  X = analogRead(LR_pin) - baseX; //update X and Y pos
  pos = sqrt(pow(X, 2) + pow(Y, 2)); //update js pos
  Y = analogRead(UD_pin);  X = analogRead(LR_pin); // redefine X and Y

  ms = millis();
  switch (EM) {
    case 0: // read joystick position
      if (pos > threshold) {
        thresholdcrossTime = millis(); EM = 1;
      }
      else {
        EM = 0;
      }
      break;

    case 1: // delay 
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        EM = 2;
      }
      else {
        EM = 1;
      }
      break;

    case 2: // deliver reward
      if ( SolOpenDuration <= millis() - SolOpenTime) {
        SolCloseTime = millis(); digitalWrite(solenoid, LOW); digitalWrite(LED_BUILTIN, LOW);
        EM = 3;
      }
      else {
        EM = 2;
      }
      break;

    case 3: // iti
      if (ITI <= millis() - SolCloseTime) {
        baseY = analogRead(UD_pin); baseX = analogRead(LR_pin);
        TrialCt++;
        EM = 0;
      }
      else {
        EM = 3;
      }
      break;
  }
  
  lcd.print(String(TrialCt));
  lcd.setCursor (5, 0);
  lcd.print(String(threshold));
  lcd.setCursor (9, 0);
  lcd.print(String(pos));
  lcd.setCursor (0, 1);
  lcd.print(String((ms / 1000) / 60));
  lcd.setCursor (11, 1);
  lcd.print("JS2");
  
  Serial.println(String(ms) + ',' +  String(EM) + ',' +  String(TrialCt) + ','  + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold));

}

// [time EM TrialCt X Y pos baseX baseY SolOpenDuration DelaytoRew ITI Threshold]
