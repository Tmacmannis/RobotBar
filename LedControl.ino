//void setLEDsHoming(){
//  for (int i = 0; i<70; i++){
//    leds[i] = CRGB::Red;
//  }
//  FastLED.show();
//}
//
//void setLEDsDoneHoming(){
//  for (int i = 0; i<70; i++){
//    leds[i] = CRGB::Green;
//  }
//  FastLED.show();
//}
//
//void moveDot(){
//  for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
//      // Turn our current led on to white, then show the leds
//      leds[whiteLed] = CRGB::White;
//
//      // Show the leds (only one of which is set to white, from above)
//      FastLED.show();
//
//      // Wait a little bit
//      delay(10);
//
//      // Turn our current led back to black for the next loop around
//      leds[whiteLed] = CRGB::Black;
//   }
//}
//
//void changeColorEverySec(){
//
//  unsigned long currentMillis = millis();
//
//  if (currentMillis - previousMillis >= interval) {
//    // save the last time you blinked the LED
//    previousMillis = currentMillis;
//
//    Serial.println("In led control");
//
//    // if the LED is off turn it on and vice-versa:
//    if (red) {
//      for (int i = 0; i<70; i++){
//        leds[i] = CRGB::Black;
//        red = false;
//      }
//    } else {
//      for (int i = 0; i<70; i++){
//        leds[i] = CRGB::Red;
//        red = true;
//      }
//    }
//
//    FastLED.show();
//  }
//  
//}
