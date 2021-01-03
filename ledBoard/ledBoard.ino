#include "I2CTransfer.h"
#include <FastLED.h>

#define DATA_PIN    5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    63
#define BRIGHTNESS  96

CRGB leds[NUM_LEDS];


I2CTransfer myTransfer;

struct __attribute__((__packed__)) STRUCT {
    byte arr[8];       // 8 bytes
    int32_t currentMode;          // 4 bytes
    int32_t currentPos;             // 4 bytes
} testStruct;

unsigned long previousMillis = 0;


/////////////////////////////////////////////////////////////////// Callbacks
void hi() {
    myTransfer.rxObj(testStruct);
    //  Serial.print(testStruct.z);
}
///////////////////////////////////////////////////////////////////


void setup() {
    Serial.begin(9600);
    Wire.begin(0);
    pinMode(13, OUTPUT);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    functionPtr callbackArr[] = { hi };

    ///////////////////////////////////////////////////////////////// Config Parameters
    configST myConfig;
    myConfig.debug        = true;
    myConfig.callbacks    = callbackArr;
    myConfig.callbacksLen = sizeof(callbackArr) / sizeof(functionPtr);
    /////////////////////////////////////////////////////////////////

    myTransfer.begin(Wire, myConfig);
}


void loop() {
    // Do nothing
    Serial.print("testing");
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 100) {
        previousMillis = currentMillis;
        Serial.print("current mode: ");
        Serial.print(testStruct.currentMode);
        Serial.print(" current position: ");
        Serial.println(testStruct.currentPos);

        if(testStruct.currentMode == 1) {
            for(int i = 0; i < NUM_LEDS; i++) {
                leds[i] = CRGB::Red;
            }
            FastLED.show();
        } else {
            for(int i = 0; i < NUM_LEDS; i++) {
                leds[i] = CRGB::Green;
            }
            FastLED.show();
        }
    }


}
