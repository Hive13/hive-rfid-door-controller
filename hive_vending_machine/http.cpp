#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>

extern "C" signed int http_get(char *req, char *host, char *path)
	{
	EthernetClient c;
	HttpClient http(c);
	signed int err;
	char str[256];

	snprintf(str, sizeof(str), "%s: Connecting to %s/%s\n", req, host, path);
	Serial.print(str);
	err = http.get(host, path);
	if (err == 0)
		{
		err = http.responseStatusCode();
		snprintf(str, sizeof(str), "%s: got status code %i\n", req, err);
		}
	else
		snprintf(str, sizeof(str), "%s: connection failed (%i)\n", req, err);
	
	Serial.print(str);
	return err;	
	}

