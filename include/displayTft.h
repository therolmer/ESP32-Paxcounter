#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "cyclic.h"

extern uint8_t DisplayIsOn;

void init_display(const char *Productname, const char *Version);
void refreshtheDisplay(void);
void refreshtheDisplay(bool nextPage);
void draw_page(time_t t, uint8_t page);
void DisplayKey(const uint8_t *key, uint8_t len, bool lsb);
void PrintText(const char *text, uint8_t x, uint8_t y, uint16_t color, uint8_t fontSize);
void printScanData(time_t t);

#endif