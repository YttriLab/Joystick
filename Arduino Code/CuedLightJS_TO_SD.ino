/*
   Cued Light Joystick task
   Move joystick past threshold in response to cue light to turn off light and receive water, a random ITI will follow
   movements in the last 20% of ITI result in time out with house lights
   
   MAN
*/
#define LR_pin A0 //anolog pin for X
#define UD_pin A1 //anolog pin for Y
#define R A10 // rng seed

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <SD.h>
#include <SPI.h>
File myFile;
File LOG;
int pinCS = 53; //communication pin for SD card reader

//Variables to change
//////////////////////////////////////////////////////////////////////
LiquidCrystal_I2C lcd(0x27, 16, 2); //0x3F  0x27

unsigned long SolOpenDuration = 150; //ms Duration
unsigned long DelayToRew = 1000; //Pause till reward.
float threshold = 90;


unsigned long m = 5000; unsigned long M = 10001; // upperbound exclusive...random 5-10 second ITI
//////////////////////////////////////////////////////////////////////

unsigned long TO = 5000; // can set to 0 and do not put LEDs in pin 11 or 10 for no TO...contact us for no TO version
unsigned long TOStart = 0;
unsigned long ITI; // ms. Intertrial Interval. SET
String tlt;

int led = 12; //pin out to cue light
int solenoid = 13; //pin out to solenoid
int HouseLight = 11; //pin out to houselight 1
int HouseLight2 = 10; //pin out to houselight 2

int Y = 0;
int X = 0;
int baseY = 0;
int baseX = 0;

int TrialCt = 0;
int EM = 0;
int TOCt = 0;

float holdsTimes[] = {0, 0, 0, 0, 0};
float avRxnTime = 0;
float sum = 0;
float pos;

unsigned long SolOpenTime = 0;
unsigned long SolCloseTime = 0;
unsigned long thresholdcrossTime = 0;
unsigned long hold = 0;
unsigned long ms = 0;
unsigned long RT = 0;
unsigned long RTStart = 0;
unsigned long voidLoopTime = 0;
unsigned long voidLoopTimeStart = 0;
int ct = 0;


/*Eventmarkers
  0 = read joystick position
  1 = delay to reward
  2 = water
  3 = random ITI
  4 = turn on cue light
  5 = time out
*/ 


void setup() {
  pinMode(solenoid, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200); //set corresponding baud rate in Ardunio Serial Monitor
  baseX = analogRead(LR_pin);
  baseY = analogRead(UD_pin);
  lcd.backlight();
  lcd.begin();
  digitalWrite(led, HIGH); // turn on cue light
  pinMode(pinCS, OUTPUT);
  pinMode(HouseLight, OUTPUT);
  pinMode(HouseLight2, OUTPUT);
  pinMode(led, OUTPUT);

  randomSeed(analogRead(R));
  ITI = random(m, M);
  hold = ITI * 0.8;
  tlt = String(ITI) + "CL.CSV";
  int ctt;
  File root;
  
  if (SD.begin()) {
    root = SD.open("/");
    ctt = printDirectory(root, 0); ctt = ctt - 2;
    Serial.println(String(ctt));
    if (ctt > 1) {
      TooManyFiles();
      return;
    }
    LOG = SD.open("Log.txt", FILE_READ);
    if (LOG) {
      while (LOG.available()) {
        Serial.println(LOG.readStringUntil('\n') + '_' + String(ct));
        ct++;
      }
      LOG.close();
    }
    else {
      SavingBroken();
      return;
    }
    LOG = SD.open("Log.txt", FILE_WRITE);
    if (LOG) {
      //      Serial.println(String(LOG.position()));
      LOG.seek(LOG.position() + 1);
      LOG.println(String(ct) + ' ' + String(tlt));
      LOG.close();
    }
  }
  else
  {
    SavingBroken();
    return;
  }
}

void loop() {
  voidLoopTimeStart = millis();
  lcd.clear();
  ms = millis();
  Y = analogRead(UD_pin) - baseY;  X = analogRead(LR_pin) - baseX;
  pos = sqrt(pow(X, 2) + pow(Y, 2));
  Y = analogRead(UD_pin);  X = analogRead(LR_pin);

  switch (EM) {
    case 0: // read joystick position
      if (pos > threshold) {
        digitalWrite(led, LOW);
        RT = millis() - RTStart;
        thresholdcrossTime = millis(); EM = 1;
        
        for (int i = 4; i != 0;   i--) {
          holdsTimes[i] = holdsTimes[i - 1];
        }
        holdsTimes[0] = float(RT);
        for (int i = 0; i != 5; i++) {
          sum = holdsTimes[i] + sum;
        }
        avRxnTime = sum / 5;
        sum = 0;
      }
      else {
        EM = 0;
      }
      break;

    case 1: // 1:delay
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        EM = 2;
      }
      else {
        EM = 1;
      }
      break;

    case 2: //close sol
      if ( SolOpenDuration <= millis() - SolOpenTime) {
        SolCloseTime = millis(); digitalWrite(solenoid, LOW); digitalWrite(LED_BUILTIN, LOW);
        TrialCt++;
        EM = 3;

      }
      else {
        EM = 2;
      }
      break;

    case 3: // random ITI
      if (ITI <= millis() - SolCloseTime) {
        baseY = analogRead(UD_pin); baseX = analogRead(LR_pin);
        EM = 4;
        break;
      }

      else if (millis() - SolCloseTime > hold && (pos > threshold) ) {
        TOStart = millis(); digitalWrite(HouseLight, HIGH); digitalWrite(HouseLight2, HIGH);
        EM = 5;
        break;
      }
      else {
        EM = 3;
      }
      break;
      
    case 4: //turn on cue light
      digitalWrite(led, HIGH);
      RTStart = millis();
      ITI = random(m, M);
      hold = ITI * 0.8;
      EM = 0;
      break;

    case 5: // time out and jump to new ITI
      if (TO <= millis() - TOStart) {
        digitalWrite(HouseLight, LOW); digitalWrite(HouseLight2, LOW);
        ITI = random(m, M); // start a new ITI after timeout
        hold = ITI * 0.8;
        SolCloseTime = millis();
        baseY = analogRead(UD_pin); baseX = analogRead(LR_pin);
        EM = 3;
        TOCt++;
        break;
      }
      else {
        EM = 5;
      }
      break;
  }

  lcd.print(String(TrialCt)); // number of trials
  lcd.setCursor (5, 0);
  lcd.print(threshold, 0); // threshold
  lcd.setCursor (9, 0);
  lcd.print(String(RT)); ///reaction time...threshold crossing time minus cue light on time
  lcd.setCursor (0, 1);
  lcd.print(String((ms / 1000) / 60)); //time
  lcd.setCursor (3, 1);
  lcd.print(TOCt); // number of time outs
  lcd.setCursor (6, 1);
  lcd.print(avRxnTime, 0); // average of last 5 reaction times
  lcd.setCursor (12, 1);
  lcd.print("JSCT"); // task name

  voidLoopTime = millis() - voidLoopTimeStart;
  while (voidLoopTime != 30) { //print only when we want
    if (voidLoopTime > 30) {
      break;
    }
    voidLoopTime = millis() - voidLoopTimeStart;
  }

  myFile = SD.open(tlt, FILE_WRITE);
  if (myFile) {
    myFile.println(String(millis()) + ',' +  String(EM) + ',' +  String(TrialCt) +  ',' + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold) + ',' + String(RT) + ',' + String(avRxnTime) + ',' + String(TOCt));
    myFile.close(); // close the file
  }
  else {
    SavingBroken();
  }
  //Serial.println(String(millis()) + ',' +  String(EM) + ',' +  String(TrialCt) +  ',' + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold) + ',' + String(RT) + ',' + String(avRxnTime) + ',' + String(TOCt));
}

void SavingBroken() {
  lcd.clear();
  lcd.print("If you did not");
  lcd.setCursor(0, 1);
  lcd.print("remove SD Card..");
  delay(3000);
  lcd.clear();
  lcd.print("Something is");
  lcd.setCursor(0, 1);
  lcd.print("broken");
  delay(3000);
  lcd.clear();
  lcd.print("Check the ");
  lcd.setCursor(0, 1);
  lcd.print("file...");
  delay(3000);
  lcd.clear();
  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();

  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();

  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();

  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();
  SavingBroken();
}


int printDirectory(File dir, int numTabs) {
  int ctt = 0;
  while (true) {
    File entry =  dir.openNextFile();
    ctt++;
    if (! entry) {
      return ctt;
      dir.rewindDirectory();
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
  }
}




void TooManyFiles() {
  lcd.clear();
  lcd.print("There are too");
  lcd.setCursor(0, 1);
  lcd.print("many files");
  delay(3000);
  lcd.clear();
  lcd.print("Run");
  lcd.setCursor(0, 1);
  lcd.print("SavemicroSDData");
  delay(3000);
  lcd.clear();
  lcd.print("on MATLAB");
  delay(3000);
  lcd.clear();
  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();

  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();

  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();

  thumbdownA();
  delay(500);
  lcd.clear();
  thumbdownB();
  delay(500);
  lcd.clear();
  TooManyFiles();
}



void   thumbdownA() {
  byte thumb1[8] = {B00001, B00010, B00011, B00100, B00011, B00100, B00011, B00100};
  byte thumb2[8] = {B00011, B00000, B00000, B00000, B00000, B00000, B00000, B00000};
  byte thumb3[8] = {B11110, B00001, B00000, B00000, B00000, B00000, B00000, B00000};
  byte thumb4[8] = {B00000, B11110, B01000, B10001, B10010, B10010, B01100, B00000};
  byte thumb5[8] = {B00000, B10000, B01110, B00010, B00010, B00010, B00010, B00010};
  byte thumb6[8] = {B00110, B01000, B10000, B00000, B00000, B00000, B00000, B00000};
  lcd.createChar(0, thumb1);
  lcd.createChar(1, thumb2);
  lcd.createChar(2, thumb3);
  lcd.createChar(3, thumb4);
  lcd.createChar(4, thumb5);
  lcd.createChar(5, thumb6);
  lcd.setCursor(4, 0);
  lcd.write(0);
  lcd.setCursor(4, 1);
  lcd.write(1);
  lcd.setCursor(5, 0);
  lcd.write(2);
  lcd.setCursor(5, 1);
  lcd.write(3);
  lcd.setCursor(6, 0);
  lcd.write(4);
  lcd.setCursor(6, 1);
  lcd.write(5);
}

void thumbdownB() {
  byte thumb1[8] = {B00000, B00001, B00010, B00011, B00100, B00011, B00100, B00011};
  byte thumb2[8] = {B00100, B00011, B00000, B00000, B00000, B00000, B00000, B00000};
  byte thumb3[8] = {B00000, B11110, B00001, B00000, B00000, B00000, B00000, B00000};
  byte thumb4[8] = {B00000, B00000, B11110, B01000, B10001, B10010, B10010, B01100};
  byte thumb5[8] = {B00000, B00000, B10000, B01110, B00010, B00010, B00010, B00010};
  byte thumb6[8] = {B00010, B00110, B01000, B10000, B00000, B00000, B00000, B00000};
  lcd.createChar(0, thumb1);
  lcd.createChar(1, thumb2);
  lcd.createChar(2, thumb3);
  lcd.createChar(3, thumb4);
  lcd.createChar(4, thumb5);
  lcd.createChar(5, thumb6);
  lcd.setCursor(4, 0);
  lcd.write(0);
  lcd.setCursor(4, 1);
  lcd.write(1);
  lcd.setCursor(5, 0);
  lcd.write(2);
  lcd.setCursor(5, 1);
  lcd.write(3);
  lcd.setCursor(6, 0);
  lcd.write(4);
  lcd.setCursor(6, 1);
  lcd.write(5);
}
