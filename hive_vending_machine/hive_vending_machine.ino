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
    val = 0;val = digitalRead(30);
    Serial.print(val);
    if(val == 0) {
      uint32_t randomLedsColor = Wheel(random(0, 255));
      Serial.print("Wooo colors!");
      randomColors(20, 5);
      Serial.print("Colors done, turning off!");
      turnOffLeds();
      Serial.print("Vending random soda!");
      switch(random(1, 9)) {
        case 1:
          leds.setPixelColor(18, randomLedsColor);
          leds.setPixelColor(19, randomLedsColor);
          leds.show();
          digitalWrite(37,0);
          Serial.print("Random soda is 1!\n");
          delay(1000);
          break;
        case 2:
          leds.setPixelColor(17, randomLedsColor);
          leds.setPixelColor(16, randomLedsColor);
          leds.show();
          digitalWrite(35,0);
          Serial.print("Random soda is 2!\n");
          delay(1000);
          break;
        case 3:
          leds.setPixelColor(15, randomLedsColor);
          leds.setPixelColor(14, randomLedsColor);
          leds.show();
          digitalWrite(33,0);
          Serial.print("Random soda is 3!\n");
          delay(1000);
          break;
        case 4:
          leds.setPixelColor(13, randomLedsColor);
          leds.setPixelColor(12, randomLedsColor);
          leds.show();
          digitalWrite(31,0);
          Serial.print("Random soda is 4!\n");
          delay(1000);
          break;
        case 5:
          leds.setPixelColor(11, randomLedsColor);
          leds.setPixelColor(10, randomLedsColor);
          leds.show();
          digitalWrite(29,0);
          Serial.print("Random soda is 5!\n");
          delay(1000);
          break;
        case 6:
          leds.setPixelColor(9, randomLedsColor);
          leds.setPixelColor(8, randomLedsColor);
          leds.show();
          digitalWrite(27,0);
          Serial.print("Random soda is 6!\n");
          delay(1000);
          break;
        case 7:
          leds.setPixelColor(7, randomLedsColor);
          leds.setPixelColor(6, randomLedsColor);
          leds.show();
          digitalWrite(25,0);
          Serial.print("Random soda is 7\n");
          delay(1000);
          break;
        case 8:
          digitalWrite(23,0);
          leds.setPixelColor(4, randomLedsColor);
          leds.setPixelColor(5, randomLedsColor);
          leds.show();
          Serial.print("Random soda is 8\n");
          delay(1000);
          break;
      }
      turnOffLeds();
    }
    // if we don't reset pin 29 (relay for button 5) it will get stuck on.
    digitalWrite(29,1);
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


void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j+=5) { // 5 cycles of all 25 colors in the wheel
    for (i=0; i < leds.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      leds.setPixelColor(i, Wheel( ((i * 256 / leds.numPixels()) + j) % 256) );
    }
    leds.show(); // write all the pixels out
    delay(wait);
    Serial.println("Rainbow!");
  }
}

void randomColors(uint8_t wait, uint8_t numberCycles) {
  int i;
  int randomLeds;
  uint32_t randomLedsColor;
  for(i=0; i < numberCycles*leds.numPixels(); i++) {
    randomLeds = random(0, 8);
    randomLedsColor = Wheel(random(0, 255));
    // Set groups of two to the same color. The +4 is to make the 16 out of 20 that turn on the end ones.
    leds.setPixelColor(randomLeds*2+4, randomLedsColor);
    leds.setPixelColor(randomLeds*2+1+4, randomLedsColor);
    leds.show();
    delay(wait);
  }
}

void turnOffLeds() {
  int i;
  for(i=0; i < leds.numPixels(); i++) {
      leds.setPixelColor(i, 0, 0, 0);
  }
  leds.show();
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
