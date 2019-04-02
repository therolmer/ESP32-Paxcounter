#ifndef _WIFIFILTER_H
#define _WIFIFILTER_H

// Hash function for scrambling MAC addresses
#include "hash.h"

#include <Arduino.h>

#define TYPE_MANAGEMENT       0x00
#define TYPE_CONTROL          0x01
#define TYPE_DATA             0x02
#define SUBTYPE_PROBE_REQUEST 0x04

#define CRC_LENGTH 4

#define MAX_MAC_ADDR 250
#define RSSI_MIN 6
#define RSSI_MAX 7
#define PAYLOAD_SIZE 8
#define PAYLOAD_BEGIN 9

#define RSSI_THRESHOLD 5

// Declare each custom function (excluding built-in, such as setup and loop) before it will be called.
// https://docs.platformio.org/en/latest/faq.html#convert-arduino-file-to-c-manually
void showMetadata(const uint8_t *buffer, unsigned int length, unsigned int rssi);
void printDataSpan(uint16_t start, uint16_t size, uint8_t* data);
void getMAC(const char *addr, const uint8_t* data, uint16_t offset);
void getsmallMAC(const char *addr, const uint8_t* data, uint16_t offset);
void printhex (const uint8_t *data, uint16_t offset, uint16_t length);
bool updateMacArray(const uint8_t *data, uint16_t offset);
uint8_t calcActiveMacs();

#endif