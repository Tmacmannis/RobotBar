boolean xSwitchActivated = false;
boolean xHomingComplete = false;
boolean yHomingFailure = false;
int initialXHoming = -1;
int xHomeCount = 0;

boolean homeStateMachine() {
    switch (currentHomingState) {
        case NOT_HOMING:
            break;

        case X_MOVING_TO_SWITCH: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 500) {
                if (digitalRead(home_switch_x)) {
                    initialXHoming = 1;
                    xSwitchActivated = true;
                    currentHomingState = X_PRESSING_SWITCH;
                }
                previousMillis1 = currentMillis;
                stepperX.moveTo(initialXHoming);  // Set the position to move to
                initialXHoming--;                 // Decrease by 1 for next move if needed
                stepperX.run();
            }
            break;
        }

        case X_PRESSING_SWITCH: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 250000) {
                Serial.println("waited the correct ammount of time");
                previousMillis1 = currentMillis;
                currentHomingState = X_MOVING_OFF_SWITCH;
                stepperX.setCurrentPosition(0);
                stepperX.moveTo(1000);
            }
            break;
        }

        case X_MOVING_OFF_SWITCH: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 500) {
                if (!digitalRead(home_switch_x)) {
                    initialXHoming = -1;
                    stepperX.setCurrentPosition(0);
                    xHomeCount++;
                    if (xHomeCount > 1) {
                        currentHomingState = PAUSE_BEFORE_Y;
                        xHomeCount = 0;
                    } else {
                        currentHomingState = X_MOVING_TO_SWITCH;
                    }
                }

                stepperX.run();
                initialXHoming++;
            }

            break;
        }

        case PAUSE_BEFORE_Y: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 100000) {
                previousMillis1 = currentMillis;
                Serial.println("chillen after x done homing");
                currentHomingState = Y_MOVING_TO_X_POSITION;
            }

            break;
        }

        case Y_MOVING_TO_X_POSITION: {
            moveToPosition(4000);
            currentHomingState = Y_MOVING_TO_SWITCH;
            initialXHoming = -1;
            break;
        }

        case Y_MOVING_TO_SWITCH: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 500) {
                if (!digitalRead(home_switch_y)) {
                    initialXHoming = 1;
                    xSwitchActivated = true;
                    currentHomingState = Y_PRESSING_SWITCH;
                }

                if (initialXHoming < -1800) {
                    if (!yHomingFailure) {
                        Serial.println("Y Stepper Homing Failed!!!");
                    }
                    yHomingFailure = true;
                    stepperY.moveTo(0);
                    stepperY.run();
                    if (stepperY.distanceToGo() == 0) {
                        initialXHoming = -1;
                        stepperY.setCurrentPosition(0);
                        currentHomingState = NOT_HOMING;
                        yHomingFailure = false;
                        return true;
                    }
                }

                if (!yHomingFailure) {
                    previousMillis1 = currentMillis;
                    stepperY.moveTo(initialXHoming);  // Set the position to move to
                    initialXHoming--;                 // Decrease by 1 for next move if needed
                    // Serial.print("initialXHoming: ");
                    // Serial.println(initialXHoming);
                    stepperY.run();
                }
            }
            break;
        }

        case Y_PRESSING_SWITCH: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 100000) {
                Serial.println("waited the correct ammount of time");
                previousMillis1 = currentMillis;
                currentHomingState = Y_MOVING_OFF_SWITCH;
                stepperY.setCurrentPosition(0);
                stepperY.moveTo(1000);
            }
            break;
        }

        case Y_MOVING_OFF_SWITCH: {
            unsigned long currentMillis = micros();
            if (currentMillis - previousMillis1 >= 500) {
                if (stepperY.distanceToGo() == 0) {
                    initialXHoming = -1;
                    stepperY.setCurrentPosition(0);
                    currentHomingState = NOT_HOMING;
                    return true;
                }

                stepperY.run();
                initialXHoming++;
            }

            break;
        }
    }

    return false;
}