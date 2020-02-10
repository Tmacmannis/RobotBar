/*
Drink1: 0
Drink2: 8750
Drink3: 17600
Drink4: 26600
Drink Home Positon: 4000
*/

char BluetoothData; // the data received from bluetooth serial link
char EnteredData;
#include <FastLED.h>
#include "AccelStepper.h" 
#include <Wire.h>

#define NUM_LEDS 70
#define DATA_PIN 10

CRGB leds[NUM_LEDS];

// AccelStepper Setup
AccelStepper stepperX(1, 4, 5);   // 1 = Easy Driver interface
AccelStepper stepperY(1, 2, 3);   // NANO Pin 2 connected to STEP pin of Easy Driver
                                  // NANO Pin 3 connected to DIR pin of Easy Driver

// Define the Pins used
#define home_switch 9 // Pin 9 connected to Home Switch (MicroSwitch)

// Stepper Travel Variables
long TravelX;  // Used to store the X value entered in the Serial Monitor
int move_finished=1;  // Used to check if move is completed
long initial_homing=-1;  // Used to Home Stepper at startup

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;

unsigned long testMicros = micros();
const long interval = 100;

unsigned char byte1, byte2, byte3, byte4;

boolean red = true;
boolean sentData = false;

void setup() {
  Serial.begin(9600);
  pinMode(home_switch, INPUT_PULLUP);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(100);
  FastLED.clear();
  FastLED.show();
  Wire.begin();
  delay(5);

  //moveDot();

  homeMainStepper();

  stepperX.setMaxSpeed(10000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
  stepperY.setMaxSpeed(5000.0);      
  stepperY.setAcceleration(5000.0);
}

void loop() {

  //Process info coming from bluetooth app
  if (Serial.available()){
    TravelX= Serial.parseInt(); //Get next character from bluetooth

    switch(TravelX){
      case 1: 
        Serial.println("one was entered");
        cocktail1();
      break;
      case 2:
        Serial.println("two was entered");
        cocktail2();
      break;
    }
//    if(BluetoothData=='G') cocktail1();
//    if(BluetoothData=='R') cocktail2();
//    if(BluetoothData=='V') cocktail3();
    
  }
}

//List of Methods used.. Can be put in separate files
//*****************************************************************************************************************************************************
void homeMainStepper(){
  setLEDsHoming();
     //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper

// Start Homing procedure of Stepper Motor at startup

  Serial.print("Stepper is Homing . . . . . . . . . . . ");

  while (digitalRead(home_switch)) {  // Make the Stepper move CCW until the switch is activated   
      // Start moving the stepper

    unsigned long currentMillis = micros();
    if (currentMillis - previousMillis1 >= 500) {
    // save the last time you blinked the LED
      previousMillis1 = currentMillis;

      stepperX.moveTo(initial_homing);  // Set the position to move to
      initial_homing--;  // Decrease by 1 for next move if needed
      stepperX.run();
      sentData = true;
      
    }

    // unsigned long currentTestMicro = micros();
    if(sentData){
       writeToSlave(stepperX.currentPosition());
    }
    // Serial.println(currentTestMicro - testMicros);
    // testMicros = currentTestMicro;
    // delay(.1);
  }
  Serial.print(initial_homing);

  stepperX.setCurrentPosition(0);  // Set the current position as zero for now
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
  initial_homing=1;

  while (!digitalRead(home_switch)) { // Make the Stepper move CW until the switch is deactivated
    stepperX.moveTo(initial_homing);  
    stepperX.run();
    initial_homing++;
    delay(1);
  }
  
  stepperX.setCurrentPosition(0);
  Serial.println("Homing Completed");
  Serial.println("");
  setLEDsDoneHoming();
  
}

//*****************************************************************************************************************************************************

void userEnteredPosition(){
  while (Serial.available()>0)  { // Check if values are available in the Serial Buffer    
    TravelX= Serial.parseInt();
      Serial.print("Moving stepper into position: ");
      Serial.println(TravelX);
      delay(1000);
      moveToPosition(0 - TravelX);
  }
}

//*****************************************************************************************************************************************************

void moveToPosition(int pos){

  move_finished=0;
  stepperX.moveTo(pos);

  while(move_finished == 0){

    // Check if the Stepper has reached desired position
    if ((stepperX.distanceToGo() != 0)) {
      
      stepperX.run();  // Move Stepper into position
      
    }

    writeToSlave(stepperX.currentPosition());
    //changeColorEverySec();

    // If move is completed display message on Serial Monitor
    if ((move_finished == 0) && (stepperX.distanceToGo() == 0)) {
      Serial.println("");
      move_finished=1;  // Reset move variable
      }
  }

}

//*****************************************************************************************************************************************************

void pourOneShot() {

  move_finished=0;
  stepperY.moveTo(4100);

  while(move_finished == 0){

    // Check if the Stepper has reached desired position
    if ((stepperY.distanceToGo() != 0)) {
      
      stepperY.run();  // Move Stepper into position
      
    }

    // If move is completed display message on Serial Monitor
    if ((move_finished == 0) && (stepperY.distanceToGo() == 0)) {
      //Serial.println("Shot Poured");
      move_finished=1;  // Reset move variable
      }
  }


  delay(1000);

  move_finished=0;
  stepperY.moveTo(0);

  while(move_finished == 0){

    // Check if the Stepper has reached desired position
    if ((stepperY.distanceToGo() != 0)) {
      
      stepperY.run();  // Move Stepper into position
      
    }

    // If move is completed display message on Serial Monitor
    if ((move_finished == 0) && (stepperY.distanceToGo() == 0)) {
      Serial.println("Shot Poured");
      move_finished=1;  // Reset move variable
      }
  }
}

//*****************************************************************************************************************************************************

//Move to drink examples

void drink1(){
  moveToPosition(0);
  pourOneShot();
  
}

//*****************************************************************************************************************************************************

void drink2(){
  moveToPosition(8750);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void drink3(){
  moveToPosition(17600);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void drink4(){
  moveToPosition(26600);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void cocktail1(){
  Serial.println("Gin & Tonic");
  drink1();
  drink3();
  moveToPosition(0);
  Serial.print("Done!");
  serialFlush();
}

//*****************************************************************************************************************************************************

void cocktail2(){
  Serial.println("Rum & Coke");
  drink2();
  drink4();
  moveToPosition(0);
  Serial.print("Done!");
  serialFlush();
  
}

void cocktail3(){
  Serial.println("Vodka Cranberry");
  drink1();
  drink4();
  moveToPosition(0);
  Serial.print("Done!");
  serialFlush();
}

void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}
//tim (is gay)


void writeToSlave(long pos){

  sentData = false;
	unsigned long currentMillis2 = millis();
	if (currentMillis2 - previousMillis >= interval) {
    // save the last time you blinked the LED
		previousMillis = currentMillis2;

		unsigned char signBit = 0;

		if (pos < 0){
			pos = 0 - pos;
			signBit = 1;
		}

		byte4 = lowByte(pos);
		pos= pos >> 8;
		byte3 = lowByte(pos);
		pos= pos >> 8;
		byte2 = lowByte(pos);
		pos= pos >> 8;
		byte1 = pos;

		Wire.beginTransmission(4);
		Wire.write(signBit); 
		Wire.write(byte1);
		Wire.write(byte2);
		Wire.write(byte3);
		Wire.write(byte4);
	               // sends one byte  
		Wire.endTransmission(); 
	}

}