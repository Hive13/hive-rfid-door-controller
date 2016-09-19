#include <ESP8266WiFi.h>

char ssid[] = "hive13int";
char pass[] = "hive13int";
int status = WL_IDLE_STATUS;

int latch_pin  = D0;
int clock_pin  = D8;

#define LED_COLS     7
#define LEDS_PER_COL 8

struct led
	{
	unsigned short r, g, b;
	};

struct led_col
	{
	int pin;
	struct led leds[LEDS_PER_COL];
	} cols[LED_COLS];

void leds_init(void)
	{
	unsigned char i;
	int pins[] = {D1, D2, D3, D4, D5, D6, D7};

	pinMode(latch_pin, OUTPUT);
	pinMode(clock_pin, OUTPUT);
	
	digitalWrite(latch_pin, LOW);
	digitalWrite(clock_pin, LOW);

	for (i = 0; i < LED_COLS; i++)
		{
		cols[i].pin = pins[i];
		pinMode(cols[i].pin, OUTPUT);
		memset(cols[i].leds, 0, LEDS_PER_COL * sizeof(struct led));
		}
	}

void leds_set_current(unsigned char current)
	{
	unsigned long packet, mask;
	unsigned short c = current & 0x7f;
	unsigned char i, j;

	digitalWrite(clock_pin, LOW);

	packet =
		(((0x01) & 0x03) << 30) |
		(c << 20) |
		(c << 10) |
		(c << 0);
	for (i = 0; i < LEDS_PER_COL; i++)
		for (mask = 0x80000000; mask; mask >>= 1)
			{
			for (j = 0; j < LED_COLS; j++)
				digitalWrite(cols[j].pin, !!(packet & mask));
			digitalWrite(clock_pin, HIGH);
			delay(2);
			digitalWrite(clock_pin, LOW);
			}

	delay(2);
	digitalWrite(latch_pin,HIGH); // latch data into registers
	delay(2);
	digitalWrite(latch_pin,LOW);
	}

void leds_update(void)
	{
	unsigned long packet[LED_COLS], mask;
	unsigned char i, j;

	digitalWrite(clock_pin, LOW);

	for (i = 0; i < LEDS_PER_COL; i++)
		{
		for (j = 0; j < LED_COLS; j++)
			{
			packet[j] =
				(((0x00) & 0x03) << 30) |
				((cols[j].leds[i].b & 0x3FF) << 20) |
				((cols[j].leds[i].r & 0x3FF) << 10) |
				((cols[j].leds[i].g & 0x3FF) << 0);
			}
		for (mask = 0x80000000; mask; mask >>= 1)
			{
			for (j = 0; j < LED_COLS; j++)
				digitalWrite(cols[j].pin, !!(packet[j] & mask));
			digitalWrite(clock_pin, HIGH);
			delay(2);
			digitalWrite(clock_pin, LOW);
			}
		}

	delay(2);
	digitalWrite(latch_pin,HIGH); // latch data into registers
	delay(2);
	digitalWrite(latch_pin,LOW);
	}

void loop(void)
	{
	static unsigned char l = 0;
	unsigned char i, j;

	for (i = 0; i < LED_COLS; i++)
		for (j = 0; j < LEDS_PER_COL; j++)
			{
			memset(&(cols[i].leds[j]), 0, sizeof(struct led));
			switch ((l + i) % 3)
				{
				case 0:
					cols[i].leds[j].r = 0x3FF;
					break;
				case 1:
					cols[i].leds[j].g = 0x3FF;
					break;
				case 2:
					cols[i].leds[j].b = 0x3FF;
					break;
				}
			}

	l = (l + 1) % 3;
	leds_update();
	delay(500);
	}

void setup(void)
	{
	Serial.begin(115200);
	delay(1);

	Serial.print("Attempting to connect to WPA SSID: ");
	Serial.println(ssid);

	status = WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED)
		{
		Serial.print(".");
		delay(1000);
		}
	Serial.print("You're connected to the network");
	printCurrentNet();
	printWifiData();

	leds_init();
	leds_set_current(0x7F);
	}

void printWifiData()
	{
	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.localIP();
	
	Serial.print("IP Address: ");
	Serial.println(ip);
	Serial.println(ip);
	
	// print your MAC address:
	byte mac[6];
	WiFi.macAddress(mac);
	Serial.print("MAC address: ");
	Serial.print(mac[5],HEX);
	Serial.print(":");
	Serial.print(mac[4],HEX);
	Serial.print(":");
	Serial.print(mac[3],HEX);
	Serial.print(":");
	Serial.print(mac[2],HEX);
	Serial.print(":");
	Serial.print(mac[1],HEX);
	Serial.print(":");
	Serial.println(mac[0],HEX);
	}

void printCurrentNet()
	{
	// print the SSID of the network you're attached to:
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());
	
	// print the received signal strength:
	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.println(rssi);
	}
