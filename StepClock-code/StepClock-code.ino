/*  LICENSE:
    STEP CLOCK
    Copyright (C) 2017  Makit: Cristobal Selma
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*This is the code for makit's STEP CLOCK
 Visit makitpro.com for more info and instructions.*/
 
#include <Wire.h>
#include "RTClib.h" //https://github.com/adafruit/RTClib
#include <AccelStepper.h> //http://www.airspayce.com/mikem/arduino/AccelStepper/
RTC_DS1307 rtc;

#define HALFSTEP 8

// Hour Motor pin definitions
#define motorHour1Pin1  11     // IN4 on the ULN2003 driver 1
#define motorHour1Pin2  10     // IN3 on the ULN2003 driver 1
#define motorHour1Pin3  9     // IN2 on the ULN2003 driver 1
#define motorHour1Pin4  8    // IN1 on the ULN2003 driver 1

// Minute pin definitions
#define motorMin1Pin1  4     // IN1 on the ULN2003 driver 2
#define motorMin1Pin2  5     // IN2 on the ULN2003 driver 2
#define motorMin1Pin3  6     // IN3 on the ULN2003 driver 2
#define motorMin1Pin4  7    // IN4 on the ULN2003 driver 2

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepperHour1(HALFSTEP, motorHour1Pin1, motorHour1Pin3, motorHour1Pin2, motorHour1Pin4);
AccelStepper stepperMin1(HALFSTEP, motorMin1Pin1, motorMin1Pin3, motorMin1Pin2, motorMin1Pin4);

int currPosition;
long positionHour;
long positionMin;

int minuteStep =4096/60; //68 the number of steps the minute motor moves every minute
int hourStep =4096/144; //28 the number of steps the hour motor moves every minute

//Initialize sensor variables
int hallHour1Pin=A0; //out pin of the hour hall sensor
int hallHour1Val=0;
int hallHour1Calib=620; //Magnet detection calibration, 
                        //you may have to change it depending on your magnet

int hallMin1Pin=A1; //out pin of the minute hall sensor
int hallMin1Val=0;
int hallMin1Calib=700; //Magnet detection calibration, 
                        //you may have to change it depending on your magnet

// Initialize time variables
int8_t hours;
int8_t hours24;
int8_t minutes;
int8_t seconds;
int8_t days;
int8_t months;

static unsigned long msTick =0;  // the number of Millisecond Ticks since we last 
                                 // incremented the second counter
                                 
void setup() {
  Serial.begin(9600);
 //RTC CLOCK
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
     
//Uncomment one of them to set time when change batteries
   //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));//this sets time manually
   //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//this sets time based on the compilation time of the computer

//Setup motors
  stepperHour1.setMaxSpeed(1000.0);
  stepperHour1.setAcceleration(200.0);
  stepperHour1.setSpeed(500);
  stepperMin1.setMaxSpeed(1000.0);
  stepperMin1.setAcceleration(200.0);
  stepperMin1.setSpeed(500);

calibration();//Calibrate giving time to insert hands

//get current millis  
  msTick=millis();
  
}//end of setup--------------------------------------


void incrementTime(void){
    // increment the time counters keeping care to rollover as required
  seconds=0;
  //Show time
  Serial.print("Time: ");
  Serial.print(hours24);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);

    Serial.println("60 seconds, move one minute"); //debug message    
    stepperMin1.enableOutputs();
    stepperMin1.move(minuteStep);
    stepperMin1.runToPosition();
    stepperMin1.disableOutputs();
  
  if (++minutes >= 60) {
    minutes=0;
      //Compensate lost steps when dividing with integers
      Serial.println("60 minutes, compensate minutes"); //debug message    
      stepperMin1.enableOutputs(); 
      stepperMin1.move(16);// 4096/60=68.26 -> 68*60=4080 ->4096-4080=16
      stepperMin1.runToPosition();
      stepperMin1.disableOutputs();
      
    if (++hours >= 12) {
      hours=0;    
      //Compensate lost steps when dividing with integers
      Serial.println("12 hours, compensate hours"); //debug message   
      stepperHour1.enableOutputs(); 
      stepperHour1.move(32);// 4096/144=28.44 ->28*144=4032 -> 4096-4032=64/2 (because of 12h) = 32
      stepperHour1.runToPosition();
      stepperHour1.disableOutputs();
    }
    //Recalibrate and read time from RTC each 24 hours
    if(++hours24 ==24){
      hours24=0;
      //Recalibratiom
    stepperMin1.runToNewPosition(0);
    stepperHour1.runToNewPosition(0);
      //move motors to correct position
    moveMotorsToPosition();
    }    
  }
  
if (minutes==10||minutes==20||minutes==30||minutes==40||minutes==50||minutes==0||minutes==60||minutes==5||minutes==15||minutes==25||minutes==35||minutes==45||minutes==55){
    //Move hour motor each 5 minutes
    Serial.println("Move hour motor each 5 minutes"); //debug message
    stepperHour1.enableOutputs(); 
    stepperHour1.move(hourStep);
    stepperHour1.runToPosition();
    stepperHour1.disableOutputs();   
  }
}//end of incrementTime--------------------------------


void stepperHome(AccelStepper stepper, int hallSensorVal, int hallSensorPin, int hallSensorCalib)
{ 
  Serial.println("Calibrating...");
  //this routine looks for magnet
  hallSensorVal = analogRead(hallSensorPin);                                
  while(hallSensorVal<hallSensorCalib)
  {
    //forward slowly till it detects the magnet
    stepper.move(100);
    stepper.run();
    hallSensorVal = analogRead(hallSensorPin);
  }
  stepper.setCurrentPosition(0); //
  Serial.println("Calibration done!");
  
}//end of stepperHome---------------------------------

void moveMotorsToPosition(){
    //get updated time and move motors to correct position
  Serial.println("Get updated time and move motors to correct position");
  DateTime now = rtc.now();
    hours=now.hour();
    hours24=hours;
    minutes=now.minute();
    seconds=now.second();
    days=now.day();
    months=now.month();

    //Adjust hours to 12h format
     if(hours>12){
      hours=hours-12;  
      }

 /* Serial.print("Date: ");//Debug
  Serial.print(days);
  Serial.print(" of ");
  Serial.print(months);
  Serial.print(" -- ");
  Serial.print("Time: ");
  Serial.print(hours24);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);*/

//Calculate position
  positionHour=(hours*4096L/12);
  positionHour+=(minutes*4096L/720);//Calculate position based on the calibration
  positionMin=(minutes*4096L/60);//Calculate position based on the calibrationntln(positionMin);
    
  //Move motors to correct position 
  stepperMin1.enableOutputs();   
  stepperMin1.move(positionMin);
  stepperMin1.runToPosition();
  stepperMin1.stop();
  stepperMin1.disableOutputs();  
  stepperHour1.enableOutputs();
  stepperHour1.move(positionHour);
  stepperHour1.runToPosition();
  stepperHour1.stop();
  stepperHour1.disableOutputs(); 
}//end of moveMotorsToPosition------------------------------------------

void calibration(){
    
  //Calibrate 12 o'clock
    stepperHome(stepperMin1, hallMin1Val, hallMin1Pin, hallMin1Calib);// search for magnet
    stepperHome(stepperHour1, hallHour1Val, hallHour1Pin, hallHour1Calib);// search for magnet
  
  //Code calibration, change these values if necessary
    stepperMin1.runToNewPosition(3675);//Manual calibration, change this value to achieve 0 position
    stepperHour1.runToNewPosition(3900);//Manual calibration, change this value to achieve 0 position
    
  //Manual calibration, comment if you are using code calibration or hands are already in place
    Serial.println("Insert minute and hour hand on the 12 o'clock position, you have 20 seconds");
    delay(20000);//Time to put the hour and minute hands on the correct position, you can change it
  
  //Set current position 0
    stepperMin1.setCurrentPosition(0);
    stepperHour1.setCurrentPosition(0);

  //move motors to correct initial position
    moveMotorsToPosition();
}//end of calibration------------------------------------------

void loop() {

// heart of the timer - keep looking at the millisecond timer on the Arduino
    // and increment the seconds counter every 1000 ms
    if ( millis() - msTick >999) {
 
      msTick=millis();
        seconds++;
    }     
    //test to see if we need to increment the time counters
    if (seconds==60) 
    {
      incrementTime();
    }     
}//end loop-------------------------------------------------------
