#include <b64.h>
#include <HttpClient.h>

#include <Wiegand.h>
#include <SPI.h>
#include <Ethernet.h>


// I'm using the Wiegand library to make reading the files easier. 
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino.git
// Pins:  6  = Relay for the door
//        13 = Red/Green light on RFID Reader.  High == Red, Low == Green.
//        12 = Buzzer attached to RFID Reader.  High == Off, Low == On.
//        A0 = Magnetic Switch attached to top of door.  
// Variables:
//        count = Integer, used in Loop for Door Open cycle
//        sensorValue = Integer, used to determine values of magnetic sensors
//        openCount = Integer, used for OTP-ish kinda shit (preventing replay attacks from the door)
//        readCount = Integer, used for OTP-ish kinda stuff  (preventing replay attacks from the door).
// 
// I'm attempting to adapt the WebClient Example code for this, but, it's not been...well, really fruitful in any way, shape, or form.



WIEGAND wg;
// MAC Address for Controller Below
byte mac[] = {
  0x48, 0x49, 0x56, 0x45, 0x31, 0x33};
char server[] = "172.16.3.233";
IPAddress ip(172,16,3,230);
EthernetClient client;

void setup() {
  Serial.begin(57600);
  // Let's set up the Ethernet Connections
  Serial.println("Initializing Ethernet Controller.");
  while (Ethernet.begin(mac) != 1) {
    Serial.println("Error obtaining DHCP address.  Let's wait a second and try again");
    delay(1000);
  }
  Serial.println("Attempting to make static connection to Door to ensure connectivity.");

  EthernetClient c;
  HttpClient http(c);
  int err = 0;
  const char kHostname[] = "door.at.hive13.org";
  const char kPath [] = "/doortest";
  //TODO:  Implement MOAR DOORS...

  err = http.get(kHostname, kPath);
  if (err == 0) {
    Serial.println("Door Test Connection OK");
    err = http.responseStatusCode();
    if (err >= 0) {
      Serial.print("Got Status Code: ");
      Serial.println(err);
      // This should be 200 OK.
    } 
    else {
      Serial.println("Door Test FAILED to get response headers.");
    }
  } 
  else {
    Serial.println("Door Test Connection FAILED.");
  }

  // Setting these up now, otherwise...well, yeah.  Not fun.    
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(6, OUTPUT);    
  // Initialize Wiegand Interface
  wg.begin();
}

void loop() {
  int count = 0;          // Used in loop for door open cycle
  int sensorValue = 0;    // Used to determine value of the magnetic sensor.
  int openCount = 0;      // Used to send to the controller the number of times the door's been opened.
  int readCount = 0;      // Used to send to the controller the number of times we've read a valid card.
  digitalWrite(13, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(12, HIGH);
  if(wg.available())
  {
    readCount = readCount++;
    // Sending Debug info to USB...
    Serial.print("Wiegand HEX = ");
    Serial.print(wg.getCode(),HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(wg.getCode());
    Serial.print(", Type W");
    Serial.print(wg.getWiegandType());
    Serial.print(" openCount = ");
    Serial.print(openCount);
    Serial.print(" readCount = ");
    Serial.print(readCount);
    Serial.println();
    // if valid, open...

    String bPath = "/doorcheck";
    bPath = bPath + wg.getCode(),HEX;
    bPath = bPath + "/go";
    EthernetClient c;
    HttpClient http(c);
    int err = 0;
    const char kHostname[] = "door.at.hive13.org";
    err = http.get(kHostname, bPath.c_str());
    if (err == 0) {
      Serial.println("RFID Badge Connection OK");
      err = http.responseStatusCode();
      if (err >= 0) {
        Serial.print("Badge receievd status code: ");
        Serial.println(err);
        if (err == 200) {
           // Door Response is OK.  Open the door... 
           Serial.println("Ok to open door...");
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
           }
      } 
      else {
        Serial.println("Badge FAILED to get response headers.");
      }
    } 
    else {
      Serial.println("Badge Connection FAILED.");
    }
  } 
  else {
  // 
  }

}


