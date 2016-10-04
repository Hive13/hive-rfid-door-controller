#include <ESP8266WiFi.h>

char ssid[] = "hive13int";
char pass[] = "hive13int";
int status = WL_IDLE_STATUS;

int on_button  = D6;
int off_button = D5;
int vacuum     = D1;
#define TIMEOUT 5000

void loop(void)
	{
	static unsigned char vacuum_state = LOW, vacuum_act = LOW;
	static unsigned long last_time = 0;
	unsigned long m = millis();
	if (digitalRead(on_button) == LOW)
		{
		vacuum_state = HIGH;
		Serial.println("On");
		}
	else if (digitalRead(off_button) == LOW)
		{
		vacuum_state = LOW;
		Serial.println("Off");
		}
	if ((last_time + TIMEOUT <= m) && (vacuum_state != vacuum_act))
		{
		last_time = m;
		digitalWrite(vacuum, vacuum_state);
		vacuum_act = vacuum_state;
		}
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
	pinMode(on_button, INPUT_PULLUP);
	pinMode(off_button, INPUT_PULLUP);
	pinMode(vacuum, OUTPUT);
	digitalWrite(vacuum, LOW);

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
