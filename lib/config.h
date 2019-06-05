#ifndef __CONFIG_H
#define __CONFIG_H

#include "config_local.h"

#ifdef PLATFORM_ARDUINO

#define LED_PIN   6

#define OPEN_PIN  A0
#define WIEGAND_D0_PIN    2
#define WIEGAND_D1_PIN    3

#define BUZZER_PIN   19
#define DOORBELL_BUTTON_PIN 18

#define ONEWIRE_PIN 19

#else
#ifdef PLATFORM_ESP8266

#define OPEN_PIN  D3
#define WIEGAND_D0_PIN    D2
#define WIEGAND_D1_PIN    D1

#define ONEWIRE_PIN D6

#define LIGHT_INV
#define BEEP_INV

#else
#error No platform defined.
#endif
#endif

/* Number of ms */
#define DOOR_OPEN_TIME 5000

#define HTTP_HOST       "http://intweb.at.hive13.org/api/access"
#define NETWORK_TIMEOUT 5000
#define NETWORK_DELAY   25
#define MULTICAST_PORT  12595

#endif /* __CONFIG_H */
