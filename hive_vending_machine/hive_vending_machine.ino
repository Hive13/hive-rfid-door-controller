#include <Ethernet.h>
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
#include <Wiegand.h>
#include <SPI.h>
// The b64 and HttpClient libraries are both in this repository:
// https://github.com/amcewen/HttpClient
#include <b64.h>
#include <HttpClient.h>
// https://github.com/adafruit/Adafruit-WS2801-Library
#include <Adafruit_WS2801.h>


WIEGAND wg;
byte mac[] = {
  0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
char server[] = "172.16.3.233";
IPAddress ip(172,16,2,254);
EthernetClient client;

uint8_t dataPin = 39;  // green wire
uint8_t clockPin = 38; // blue wire
Adafruit_WS2801 leds = Adafruit_WS2801(20, dataPin, clockPin);

int val = 0;
int randomSoda = 1;

void setup() {
    Serial.begin(57600);
      // Let's set up the Ethernet Connections
      Serial.println("Hive13 Vending Arduino Shiled v.02");
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
      const char kPath [] = "/vendtest";
      //TODO:  Implement MOAR DOORS...

      err = http.get(kHostname, kPath);
      if (err == 0) {
        Serial.println("Vending Machine Test Connection OK");
        err = http.responseStatusCode();
        if (err >= 0) {
          Serial.print("Got Status Code: ");
          Serial.println(err);
          // This should be 200 OK.
        }
        else {
          Serial.println("Vending Machine Test FAILED to get response headers.");
        }
      }
      else {
        Serial.println("Vending Machine Test Connection FAILED.");
      }

    wg.begin();
    // wiegand/rfid reader pins
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    // soda buttons
    pinMode(22,INPUT);
    digitalWrite(22,HIGH);
    pinMode(24,INPUT);
    digitalWrite(24,HIGH);
    pinMode(26,INPUT);
    digitalWrite(26,HIGH);
    pinMode(28,INPUT);
    digitalWrite(28,HIGH);
    pinMode(30,INPUT);
    digitalWrite(30,HIGH);
    pinMode(32,INPUT);
    digitalWrite(32,HIGH);
    pinMode(34,INPUT);
    digitalWrite(34,HIGH);
    pinMode(36,INPUT);
    digitalWrite(36,HIGH);
    // soda relays
    pinMode(23,OUTPUT);
    pinMode(25,OUTPUT);
    pinMode(27,OUTPUT);
    pinMode(29,OUTPUT);
    pinMode(31,OUTPUT);
    pinMode(33,OUTPUT);
    pinMode(35,OUTPUT);
    pinMode(37,OUTPUT);
    
    leds.begin();
  }

void loop() {
    digitalWrite(7, HIGH);
    digitalWrite(8, LOW);
    digitalWrite(9, HIGH);
    if(wg.available())
    {
        Serial.print("Wiegand HEX = ");
        Serial.print(wg.getCode(),HEX);
        Serial.print(", DECIMAL = ");
        Serial.print(wg.getCode());
        Serial.print(", Type W");
        Serial.println(wg.getWiegandType());
        // This is what we do when we actually get the OK to vend...
        String bPath = "/vendcheck/";
        bPath = bPath + wg.getCode();
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
                // Door Response is OK.  vend!!
                Serial.println("Ok to vend");
                digitalWrite(7,LOW);
                digitalWrite(8, HIGH);
                digitalWrite(9, LOW);
                delay(100);
                digitalWrite(8, LOW);
                digitalWrite(9, HIGH);
                delay(900);
                digitalWrite(7,HIGH);

            } else {
                Serial.println("Didn't receive the OK to vend...");
            }
          } else {
            Serial.println("Err connecting to door controller.");
          }
        } else {
          Serial.println("Badge Connection FAILED.");
    }
    }
    val = 0;
    val = digitalRead(22);
    Serial.print("Channel Value is ");
    Serial.print(val);
    digitalWrite(37,val);
    val = 0;
    val = digitalRead(24);
    Serial.print(val);
    digitalWrite(35,val);
    val = 0;
    val = digitalRead(26);
    Serial.print(val);
    digitalWrite(33,val);
    val = 0;
    val = digitalRead(28);
    Serial.print(val);
    digitalWrite(31,val);
    val = 0;
    val = digitalRead(30);
    Serial.print(val);
    digitalWrite(29,val);
    val = 0;
    val = digitalRead(32);
    Serial.print(val);
    digitalWrite(27,val);
    val = 0;
    val = digitalRead(34);
    Serial.print(val);
    digitalWrite(25,val);
    val = 0;
    val = digitalRead(36);
    Serial.println(val);
    digitalWrite(23,val);
}
