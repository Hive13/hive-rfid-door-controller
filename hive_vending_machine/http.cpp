#include <Arduino.h>
#include <Ethernet.h>
#include <b64.h>
#include <HttpClient.h>

#include "API.h"
#include "log.h"
#include "http.h"
#include "leds.h"

#define NETWORK_DELAY   100
#define NETWORK_TIMEOUT 5000

signed int http_get_json(char *host, char *path, char *req_body, char **resp)
	{
	EthernetClient c;
	HttpClient hc(c);
	signed int err;
	unsigned long l = strlen(req_body), start, body_len;
	char *resp_body;

	leds_busy();
	log_msg("http_get_json: Connecting to %s/%s", host, path);

	hc.beginRequest();
  hc.post(host, path);
	hc.flush();
	hc.sendHeader("Content-Type", "application/json");
	hc.flush();
	hc.sendHeader("Content-Length", l);
	hc.flush();
	hc.write(req_body, l);
	hc.flush();

	err = hc.responseStatusCode();
	if (err != 200)
		return RESPONSE_BAD_HTTP;
	
	if (hc.skipResponseHeaders() > 0)
		{
		Serial.println("Header error.");
		return RESPONSE_BAD_HTTP;
		}

	body_len = hc.contentLength();
	resp_body = malloc(body_len + 1);
	
	start = millis();
	l = 0;
	while ((hc.connected() || hc.available()) && ((millis() - start) < NETWORK_TIMEOUT))
		{
		if (hc.available())
			{
			resp_body[l++] = hc.read();
			body_len--;
			start = millis();
			}
		else
			delay(NETWORK_DELAY);
		}
	resp_body[l++] = 0;
	hc.stop();

	Serial.print("L: ");
	Serial.println(l, DEC);
	Serial.println(resp_body);

	if (resp)
		*resp = resp_body;
	else
		free(resp_body);

	return RESPONSE_GOOD;
	}


signed int http_get(char *req, char *host, char *path)
	{
	EthernetClient c;
	HttpClient http(c);
	signed int err;

	leds_busy();
	log_msg("%s: Connecting to %s/%s", req, host, path);
	err = http.get(host, path);
	if (err == 0)
		{
		err = http.responseStatusCode();
		log_msg("%s: got status code %i", req, err);
		}
	else
		log_msg("%s: connection failed (%i)", req, err);
	leds_off();
	return err;	
	}

