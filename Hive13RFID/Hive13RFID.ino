#include <b64.h>
#include <HttpClient.h>

#include <SHA512.h>

#include <Wiegand.h>
#include <SPI.h>
#include <Ethernet.h>

#define SHA512_SZ 64
#define BODY_SZ 1024

char *location   = "main_door";
char key[]       = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};
static char *hex = "0123456789ABCDEF";

// I'm using the Wiegand library to make reading the files easier. 
// https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino.git
// Pins:  6  = Relay for the door
//        13 = Red/Green light on RFID Reader.  High == Red, Low == Green.
//        12 = Buzzer attached to RFID Reader.  High == Off, Low == On.
//        A0 = Magnetic Switch attached to top of door.  

char check_badge(unsigned long badge)
	{
	static unsigned long scan_count = 0, l;
	char sha_buf[SHA512_SZ], body[BODY_SZ], out[BODY_SZ];
	unsigned char i;
	EthernetClient ec;
	HttpClient hc(ec);
	SHA512 sha;
	int err;

	l = snprintf(body, BODY_SZ, "{\"badge\":%lu,\"http\":true,\"item\":\"%s\",\"random\":[%lu,%lu],\"version\":1}",
		badge, location, millis(), scan_count++);
	
	sha.reset();
	sha.update(key, sizeof(key));
	sha.update(body, l);
	sha.finalize(sha_buf, sha.hashSize());

	l = snprintf(out, BODY_SZ, "{\"data\":%s,\"device\":\"%s\",\"checksum\":\"", body, location);
	
	for (i = 0; i < SHA512_SZ; i++)
		{
		out[l++] = hex[((sha_buf[i] & 0xF0) >> 4)];
		out[l++] = hex[(sha_buf[i] & 0x0F)];
		}

	out[l++] = '\"';
	out[l++] = '}';

	hc.beginRequest();
  hc.post("intweb.at.hive13.org", "/api/access");
	hc.flush();
	hc.sendHeader("Content-Type", "application/json");
	hc.flush();
	hc.sendHeader("Content-Length", l);
	hc.flush();
	hc.write(out, l);
	hc.flush();
	err = hc.responseStatusCode();
	return err == 200;
	}

void open_door(void)
	{
	unsigned char count = 0;
	int sensorValue;

	// switch the relay to open the door.
	digitalWrite(6, LOW);
	delay(350);
	// beep buzzer for up to 50 cycles or until the magnetic switch/door has been opened.
	digitalWrite(13, LOW);
	digitalWrite(12, LOW);
	while (count < 50)
		{
		sensorValue = analogRead(A0);
		digitalWrite(13, HIGH);
		digitalWrite(12, HIGH);
		delay(100);
		digitalWrite(13, LOW);
		digitalWrite(12, LOW);
		delay(100);
		if(sensorValue < 670)
			break;
		count++;
		}

	// turn off buzzer now otherwise it will take a second or two for the loop to complete and turn it off.
	digitalWrite(13, HIGH);
	digitalWrite(12, HIGH);
	}


WIEGAND wg;
byte mac[] = { 0x48, 0x49, 0x56, 0x45, 0x31, 0x33};
IPAddress ip(172,16,3,230);
EthernetClient client;

// Cache of badge numbers that have succesfully opened the door.
unsigned long usersCache[100];

void setup()
	{
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(6, OUTPUT);    

	digitalWrite(13, HIGH);
	digitalWrite(12, HIGH);
	digitalWrite(6,  HIGH);

	Serial.begin(57600);

  Serial.println("Initializing Ethernet Controller.");
	while (Ethernet.begin(mac) != 1)
		{
		Serial.println("Error obtaining DHCP address.  Let's wait a second and try again");
		delay(1000);
		}

  // Initialize Wiegand Interface
  wg.begin();
  
  // initialize the usersCache array as all 0.
	memset(usersCache, 0, sizeof(usersCache));
	
	/* Beep twice for start of day */
	digitalWrite(13, LOW);
	digitalWrite(12, LOW);
	delay(100);
	digitalWrite(13, HIGH);
	digitalWrite(12, HIGH);
	delay(50);
	digitalWrite(13, LOW);
	digitalWrite(12, LOW);
	delay(100);
	digitalWrite(13, HIGH);
	digitalWrite(12, HIGH);
	}

void loop()
	{
	char badge_ok;
	unsigned long badgeID = 0;
	unsigned long time = 0;
	boolean cached = false;
	unsigned char i;
  
  digitalWrite(6, HIGH);
  // Turn off buzzer in case it has been left on
  digitalWrite(13, HIGH);
  digitalWrite(12, HIGH);

	if (!wg.available())
		return;

	badgeID = wg.getCode();

	// Sending Debug info to USB...
	Serial.print("Wiegand HEX = ");
	Serial.print(badgeID, HEX);
	Serial.print(", DECIMAL = ");
	Serial.print(badgeID);
	Serial.print(", Type W");
	Serial.println(wg.getWiegandType());

	// Check to see if the user has been cached
	for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
		{
		if(usersCache[i] == badgeID && usersCache[i] > 0)
			{
			// If it has been cached open the door.
			Serial.println("Verified via cache ok to open door...");
			cached = true;
			open_door();
			}
		}

	badge_ok = check_badge(badgeID);
	Serial.print("Badge OK: ");
	Serial.println(badge_ok, DEC);

	if (badge_ok)
		{
		// We only need to open the door here if it wasn't opened from checking the cache.
		if (!cached)
			{
			open_door();
			// user is allowed in but hasn't been cached, add them to the cache.
			Serial.println("Adding user to cache.");
			for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
				{
				// loop until we find a spot that is still zero and stick the badge id there.
				if (usersCache[i] == 0)
					{
					usersCache[i] = badgeID;
					break;
					}
				}
			}
		}
	else if (cached == true)
		{
		// remove them from the cached list as they are not allowed in
		for (i = 0; i < (sizeof(usersCache) / sizeof(usersCache[0])); i++)
			if (usersCache[i] == badgeID)
				usersCache[i] = 0;
		}
	}
