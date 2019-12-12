/*
This sketch will display JS position and threshold as well as task detail
User must change "MouseName" of "String NAME = "MouseName";" 
This sketchas is will save the file as "data.csv" in this sketch's processing folder, to save file, uncomment line 40 and correct save locaiton
MAN
*/
import processing.serial.*;
import java.util.Collections;
String inString;
String[] data;
Serial myPort; 
PrintWriter output;

int ClockStart= 0;
int ClockNow = 0;
int ITIend = 0;
int trialstart = 0;
ArrayList<Float> ITIs;

int dy = day(); 
int mon = month(); 
int yr = year(); 
int sec = second(); 
int min = minute(); 
int hr = hour();


//NAME MOUSE HERE BELOW
////////////////////////

String NAME = "MouseName";

///////////////////////

void setup()
{
  String portName = Serial.list()[2]; //make sure this port is the same that Arduino is connceted to (zero indexed)
  myPort = new Serial(this, portName, 115200); // match baud rate from arduino
  output = createWriter("data.csv"); // this will save to Processing sketch folder. Can save to dif folder with full pth as below
  //output = createWriter("N:\\yttri-lab\\Data Transfer\\Segovia\\recieveJoyStick2\\" + NAME +"_" + str(mon) +"_"+ str(dy) +"_"+ str(yr) +"_"+ str(hr)+"_"+ str(min) +"_"+ str(sec)+".csv"); // can save to dif folder with full pth///change to .csv
  myPort.bufferUntil('\n');
  size(800, 800);
  background(0);
  println(Serial.list());
}

void serialEvent(Serial myPort) {

  String inString = myPort.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString);
    data = split(inString, ",");
    output.println(inString);
    output.flush();
  }
}

void draw()
{
  if (data!= null) {
    clear();
    int xPos = int(data[3])/2+150;
    int yPos = int(data[4])/2+150;
    fill(0, 255, 0);
    text("Mouse:", 10, 20, 10); textSize(20);
    text(NAME, 140, 20, 10); 

    float ms = int(data[0]);
    if (ms<60000) {
      ms = ms/1000;
      text("Time (sec):", 10, 45, 10);
      text(nfs(ms, 0, 2), 133, 45, 10);
    } else {
      ms = (ms/1000)/60;
      float sec = (ms-floor(ms))*60;
      text("Time (m:s):", 10, 45, 10);
      text(floor(ms), 140, 45, 10);
      text(":", 165, 45, 10);
      text(floor(sec), 175, 45, 10);
    }

    String EM = data[1];
    text("EventMarker:", 10, 70, 10);
    text(EM, 140, 70, 10);

    String trial = data[2];
    text("Trials:", 10, 95, 10);
    text(trial, 140, 95, 10);

    int thresh =  int(data[11]);
    text("Threshold:", 10, 120, 10);
    text(thresh, 140, 120, 10);

    fill(20);
    int xbase = int(data[6])/2+150; 
    int ybase = int(data[7])/2+150;
    fill(100);

    ellipse(xbase, ybase, thresh, thresh);
    fill(255, 0, 0);
    ellipse(xPos, yPos, 10, 10);
  }
}
