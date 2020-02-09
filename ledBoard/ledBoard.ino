#include <Wire.h>

unsigned long recomb;

unsigned char byte0, byte1, byte2, byte3, byte4;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop()
{
  delay(100);
  // Serial.println("hello");
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  //Serial.print("received: ");
  //Serial.println(howMany);
  while(0 < Wire.available()) // loop through all but the last
  {

    byte0 = Wire.read();

    switch(byte0){
      case 1:
        //Serial.println("1 sent!");
        break;
      case 0:
        //Serial.println("0 sent!");
        break;
    }
    


    
    byte1 = Wire.read(); // receive byte as a character
    byte2 = Wire.read();
    byte3 = Wire.read();
    byte4 = Wire.read();


    recomb = byte1 ;
    recomb = (recomb << 8) | byte2 ;
    recomb = (recomb << 8) | byte3 ;
    recomb = (recomb << 8) | byte4 ;
    Serial.println(recomb);         // print the character
  }
  //int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);
  //Serial.println();// print the integer
}
