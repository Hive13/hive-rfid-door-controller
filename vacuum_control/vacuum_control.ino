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
	vacuum_cmd   = 0,
	locked_out   = 0,
	red_state    = 0,
	green_state  = 0,
	vacuum_state = 0;
	
char ssid[]     = "hive13int";
char pass[]     = "hive13int";
int  status     = WL_IDLE_STATUS;

ESP8266WebServer server(80);
char **names_commanded_on = NULL;
unsigned short nco_alloc = 0;

#define TIMEOUT 5000

char state_func(void *ptr, unsigned long *time, unsigned long m)
	{
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
		if ((vacuum_change_time + TIMEOUT) > m)
			goto schedule_and_run_it;

		vacuum_state = 1;
		red_state    = 0;
		green_state  = !green_state || vacuum_cmd;
		*time        = m + 200;
		goto run_it;
		}

	if (vacuum_cmd != vacuum_state)
		{
		vacuum_change_time = m;
		vacuum_state       = vacuum_cmd;
		}

	green_state = vacuum_state;
	red_state   = !vacuum_state;

schedule_and_run_it:
	*time = vacuum_change_time + TIMEOUT;
run_it:
	digitalWrite(GREEN_LED_PIN, green_state);
	digitalWrite(RED_LED_PIN,   red_state);
	digitalWrite(VACUUM_PIN,    vacuum_state);
	return SCHEDULE_REDO;
	}

void tao_s(void)
	{
	turn_all_off(&server);
	}

void turn_all_off(ESP8266WebServer *server)
	{
	unsigned short i;

	for (i = 0; i < nco_alloc; i++)
		if (names_commanded_on[i])
			{
			free(names_commanded_on[i]);
			names_commanded_on[i] = NULL;
			}
	if (server)
		server->send(200, "text/plain", "All commanded on byname revoked.");
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
		if (names_commanded_on[i])
			{
			if (!strcasecmp(names_commanded_on[i], c_plus_plus_sucks))
				{
				server.send(302, "text/plain", "Already commanded on by name.");
				free(c_plus_plus_sucks);
				return;
				}
			else
				break;
			}
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
	names_commanded_on[i] = c_plus_plus_sucks;
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
		if (names_commanded_on[i] && !strcasecmp(names_commanded_on[i], name.c_str()))
			{
			free(names_commanded_on[i]);
			names_commanded_on[i] = NULL;
			server.send(200, "text/plain", "Commanded off by name.");
			return;
			}
	server.send(404, "text/plain", "Can't find name to command off.");
	}

void loop(void)
	{
	unsigned long m = millis();
	static unsigned long
		on_button_time     = 0,
		off_button_time    = 0,
		on_button_handled  = 0,
		off_button_handled = 0;
	static unsigned char
		on_button_detect   = 0,
		off_button_detect  = 0;
	unsigned char b;


	run_schedule();

	server.handleClient();

	/*
		This switches around the logic states.
		This is by design, so that *_button_detect
		is 0 if the button is not pressed, and
		1 if it is.
	*/
	b = digitalRead(ON_BUTTON_PIN);
	if (b == on_button_detect)
		{
		on_button_detect  = !b;
		on_button_time    = m;
		on_button_handled = 0;
		}
	b = digitalRead(OFF_BUTTON_PIN);
	if (b == off_button_detect)
		{
		off_button_detect  = !b;
		off_button_time    = m;
		off_button_handled = 0;
		}

	if (off_button_detect && ((off_button_time + 100) < m) && (off_button_handled < 100))
		{
		off_button_handled = 100;
		vacuum_cmd = 0;
		Serial.println("Off");
		}
	if (off_button_detect && ((off_button_time + 2000) < m) && (off_button_handled < 2000))
		{
		off_button_handled = 2000;
		turn_all_off(NULL);
		Serial.println("All off");
		}
	if (off_button_detect && ((off_button_time + 5000) < m) && (off_button_handled < 5000))
		{
		off_button_handled = 5000;
		locked_out = 1;
		Serial.println("Locked out");
		}
	if (on_button_detect && ((on_button_time + 100) < m) && (on_button_handled < 100))
		{
		on_button_handled = 100;
		if (!locked_out)
			{
			vacuum_cmd = 1;
			Serial.println("On");
			}
		}
	if (on_button_detect && ((on_button_time + 5000) < m) && (on_button_handled < 5000))
		{
		on_button_handled = 5000;
		locked_out = 0;
		Serial.println("No longer locked out");
		}
	}

void setup(void)
	{
	pinMode(ON_BUTTON_PIN,  INPUT);
	pinMode(OFF_BUTTON_PIN, INPUT);
	pinMode(VACUUM_PIN,     OUTPUT);
	pinMode(RED_LED_PIN,    OUTPUT);
	pinMode(GREEN_LED_PIN,  OUTPUT);

	digitalWrite(VACUUM_PIN,    LOW);
	digitalWrite(GREEN_LED_PIN, HIGH);
	digitalWrite(RED_LED_PIN,   HIGH);

	Serial.begin(57600);
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
	server.on("/all_off", tao_s);
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
	a = cJSON_CreateBool(vacuum_cmd);
	cJSON_AddItemToObject(root, "commanded_state", a);
	a = cJSON_CreateBool(vacuum_state);
	cJSON_AddItemToObject(root, "actual_state", a);
	a = cJSON_CreateBool(locked_out);
	cJSON_AddItemToObject(root, "locked_out", a);

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
