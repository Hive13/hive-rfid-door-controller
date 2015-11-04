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
// https://github.com/PaulStoffregen/OneWire
#include <OneWire.h>

#define COMPRESSOR_RELAY (-1)
#define COMPRESSOR_ON 38.0
#define COMPRESSOR_OFF 34.0
#define COMPRESSOR_ON_DELAY_MILLIS 30000

#define TEMPERATURE_PIN (-1)
#define TEMPERATURE_UPDATE_INTERVAL 10000
#define TEMPERATURE_READ_TIME 1000

#define RANDOM_SODA_BUTTON 30

WIEGAND wg;
byte mac[] = {0x90, 0xa2, 0xda, 0x0d, 0x7c, 0x9a};
const char kHostname[] = "door.at.hive13.org";

uint8_t dataPin = 39;  // green wire
uint8_t clockPin = 38; // blue wire
Adafruit_WS2801 leds = Adafruit_WS2801(20, dataPin, clockPin);
OneWire ds(TEMPERATURE_PIN);
byte addr[8];


// All eight soda buttons where 0 is the top button and 7 is the bottom button.
// In a format of switch pin number, relay pin number, and then the two led numbers
int sodaButtons[8][4] = {
	{22, 37, 18, 19},
	{24, 35, 16, 17},
	{26, 33, 14, 15},
	{28, 31, 12, 13},
	{30, 29, 10, 11},
	{32, 27, 8, 9},
	{34, 25, 6, 7},
	{36, 23, 4, 5},
};

#define SODA_COUNT (sizeof(sodaButtons) / sizeof(sodaButtons[0]))

void setup() {
	EthernetClient c;
	HttpClient http(c);
	int err = 0, i;
	const char kPath[] = "/vendtest";

	Serial.begin(57600);
	// Let's set up the Ethernet Connections
	Serial.println("Hive13 Vending Arduino Shield v.03");
	Serial.print(SODA_COUNT);
	Serial.println(" sodas configured.");
	Serial.println("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1) {
		Serial.println("Error obtaining DHCP address.  Let's wait a second and try again");
		delay(1000);
	}
	
	Serial.println("Attempting to make static connection to Door to ensure connectivity.");
	//TODO:  Implement MOAR DOORS...
	
	err = http.get(kHostname, kPath);
	if (err == 0) {
		Serial.println("Vending Machine Test Connection OK");
		err = http.responseStatusCode();
		if (err >= 0) {
			Serial.print("Got Status Code: ");
			Serial.println(err);
			// This should be 200 OK.
		} else {
			Serial.println("Vending Machine Test FAILED to get response headers.");
		}
	} else {
		Serial.println("Vending Machine Test Connection FAILED.");
	}
	
	wg.begin();
	// wiegand/rfid reader pins
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(COMPRESSOR_RELAY, OUTPUT);
	digitalWrite(COMPRESSOR_RELAY, LOW);
	
	/*
		Set soda button switch pins to input and pull them high
		Set soda relay pins to output
	*/
	for(i = 0; i < SODA_COUNT; i++) {
		pinMode(sodaButtons[i][0], INPUT);
		digitalWrite(sodaButtons[i][0], HIGH);
		pinMode(sodaButtons[i][1], OUTPUT);
	}

	leds.begin();
}

void do_random_vend(void) {
	uint32_t randomSodaColor;
	int randomSoda;

	// Pick the color that the chosen soda will be
	randomSodaColor = Wheel(random(0, 255));
	Serial.print("Wooo colors!");
	// Display the light show
	randomColors(20, 5);
	Serial.print("Colors done, turning off!");
	turnOffLeds();
	Serial.print("Vending random soda!");
	// Choose the random soda to vend
	randomSoda = random(0, SODA_COUNT);
	leds.setPixelColor(sodaButtons[randomSoda][2], randomSodaColor);
	leds.setPixelColor(sodaButtons[randomSoda][3], randomSodaColor);
	leds.show();
	digitalWrite(sodaButtons[randomSoda][1], 0);
	Serial.print("Random soda is ");
	Serial.print(randomSoda);
	Serial.print("!\n");
	// Let the chosen soda stay lit for one second
	delay(1000);
	// Turn off the LED
	turnOffLeds();
}

void do_vend(void) {
	Serial.println("Vending.");
	digitalWrite(7, LOW);
	digitalWrite(8, HIGH);
	digitalWrite(9, LOW);
	delay(100);
	digitalWrite(8, LOW);
	digitalWrite(9, HIGH);
	delay(900);
	digitalWrite(7, HIGH);
}

char start_read_temperature(void) {
	if (!ds.search(addr)) {
		//no more sensors on chain, reset search
		ds.reset_search();
		return -1;
	}
	
	if (OneWire::crc8(addr, 7) != addr[7]) {
		Serial.println("CRC is not valid!");
		return -1;
	}
	
	if (addr[0] != 0x10 && addr[0] != 0x28) {
		Serial.print("Device is not recognized");
		return -1;
	}
	
	ds.reset();
	ds.select(addr);
	ds.write(0x44);
	return 0;
}

float get_temperature(void) {
	//returns the temperature from one DS18S20 in Fahrenheit
	byte data[12], present;
	float tempRead;
	
	present = ds.reset();
	ds.select(addr);  
	ds.write(0xBE); // Read Scratchpad
	
	for (int i = 0; i < 9; i++) { // we need 9 bytes
		data[i] = ds.read();
	}
	
	ds.reset_search();
	
	tempRead = ((data[1] << 8) | data[0]); //using two's compliment
	tempRead /= 16;
	return (tempRead * 1.8 + 32); /* De-suckigrade the temp */
}

void handle_temperature() {
	float temp = get_temperature();
	static unsigned long start_at = 0, update_temperature_at = 0;
	char webstr[255];
	unsigned long m = millis();
	
	if (temp <= COMPRESSOR_OFF) {
		digitalWrite(COMPRESSOR_RELAY, LOW);
		start_at = m + COMPRESSOR_ON_DELAY_MILLIS;
	} else if (temp >= COMPRESSOR_ON && start_at <= m) {
		digitalWrite(COMPRESSOR_RELAY, HIGH);
	}

	if (update_temperature_at <= m) {
		update_temperature_at = m + TEMPERATURE_UPDATE_INTERVAL;
		snprintf(webstr, sizeof(webstr), "/tempset/%.1f", temp);
	}
}

void loop() {
	char host_path[255], ch;
	unsigned int code, m = millis();
	int err, i, buttonValue;
	static unsigned int temp_ready_time = 0;
	EthernetClient c;
	HttpClient http(c);
	
	digitalWrite(7, HIGH);
	digitalWrite(8, LOW);
	digitalWrite(9, HIGH);
	
	if (!temp_ready_time) {
		ch = start_read_temperature();
		if (!ch)
			temp_ready_time = m + TEMPERATURE_READ_TIME;
	} else if (temp_ready_time <= m)
		handle_temperature();
	
	if(wg.available()) {
		code = wg.getCode();
		Serial.print("Wiegand HEX = ");
		Serial.print(code, HEX);
		Serial.print(", DECIMAL = ");
		Serial.print(code);
		Serial.print(", Type W");
		Serial.println(wg.getWiegandType());
		// This is what we do when we actually get the OK to vend...
		snprintf(host_path, sizeof(host_path), "/vendcheck/%d/go", code);
		err = http.get(kHostname, host_path);
		if (err == 0) {
			Serial.println("RFID Badge Connection OK");
			err = http.responseStatusCode();
			if (err >= 0) {
				Serial.print("Badge receievd status code: ");
				Serial.println(err);
				if (err == 200) {
					do_vend();
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
	// Cycle through all eight buttons, check their values, and do the appropriate event
	for(i = 0; i < SODA_COUNT; i++) {
		// For Ryan: If buttons 1 and 3 are pressed at the same time then vend the fifth soda.
		// This is the soda that normally would have vended with the button that random currently takes.
		if(!digitalRead(sodaButtons[0][0]) && !digitalRead(sodaButtons[2][0])) {
			// turn off relays for soda 0 and 2 if they weren't pressed exactly together
			digitalWrite(sodaButtons[0][1], 1);
			digitalWrite(sodaButtons[2][1], 1);
			// turn on the relay for soda 4
			digitalWrite(sodaButtons[4][1], 0);
			break;
		}

		buttonValue = digitalRead(sodaButtons[i][0]);
		//Serial.print("Button value is: ");
		//Serial.print(buttonValue);
		if(sodaButtons[i][0] == RANDOM_SODA_BUTTON && buttonValue == 0) {
			do_random_vend();
		} else {
			digitalWrite(sodaButtons[i][1], buttonValue);
		}
	}
	// Turn off all LEDs after every cycle just as general house keeping.
	turnOffLeds();
}

/* LED Helper functions */
// Most of the LED helper functions taken from adafruit's example code.
void rainbowCycle(uint8_t wait) {
	int i, j;
	
	for (j = 0; j < 256 * 5; j += 5) { // 5 cycles of all 25 colors in the wheel
		for (i = 0; i < leds.numPixels(); i++) {
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
  for(i = 0; i < numberCycles * leds.numPixels(); i++) {
    randomLeds = random(0, 8);
    randomLedsColor = Wheel(random(0, 255));
    // Set groups of two to the same color. The +4 is to make the 16 out of 20 that turn on the end ones.
    leds.setPixelColor(randomLeds * 2 + 4, randomLedsColor);
    leds.setPixelColor(randomLeds * 2 + 1 + 4, randomLedsColor);
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

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b) {
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
uint32_t Wheel(byte WheelPos) {
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

/* vim:set filetype=c: */
