/*
  Increases threshold by one if 5 reaches occur with no more than 2 minutes between any reach
  If no reaches in 3 minutes, threshold drops by 3 and can no longer increase
  MAN
*/ 


#define LR_pin A0 //anolog pin for X
#define UD_pin A1 //anolog pin for Y
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

#define R A10 // rng seed
#include <SD.h>
#include <SPI.h>
File myFile;
File LOG;
int pinCS = 53;
String tlt;
int Andy;

//Variables to change
//////////////////////////////////////////////////////////////////////
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 or 0x3F

int threshold = 35;
int SolOpenDuration = 150; //ms Duration. SET
int DelayToRew = 1000; //Pause till reward.
//////////////////////////////////////////////////////////////////////

int ITI = 3000; // ms. Intertrial Interval. SET

int solenoid = 13; //pin out to solenoid

int Y = 0;
int X = 0;
int baseY = 0;
int baseX = 0;

int solenoidOpen = 0; // 0 =closed, 1=open
int solenoidOpenTime = 0;
int TrialCt = 0;
int ct = 0;
int met = 0;

unsigned long ClockStart = 0;
int msEnd;
int msDiff = 0;
int EM = 0;
int inc = 1; // threshold still increasing 
int dropped = 0; // threshold has dropped and will not increase again
unsigned long incClockStart = 0;
unsigned long SolOpenTime = 0;
unsigned long SolCloseTime = 0;
unsigned long thresholdcrossTime = 0;
unsigned long ms = 0;
float pos;

/*Eventmarkers
  0 = not in trial
  1 = delay
  2 = deliver water
  3 = iti
*/

void setup() {
  pinMode(solenoid, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  baseX = analogRead(LR_pin);
  baseY = analogRead(UD_pin);
  incClockStart = millis();
  lcd.backlight();
  lcd.begin();

  pinMode(pinCS, OUTPUT);
  randomSeed(analogRead(R));
  Andy = random(5000, 10001);
  tlt = String(Andy) + "_JSA.CSV";
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
  ct = 0;
}

void loop() {
  lcd.clear();
  Y = analogRead(UD_pin) - baseY;  X = analogRead(LR_pin) - baseX;
  pos = sqrt(pow(X, 2) + pow(Y, 2));
  ms = millis();

  if ((millis() - ClockStart) > 120000) { //check for increase time
    ct = 0;
    ClockStart = 0;
  }

  if ( dropped == 0 and (millis() - incClockStart) >= 180000) { //check for drop time
    inc = 0; dropped = 1;
    threshold = threshold - 3;
  }

  switch (EM) {
    case 0: // read joystick position
      if (pos > threshold) {
        thresholdcrossTime = millis(); EM = 1; met = 1;
        if (ct == 0 ) {
          ClockStart = millis();
          ct++;
        }
        else if (ct == 4 and (millis() - ClockStart) < 120000 and inc == 1) {//completed 5 trials under 2 mins
          threshold++;
          ct = 0;
          ClockStart = 0;
        }
        else {
          ct++;
          ClockStart = millis();
          incClockStart = millis();
        }
      }
      else {
        EM = 0; met = 0;
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

    case 2: // deliver Water
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
        EM = 0; met = 0;
      }
      else {
        EM = 3;
      }
      break;
  }

  lcd.print(String(TrialCt)); // number of trials
  lcd.setCursor (5, 0);
  lcd.print(String(threshold)); // Threshold
  lcd.setCursor (9, 0);
  lcd.print(String(pos)); // JS position
  lcd.setCursor (0, 1);
  lcd.print(String((ms / 1000) / 60)); // time
  lcd.setCursor (5, 1);
  lcd.print(String(inc)); //Threhold still increasing?
  lcd.setCursor (13, 1);
  lcd.print("JSA");

  //Serial.println(String(ms) + ',' +  String(EM) + ',' +  String(TrialCt) + ','  + String(analogRead(UD_pin)) + ',' + String(analogRead(LR_pin)) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold) + ',' + String(ct) + ',' + String(inc) + ',' + String(met) );

  myFile = SD.open(tlt, FILE_WRITE);
  if (myFile) {
    myFile.println(String(ms) + ',' +  String(EM) + ',' +  String(TrialCt) + ','  + String(analogRead(UD_pin)) + ',' + String(analogRead(LR_pin)) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) + ',' + String(ITI) + ',' + String(threshold) + ',' + String(ct) + ',' + String(inc) + ',' + String(met) );
    myFile.close(); // close the file
  }
  else {
    SavingBroken();
  }
}
// [time EM TrialCt X Y pos baseX baseY SolOpenDuration DelaytoRew ITI Threshold ct inc met]

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
