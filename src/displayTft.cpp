#ifdef TFT_DISPLAY

/*

Display-Mask (128 x 64 pixel):

 |          111111
 |0123456789012345
------------------
0|PAX:aabbccddee
1|PAX:aabbccddee
2|B:a.bcV  Sats:ab
3|BLTH:abcde SF:ab
4|WIFI:abcde ch:ab
5|RLIM:abcd abcdKB
6|xxxxxxxxxxxxxxxx
6|20:27:00* 27.Feb
7|yyyyyyyyyyyyyyab
  
line 6: x = Text for LORA status OR time/date
line 7: y = Text for LMIC status; ab = payload queue

*/

// Basic Config
#include "globals.h"
#include <esp_spi_flash.h> // needed for reading ESP32 chip attributes

#define DISPLAY_PAGES (3) // number of display pages

Adafruit_ST7735 tft = Adafruit_ST7735(16, 17, 23, 5, 9); // CS,A0,SDA,SCK,RESET

// helper string for converting LoRa spread factor values
#if defined(CFG_eu868)
const char lora_datarate[] = {"1211100908077BFSNA"};
#elif defined(CFG_us915)
const char lora_datarate[] = {"100908078CNA121110090807"};
#elif defined(CFG_as923)
const char lora_datarate[] = {"1211100908077BFSNA"};
#elif defined(CFG_au921)
const char lora_datarate[] = {"1211100908078CNA1211109C8C7C"};
#elif defined(CFG_in866)
const char lora_datarate[] = {"121110090807FSNA"};
#endif

// helper arry for converting month values to text
const char *printmonth[] = {"xxx", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

uint8_t DisplayIsOn = 0;
const int16_t TextColor = 0x5FCC;
const int8_t TextOffset = 25;
const int8_t TextRowHeight = 12;

// helper function, prints a hex key on display
void DisplayKey(const uint8_t *key, uint8_t len, bool lsb) {
  const uint8_t *p;
  for (uint8_t i = 0; i < len; i++) {
    p = lsb ? key + len - i - 1 : key + i;
    tft.printf("%02X", *p);
  }
  tft.printf("\n");  
}

void PrintText(const char *text, uint8_t x, uint8_t y, uint16_t color, uint8_t fontSize) 
{  
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(fontSize);  
  tft.print(text);
}

void init_display(const char *Productname, const char *Version) {

  Productname = "clientSensor";
  Version = "v 0.2";

  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {

    // show startup screen
    uint8_t buf[32];
    tft.initR(INITR_18GREENTAB);  // 1.44 v2.1
    tft.fillScreen(ST7735_BLACK); // CLEAR
    tft.setTextWrap(false);       
    tft.setRotation(0);           

    PrintText(Productname, 30, 60, TextColor, 1);
    PrintText(Version, 50, 75, TextColor, 1);
    delay(2000);

    tft.fillScreen(ST7735_BLACK); 
    tft.setTextSize(1);  

#ifdef DISPLAY_FLIP
   tft.setRotation(1);
#endif

// Display chip information
#if (VERBOSE)
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    tft.setCursor(1, 2);  
    tft.printf("ESP32 %d cores\nWiFi%s%s\n", chip_info.cores,
                 (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                 (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    tft.printf("ESP Rev.%d\n", chip_info.revision);
    tft.printf("%dMB %s Flash\n", spi_flash_get_chip_size() / (1024 * 1024),
                 (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "int."
                                                               : "ext.");
    delay(1000);
#endif // VERBOSE

    tft.fillScreen(ST7735_BLACK); 
    tft.setTextSize(2);  
    tft.setCursor(1, 1);  
    tft.print( "CL:0");    

    tft.setTextSize(1);  
    tft.setCursor(1, TextOffset);   
    tft.printf(!cfg.rssilimit ? "RLIM:off " : "RLIM:%d", cfg.rssilimit);

    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }                     // mutex
} // init_display

void refreshtheDisplay(bool nextPage) {

  static uint8_t DisplayPage = 0;
  const time_t t =
      myTZ.toLocal(now()); // note: call now() here *before* locking mutex!

  // if display is switched off we don't refresh it to relax cpu
  if (!DisplayIsOn && (DisplayIsOn == cfg.screenon))
    return;

  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {

    if (nextPage) {
      DisplayPage = (DisplayPage >= DISPLAY_PAGES - 1) ? 0 : (DisplayPage + 1);
      tft.fillScreen(ST7735_BLACK); 
    }

    draw_page(t, DisplayPage);

    I2C_MUTEX_UNLOCK(); // release i2c bus access

  } // mutex
} // refreshDisplay()

void refreshtheDisplay() { refreshtheDisplay(false); }

void printScanData(time_t t)
{
    char buff[40];

    auto globalMacs = GlobalMacs.size();
    auto localMacs = LocalMacsHash.size();

    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);

    snprintf(
        buff, sizeof(buff), "CL:%-4d",
        (int) (globalMacs + localMacs)); 
     
    tft.setTextSize(2);  
    tft.setCursor(1, 1);
    tft.print(buff); 

    tft.setTextSize(1); 

    tft.setCursor(1, TextOffset);
    tft.printf(!cfg.rssilimit ? "RLIM:off " : "RLIM:%-4d", cfg.rssilimit);
    
    tft.setCursor(60, TextOffset);
    tft.printf("RAM: %4dKB", getFreeRAM() / 1024);
    
    tft.setCursor(1, TextOffset + TextRowHeight);
    tft.printf("Ch:%02d", channel);

    tft.setCursor(1, TextOffset + TextRowHeight * 2);
    tft.printf("Global:%04d", globalMacs);

    tft.setCursor(1, TextOffset + TextRowHeight * 3);
    tft.printf("Local:%04d", localMacs);

    tft.setCursor(1, TextOffset + TextRowHeight * 4);
    tft .printf("%02d:%02d:%02d", hour(t), minute(t), second(t));
}

void draw_page(time_t t, uint8_t page) {

  char timeState, buff[16];
  uint8_t msgWaiting;
  static bool wasnofix = true;

  switch (page % DISPLAY_PAGES) {

    // page 0: parameters overview
    // page 1: time
    // page 2: GPS
    // page 3: BME280/680

  case 0:

printScanData(t);

// update Battery status (line 2)
#ifdef HAS_BATTERY_PROBE
   // u8x8.setCursor(0, 2);
   // u8x8.printf("B:%.2fV", batt_voltage / 1000.0);
#endif

// update GPS status (line 2)
#if (HAS_GPS)
    // have we ever got valid gps data?
    if (gps.passedChecksum() > 0) {
      //u8x8.setCursor(9, 2);
      if (!gps.location.isValid()) // if no fix then display Sats value inverse
      {
        //u8x8.setInverseFont(1);
        //u8x8.printf("Sats:%.2d", gps.satellites.value());
        //u8x8.setInverseFont(0);
      } else
        //u8x8.printf("Sats:%.2d", gps.satellites.value());
    }
#endif


    // line 6: update time-of-day 
    //u8x8.setCursor(0, 6);
#if (TIME_SYNC_INTERVAL)
    // we want a systime display instead LoRa status
    timeState = TimePulseTick ? ' ' : timeSetSymbols[timeSource];
    TimePulseTick = false;
    
    // display inverse timeState if clock controller is enabled
    #if (defined HAS_DCF77) || (defined HAS_IF482)
        //u8x8.printf("%02d:%02d:%02d", hour(t), minute(t), second(t));
        //u8x8.setInverseFont(1);
        //u8x8.printf("%c", timeState);
        //u8x8.setInverseFont(0);
        //u8x8.printf(" %2d.%3s", day(t), printmonth[month(t)]);
    #else
        //u8x8.printf("%02d:%02d:%02d%c %2d.%3s", hour(t), minute(t), second(t),
                    timeState, day(t), printmonth[month(t)]);
    #endif // HAS_DCF77 || HAS_IF482
#endif // TIME_SYNC_INTERVAL

  break; // page0

  case 1:
    printScanData(t);
    break; 

  case 2: 
    printScanData(t);

#if (HAS_GPS)
    if (gps.location.age() < 1500) {
      // line 5: clear "No fix"
      if (wasnofix) {
        snprintf(buff, sizeof(buff), "      ");
       // u8x8.draw2x2String(2, 5, buff);
        wasnofix = false;
      }
      // line 3-4: GPS latitude
      snprintf(buff, sizeof(buff), "%c%-07.4f",
               gps.location.rawLat().negative ? 'S' : 'N', gps.location.lat());
     // u8x8.draw2x2String(0, 3, buff);

      // line 6-7: GPS longitude
      snprintf(buff, sizeof(buff), "%c%-07.4f",
               gps.location.rawLat().negative ? 'W' : 'E', gps.location.lng());
      //u8x8.draw2x2String(0, 6, buff);

    } else {
      snprintf(buff, sizeof(buff), "No fix");
      //u8x8.setInverseFont(1);
      //u8x8.draw2x2String(2, 5, buff);
      //u8x8.setInverseFont(0);
      wasnofix = true;
    }

#else
    // snprintf(buff, sizeof(buff), "No GPS");
    // u8x8.draw2x2String(2, 5, buff);
#endif

    break; // page2

  default:
    break; // default

  } // switch

} // draw_page

#endif // TFT_DISPLAY