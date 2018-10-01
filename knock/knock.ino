#include <TimerOne.h>
#include <DFPlayer_Mini_Mp3.h>
#define soundDetectedPin 2

#define DEBUG 0
#define DELAY_TIME 50

int soundDetectedVal = 1; // This is where we record our Sound Measurement
boolean bAlarm = false;
unsigned long prevSoundDetectTime;
unsigned long lastSoundDetectTime; // Record the time that we measured a sound
int index = -1;
int knocks[] = {3,3,3,3,3};
int trueKnocks[] = {1,0,1,0,1};

int shortKnock = 0;
int longKnock = 0;

int soundAlarmTime = 500; // Number of milli seconds to keep the sound alarm high
#include "./Timer_ISR.h"

void setup() {
    // put your setup code here, to run once:
   Serial.begin(9600);
   while (!Serial);
  
    mp3_set_serial (Serial); //set Serial1 for DFPlayer-mini mp3 module 
    mp3_set_volume (15);   
    mp3_stop();   
    
    pinMode (soundDetectedPin, INPUT_PULLUP);
    //Timer1.initialize(100000); //100ms
    
    }


void loop() {
  soundDetectedVal = digitalRead (soundDetectedPin);
  if (soundDetectedVal == LOW) // If we hear a sound
  {    
    if(index == -1){
      Timer1.initialize(100000); //100ms
      Timer1.attachInterrupt(Timer_ISR);
      counter = 0;     
      index++; 
    }
    else{
      //short
      if (index == 0){
        shortKnock = counter/3;
        //longKnock = counter;
      }
      
      if ( counter <= shortKnock ){
            knocks[index] = 0;
            #if DEBUG==1
            Serial.println("Short");
            #endif
            index++;
      }
      else if ( counter > shortKnock && counter <= longKnock){
         knocks[index] = 1;
         #if DEBUG==1
         Serial.println("Long");
         #endif
         index++;
      }
      counter = 0;   
      Timer1.restart();
    }
    
    if (index == 5)
    {
      index = -1;
      Timer1.stop();
      #if DEBUG==1
      Serial.print(knocks[0]);
      Serial.print(knocks[1]);
      Serial.print(knocks[2]);
      Serial.print(knocks[3]);
      Serial.println(knocks[4]);
      #endif
      if (is_equal_int_array(knocks, trueKnocks, 5)){
        mp3_play (1);    
        while(1);
      }
    }
    delay( DELAY_TIME);
  }
    
    
}
bool is_equal_int_array(int * buffer1, int * buffer2, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    if ( buffer1[i] != buffer2[i] ) return false;

  }
  return true;
};

