/*
Drink1: 0
Drink2: 8750
Drink3: 17600
Drink4: 26600
Drink Home Positon: 4000
*/

char BluetoothData; // the data received from bluetooth serial link
#include "AccelStepper.h" 
#include <Wire.h>

#define home_switch_x 9 // Pin 9 connected to Home Switch (MicroSwitch)
#define home_switch_y 10

// AccelStepper Setup
AccelStepper stepperX(1, 15, 14);   // 1 = Easy Driver interface, 4 = STEP Pin, 5 = DIR Pin
AccelStepper stepperY(1, 17, 16);

// Define the Pins used

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
  Serial1.begin(9600);
  pinMode(home_switch_x, INPUT_PULLUP);
  pinMode(home_switch_y, INPUT_PULLUP);
  Wire.begin();
  delay(5);

  //moveDot();

  homeSteppers();

  stepperX.setMaxSpeed(15000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
  stepperY.setMaxSpeed(5000.0);      
  stepperY.setAcceleration(5000.0);
}

void loop() {

  //Process info coming from bluetooth app
  if (Serial1.available()){
    BluetoothData=Serial1.read(); //Get next character from bluetooth
    Serial1.print(BluetoothData);
    
    if(BluetoothData=='G'){
      cocktail1();
    } 
    if(BluetoothData=='R') cocktail2();
    if(BluetoothData=='V') cocktail3();
  }
}

//List of Methods used.. Can be put in separate files
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
  while(Serial1.available() > 0) {
    char t = Serial1.read();
  }
}


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
		Wire.endTransmission(); 
	}

}
