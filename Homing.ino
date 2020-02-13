void homeSteppers(){
	homeXStepper();
	homeYStepper();
}

void homeXStepper(){
//  setLEDsHoming();
     //  Set Max Speed and Acceleration of each Steppers at startup for homing
  //Todo: not sure if this does anything here...
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper

  Serial.print("X Stepper is Homing . . . . . . . . . . . ");
  initial_homing = -1;
  while (digitalRead(home_switch_x)) {  // Make the Stepper move CCW until the switch is activated   
    unsigned long currentMillis = micros();
    if (currentMillis - previousMillis1 >= 500) {
      previousMillis1 = currentMillis;
      stepperX.moveTo(initial_homing);  // Set the position to move to
      initial_homing--;  // Decrease by 1 for next move if needed
      stepperX.run();
      sentData = true;
    }
    if(sentData){
       writeToSlave(stepperX.currentPosition());
    }
  }
  Serial.print(initial_homing);

  //********* Todo: not sure what these lines are doing *********
  stepperX.setCurrentPosition(0);  // Set the current position as zero for now
  stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
  //*************************************************************

  initial_homing=1;

  while (!digitalRead(home_switch_x)) { // Make the Stepper move CW until the switch is deactivated
    stepperX.moveTo(initial_homing);  
    stepperX.run();
    initial_homing++;
    delay(1);
  }
  stepperX.setCurrentPosition(0);
  Serial.println("X Axis Homing Completed");
  Serial.println("");
//  setLEDsDoneHoming();
}


void homeYStepper(){

	//move to correct position first.. Add back after initial direction test.. 
	moveToPosition(4000);

	//Todo: not sure if this does anything here...
	stepperY.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
	stepperY.setAcceleration(5000.0);  // Set Acceleration of Stepper

	Serial.print("Y Stepper is Homing . . . . . . . . . . . ");

  initial_homing = -1;
  while (digitalRead(home_switch_y)) {  // Make the Stepper move CCW until the switch is activated   
    unsigned long currentMillis = micros();
    if (currentMillis - previousMillis1 >= 500) {
      previousMillis1 = currentMillis;
      stepperY.moveTo(initial_homing);  // Set the position to move to
      initial_homing--;  // Decrease by 1 for next move if needed
      stepperY.run();
      sentData = true;
    }
    if(sentData){
       writeToSlave(stepperY.currentPosition());
    }
  }
  Serial.print(initial_homing);

  //********* Todo: not sure what these lines are doing *********
  stepperY.setCurrentPosition(0);  // Set the current position as zero for now
  stepperY.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperY.setAcceleration(5000.0);  // Set Acceleration of Stepper
  //*************************************************************

  initial_homing=1;

  while (!digitalRead(home_switch_y)) { // Make the Stepper move CW until the switch is deactivated
    stepperY.moveTo(initial_homing);  
    stepperY.run();
    initial_homing++;
    delay(1);
  }

  move_finished=0;
  stepperY.moveTo(750);

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

  stepperY.setCurrentPosition(0);
  Serial.println("Y Axis Homing Completed");
  Serial.println("");
//  setLEDsDoneHoming();
}