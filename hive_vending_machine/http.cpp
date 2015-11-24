#include <Arduino.h>
#include <Ethernet.h>
#include <HttpClient.h>

#include "log.h"

extern "C" signed int http_get(char *req, char *host, char *path)
	{
	EthernetClient c;
	HttpClient http(c);
	signed int err;

	log_msg("%s: Connecting to %s/%s\n", req, host, path);
	err = http.get(host, path);
	if (err == 0)
		{
		err = http.responseStatusCode();
		log_msg("%s: got status code %i\n", req, err);
		}
	else
		log_msg("%s: connection failed (%i)\n", req, err);
	return err;	
	}

