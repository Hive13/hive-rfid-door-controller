#include "config.h"

#include <Arduino.h>
#ifdef PLATFORM_ARDUINO
#include <Ethernet.h>
#include <EthernetUdp.h>
#endif
#ifdef PLATFORM_ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#endif

#include "log.h"
#include "ui.h"
#include "schedule.h"
#include "doorbell.h"

#ifdef PLATFORM_ARDUINO
static EthernetUDP            udp;
#else
static WiFiUDP                udp;
#endif
static IPAddress              mc_ip(239, 72, 49, 51);

#ifdef HAS_DOORBELL_BUZZER
static volatile unsigned char doorbell_data = 5;

/*
	Buzz for 1000ms, sending out a packet every 250ms.
	Wait an additional 4000ms before accepting the next
	button press.
*/
static char doorbell(char *data, unsigned long *time, unsigned long now)
	{
	if (++(*data) < 4)
		{
		udp.beginPacket(mc_ip, MULTICAST_PORT);
		udp.write("doorbell");
		udp.endPacket();
		*time = now + 250;
		return SCHEDULE_REDO;
		}
	digitalWrite(BUZZER_PIN, LOW);
	*time = now + 4000;
	if (*data == 4)
		return SCHEDULE_REDO;
	return SCHEDULE_DONE;
	}

static char doorbell_holdoff(char *data, unsigned long *time, unsigned long now)
	{
	log_msg("holdoff()");
	if (doorbell_data == 5)
		ring_doorbell(1);
	return SCHEDULE_DONE;
	}

/*
	This function must work inside or outside of an ISR.
*/
void ring_doorbell(char send_packet)
	{
	digitalWrite(BUZZER_PIN, HIGH);
	if (send_packet)
		{
		doorbell_data = 0;
		schedule(0, (time_handler *)doorbell, (void *)&doorbell_data);
		}
	else
		{
		doorbell_data = 3;
		schedule(millis() + 1000, (time_handler *)doorbell, (void *)&doorbell_data);
		}
	}
#endif

#ifdef HAS_DOORBELL_BUTTON
void doorbell_isr(void)
	{
	static volatile void *doorbell_t = NULL;
	char pressed = !digitalRead(DOORBELL_BUTTON_PIN);
	unsigned long m = millis();

	if (pressed)
		doorbell_t = schedule(m + 50, (time_handler *)doorbell_holdoff, NULL);
	else if (doorbell_t)
		{
		schedule_cancel((void *)doorbell_t);
		doorbell_t = NULL;
		}
	}
#endif

char doorbell_network(char *data, unsigned long *time, unsigned long now)
	{
	static unsigned long last_beep = 0;
	char buffer[UDP_TX_PACKET_MAX_SIZE];
	int sz;

	sz = udp.parsePacket();
	
	if (sz >= 8
#ifdef PLATFORM_ESP8266
		&& udp.destinationIP() == mc_ip
#endif
		)
		{
		udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
		
		if (sz == 8 && !memcmp(buffer, "doorbell", 8))
			{
			if ((last_beep + 5000) <= now)
				beep_it(BEEP_PATTERN_DOORBELL);
			last_beep = now;
			}
		}	
	udp.flush();
	return SCHEDULE_REDO;
	}

void doorbell_init(void)
	{
#ifdef HAS_DOORBELL_BUTTON
	digitalWrite(BUZZER_PIN,     LOW);
	pinMode(DOORBELL_BUTTON_PIN, INPUT_PULLUP);
	pinMode(BUZZER_PIN,          OUTPUT);
	attachInterrupt(digitalPinToInterrupt(DOORBELL_BUTTON_PIN), doorbell_isr, CHANGE);
#endif

#ifdef PLATFORM_ARDUINO
	udp.beginMulticast(mc_ip, MULTICAST_PORT);
#else
	//udp.beginMulticast(WiFi.localIP(), mc_ip, MULTICAST_PORT);
#endif
	//schedule(0, (time_handler *)doorbell_network, &udp);
	}
