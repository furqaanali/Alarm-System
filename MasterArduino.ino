// include libraries
#include <LiquidCrystal.h>
#include <TimeLib.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// initialize pins
const int LED = 7;
const int buzzer = 6;
const int photoresistor = A1;
const int tempSensor = A0;
const int soundSensor = 2;

const int decreaseButton = 5;
const int increaseButton = 4;
const int setButton = 3;


// Variables for the thermistor
int thermistorPin = A0;
int Vo;
float R1 = 100000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

// Time Strings and Values
char *myStrings[] = {"Year", "Month", "Day", "Hour", "Minute"};
int timeValues[]  = {0,       0,       0,     0,      0      };
int alarmValues[] = {0,       0,       0,     0,      0      }; 

// Window
const int soundWindow = 2;
const int gradualIncreaseWindow = 1;

// Time variables
unsigned long time;
int lastSecond;

// Play Alarm Only Once
int hasAlarmPlayed;


// FOR TESTING PURPOSES
void skipTest() {
  timeValues[0] = 2020;
  timeValues[1] = 11;
  timeValues[2] = 22;
  timeValues[3] = 5; //hour
  timeValues[4] = 2; //minute
  alarmValues[0] = 2020;
  alarmValues[1] = 11;
  alarmValues[2] = 22;
  alarmValues[3] = 5; //hour
  alarmValues[4] = 4; //minute
  setTime(timeValues[3], timeValues[4], 0, timeValues[2], timeValues[1], timeValues[0]);
}

void setInputsAndOutputs() {
  lcd.begin(16, 2);
  Serial.begin(38400);
  
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  pinMode(decreaseButton, INPUT);
  pinMode(increaseButton, INPUT);
  pinMode(setButton, INPUT);
  pinMode(tempSensor, INPUT);
  pinMode(soundSensor, INPUT);
  pinMode(photoresistor, INPUT);
}

void checkButtonStateChange(int& buttonState, int& lastButtonState, int& counter, int i, int countUp) {
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      if (countUp) counter++;
      else counter--;
      lcd.clear();
      lcd.print(myStrings[i]);
      lcd.print(": ");
      lcd.print(counter);
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;
}


void getTime(int alarm) {

  // Button states
  int incrementButtonState = 0;
  int lastIncrementButtonState = 0;
  int decrementButtonState = 0;
  int lastDecrementButtonState = 0;
  int setButtonState = 0;
  int lastSetButtonState = 0;
      for (int i = 0; i < 5; ++i) {
  
      int counter = 0;
  
      lcd.clear();
      if (!alarm)
        lcd.print("Set Current ");
      else
        lcd.print("Set Alarm ");
      
      lcd.setCursor(0,1);
      lcd.print(myStrings[i]);
      
      while(setButtonState == LOW) {
      
        // read the pushbuttons input pins:
        incrementButtonState = digitalRead(increaseButton);
        decrementButtonState = digitalRead(decreaseButton);
        setButtonState = digitalRead(setButton);
        
        // compare the buttonState to its previous state
        checkButtonStateChange(incrementButtonState, lastIncrementButtonState, counter, i, 1);
        checkButtonStateChange(decrementButtonState, lastDecrementButtonState, counter, i, 0);
    
      }
      setButtonState = LOW;

      if (!alarm) timeValues[i] = counter;
      else alarmValues[i] = counter;
  
      lcd.clear();
      lcd.print(myStrings[i]);
      if (!alarm) {
        lcd.print(" is ");
        lcd.print(timeValues[i]);
      }
      else {
        lcd.print(" at ");
        lcd.print(alarmValues[i]);
      }
      
      delay(2000);
    }
}


void setCurrentAndAlarmTime() {

  getTime(0);   // get current time into timeValues array
  getTime(1);   // get alarm time into alarmValues array
  lcd.clear();

  // set current time
  setTime(timeValues[3], timeValues[4], 0, timeValues[2], timeValues[1], timeValues[0]);
  
}

void startAlarm() { 
  hasAlarmPlayed = 1;
  
  int startTime = millis();
  int buzzerVal;
  
  lcd.clear();
  lcd.print("WAKE UP!");
  digitalWrite(LED, HIGH);

  int lightValue = 0;
  char state = '0';
  while(state == '0' || lightValue < 250) {
    
   lightValue = analogRead(photoresistor);

   if(Serial.available() > 0){ // Checks whether data is comming from the serial port
    state = Serial.read(); // Reads the data from the serial port
   }
    
   time = millis();
   if(buzzerVal < 70) {
      buzzerVal = (time - startTime) / 200; 
   }
   
   analogWrite(buzzer, buzzerVal);

   delay(1000); 
  }
  
  digitalWrite(LED, LOW);
  analogWrite(buzzer, 0);
  
  lcd.clear();
  lcd.print("Alarm Disabled");
  delay(2000);
}

void displayTime() {
  if (lastSecond != second()) {
    lcd.clear();
    lcd.setCursor(0, 0);

    // display date
    lcd.print(month());
    lcd.print('/');
    lcd.print(day());
    lcd.print('/');
    lcd.print(year());
  
    // display temperature
    Vo = analogRead(thermistorPin);
    R2 = R1 * (1023.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    T = T - 273.15;
    T = (T * 9.0)/ 5.0 + 32.0; 
    lcd.print("  ");
    lcd.print((int)T);
    lcd.print("F");
  
    // display time
    lcd.setCursor(0, 1);
    lcd.print(hour());
    lcd.print(':');
    lcd.print(minute());
    lcd.print(':');
    lcd.print(second());
   
    lastSecond = second();
  }
  
}

int getMinutesToAlarm(){
  
  int hourA=alarmValues[3];
  int minuteA=alarmValues[4];
  int hourN=hour();
  int minuteN=minute();
  
  int minToAlarm=0;

  //the alarm is on the same day
  if(hourA>hourN) {
    minToAlarm=(hourA-hourN)*60; //add the hours
    minToAlarm+=(minuteA)+(60-minuteN);//add the minutes
  }
  //the alarm is on the next day
  else if(hourA<hourN) {
    minToAlarm=(24-hourN)*60+(hourA)*60; //add the hours
    minToAlarm+=(minuteA)+(60-minuteN);  //add the minutes
  }
  // hourA==hourN()
  else {
    if (minuteA>minuteN)
      minToAlarm=(minuteA-minuteN);
    else if(minuteA==minuteN)
      minToAlarm=0;
    else //minuteA<minuteN
      minToAlarm=(24-hourN)*60+(hourA)*60+(minuteA)+(60-minuteN); //wrap around the whole day
  }

  return minToAlarm;

}

void setup() {
  setInputsAndOutputs();
  setCurrentAndAlarmTime();
  //skipTest(); // TESTING PURPOSES
}

void loop() {

  displayTime();
  int soundDetected = digitalRead(soundSensor); // low = ON
  int minToAlarm = getMinutesToAlarm();
  if (minToAlarm <= gradualIncreaseWindow && !hasAlarmPlayed ) {
    startAlarm();
  }
  if ( minToAlarm <= soundWindow && minToAlarm >= gradualIncreaseWindow) {    //within sound window ONLY
    if (soundDetected == LOW && !hasAlarmPlayed) {                           // and we detect sound
      startAlarm();
    }
  }
  
}
