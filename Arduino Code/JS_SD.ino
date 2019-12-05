/*
 * Change the desired task variables and upload the sketch without micro SD card loaded. 
 * The sketch will start acquiring data upon uploading the sketch if the micro SD card is present.
 * 
 * The sketch will start up in the “SavingBroken” function and will not start until the Arduino is reset with a micro SD card loaded
 * 
 * Load the mouse and the micro SD card and press the onboard reset button to start the task
 * 
 * micro SD card should have only one file titled "Log.txt"
 * after collecting data run SavemicroSDData.m on MATLAB
 * 
 */
 
#define LR_pin A0 //anolog pin for X
#define UD_pin A1 //anolog pin for Y

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

#define R A10 //random seed

#include <SD.h>
#include <SPI.h>
File myFile;
File LOG;
int pinCS = 53; // Communication pin out to SD
String tlt;
int Andy;
int ct = 0;


//Variables to change
//////////////////////////////////////////////////////////////////////
// Set the LCD address to  for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);//0x27  0x3F


int threshold = 90;
unsigned long SolOpenDuration = 115; //ms Duration. SET
unsigned long DelayToRew = 1000; //Pause till reward.
//////////////////////////////////////////////////////////////////////

unsigned long ITI = 3000; // ms. Intertrial Interval. SET

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

  pinMode(pinCS, OUTPUT);
  randomSeed(analogRead(R));
  Andy = random(5000, 10001);
  tlt = String(Andy) + "_JS2.CSV"; //get random file name
  int ctt;
  File root;

  if (SD.begin()) { //initialize SD
    root = SD.open("/");
    ctt = printDirectory(root, 0); ctt = ctt - 2;
    Serial.println(String(ctt));
    if (ctt > 1) {
      TooManyFiles();
      return;
    }
    LOG = SD.open("Log.txt", FILE_READ); //SD card sgould have one file save titled "Log.txt"
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
    LOG = SD.open("Log.txt", FILE_WRITE); //SD card sgould have one file save titled "Log.txt"
    if (LOG) {
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
  lcd.clear();
  Y = analogRead(UD_pin) - baseY;  X = analogRead(LR_pin) - baseX;
  pos = sqrt(pow(X, 2) + pow(Y, 2));
  Y = analogRead(UD_pin);  X = analogRead(LR_pin);

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

    case 1: // 1delay 2deliverWater 3iti
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        EM = 2;
      }
      else {
        EM = 1;
      }
      break;

    case 2:
      if ( SolOpenDuration <= millis() - SolOpenTime) {
        SolCloseTime = millis(); digitalWrite(solenoid, LOW); digitalWrite(LED_BUILTIN, LOW);
        EM = 3;
      }
      else {
        EM = 2;
      }
      break;

    case 3:
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
  lcd.print("JS2SD");




  myFile = SD.open(tlt, FILE_WRITE);
  if (myFile) {
    myFile.println(String(ms) + ',' +  String(EM) + ',' +  String(TrialCt) + ','  + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold));
    myFile.close(); 
  }
  else {
    SavingBroken();
  }
  //  Serial.println(String(ms) + ',' +  String(EM) + ',' +  String(TrialCt) + ','  + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold));
  //  ^ debugging 
}

// [time EM TrialCt X Y pos baseX baseY SolOpenDuration DelaytoRew ITI Threshold]


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
      // no more files
      // return to the first file in the directory
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
