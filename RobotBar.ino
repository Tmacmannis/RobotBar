/*
Drink1: 0
Drink2: 8750
Drink3: 17600
Drink4: 26600
Drink Home Positon: 4000
*/

char BluetoothData;  // the data received from bluetooth serial link
#include <Wire.h>

#include "AccelStepper.h"
#include "SerialTransfer.h"

#define home_switch_x 13  // Pin 9 connected to Home Switch (MicroSwitch)
#define home_switch_y 12

// AccelStepper Setup
AccelStepper stepperX(1, 15, 14);  // 1 = Easy Driver interface, 4 = STEP Pin, 5 = DIR Pin
AccelStepper stepperY(1, 19, 18);

SerialTransfer myTransfer;

struct __attribute__((__packed__)) STRUCT {
    byte arr[8];          // 8 bytes
    int32_t currentMode;  // 4 bytes
    int32_t currentPos;   // 4 bytes
} testStruct;

TaskHandle_t Task1;

// Define the Pins used

// Stepper Travel Variables
long TravelX;              // Used to store the X value entered in the Serial Monitor
int move_finished = 1;     // Used to check if move is completed
long initial_homing = -1;  // Used to Home Stepper at startup

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
unsigned long previousMoveMillis = 0;
unsigned long previousSendNanoMillis = 0;

unsigned long testWriteTime = 0;

unsigned long testMicros = micros();
const long interval = 100;

int currentPositionTest;

boolean steppersCalibrated = false;

int testNum = 0;

void setup() {
    Serial.begin(9600);
    Serial2.begin(115200);
    pinMode(home_switch_x, INPUT_PULLUP);
    pinMode(home_switch_y, INPUT_PULLUP);

    // pinMode(18, OUTPUT);
    stepperX.setEnablePin(5);
    stepperX.setPinsInverted(false, false, true);
    stepperX.enableOutputs();

    myTransfer.begin(Serial2);

    // testStruct.currentMode = 0;
    myTransfer.sendDatum(testStruct);

    delay(2000);

    Serial.print("core num: ");
    Serial.println(xPortGetCoreID());

    //moveDot();

    stepperX.setMaxSpeed(15000.0);     // Set Max Speed of Stepper (Faster for regular movements)
    stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
    stepperY.setMaxSpeed(5000.0);
    stepperY.setAcceleration(5000.0);

    xTaskCreatePinnedToCore(
        Task1code, /* Task function. */
        "Task1",   /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        1,         /* priority of the task */
        &Task1,    /* Task handle to keep track of created task */
        0);        /* pin task to core 0 */
    delay(500);

    // homeSteppers();
    delay(500);
    stepperX.disableOutputs();
}

void loop() {
    // updateNano();

    //Process info coming from bluetooth app
    if (Serial.available()) {
        BluetoothData = Serial.read();  //Get next character from bluetooth
        Serial.print(BluetoothData);
        steppersCalibrated = false;
        if (BluetoothData == 'G') {
            cocktail1();
        }
        if (BluetoothData == 'R') cocktail2();
        if (BluetoothData == 'V') cocktail3();
    }
}

//List of Methods used.. Can be put in separate files
//*****************************************************************************************************************************************************

void userEnteredPosition() {
    while (Serial.available() > 0) {  // Check if values are available in the Serial Buffer
        TravelX = Serial.parseInt();
        Serial.print("Moving stepper into position: ");
        Serial.println(TravelX);
        delay(1000);
        moveToPosition(0 - TravelX);
    }
}

//*****************************************************************************************************************************************************

void moveToPosition(int pos) {
    move_finished = 0;
    stepperX.moveTo(pos);

    while (move_finished == 0) {
        // Check if the Stepper has reached desired position
        unsigned long currentMillis = micros();
        if (currentMillis - previousMoveMillis >= 10) {
            previousMoveMillis = currentMillis;
            if ((stepperX.distanceToGo() != 0)) {
                stepperX.run();  // Move Stepper into position
            }
        } else {
            updateNano();
        }

        // If move is completed display message on Serial Monitor
        if ((move_finished == 0) && (stepperX.distanceToGo() == 0)) {
            Serial.println("");
            move_finished = 1;  // Reset move variable
        }
    }
}

//*****************************************************************************************************************************************************

void pourOneShot() {
    move_finished = 0;
    stepperY.moveTo(4100);

    while (move_finished == 0) {
        // Check if the Stepper has reached desired position
        if ((stepperY.distanceToGo() != 0)) {
            stepperY.run();  // Move Stepper into position
        }

        // If move is completed display message on Serial Monitor
        if ((move_finished == 0) && (stepperY.distanceToGo() == 0)) {
            move_finished = 1;  // Reset move variable
        }
    }

    delay(1000);

    move_finished = 0;
    stepperY.moveTo(0);

    while (move_finished == 0) {
        // Check if the Stepper has reached desired position
        if ((stepperY.distanceToGo() != 0)) {
            stepperY.run();  // Move Stepper into position
        }

        // If move is completed display message on Serial Monitor
        if ((move_finished == 0) && (stepperY.distanceToGo() == 0)) {
            Serial.println("Shot Poured");
            move_finished = 1;  // Reset move variable
        }
    }
}

//*****************************************************************************************************************************************************

//Move to drink examples

void drink1() {
    moveToPosition(0);
    pourOneShot();
}

//*****************************************************************************************************************************************************

void drink2() {
    moveToPosition(8750);
    pourOneShot();
}

//*****************************************************************************************************************************************************

void drink3() {
    moveToPosition(17600);
    pourOneShot();
}

//*****************************************************************************************************************************************************

void drink4() {
    moveToPosition(26600);
    pourOneShot();
}

//*****************************************************************************************************************************************************

void cocktail1() {
    stepperX.enableOutputs();
    Serial.println("Gin & Tonic");
    homeSteppers();
    drink1();
    drink3();
    moveToPosition(0);
    Serial.print("Done!");
    serialFlush();
    stepperX.disableOutputs();
}

//*****************************************************************************************************************************************************

void cocktail2() {
    stepperX.enableOutputs();
    Serial.println("Rum & Coke");
    homeSteppers();
    drink2();
    drink4();
    moveToPosition(0);
    Serial.print("Done!");
    serialFlush();
    stepperX.disableOutputs();
}

void cocktail3() {
    stepperX.enableOutputs();
    Serial.println("Vodka Cranberry");
    homeSteppers();
    drink1();
    drink4();
    moveToPosition(0);
    Serial.print("Done!");
    serialFlush();
    stepperX.disableOutputs();
}

void serialFlush() {
    while (Serial1.available() > 0) {
        char t = Serial1.read();
    }
}

void Task1code(void* pvParameters) {
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());

    for (;;) {
        unsigned long currentMillis = micros();
        testStruct.currentPos = stepperX.currentPosition();
        myTransfer.sendDatum(testStruct);
        Serial.println(micros() - currentMillis);
        delay(28);
    }
}