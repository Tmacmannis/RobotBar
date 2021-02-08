#include <Wire.h>

#include "AccelStepper.h"
#include "EspMQTTClient.h"
#include "SerialTransfer.h"
#include <credentials.h>

#define home_switch_x 13  // Pin 9 connected to Home Switch (MicroSwitch)
#define home_switch_y 12

#define IDLE 0
#define HOMING 1
#define MAKE_DRINK 2

//State machine states
#define NOT_HOMING 0
#define X_MOVING_TO_SWITCH 1
#define X_PRESSING_SWITCH 2
#define X_MOVING_OFF_SWITCH 3
#define PAUSE_BEFORE_Y 4
#define Y_MOVING_TO_X_POSITION 5
#define Y_MOVING_TO_SWITCH 6
#define Y_PRESSING_SWITCH 7
#define Y_MOVING_OFF_SWITCH 8

EspMQTTClient client(
    mySSID,
    myPASSWORD,
    mqttIP,                                                     // MQTT Broker server ip
    "tim",                                                               // Can be omitted if not needed
    "14Q4YsC6YrXl",                                                      // Can be omitted if not needed
    haPASSWORD,  // Client name that uniquely identify your device
    haPORT                                                                 // The MQTT port, default to 1883. this line can be omitted
);

// AccelStepper Setup
AccelStepper stepperX(1, 15, 14);  // 1 = Easy Driver interface, 4 = STEP Pin, 5 = DIR Pin
AccelStepper stepperY(1, 19, 18);

SerialTransfer myTransfer;

struct __attribute__((__packed__)) STRUCT {
    byte arr[8];          // 8 bytes
    int32_t currentMode;  // 4 bytes
    int32_t currentPos;   // 4 bytes
    int32_t brightness;   // 4 bytess
    int32_t redValue;
    int32_t greenValue;
    int32_t blueValue;
    int32_t lightState;
    int32_t animation;
} testStruct;

TaskHandle_t Task1;

// Stepper Travel Variables
long TravelX;              // Used to store the X value entered in the Serial Monitor
int move_finished = 1;     // Used to check if move is completed
long initial_homing = -1;  // Used to Home Stepper at startup

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
unsigned long homingTime = 0;

boolean steppersCalibrated = false;
char BluetoothData;  // the data received from bluetooth serial link
boolean drinkSelected = false;
int currentDrink = 0;
boolean movingToPosition = false;
boolean pour1 = false;
boolean pour2 = false;
boolean pour3 = false;
boolean pour4 = false;
boolean pour1Move = false;
boolean pour2Move = false;
boolean pour3Move = false;
boolean pour4Move = false;

int sendRGB[3];

//Master switch for pouring shots
boolean pourShots = true;

int initialXHoming = -1;

int currentHomingState = NOT_HOMING;

void setup() {
    Serial.begin(9600);
    Serial2.begin(115200);
    pinMode(home_switch_x, INPUT_PULLUP);
    pinMode(home_switch_y, INPUT_PULLUP);

    client.enableDebuggingMessages();                                           // Enable debugging messages sent to serial output
    client.enableHTTPWebUpdater();                                              // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
    client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

    stepperX.setEnablePin(5);
    stepperX.setPinsInverted(false, false, true);
    stepperX.enableOutputs();

    stepperY.setEnablePin(21);
    stepperY.setPinsInverted(false, false, true);
    stepperY.enableOutputs();

    myTransfer.begin(Serial2);

    delay(2000);

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

    stepperX.disableOutputs();
    stepperY.disableOutputs();

    testStruct.currentMode = IDLE;
}

void loop() {
    stateMachine();
}

void stateMachine() {
    switch (testStruct.currentMode) {
        case IDLE:
            if (drinkSelected) {
                drinkSelected = false;
                stepperX.enableOutputs();
                stepperY.enableOutputs();
                stepperX.setMaxSpeed(5000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
                stepperX.setAcceleration(5000.0);  // Set Acceleration of Stepper
                testStruct.currentMode = HOMING;
            }
            break;
        case HOMING:
            if (currentHomingState == NOT_HOMING) {
                currentHomingState = X_MOVING_TO_SWITCH;
            }
            if (homeStateMachine()) {
                testStruct.currentMode = MAKE_DRINK;
            }
            break;
        case MAKE_DRINK:
            if (makeDrink()) {
                stepperX.disableOutputs();
                stepperY.disableOutputs();
                testStruct.currentMode = IDLE;
            }

            break;
    }
}

boolean makeDrink() {
    switch (currentDrink) {
        case 0:
            return true;
            break;
        case 1:  //rum and coke
            if (makeCocktail(17600, 0)) {
                return true;
            }
            break;
        case 2:  //jack and coke
            if (makeCocktail(8750, 26600)) {
                return true;
            }
            break;
    }

    return false;
}

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

boolean moveToPosition(int pos) {
    if (!movingToPosition) {
        movingToPosition = true;
        stepperX.moveTo(pos);
    }

    stepperX.run();  // Move Stepper into position

    if (stepperX.distanceToGo() == 0) {
        Serial.println("New Move over");
        movingToPosition = false;
        return true;
    }

    return false;
}

//*****************************************************************************************************************************************************

void pourOneShot() {
    if (!pourShots) {
        return;
    }
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

boolean makeCocktail(int pos1, int pos2) {
    if (!pour1Move) {
        if (moveToPosition(pos1)) {
            Serial.println("Made it into position 1");
            pour1Move = true;
        }
    }

    if (!pour1 && pour1Move) {
        pour1 = true;
        pourOneShot();
    }

    if (!pour3Move && pour1 && pour1Move) {
        if (moveToPosition(pos2)) {
            Serial.println("Made it into position 3");
            pour3Move = true;
        }
    }

    if (!pour3 && pour3Move) {
        pour3 = true;
        pourOneShot();
    }

    if (pour1Move && pour3Move && moveToPosition(0)) {
        pour1 = false;
        pour3 = false;
        pour1Move = false;
        pour3Move = false;
        return true;
    }

    return false;
}

//*****************************************************************************************************************************************************

void Task1code(void* pvParameters) {
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());

    for (;;) {
        delay(28);

        client.loop();  // takes 60 micro seconds to complete, fast...

        testStruct.currentPos = stepperX.currentPosition();
        myTransfer.sendDatum(testStruct);
    }
}

void onConnectionEstablished() {
    // Subscribe to "mytopic/test" and display received message to Serial
    client.subscribe("mytopic/test", [](const String& payload) {
        Serial.print("payload is: ");
        Serial.println(payload);
        Serial.print("sub task running on core ");
        Serial.println(xPortGetCoreID());
        Serial.print("current mode is: ");
        Serial.println(testStruct.currentMode);
        if (payload == "Rum & Coke") {
            Serial.println("we have a match");
            currentDrink = 1;
            drinkSelected = true;
        }
        if (payload == "Jack & Coke") {
            Serial.println("we have a match");
            currentDrink = 2;
            drinkSelected = true;
        }
        // if (payload.indexOf("brightness") >= 0) {
        //     testStruct.brightness = payload.substring(11).toInt();
        // }

        if (payload == "Stop!") {
            testStruct.currentMode = IDLE;
            currentHomingState = NOT_HOMING;
            pour1 = false;
            pour2 = false;
            pour3 = false;
            pour4 = false;
            pour1Move = false;
            pour2Move = false;
            pour3Move = false;
            pour4Move = false;
            initialXHoming = -1;
            movingToPosition = false;
        }
    });

    client.subscribe("barbot/color", [](const String& payload) {
        Serial.print("Color payload is: ");
        Serial.println(payload);

        String rval = getValue(payload, ',', 0);
        String gval = getValue(payload, ',', 1);
        String bval = getValue(payload, ',', 2);

        int testRed = rval.toInt();
        int testGreen = gval.toInt();
        int testBlue = bval.toInt();
        Serial.print("testing red: ");
        Serial.println(testRed);
        Serial.print("testing green: ");
        Serial.println(testGreen);
        Serial.print("testing blue: ");
        Serial.println(testBlue);

        testStruct.redValue = testRed;
        testStruct.greenValue = testGreen;
        testStruct.blueValue = testBlue;
    });

    client.subscribe("barbot/brightness", [](const String& payload) {
        Serial.print("brightness payload is: ");
        Serial.println(payload);
        testStruct.brightness = payload.toInt();
    });

    client.subscribe("barbot/lightsOnOFF", [](const String& payload) {
        Serial.print("lightsOnOFF payload is: ");
        Serial.println(payload);
        if (payload == "ON") {
            testStruct.lightState = 1;
            Serial.println("Lights set to ON");
        } else if (payload == "OFF") {
            testStruct.lightState = 0;
            Serial.println("Lights set to OFF");
        } else {
        }
    });

    client.subscribe("barbot/effects", [](const String& payload) {
        Serial.print("effects payload is: ");
        Serial.println(payload);
        setAnimation(payload);
    });

    // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
    client.subscribe("mytopic/wildcardtest/#", [](const String& topic, const String& payload) {
        Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload);
    });

    // Publish a message to "mytopic/test"
    client.publish("mytopic/test", "This is a message");  // You can activate the retain flag by setting the third parameter to true
}

String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setAnimation(String payload) {
    if (payload == "Solid Color") {
        testStruct.animation = 0;
    }
    if (payload == "Demo Reel") {
        testStruct.animation = 1;
    }
    if (payload == "Breathing Single Color") {
        testStruct.animation = 2;
    }
    if (payload == "Breathing Multi Color") {
        testStruct.animation = 3;
    }
    if (payload == "Scanner") {
        testStruct.animation = 4;
    }
    if (payload == "Rainbow") {
        testStruct.animation = 5;
    }
}