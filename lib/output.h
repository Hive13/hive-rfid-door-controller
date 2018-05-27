#ifndef __OUTPUT_H
#define __OUTPUT_H

#define WD0 16
#define WD1 5
#define WD2 4
#define WD3 0
#define WD4 2
#define WD5 14
#define WD6 12
#define WD7 13
#define WD8 15
#define WRX 3
#define WTX 1

#ifdef __cplusplus
extern "C" {
#endif

#define OUTPUT_DOOR_LATCH     0
#define OUTPUT_SCANNER_BEEPER 1
#define OUTPUT_SCANNER_LIGHT  2


#define OUTPUT_TYPE_ARDUINO 1
#define OUTPUT_TYPE_AVR     2
#define OUTPUT_TYPE_ESP     3
#define OUTPUT_TYPE_DS2408  4

struct output
	{
	unsigned char type;
	unsigned char init;
	unsigned char pin;
	union
		{
		struct
			{
			volatile unsigned char *port;
			volatile unsigned char *ddr;
			} avr;
		unsigned char ds2408_addr[8];
		} data;
	};

void set_output(unsigned char output, unsigned char state);
void output_init(unsigned char pin);

#ifdef __cplusplus
};
#endif

#endif /* __OUTPUT_H */
