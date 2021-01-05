#include <FastLED.h>

#include "SerialTransfer.h"

#define DATA_PIN 6
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 63
#define BRIGHTNESS 200

CRGB leds[NUM_LEDS];

SerialTransfer myTransfer;

struct __attribute__((__packed__)) STRUCT {
    byte arr[8];          // 8 bytes
    int32_t currentMode;  // 4 bytes
    int32_t currentPos;   // 4 bytes
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
    Serial1.begin(115200);
    myTransfer.begin(Serial1);

    pinMode(13, OUTPUT);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    // Do nothing
    if (myTransfer.available()) {
        myTransfer.rxObj(testStruct);
        // Serial.print("current mode: ");
        // Serial.print(testStruct.currentMode);
        Serial.print(" current position: ");
        Serial.println(testStruct.currentPos);

        if (testStruct.currentMode == 1) {
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = CRGB::Red;
            }
            FastLED.show();
        } else if(testStruct.currentMode == 0){
            
            int followPos = map(testStruct.currentPos, 0, 26600, 4, 45);
            leds[followPos] = CRGB::Green;
            leds[followPos + 1] = CRGB::Green;
            leds[followPos + 2] = CRGB::Green;
            leds[followPos + 3] = CRGB::Green;
            leds[followPos + 4] = CRGB::Green;
            fadeToBlackBy( leds, NUM_LEDS, 30);

            FastLED.show();
        } else{}
    }
}
