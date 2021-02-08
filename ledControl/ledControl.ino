#include <FastLED.h>

#include "SerialTransfer.h"

#define DATA_PIN 13
#define BOTTLE_DATA_PIN 15
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 63
#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 120

#define IDLE 0
#define HOMING 1
#define MAKE_DRINK 2

CRGB leds[NUM_LEDS];
CRGB bottleLeds[12];

int prevBrightness = 0;
int currentSwitchState = 0;
int prevAnimation = 0;

SerialTransfer myTransfer;

struct __attribute__((__packed__)) STRUCT {
    byte arr[8];          // 8 bytes
    int32_t currentMode;  // 4 bytes
    int32_t currentPos;   // 4 bytes
    int32_t brightness;   // 4 bytes
    int32_t redValue;
    int32_t greenValue;
    int32_t blueValue;
    int32_t lightState;
    int32_t animation;
} testStruct;

unsigned long previousMillis = 0;

boolean becameIdle;
boolean becameHoming;
boolean animationChanged = false;
boolean brightnessIncreasing = false;
boolean randomDrinkColors = false;
boolean bottlesInSync = true;

uint8_t tempColor1;
uint8_t tempColor2;
uint8_t tempColor3;
uint8_t tempColor4;

/////////////////////////////////////////////////////////////////// Callbacks
void hi() {
    myTransfer.rxObj(testStruct);
    //  Serial.print(testStruct.z);
}
///////////////////////////////////////////////////////////////////

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};

uint8_t gCurrentPatternNumber = 0;  // Index number of which pattern is current
uint8_t gHue = 0;                   // rotating "base color" used by many of the patterns
uint8_t breathingBrightness = 0;
uint8_t breathingHue = 0;

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200);
    myTransfer.begin(Serial1);

    pinMode(13, OUTPUT);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, BOTTLE_DATA_PIN, COLOR_ORDER>(bottleLeds, 12).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    // Do nothing
    if (myTransfer.available()) {
        myTransfer.rxObj(testStruct);

        // EVERY_N_MILLISECONDS(100) {
        //     Serial.print(" current position: ");
        //     Serial.println(testStruct.currentPos);
        //     Serial.print(" current brightness: ");
        //     Serial.println(testStruct.brightness);
        //     Serial.print(" current mode: ");
        //     Serial.println(testStruct.currentMode);
        //     Serial.print(" red value: ");
        //     Serial.println(testStruct.redValue);
        //     Serial.print(" green value: ");
        //     Serial.println(testStruct.greenValue);
        //     Serial.print(" blue blue: ");
        //     Serial.println(testStruct.blueValue);
        //     Serial.print(" lightState: ");
        //     Serial.println(testStruct.lightState);
        //     Serial.print(" animation: ");
        //     Serial.println(testStruct.animation);
        //     Serial.println();
        //     Serial.println();
        // }

        //check brightness
        if (testStruct.brightness != prevBrightness && currentSwitchState == 1) {
            FastLED.setBrightness(map(testStruct.brightness, 3, 255, 0, 255));
            prevBrightness = testStruct.brightness;
            FastLED.show();
        }

        if (testStruct.lightState != currentSwitchState) {
            if (testStruct.lightState == 0) {
                prevBrightness = testStruct.brightness;
                FastLED.setBrightness(0);
                currentSwitchState = 0;
            } else {
                FastLED.setBrightness(prevBrightness);
                currentSwitchState = 1;
            }
        }
    }
    lightStateMachine();
}

void lightStateMachine() {
    switch (testStruct.currentMode) {
        case IDLE: {
            selectAnimation();
            bottlesInSync = true;
            randomDrinkColors = false;

            break;
        }
        case HOMING: {
            selectAnimation();
            bottlesInSync = true;
            // if (!becameHoming) {
            //     for (int i = 0; i < NUM_LEDS; i++) {
            //         leds[i] = CRGB::Red;
            //     }
            //     FastLED.show();
            //     becameIdle = false;
            //     becameHoming = true;
            // }
            break;
        }
        case MAKE_DRINK: {
            // int followPos = map(testStruct.currentPos, 0, 26600, 4, 45);
            // leds[followPos] = CRGB::Green;
            // leds[followPos + 1] = CRGB::Green;
            // leds[followPos + 2] = CRGB::Green;
            // leds[followPos + 3] = CRGB::Green;
            // leds[followPos + 4] = CRGB::Green;
            // fadeToBlackBy(leds, NUM_LEDS, 30);
            bottlesInSync = false;
            selectAnimation();

            if (!randomDrinkColors) {
                tempColor1 = random8();
                tempColor2 = random8();
                tempColor3 = random8();
                tempColor4 = random8();
                randomDrinkColors = true;
            }

            int tempPos = testStruct.currentPos;

            if (testStruct.currentPos == 0) {
                bottleLeds[0] = CHSV(tempColor1, 255, 255);
                bottleLeds[1] = CHSV(tempColor1, 255, 255);
                bottleLeds[2] = CHSV(tempColor1, 255, 255);
            }

            if (testStruct.currentPos == 8750) {
                bottleLeds[3] = CHSV(tempColor2, 255, 255);
                bottleLeds[4] = CHSV(tempColor2, 255, 255);
                bottleLeds[5] = CHSV(tempColor2, 255, 255);
            }

            if (testStruct.currentPos == 17600) {
                bottleLeds[6] = CHSV(tempColor3, 255, 255);
                bottleLeds[7] = CHSV(tempColor3, 255, 255);
                bottleLeds[8] = CHSV(tempColor3, 255, 255);
            }

            if (testStruct.currentPos == 26600) {
                bottleLeds[9] = CHSV(tempColor4, 255, 255);
                bottleLeds[10] = CHSV(tempColor4, 255, 255);
                bottleLeds[11] = CHSV(tempColor4, 255, 255);
            }

            fadeToBlackBy(bottleLeds, 12, 5);

            FastLED.show();
            break;
        }
    }
}

void selectAnimation() {
    if (testStruct.animation != prevAnimation) {
        animationChanged = true;
    }

    switch (testStruct.animation) {
        case 0: {  //solid colors
                   //if (animationChanged) {
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i].setRGB(testStruct.redValue, testStruct.greenValue, testStruct.blueValue);
            }
            if (bottlesInSync) {
                for (int i = 0; i < 12; i++) {
                    bottleLeds[i].setRGB(testStruct.redValue, testStruct.greenValue, testStruct.blueValue);
                }
            }

            FastLED.show();
            // }
            break;
        }
        case 1: {  //demo reel
            gPatterns[gCurrentPatternNumber]();

            // send the 'leds' array out to the actual LED strip
            FastLED.show();
            // insert a delay to keep the framerate modest
            FastLED.delay(1000 / FRAMES_PER_SECOND);

            // do some periodic updates
            EVERY_N_MILLISECONDS(20) { gHue++; }    // slowly cycle the "base color" through the rainbow
            EVERY_N_SECONDS(10) { nextPattern(); }  // change patterns periodically

            break;
        }
        case 2: {  //Breathing Single
            CHSV temp1 = hsv2rgb();
            temp1.value = breathingBrightness;

            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = temp1;
            }
            if (bottlesInSync) {
                for (int i = 0; i < 12; i++) {
                    bottleLeds[i] = temp1;
                }
            }

            FastLED.show();

            EVERY_N_MILLISECONDS(10) {
                if (breathingBrightness >= testStruct.brightness) {
                    brightnessIncreasing = false;
                }
                if (breathingBrightness == 0) {
                    brightnessIncreasing = true;
                }
                if (brightnessIncreasing) {
                    Serial.print("increasing ");
                    Serial.println(breathingBrightness);
                    breathingBrightness++;
                } else {
                    Serial.print("decreasing ");
                    Serial.println(breathingBrightness);
                    breathingBrightness--;
                }
            }
            break;
        }

        case 3: {  //breathing multi
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = CHSV(breathingHue, 255, breathingBrightness);
            }
            if (bottlesInSync) {
                for (int i = 0; i < 12; i++) {
                    bottleLeds[i] = CHSV(breathingHue, 255, breathingBrightness);
                }
            }

            FastLED.show();

            EVERY_N_MILLISECONDS(10) {
                if (breathingBrightness >= testStruct.brightness) {
                    brightnessIncreasing = false;
                }
                if (breathingBrightness == 0) {
                    brightnessIncreasing = true;
                    breathingHue = random8();
                }
                if (brightnessIncreasing) {
                    Serial.print("increasing ");
                    Serial.println(breathingBrightness);
                    breathingBrightness++;
                } else {
                    Serial.print("decreasing ");
                    Serial.println(breathingBrightness);
                    breathingBrightness--;
                }
            }
            break;
        }
        case 4: {  //scanner
            CHSV temp1 = hsv2rgb();
            fadeToBlackBy(leds, NUM_LEDS, 20);
            fadeToBlackBy(bottleLeds, 12, 20);
            int pos = beatsin16(13, 0, NUM_LEDS - 1);
            int pos2 = (beatsin16(13, 0, 4 - 1) * 3);
            leds[pos] += temp1;
            if (bottlesInSync) {
                bottleLeds[pos2] += temp1;
                bottleLeds[pos2 + 1] += temp1;
                bottleLeds[pos2 + 2] += temp1;
            }

            FastLED.show();
            FastLED.delay(1000 / FRAMES_PER_SECOND);
            break;
        }
        case 5: {  //rainbow
            fill_rainbow(leds, NUM_LEDS, gHue, 7);
            if (bottlesInSync) {
                fill_rainbow(bottleLeds, 12, gHue, 7);
            }

            EVERY_N_MILLISECONDS(20) { gHue++; }
            FastLED.show();
            break;
        }
    }

    animationChanged = false;
}

CHSV hsv2rgb() {
    CRGB color1temp;
    color1temp.r = testStruct.redValue;
    color1temp.g = testStruct.greenValue;
    color1temp.b = testStruct.blueValue;
    CHSV temp1 = rgb2hsv_approximate(color1temp);
    return temp1;
}

//IDLE Animations *******************************************************************************************

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern() {
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void rainbow() {
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
    if (bottlesInSync) {
        fill_rainbow(bottleLeds, 12, gHue, 7);
    }
}

void rainbowWithGlitter() {
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter) {
    if (random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
        if (bottlesInSync) {
            bottleLeds[random16(12)] += CRGB::White;
        }
    }
}

void confetti() {
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    fadeToBlackBy(bottleLeds, 12, 10);
    int pos = random16(NUM_LEDS);
    int pos2 = random16(12);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
    if (bottlesInSync) {
        bottleLeds[pos2] += CHSV(gHue + random8(64), 200, 255);
    }
}

void sinelon() {
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    fadeToBlackBy(bottleLeds, 12, 20);
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    int pos2 = (beatsin16(13, 0, 4 - 1) * 3);
    leds[pos] += CHSV(gHue, 255, 192);
    if (bottlesInSync) {
        bottleLeds[pos2] += CHSV(gHue, 255, 192);
        bottleLeds[pos2 + 1] += CHSV(gHue, 255, 192);
        bottleLeds[pos2 + 2] += CHSV(gHue, 255, 192);
    }
}

void bpm() {
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++) {  //9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
    if (bottlesInSync) {
        for (int i = 0; i < 12; i++) {  //9948
            bottleLeds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }
}

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    fadeToBlackBy(bottleLeds, 12, 20);
    byte dothue = 0;
    for (int i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        if (bottlesInSync) {
            bottleLeds[beatsin16(i + 7, 0, 12 - 1)] |= CHSV(dothue, 200, 255);
        }
        dothue += 32;
    }
}