#ifndef __CONFIG_H
#define __CONFIG_H

#include "config_local.h"

#ifdef PLATFORM_ARDUINO

#define DOOR_PIN  6
#define LIGHT_PIN 13
#define BEEP_PIN  12
#define OPEN_PIN  A0
#define D0_PIN    2
#define D1_PIN    3

#define BUZZER_PIN   19
#define DOORBELL_PIN 18

#define TEMPERATURE_PIN 19
#define TEMPERATURE_POWER_PIN 18

#else
#ifdef PLATFORM_ESP8266

#define DOOR_PIN  D4
#define LIGHT_PIN D0
#define BEEP_PIN  D8
#define OPEN_PIN  D3
#define D0_PIN    D1
#define D1_PIN    D2

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

#define HTTP_HOST "http://intweb.at.hive13.org/api/access"
#define NETWORK_TIMEOUT 5000
#define NETWORK_DELAY   25

#endif /* __CONFIG_H */