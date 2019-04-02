#include "wifiFilter.h"

uint8_t macAddr[MAX_MAC_ADDR][PAYLOAD_BEGIN + 112];
int NumberOfMacs = 0;
int localWithSSID = 0;
char emptyStr[] = "                ";
uint8_t cleanBuffer[300];

void showMetadata(const uint8_t *buffer, unsigned int length, unsigned int rssi) 
{
    unsigned int frameControl = ((unsigned int)buffer[1] << 8) + buffer[0];

    uint8_t version = (frameControl & 0b0000000000000011) >> 0;
    uint8_t frameType = (frameControl & 0b0000000000001100) >> 2;
    uint8_t frameSubType = (frameControl & 0b0000000011110000) >> 4;
    uint8_t toDS = (frameControl & 0b0000000100000000) >> 8;
    uint8_t fromDS = (frameControl & 0b0000001000000000) >> 9;

    // Only look for probe request packets
    if (frameType != TYPE_MANAGEMENT || frameSubType != SUBTYPE_PROBE_REQUEST)
        return;

    char tempS[50];

    // Global or local
    if ((buffer[10] & 0x2) == 0x2) 
    {
        uint16_t sequence = (buffer[16] >> 8) + (buffer[15] << 4);
        sprintf(tempS, "Local");
    } 
    else
    {
        sprintf(tempS, "Global");    
    }

    char addr1[] = "00:00:00:00:00:00";
    getMAC(addr1, buffer, 10);

    char tempS2[150];
    sprintf(tempS2, "%s Len: %i %s RSSI: %i SsidLen: %i", addr1, length, tempS, rssi, buffer[25]);
    Serial.print(tempS2);

    // SSID
    if (buffer[25] > 0 )
    {         
        //  ESP_LOGI(TAG, "SSID: %i", buffer[26]);        
    }

    bool notAllFound = true;
    bool firstIe = true;
    int curPos = 24;
    int dataBegin = buffer[25] + 2;
    int cleanBufferPos = 0;

    do
    {
        uint8_t ieId = buffer[curPos];
        uint8_t ieLen = buffer[curPos + 1];
                
        Serial.print(" id: ");
        Serial.print(ieId);
       
        Serial.print("(");
        Serial.print(ieLen);
        Serial.print(")");
       
        if (ieLen == 0)
        {
            // Сдвиг только для первого IE
            if (!firstIe)
            {
                // Хрень, на будущее отбрасывать
                curPos = curPos - 2;
                notAllFound = false;
            }
            else
            {
                dataBegin = 2;
            }         
        }

        if ((ieId != 0) && (ieId != 3))
        {
            for (int k = 0; k < ieLen + 2; k++)
            {
                cleanBuffer[cleanBufferPos] = buffer[curPos + k];
                cleanBufferPos++;
            }
        }

        firstIe = false;
      
        curPos = curPos + ieLen + 2;
     
        if (curPos >= (length - CRC_LENGTH))
        {
            curPos = length - CRC_LENGTH;
            notAllFound = false;
        }
    }
    while (notAllFound);

    uint32_t hashedmac = rokkit((const char*)&buffer[24 + dataBegin], curPos - dataBegin - 24);
    Serial.print(" ##");
    Serial.print(hashedmac);
    Serial.print("##");
    
    Serial.println(" ");
    printhex(cleanBuffer, 0, cleanBufferPos);
    Serial.println(" ");

    hashedmac = rokkit((const char*)&cleanBuffer[0], cleanBufferPos);
    Serial.print(" 2##");
    Serial.print(hashedmac);
    Serial.print("2##");

    Serial.println(" ");
    printhex(buffer, 0, length);
    Serial.println(" ");

  //     sprintf(cstr, " Ch: %i", wifi_get_channel());
  //     Serial.print(cstr);

  //   if (updateMacArray(snifferPacket->data, 10, rssi))
  //   {

  //     u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);

  //     for (int i = 0; i<4; i++)
  //     {
  //         u8x8.drawString(0, i, emptyStr);
  //     }

  //     char addr[] = "00:00:00";
  //     getsmallMAC(addr, snifferPacket->data, 10);
  //     sprintf(cstr, " Mac: %s", addr);
  //     u8x8.drawString(0, 0, cstr);

  //     sprintf(cstr, "RSSI  %i ch ", snifferPacket->rx_ctrl.rssi,
  //     wifi_get_channel()); u8x8.drawString(0, 1, cstr);

  //     sprintf(cstr, "Dev: %i", NumberOfMacs);
  //     u8x8.drawString(0, 3, cstr);

  //     int local;
  //     int global;

  //     uint8_t activeDevices = calcActiveMacs(local, global);
  //     sprintf(cstr, "Dev: %i LwS: %i", NumberOfMacs, localWithSSID);
  //     u8x8.drawString(0, 3, cstr);

  //     sprintf(cstr, " Glob: %i Loca: %i", global, local);
  //     u8x8.drawString(0, 2, cstr);

  //     //Serial.print(" Peer MAC: ");
  //     //Serial.print(addr);

  //     //uint8_t SSID_length = snifferPacket->data[25];
  //     //Serial.print(" SSID: ");

  //     //printDataSpan(26, SSID_length, snifferPacket->data);

  //     Serial.println();
  //   }
  //   else
  //   {
  //         char cstr[20];
  //         u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);

  //         u8x8.drawString(0, 2, emptyStr);
  //         u8x8.drawString(0, 3, emptyStr);

  //         int global;
  //         int local;
  //         int localWithSSID;
  //         uint8_t activeDevices = calcActiveMacs(global, local);
  //         sprintf(cstr, "Dev: %i LwS: %i", NumberOfMacs, localWithSSID);
  //         u8x8.drawString(0, 3, cstr);

  //         sprintf(cstr, "Glo: %i Loc: %i", global, local);
  //         u8x8.drawString(0, 2, cstr);
  //   }
}

void printDataSpan(uint16_t start, uint16_t size, uint8_t *data) {
  for (uint16_t i = start; i < 112 && i < start + size; i++) {
    Serial.write(data[i]);
  }
}

void printhex(const uint8_t *data, uint16_t offset, uint16_t length) 
{
    char tempS[10];
 
    // Serial.println("####");
    // Serial.println(offset);
    
    for (int i = 0; i < length; i++) 
    {        
        sprintf(tempS, "%i-%02x ", offset + i, data[offset + i]);
        Serial.print(tempS);
    }
}

void getMAC(const char *addr, const uint8_t *data, uint16_t offset) 
{
  sprintf((char *) addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset + 0],
          data[offset + 1], data[offset + 2], data[offset + 3],
          data[offset + 4], data[offset + 5]);
}

void getsmallMAC(const char *addr, const uint8_t *data, uint16_t offset) 
{
  sprintf((char *)addr, "%02x:%02x:%02x", data[offset + 3], data[offset + 4],
          data[offset + 5]);
}

bool updateMacArray(const uint8_t *data, uint16_t offset, uint8_t rssi) 
{
  if (NumberOfMacs == 0) 
  {
    for (int j = 0; j < 6; j++) {
      macAddr[NumberOfMacs][j] = data[offset + j];
    }

    macAddr[NumberOfMacs][RSSI_MIN] = rssi;
    macAddr[NumberOfMacs][RSSI_MAX] = rssi;

    NumberOfMacs++;
    return true;
  }

  // Is this Mac new or not
  bool newMacFound = true;

  for (int i = 0; i < NumberOfMacs; i++) {

    bool macEqual = true;
    for (int j = 0; j < 6; j++) {
      // Compare byte by byte
      if (macAddr[i][j] != data[offset + j]) {
        macEqual = false;
        break;
      }
    }

    if (macEqual) {
      // Already have this mac
      newMacFound = false;

      // Compare Rssi
      if (macAddr[i][RSSI_MIN] > rssi) {
        macAddr[i][RSSI_MIN] = rssi;
      }

      if (macAddr[i][RSSI_MAX] < rssi) {
        macAddr[i][RSSI_MAX] = rssi;
      }

      break;
    }
  }

  if (newMacFound) 
  {
    // Copy mac address to array
    for (int j = 0; j < 6; j++) {
      macAddr[NumberOfMacs][j] = data[offset + j];
    }

    macAddr[NumberOfMacs][RSSI_MIN] = rssi;
    macAddr[NumberOfMacs][RSSI_MAX] = rssi;
    NumberOfMacs++;
  }

  return newMacFound;
}

uint8_t calcActiveMacs(int &global, int &local) 
{
  int activeMacs = 0;
  global = 0;
  local = 0;

  for (int i = 0; i < NumberOfMacs; i++) {
    if ((macAddr[i][RSSI_MAX] - macAddr[i][RSSI_MIN]) > RSSI_THRESHOLD) {
      activeMacs++;
    }

    if ((macAddr[i][0] & 0x2) == 0x2) {
      local++;
    } else {
      global++;
    }
  }

  return activeMacs;
}
