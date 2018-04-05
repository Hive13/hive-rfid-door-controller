#ifndef __CONFIG_H
#define __CONFIG_H

#include "config_local.h"

#ifdef PLATFORM_ARDUINO

#define DOOR_PIN  6
#define LIGHT_PIN 13
#ifndef BEEP_PIN
#define BEEP_PIN  12
#endif
#define OPEN_PIN  A0
#define WIEGAND_D0_PIN    2
#define WIEGAND_D1_PIN    3

#define BUZZER_PIN   19
#define DOORBELL_PIN 18

#define TEMPERATURE_PIN 19
#define TEMPERATURE_POWER_PIN 18

#else
#ifdef PLATFORM_ESP8266

#define DOOR_PIN  D4
#define LIGHT_PIN D0
#ifndef BEEP_PIN
#define BEEP_PIN  D8
#endif
#define OPEN_PIN  D3
#define WIEGAND_D0_PIN    D1
#define WIEGAND_D1_PIN    D2

#define TEMPERATURE_PIN D5
#define TEMPERATURE_POKE_PIN D6

#define LIGHT_INV
#define BEEP_INV

#else
#error No platform defined.
#endif
#endif

/* Number of ms */
#define DOOR_OPEN_TIME 5000
#define TEMPERATURE_UPDATE_INTERVAL 60000

#define HTTP_HOST       "http://intweb.at.hive13.org/api/access"
#define NETWORK_TIMEOUT 5000
#define NETWORK_DELAY   25
#define MULTICAST_PORT  12595

#endif /* __CONFIG_H */