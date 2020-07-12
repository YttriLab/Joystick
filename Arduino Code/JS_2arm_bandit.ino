//MAN

#define LR_pin A1 //anolog pin for X
#define UD_pin A0 //anolog pin for Y
#define R A10 // rng seed
#include <SD.h>
#include <SPI.h>
File myFile;
File LOG;
int pinCS = 53;
String tlt;
int Andy;
String d;
int solenoid = 13; //pin out to solenoid

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library


//Variables to change
//////////////////////////////////////////////////////////////////////////
// Set the LCD address to  for a 16 chars and 2 line display //0x27 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);//0x27 0x3F

int SolOpenDuration = 90;
int DelayToRew = 1000;
int threshold = 90;

//high and low prob
long HP = 80;
long LP = 20;

//lower and upper bound of trials in each block
int nLB = 25;
int nUB = 35; 
//////////////////////////////////////////////////////////////////////////


int ITI = 3000; // ms. Intertrial interval. SET
int Y = 0;
int X = 0;
int baseY = 0;
int baseX = 0;
int solenoidOpen = 0; // 0 =closed, 1=open
int solenoidOpenTime = 0;

int RewardedTrialCt = 0;
int ct = 0;

unsigned long ms;
int EM = 0;
unsigned long SolOpenTime = 0;
unsigned long SolCloseTime = 0;
unsigned long thresholdcrossTime = 0;
float pos;
int numDirectionalTrials;
int currentTrialNum = 0;
int baitedDirection = -1; //none
int isRewarded = -1;
int currentJoystickDirection = -1;
int HeavyCoinFlip = 0;
int flip = 0;
int solopen = 0;

int nUpHighR = 0;
int nUpLowR = 0;
int nDownHighR = 0;
int nDownLowR = 0;

int nUpHighUR = 0;
int nUpLowUR = 0;
int nDownHighUR = 0;
int nDownLowUR = 0;
int nNotUDreach = 0;

float CorrectDir = 0, TotalReach = 0;
float PercentCorrect;
int BlockCt = 1;

/*Eventmarkers
  0 = read joystick position
  1 = correct Up baited Rewarded
  2 = correct Up baited UNrewarded
  3 = correct Up unbated Rewarded
  4 = correct Up unbated UNrewarded
  5 = correct Down baited Rewarded
  6 = correct Down baited UNrewarded
  7 = correct Down unbated Rewarded
  8 = correct Down unbated UNrewarded
  9 = cross threshold not to Up or Down
  11 = close sol
  12 = ITI
*/

void setup() {
  pinMode(solenoid, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pinCS, OUTPUT);

  Serial.begin(115200); //set corresponding baud rate in Ardunio Serial Monitor
  baseX = analogRead(LR_pin);
  baseY = analogRead(UD_pin);
  randomSeed(analogRead(R));
  numDirectionalTrials = random(nLB, nUB); // upperbound exclusive, total trials in a direction (correct or incorrect)

  lcd.backlight();
  lcd.begin();

// make file to save data on SD
  Andy = random(5000, 10001);
  tlt = String(Andy) + "_JSD.CSV";
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

  //first baited direction
  flip = random(0, 2);
  if (flip == 0) {
    baitedDirection = 0; // 0= Up
  }
  else if (flip == 1) {
    baitedDirection = 1; // 1= Down
  }
}


void loop() {
  lcd.clear(); ms = millis();
  Y = (analogRead(UD_pin) - baseY) / 2;  X = (analogRead(LR_pin) - baseX) / 2;
  pos = sqrt(pow(X, 2) + pow(Y, 2));

  switch (EM) {

    case 0: // read joystick position

      if ((Y < -3) and  currentJoystickDirection == -1) { //joystick neutral
        currentJoystickDirection = 0; //UP
      }
      else if ((Y > 3) and  currentJoystickDirection == -1) {
        currentJoystickDirection = 1; //DOWN
      }
      else {
        currentJoystickDirection = -1; //NONE
      }


      //if threshold met, move to next case
      if (pos > threshold) {
        thresholdcrossTime = millis(); currentTrialNum++; //count as a trial regardless of correct direction
        HeavyCoinFlip = random(0, 101);//current trial reward prob
        TotalReach++;
        
        //determines if correct direction
        if ((currentJoystickDirection == 0) and ((abs(X)) < abs(Y))) {
          if (currentJoystickDirection == baitedDirection) { //UP is rewarded
            if (HeavyCoinFlip < HP) {
              nUpHighR++; CorrectDir++;
              EM = 1; //correct Up baited Rewarded
            }
            else {
              nUpHighUR++; CorrectDir++;
              EM = 2; //correct Up baited UNrewarded
            }
          }
          else {
            if (HeavyCoinFlip < LP) { //correct reach to not the baited side
              nUpLowR++;
              EM = 3; //correct Up unbated Rewarded
            }
            else {
              nUpLowUR++;
              EM = 4; //correct Up unbated UNrewarded
            }
          }
        }
        else if ((currentJoystickDirection == 1) and ((X) < abs(Y))) { 
          if (currentJoystickDirection == baitedDirection) { //DOWN is rewarded
            if (HeavyCoinFlip < HP) {
              nDownHighR++; CorrectDir++;
              EM = 5; //correct Down baited Rewarded

            }
            else {
              nDownHighUR++; CorrectDir++;
              EM = 6; //correct Down baited UNrewarded
            }
          }
          else {
            if (HeavyCoinFlip < LP) { //correct reach to not the baited side
              nDownLowR++;
              EM = 7; //correct Down unbated Rewarded
            }
            else {
              nDownLowUR++;
              EM = 8; //correct Down unbated UNrewarded
            }
          }

        }
        else { //crossed threshold, not to Up or Down
          nNotUDreach++;
          EM = 9;
        }
      }
      else {
        EM = 0; currentJoystickDirection = -1;
      }
      break;

    case 1: //correct Up baited Rewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        solopen = 1; EM = 11;
      }
      else {
        EM = 1;
      }
      break;

    case 2: //correct Up baited UNrewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); solopen = 0;
        EM = 11;
      }
      else {
        EM = 2;
      }
      break;

    case 3: //correct Up unbated Rewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        solopen = 1; EM = 11;
      }
      else {
        EM = 3;
      }
      break;

    case 4: //correct Up unbated UNrewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); solopen = 0;
        EM = 11;
      }
      else {
        EM = 4;
      }
      break;

    case 5: //correct Down baited Rewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        solopen = 1; EM = 11;
      }
      else {
        EM = 5;
      }
      break;

    case 6: //correct Down baited UNrewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); solopen = 0;
        EM = 11;
      }
      else {
        EM = 6;
      }
      break;

    case 7: //correct Down unbated Rewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); digitalWrite(solenoid, HIGH); digitalWrite(LED_BUILTIN, HIGH);
        solopen = 1; EM = 11;
      }
      else {
        EM = 7;
      }
      break;

    case 8: //correct Down unbated UNrewarded
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); solopen = 0;
        EM = 11;
      }
      else {
        EM = 8;
      }
      break;

    case 9: //cross threshold not to Up or Down
      if (DelayToRew <= millis() - thresholdcrossTime) {
        SolOpenTime = millis(); solopen = 0;
        EM = 11;
      }
      else {
        EM = 9;
      }
      break;

    case 11: // close sol
      if (solopen == 1) {
        if ( SolOpenDuration <= millis() - SolOpenTime) {
          SolCloseTime = millis(); digitalWrite(solenoid, LOW); digitalWrite(LED_BUILTIN, LOW); solopen = 0;
          RewardedTrialCt++;
          EM = 12;
        }
        else {
          EM = 11;
        }
      }
      else {
        if ( SolOpenDuration <= millis() - SolOpenTime) {
          SolCloseTime = millis(); solopen = 0;
          EM = 12;
        }
        else {
          EM = 11;
        }
      }
      break;

    case 12: //ITI
      if (ITI <= millis() - SolCloseTime) {
        //switching rewarded direction
        if (currentTrialNum >= numDirectionalTrials) {
          if (baitedDirection == 1) {
            baitedDirection = 0; // 0= UP
          }
          else if (baitedDirection == 0) {
            baitedDirection = 1;
          }
          numDirectionalTrials = random(nLB, nUB);
          currentTrialNum = 0; BlockCt++;
        }
        if (pos < threshold) { //if there isn't another reach during the ITI
          currentJoystickDirection = -1;
          EM = 0;
          baseY = analogRead(UD_pin); baseX = analogRead(LR_pin);
        }
      }
      else {
        EM = 12;
      }
      break;
  }

  lcd.print(String(RewardedTrialCt)); //rewarded reaches
  lcd.setCursor (4, 0);
  lcd.print(TotalReach, 0); // total reaches
  lcd.setCursor (10, 0);
  if (baitedDirection == 1) {
    d = "D";
  }
  else {
    d = "U";
  }
  lcd.print(d); // block direction
  lcd.setCursor (12, 0);
  PercentCorrect = (CorrectDir / TotalReach) * 100;
  lcd.print(PercentCorrect, 0); //number of correct reaches divided by all reaches
  lcd.setCursor (15, 0);
  lcd.print("%");
  lcd.setCursor (0, 1);
  lcd.print(String((ms / 1000) / 60)); // time
  lcd.setCursor (4, 1);
  lcd.print(String(BlockCt)); //block number
  lcd.setCursor (7, 1);
  lcd.print(String(currentTrialNum)); // within block trial number
  lcd.setCursor (9, 1);
  lcd.print("/");
  lcd.setCursor (10, 1);
  lcd.print(String(numDirectionalTrials)); // total trials in block
  lcd.setCursor (13, 1);
  lcd.print("JSD"); //sketch name

  //  Serial.println(String(millis()) + ',' +  String(EM) + ',' +  String(RewardedTrialCt) + ','  + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY)\
  //                 + ','  + String(ITI) + ',' + String(threshold) + ','  + String(numDirectionalTrials) + ',' + String(currentTrialNum) + ',' +  String(baitedDirection) +  ',' + String(isRewarded) \
  //                 + ',' + String(currentJoystickDirection) + "," + String(analogRead(LR_pin)) + "," + String(analogRead(UD_pin)) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) \
  //                 + ',' + String(threshold) + ',' + String(LP) + ',' + String(HP) + ',' + String(nLB) + ',' + String(nUB) + ',' + String(BlockCt));

  myFile = SD.open(tlt, FILE_WRITE);
  if (myFile) {
    myFile.println(String(millis()) + ',' +  String(EM) + ',' +  String(RewardedTrialCt) + ','  + String(X) + ',' + String(Y) + ',' + String(pos) + ',' + String(baseX) + ',' +  String(baseY)\
                   + ','  + String(ITI) + ',' + String(threshold) + ','  + String(numDirectionalTrials) + ',' + String(currentTrialNum) + ',' +  String(baitedDirection) +  ',' + String(isRewarded) \
                   + ',' + String(currentJoystickDirection) + "," + String(analogRead(LR_pin)) + "," + String(analogRead(UD_pin)) + ',' + String(SolOpenDuration) + ',' + String(DelayToRew) \
                   + ',' + String(threshold) + ',' + String(LP) + ',' + String(HP) + ',' + String(nLB) + ',' + String(nUB) + ',' + String(BlockCt) + ',' + String(TotalReach));

    //[0ms EM RewardedTrialCt X Y 5pos baseX baseY ITI threshold 10numDirectionalTrials currentTrialNum baitedDirection isRewarded currentJoystickDirection 15analogRead(LR_pin)analogRead(UD_pin)
    //SolOpenDuration DelayToRew threshold 20LP HP nLB nUB BlockCt 25TotalReach)

    myFile.close(); // close the file
  }
  else {
    SavingBroken();
  }
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
