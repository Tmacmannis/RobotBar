#include <FastLED.h>
#include <Wire.h>
#include "AccelStepper.h"
#include "SerialTransfer.h"
#include "EspMQTTClient.h"

#define home_switch_x 13  // Pin 9 connected to Home Switch (MicroSwitch)
#define home_switch_y 12
#define DATA_PIN 22
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 63
#define BRIGHTNESS 200

EspMQTTClient client(
    "mySSID",
    "myPASSWORD",
    "mqttIP",                                                     // MQTT Broker server ip
    "tim",                                                               // Can be omitted if not needed
    "14Q4YsC6YrXl",                                                      // Can be omitted if not needed
    "haPASSWORD",  // Client name that uniquely identify your device
    1883                                                                 // The MQTT port, default to 1883. this line can be omitted
);

CRGB leds[NUM_LEDS];

// AccelStepper Setup
AccelStepper stepperX(1, 15, 14);  // 1 = Easy Driver interface, 4 = STEP Pin, 5 = DIR Pin
AccelStepper stepperY(1, 19, 18);

int currentMode;

SerialTransfer myTransfer;

struct __attribute__((__packed__)) STRUCT {
    byte arr[8];          // 8 bytes
    int32_t currentMode;  // 4 bytes
    int32_t currentPos;   // 4 bytes
    int32_t brightness;   // 4 bytess
} testStruct;

TaskHandle_t Task1;

// Stepper Travel Variables
long TravelX;              // Used to store the X value entered in the Serial Monitor
int move_finished = 1;     // Used to check if move is completed
long initial_homing = -1;  // Used to Home Stepper at startup

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;

boolean steppersCalibrated = false;
char BluetoothData;  // the data received from bluetooth serial link
boolean drinkSelected = false;
int currentDrink = 0;

void setup() {
    Serial.begin(9600);
    Serial2.begin(115200);
    pinMode(home_switch_x, INPUT_PULLUP);
    pinMode(home_switch_y, INPUT_PULLUP);

    client.enableDebuggingMessages();                                           // Enable debugging messages sent to serial output
    client.enableHTTPWebUpdater();                                              // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
    client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true 

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

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
}

void loop() {

    if (drinkSelected) {
        if (currentDrink == 1) {
            Serial.print("main task running on core ");
            Serial.println(xPortGetCoreID());
            steppersCalibrated = false;
            cocktail1();
        }

        if (currentDrink == 2) {
            Serial.print("main task running on core ");
            Serial.println(xPortGetCoreID());
            steppersCalibrated = false;
            cocktail2();
        }

        drinkSelected = false;
    }

    


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
        if ((stepperX.distanceToGo() != 0)) {
            stepperX.run();  // Move Stepper into position
        }
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
    // pourOneShot();
}

//*****************************************************************************************************************************************************

void drink2() {
    moveToPosition(8750);
    // pourOneShot();
}

//*****************************************************************************************************************************************************

void drink3() {
    moveToPosition(17600);
    // pourOneShot();
}

//*****************************************************************************************************************************************************

void drink4() {
    moveToPosition(26600);
    // pourOneShot();
}

//*****************************************************************************************************************************************************

void cocktail1() {
    stepperX.enableOutputs();
    stepperY.enableOutputs();
    Serial.println("Gin & Tonic");
    homeSteppers();
    drink1();
    drink3();
    moveToPosition(0);
    Serial.print("Done!");
    serialFlush();
    stepperX.disableOutputs();
    stepperY.disableOutputs();
}

//*****************************************************************************************************************************************************

void cocktail2() {
    stepperX.enableOutputs();
    stepperY.enableOutputs();
    Serial.println("Rum & Coke");
    homeSteppers();
    drink2();
    drink4();
    moveToPosition(0);
    Serial.print("Done!");
    serialFlush();
    stepperX.disableOutputs();
    stepperY.disableOutputs();
}

void cocktail3() {
    stepperX.enableOutputs();
    stepperY.enableOutputs();
    Serial.println("Vodka Cranberry");
    homeSteppers();
    drink1();
    drink4();
    moveToPosition(0);
    Serial.print("Done!");
    serialFlush();
    stepperX.disableOutputs();
    stepperY.disableOutputs();
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
        delay(28);

        EVERY_N_MILLISECONDS(50) {
            client.loop();  // takes 60 micro seconds to complete, fast...
        }

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
        if(payload.indexOf("brightness") >= 0){
            testStruct.brightness = payload.substring(11).toInt();;
        }
    });

    // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
    client.subscribe("mytopic/wildcardtest/#", [](const String& topic, const String& payload) {
        Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload);
    });

    // Publish a message to "mytopic/test"
    client.publish("mytopic/test", "This is a message");  // You can activate the retain flag by setting the third parameter to true

    // Execute delayed instructions
    client.executeDelayed(5 * 1000, []() {
        client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
    });
}