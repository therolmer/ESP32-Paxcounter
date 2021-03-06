// clang-format off

#ifndef _TTGOV1_H
#define _TTGOV1_H

#include <stdint.h>

// Hardware related definitions for TTGOv1 board

//#define HAS_LORA 1       // comment out if device shall not send data via LoRa
#define CFG_sx1276_radio 1

#define HAS_DISPLAY 1
#define TFT_DISPLAY 1 // TFT-Display on board

//#define OLED_DISPLAY_TYPE U8X8_SSD1306_128X64_NONAME_HW_I2C
//#define OLED_DISPLAY 1 // OLED-Display on board
//#define DISPLAY_FLIP  1 // uncomment this for rotated display
#define HAS_LED LED_BUILTIN
#define LED_ACTIVE_LOW 1  // Onboard LED is active when pin is LOW
#define HAS_BUTTON KEY_BUILTIN

// Pins for I2C interface of OLED Display
#define MY_OLED_SDA (4)
#define MY_OLED_SCL (15)
#define MY_OLED_RST (16)

// Pins for LORA chip SPI interface come from board file, we need some
// additional definitions for LMIC
#define LORA_IO1  (33)
#define LORA_IO2  LMIC_UNUSED_PIN

#endif
