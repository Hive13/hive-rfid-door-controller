#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "cJSON.h"

static unsigned char vacuum_state = LOW, vacuum_act = LOW;
char ssid[] = "hive13int";
char pass[] = "hive13int";
int status = WL_IDLE_STATUS;

int on_button  = D6;
int off_button = D5;
int vacuum     = D1;
ESP8266WebServer server(80);
#define TIMEOUT 5000

void turn_on(void)
	{
	vacuum_state = HIGH;
	server.send(200, "text/plain", "Commanded on.");
	}

void turn_off(void)
	{
	vacuum_state = LOW;
	server.send(200, "text/plain", "Commanded off.");
	}

void loop(void)
	{
	static unsigned long last_time = 0;
	unsigned long m = millis();

	server.handleClient();
	
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
	pinMode(on_button, INPUT_PULLUP);
	pinMode(off_button, INPUT_PULLUP);
	pinMode(vacuum, OUTPUT);
	digitalWrite(vacuum, LOW);
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

	server.on("/", handleRoot);
	server.on("/on", turn_on);
	server.on("/off", turn_off);
	//server.on ( "/test.svg", drawGraph );
	server.on ("/inline", []()
		{
		server.send(200, "text/plain", "this works as well");
		});
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println("HTTP server started");
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


void handleRoot()
	{
	struct cJSON *root = cJSON_CreateObject(), *a;
	char *c;

	a = cJSON_CreateNumber(millis());
	cJSON_AddItemToObject(root, "uptime", a);
	a = cJSON_CreateBool(vacuum_state);
	cJSON_AddItemToObject(root, "commanded_state", a);
	a = cJSON_CreateBool(vacuum_act);
	cJSON_AddItemToObject(root, "actual_state", a);

	c = cJSON_Print(root);
	server.send(200, "text/json", c);
	free(c);
	}

void handleNotFound()
	{
	unsigned char i;
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	
	for (i = 0; i < server.args(); i++)
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	
	server.send(404, "text/plain", message);
	}
