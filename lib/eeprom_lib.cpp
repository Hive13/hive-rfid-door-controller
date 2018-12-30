#include "config.h"

#include <EEPROM.h>

#include "eeprom_lib.h"
#include "log.h"

static const unsigned long crc_table[16] =
	{
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
	};

struct eeprom_actual
	{
	struct eeprom_data d;
	unsigned char padding[508 - sizeof(struct eeprom_data)];
	unsigned long crc16;
	} eeprom_d;

struct eeprom_data *config = &(eeprom_d.d);

void eeprom_save(void)
	{
	unsigned short i;
	unsigned long crc = ~0L;
	unsigned char *data = (unsigned char *)&eeprom_d;

	memset(eeprom_d.padding, 0, sizeof(eeprom_d.padding));
	for (i = 0 ; i < sizeof(eeprom_d) - sizeof(unsigned long); i++)
		{
		EEPROM.write(i, data[i]);
		crc = crc_table[(crc ^ data[i]) & 0x0f] ^ (crc >> 4);
		crc = crc_table[(crc ^ (data[i] >> 4)) & 0x0f] ^ (crc >> 4);
		crc = ~crc;
		}
	data = (unsigned char *)&crc;
	for (i = 0; i < sizeof(unsigned long); i++)
		EEPROM.write(i + sizeof(eeprom_d) - sizeof(unsigned long), data[i]);
#ifdef PLATFORM_ESP8266
	EEPROM.commit();
#endif
	}

void eeprom_init(void)
	{
	unsigned short i;
	unsigned long crc = ~0L;
	unsigned char *data = (unsigned char *)&eeprom_d;
	unsigned char key[] = KEY;
	char *device = DEVICE;

	log_msg("Loading config data from EEPROM.");
#ifdef PLATFORM_ESP8266
	EEPROM.begin(sizeof(eeprom_d));
#endif

	for (i = 0 ; i < sizeof(eeprom_d); i++)
		{
		data[i] = EEPROM.read(i);
		if (i + sizeof(unsigned long) >= sizeof(eeprom_d))
			continue;
		crc = crc_table[(crc ^ data[i]) & 0x0f] ^ (crc >> 4);
		crc = crc_table[(crc ^ (data[i] >> 4)) & 0x0f] ^ (crc >> 4);
		crc = ~crc;
		}
	
	if (crc == eeprom_d.crc16)
		return;

	/* Doesn't match; load from code */
	memmove(config->key, key, sizeof(config->key));
	memset(config->name, 0, sizeof(config->name));
	memset(config->bulbs, 1, sizeof(config->bulbs));
	strncpy(config->name, device, sizeof(config->name) - 1);
	eeprom_save();
	}
