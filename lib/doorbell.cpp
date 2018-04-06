#include <Ethernet.h>
#include <EthernetUdp.h>

#include "config.h"
#include "log.h"
#include "ui.h"
#include "schedule.h"
#include "doorbell.h"

static EthernetUDP            udp;
static IPAddress              mc_ip(239, 72, 49, 51);
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
		schedule(0, doorbell, &doorbell_data);
		}
	else
		{
		doorbell_data = 3;
		schedule(millis() + 1000, doorbell, &doorbell_data);
		}
	}

void doorbell_isr(void)
	{
	static volatile void *doorbell_t = NULL;
	char pressed = !digitalRead(DOORBELL_PIN);
	unsigned long m = millis();

	if (pressed)
		doorbell_t = schedule(m + 50, doorbell_holdoff, NULL);
	else if (doorbell_t)
		{
		schedule_cancel(doorbell_t);
		doorbell_t = NULL;
		}
	}

char doorbell_network(char *data, unsigned long *time, unsigned long now)
	{
	char buffer[UDP_TX_PACKET_MAX_SIZE];
	int sz;

	sz = udp.parsePacket();
	if (sz)
		{
		log_msg("Packet!");
		udp.read(buffer, UDP_TX_PACKET_MAX_SIZE);
		}
	}

void doorbell_init(void)
	{
	digitalWrite(BUZZER_PIN, LOW);
	pinMode(DOORBELL_PIN,    INPUT_PULLUP);
	pinMode(BUZZER_PIN,      OUTPUT);
	attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), doorbell_isr, CHANGE);

	udp.beginMulticast(mc_ip, MULTICAST_PORT);
	}
