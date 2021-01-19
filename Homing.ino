boolean homeSteppers() {
    testStruct.currentMode = 1;
    while(!steppersCalibrated) {
        homeXStepper();
        delay(250);

        homeXStepper();
        delay(250);

        while(homeXStepper() < -150) {
            delay(250);
        }

        if(homeYStepper()) {
            steppersCalibrated = true;
            testStruct.currentMode = 0;
            return true;
        } else {
            Serial.println("Y Axis Homing Failed!!!!");
        }
    }
    testStruct.currentMode = 0;
    return false;

}

int homeXStepper() {
    stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
    stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
    int inital_homing_test = -1;
    int finalXMovement = 0;
    Serial.println("X Stepper is Homing . . . . . . . . . . . ");
    inital_homing_test = -1;
    while (true) {  // Make the Stepper move CCW until the switch is activated
        unsigned long currentMillis = micros();
        if (currentMillis - previousMillis1 >= 500) {
            if(digitalRead(home_switch_x)) {
                break;
            }
            previousMillis1 = currentMillis;
            stepperX.moveTo(inital_homing_test);  // Set the position to move to
            inital_homing_test--;  // Decrease by 1 for next move if needed
            stepperX.run();
        }
    }

    finalXMovement = inital_homing_test;

    Serial.println(inital_homing_test);

    stepperX.setCurrentPosition(0);  // Set the current position as zero for now

    inital_homing_test = 1;

    while (digitalRead(home_switch_x)) { // Make the Stepper move CW until the switch is deactivated
        stepperX.moveTo(inital_homing_test);
        stepperX.run();
        inital_homing_test++;
        delay(1);
    }
    stepperX.setCurrentPosition(0);
    Serial.println("X Axis Homing Completed");
    return finalXMovement;
}



boolean homeYStepper() {

    //move to correct position first.. Add back after initial direction test..
    moveToPosition(4000);

    //Todo: not sure if this does anything here...
    stepperY.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
    stepperY.setAcceleration(5000.0);  // Set Acceleration of Stepper

    Serial.println("Y Stepper is Homing . . . . . . . . . . . ");

    initial_homing = -1;
    while (digitalRead(home_switch_y)) {  // Make the Stepper move CCW until the switch is activated
        unsigned long currentMillis = micros();
        if (currentMillis - previousMillis1 >= 500) {
            previousMillis1 = currentMillis;
            stepperY.moveTo(initial_homing);  // Set the position to move to
            initial_homing--;  // Decrease by 1 for next move if needed
            stepperY.run();
        }

        if(initial_homing < -1650) {
            move_finished = 0;
            stepperY.moveTo(0);

            while(move_finished == 0) {

                // Check if the Stepper has reached desired position
                if ((stepperY.distanceToGo() != 0)) {
                    stepperY.run();  // Move Stepper into position
                }

                // If move is completed display message on Serial Monitor
                if ((move_finished == 0) && (stepperY.distanceToGo() == 0)) {
                    move_finished = 1; // Reset move variable
                }
            }
            return false;
        }
    }
    Serial.print("Y homing value is: ");
    Serial.println(initial_homing);

    stepperY.setCurrentPosition(0);  // Set the current position as zero for now

    initial_homing = 1;

    while (!digitalRead(home_switch_y)) { // Make the Stepper move CW until the switch is deactivated
        stepperY.moveTo(initial_homing);
        stepperY.run();
        initial_homing++;
        delay(1);
    }

    move_finished = 0;
    stepperY.moveTo(1000);

    while(move_finished == 0) {

        // Check if the Stepper has reached desired position
        if ((stepperY.distanceToGo() != 0)) {
            stepperY.run();  // Move Stepper into position
        }

        // If move is completed display message on Serial Monitor
        if ((move_finished == 0) && (stepperY.distanceToGo() == 0)) {
            move_finished = 1; // Reset move variable
        }
    }

    stepperY.setCurrentPosition(0);
    Serial.println("Y Axis Homing Completed");
    Serial.println("");
    return true;
    //  setLEDsDoneHoming();
}