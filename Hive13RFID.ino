#include <Wiegand.h>

// I'm using the Wiegand library to make reading the files easier.  
// Pins:  6  = Relay for the door
//        13 = Red/Green light on RFID Reader.  High == Red, Low == Green.
//        12 = Buzzer attached to RFID Reader.  High == Off, Low == On.
//        A0 = Magnetic Switch attached to top of door.  
// Variables:
//        count = Integer, used in Loop for Door Open cycle
//        sensorValue = Integer, used to determine values of magnetic sensors
//        openCount = Integer, used for OTP-ish kinda shit (preventing replay attacks from the door)
//        readCount = Integer, used for OTP-ish kinda stuff  (preventing replay attacks from the door).



WIEGAND wg;

void setup() {
    Serial.begin(57600);
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(6, OUTPUT);
    wg.begin();
    int count = 0;          // Used in loop for door open cycle
    int sensorValue = 0;    // Used to determine value of the magnetic sensor.
    int openCount = 0;
    int readCount = 0;
    
    
}

void loop() {
  digitalWrite(13, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(12, HIGH);
  if(wg.available())
    {
        readCount = readCount++;
        
        Serial.print("Wiegand HEX = ");
        Serial.print(wg.getCode(),HEX);
        Serial.print(", DECIMAL = ");
        Serial.print(wg.getCode());
        Serial.print(", Type W");
        Serial.print(wg.getWiegandType());
        Serial.print(" openCount = ");
        Serial.print(openCount);
        Serial.print(
        
        Serial.println();
        digitalWrite(6, LOW);
        delay(350);
        digitalWrite(13, LOW);
        digitalWrite(12, LOW);
        int count=0;
        while (count < 50) {
          sensorValue = analogRead(A0);
          digitalWrite(13, HIGH);
          digitalWrite(12, HIGH);
          delay(100);
          digitalWrite(13, LOW);
          digitalWrite(12, LOW);
          delay(100);
          if(analogRead(A0) != sensorValue){
            break;
          }
          count++;  
        }
          
           
    } else {
      
    }
      
}
