#include "AccelStepper.h" 

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


void setup() {
  Serial.begin(9600);
  pinMode(home_switch, INPUT_PULLUP);
  delay(5);

  homeMainStepper();


  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
  stepperY.setMaxSpeed(5000.0);      
  stepperY.setAcceleration(5000.0);
}

void loop() {

  userEnteredPosition();

  delay(2000);
  cocktail1();
  delay(1000);
  cocktail2();
  delay(10000);

}



//List of Methods used.. Can be put in separate files
//*****************************************************************************************************************************************************
void homeMainStepper(){
     //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
 

// Start Homing procedure of Stepper Motor at startup

  Serial.print("Stepper is Homing . . . . . . . . . . . ");

  while (digitalRead(home_switch)) {  // Make the Stepper move CCW until the switch is activated   
    stepperX.moveTo(initial_homing);  // Set the position to move to
    initial_homing--;  // Decrease by 1 for next move if needed
    stepperX.run();  // Start moving the stepper
    delay(.01);
  }

  stepperX.setCurrentPosition(0);  // Set the current position as zero for now
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
  initial_homing=1;

  while (!digitalRead(home_switch)) { // Make the Stepper move CW until the switch is deactivated
    stepperX.moveTo(initial_homing);  
    stepperX.run();
    initial_homing++;
    delay(.01);
  }
  
  stepperX.setCurrentPosition(0);
  Serial.println("Homing Completed");
  Serial.println("");
  
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

    // If move is completed display message on Serial Monitor
    if ((move_finished == 0) && (stepperX.distanceToGo() == 0)) {
      Serial.println("COMPLETED!");
      Serial.println("");
      Serial.println("Enter Travel distance (Positive for CW / Negative for CCW and Zero for back to Home): ");
      move_finished=1;  // Reset move variable
      }
  }
}

//*****************************************************************************************************************************************************

void pourOneShot() {

  move_finished=0;
  stepperY.moveTo(2000);

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


  delay(3000);

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
  moveToPosition(5000);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void drink2(){
  moveToPosition(8000);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void drink3(){
  moveToPosition(11000);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void drink4(){
  moveToPosition(15000);
  pourOneShot();
}

//*****************************************************************************************************************************************************

void cocktail1(){
  drink1();
  drink3();
  moveToPosition(0);
}

//*****************************************************************************************************************************************************

void cocktail2(){
  drink2();
  drink4();
  moveToPosition(0);
}
