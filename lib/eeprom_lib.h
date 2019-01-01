#ifndef __EEPROM_LIB_H
#define __EEPROM_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

struct eeprom_data
	{
	unsigned char key[16];
	char name[25];
	unsigned char bulbs[4];
	unsigned char onewire_pin;
	unsigned char soda_type[8];
	};

void eeprom_init(void);
void eeprom_save(void);
extern struct eeprom_data *config;

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_LIB_H */
