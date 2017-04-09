#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "cJSON.h"
#include "schedule.h"

#define ON_BUTTON_PIN  D6
#define OFF_BUTTON_PIN D5
#define GREEN_LED_PIN  D3
#define RED_LED_PIN    D2
#define VACUUM_PIN     D1

static unsigned char
	vacuum_cmd :1 = 0,
	locked_out :1 = 0;

char ssid[]     = "hive13int";
char pass[]     = "hive13int";
int  status     = WL_IDLE_STATUS;

ESP8266WebServer server(80);
char **names_commanded_on = NULL;
unsigned short nco_alloc = 0;

#define TIMEOUT 5000

char state_func(void *ptr, unsigned long *time, unsigned long m)
	{
	static unsigned char
		red_state    :1 = 0,
		green_state  :1 = 0,
		vacuum_state :1 = 0;
	static unsigned long vacuum_change_time = 0;
	unsigned short i;

	if (locked_out)
		{
		green_state  = 0;
		vacuum_state = 0;
		red_state    = !red_state;
		*time        = m + 200;
		goto run_it;
		}

	for (i = 0; i < nco_alloc; i++)
		if (names_commanded_on[i])
			break;

	if (i < nco_alloc)
		{
		if ((vacuum_change_time + TIMEOUT) < m)
			goto schedule_and_run_it;

		red_state   = 0;
		green_state = !green_state; || vacuum_cmd;
		*time       = m + 200;
		goto run_it;
		}

	if (vacuum_cmd != vacuum_state)
		{
		vacuum_change_time = m;
		vacuum_state       = vacuum_cmd;
		green_state        = vacuum_state;
		red_state          = !vacuum_state;
		}

schedule_and_run_it:
	*time = vacuum_change_time + TIMEOUT;
run_it:
	digitalWrite(GREEN_LED_PIN, green_state);
	digitalWrite(RED_LED_PIN,   red_state);
	digitalWrite(VACUUM_PIN,    vacuum_state);
	return SCHEDULE_REDO;
	}

void turn_on(void)
	{
	String name;
	char *c_plus_plus_sucks;
	void *p;
	unsigned short i;

	if (!server.hasArg("name"))
		{
		server.send(200, "text/plain", "Commanded on.");
		vacuum_cmd = 1;
		return;
		}
	name = server.arg("name");
	c_plus_plus_sucks = strdup(name.c_str());
	for (i = 0; i < nco_alloc; i++)
		if (name_commanded_on[i])
			break;
	if (i == nco_alloc)
		{
		if (!nco_alloc)
			nco_alloc = 4;
		else
			nco_alloc *= 2;
		p = realloc(names_commanded_on, nco_alloc * sizeof(void *));
		if (!p)
			{
			server.send(500, "text/plain", "No memory.  Shit.");
			return;
			}
		names_commanded_on = (char **)p;
		}
	name_commanded_on[i] = c_plus_plus_sucks;
	server.send(200, "text/plain", "Commanded on by name.");
	}

void turn_off(void)
	{
	String name;
	unsigned short i;

	if (!server.hasArg("name"))
		{
		vacuum_cmd = 0;
		server.send(200, "text/plain", "Commanded off.");
		return;
		}
	name = server.arg("name");
	for (i = 0; i < nco_alloc; i++)
		if (name_commanded_on[i] && !strcasecmp(name_commanded_on[i], name.c_str()))
			{
			free(name_commanded_on[i]);
			name_commanded_on[i] = NULL;
			server.send(200, "text/plain", "Commanded off by name.");
			return;
			}
	server.send(404, "text/plain", "Can't find name to command off.");
	}

void loop(void)
	{
	unsigned long m = millis();

	run_schedule();

	server.handleClient();
	
	if (digitalRead(ON_BUTTON_PIN) == LOW)
		{
		vacuum_cmd = 1;
		Serial.println("On");
		}
	else if (digitalRead(OFF_BUTTON_PIN) == LOW)
		{
		vacuum_cmd = 0;
		Serial.println("Off");
		}
	}

void setup(void)
	{
	pinMode(ON_BUTTUN_PIN,  INPUT);
	pinMode(OFF_BUTTON_PIN, INPUT);
	pinMode(VACUUM_PIN,     OUTPUT);
	pinMode(RED_LED_PIN,    OUTPUT);
	pinMode(GREEN_LED_PIN,  OUTPUT);

	digitalWrite(VACUUM_PIN,    LOW);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(RED_LED_PIN,   HIGH);

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
	server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");
	schedule(0, state_func, NULL);
	}

void printWifiData()
	{
	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.localIP();
	
	Serial.print("IP Address: ");
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
