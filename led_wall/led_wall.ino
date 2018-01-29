#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define LED_WALL_HEIGHT  8
#define LED_WALL_WIDTH   7
#define LED_WALL_PIN     6
#define LED_WALL_LAYOUT  (NEO_MATRIX_TOP | NEO_MATRIX_LEFT | NEO_MATRIX_ROWS | NEO_MATRIX_ZIGZAG)
#define LED_WALL_OPTIONS (NEO_GRB | NEO_KHZ800)

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(LED_WALL_WIDTH, LED_WALL_HEIGHT, LED_WALL_PIN, LED_WALL_LAYOUT, LED_WALL_OPTIONS);

const uint16_t colors[] =
	{
	matrix.Color(255, 255, 0),
	matrix.Color(0,   255, 255),
	};

void setup()
	{
	matrix.begin();
	matrix.setTextWrap(false);
	matrix.setBrightness(255);
	matrix.setTextColor(colors[0]);
	}

int pass = 0;

void show_text(char *text, unsigned short color)
	{
	signed int x = matrix.width();
	
	while (1)
		{
		matrix.fillScreen(0);
		matrix.setTextColor(color);
		matrix.setCursor(x, 0);
		matrix.print(text);
		if (--x < -36)
			return;
		matrix.show();
		delay(100);
		}
	}

void loop()
	{
	show_text("Hive13", colors[0]);
	show_text("Fronk", colors[1]);
	}