/*
***************************************************************************  
**  Program  : DSMRfirmwareAPI (restAPI)
*/
#define _FW_VERSION "v0.0.3 (18-12-2019)"
/*
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      
  Arduino-IDE settings for DSMR-logger Version 4 (ESP-12):

    - Board: "Generic ESP8266 Module"
    - Flash mode: "DOUT" | "DIO"    // if you change from one to the other OTA may fail!
    - Flash size: "4M (1M SPIFFS)"  // ESP-01 "1M (256K SPIFFS)"  // PUYA flash chip won't work
    - DebugT port: "Disabled"
    - DebugT Level: "None"
    - IwIP Variant: "v2 Lower Memory"
    - Reset Method: "none"   // but will depend on the programmer!
    - Crystal Frequency: "26 MHz" 
    - VTables: "Flash"
    - Flash Frequency: "40MHz"
    - CPU Frequency: "80 MHz"
    - Buildin Led: "2"  // ESP-01 (Black) GPIO01 - Pin 2 // "2" for Wemos and ESP-01S
    - Upload Speed: "115200"                                                                                                                                                                                                                                                 
    - Erase Flash: "Only Sketch"
    - Port: <select correct port>
*/

/******************** compiler options  ********************************************/
#define IS_ESP12                  // define if it's a 'bare' ESP-12 (no reset/flash functionality on board)
#define USE_UPDATE_SERVER         // define if there is enough memory and updateServer to be used
#define HAS_OLED_SSD1306          // define if a 0.96" OLED display is present
//  #define HAS_OLED_SH1106           // define if a 1.3" OLED display is present
//  #define USE_PRE40_PROTOCOL        // define if Slimme Meter is pre DSMR 4.0 (2.2 .. 3.0)
//  #define USE_NTP_TIME              // define to generate Timestamp from NTP (Only Winter Time for now)
//  #define SM_HAS_NO_FASE_INFO       // if your SM does not give fase info use total delevered/returned
#define USE_MQTT                  // define if you want to use MQTT
//  #define USE_MINDERGAS             // define if you want to update mindergas (also add token down below)
//  #define SHOW_PASSWRDS             // well .. show the PSK key and MQTT password, what else?
/******************** don't change anything below this comment **********************/

#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time
#include <TelnetStream.h>       // Version 0.0.1 - https://github.com/jandrassy/TelnetStream
#include <ArduinoJson.h>

#ifdef USE_PRE40_PROTOCOL                                       //PRE40
  //  https://github.com/mrWheel/arduino-dsmr30.git             //PRE40
  #include <dsmr30.h>                                           //PRE40
#else                                                           //else
  //  https://github.com/matthijskooijman/arduino-dsmr
  #include <dsmr.h>               // Version 0.1 - Commit f79c906 on 18 Sep 2018
#endif

#ifdef ARDUINO_ESP8266_GENERIC
  #define _HOSTNAME     "DSMR-API"  
  #ifdef IS_ESP12
    #define DTR_ENABLE  12
  #endif  // is_esp12
#else // not arduino_esp8266_generic
  #define _HOSTNAME     "ESP12-DSMR"
  #ifdef IS_ESP12
    #define DTR_ENABLE  12
  #endif
#endif  // arduino_esp8266_generic

#define SETTINGS_FILE      "/DSMRsettings.ini"
#define GUI_COLORS_FILE    "/DSMRchartColors.ini"

#define LED_ON            LOW
#define LED_OFF          HIGH
#define FLASH_BUTTON        0
#define MAXCOLORNAME       15

//-------------------------.........1....1....2....2....3....3....4....4....5....5....6....6....7....7
//-------------------------1...5....0....5....0....5....0....5....0....5....0....5....0....5....0....5
#define DATA_FORMAT       "%-8.8s;%10.3f;%10.3f;%10.3f;%10.3f;%10.3f;\n"
#define DATA_CSV_HEADER   "YYMMDDHH;      EDT1;      EDT2;      ERT1;      ERT2;       GDT;"
#define DATA_RECLEN       75
#define KEEP_DAYS_HOURS   3
#define HOURS_FILE        "/RINGhours.csv"
#define _NO_HOUR_SLOTS_   (KEEP_DAYS_HOURS * 24)
#define DAYS_FILE         "/RINGdays.csv"
#define KEEP_WEEK_DAYS    2  
#define _NO_DAY_SLOTS_    (KEEP_WEEK_DAYS * 7)
#define MONTHS_FILE       "/RINGmonths.csv"
#define KEEP_YEAR_MONTHS  1  
#define _NO_MONTH_SLOTS_  (KEEP_YEAR_MONTHS * 12)

enum    { PERIOD_UNKNOWN, HOURS, DAYS, MONTHS, YEARS };

#include "Debug.h"
uint8_t   settingSleepTime; // needs to be declared before the oledStuff.h include
#if defined( HAS_OLED_SSD1306 ) && defined( HAS_OLED_SH1106 )
  #error Only one OLED display can be defined
#endif
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  #include "oledStuff.h"
#endif
#include "networkStuff.h"

/**
 * Define the DSMRdata we're interested in, as well as the DSMRdatastructure to
 * hold the parsed DSMRdata. This list shows all supported fields, remove
 * any fields you are not using from the below list to make the parsing
 * and printing code smaller.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
  /* String */         identification
  /* String */        ,p1_version
  /* String */        ,timestamp
  /* String */        ,equipment_id
  /* FixedValue */    ,energy_delivered_tariff1
  /* FixedValue */    ,energy_delivered_tariff2
  /* FixedValue */    ,energy_returned_tariff1
  /* FixedValue */    ,energy_returned_tariff2
  /* String */        ,electricity_tariff
  /* FixedValue */    ,power_delivered
  /* FixedValue */    ,power_returned
  /* FixedValue */    ,electricity_threshold
  /* uint8_t */       ,electricity_switch_position
  /* uint32_t */      ,electricity_failures
  /* uint32_t */      ,electricity_long_failures
  /* String */        ,electricity_failure_log
  /* uint32_t */      ,electricity_sags_l1
  /* uint32_t */      ,electricity_sags_l2
  /* uint32_t */      ,electricity_sags_l3
  /* uint32_t */      ,electricity_swells_l1
  /* uint32_t */      ,electricity_swells_l2
  /* uint32_t */      ,electricity_swells_l3
  /* String */        ,message_short
  /* String */        ,message_long
  /* FixedValue */    ,voltage_l1
  /* FixedValue */    ,voltage_l2
  /* FixedValue */    ,voltage_l3
  /* FixedValue */    ,current_l1
  /* FixedValue */    ,current_l2
  /* FixedValue */    ,current_l3
  /* FixedValue */    ,power_delivered_l1
  /* FixedValue */    ,power_delivered_l2
  /* FixedValue */    ,power_delivered_l3
  /* FixedValue */    ,power_returned_l1
  /* FixedValue */    ,power_returned_l2
  /* FixedValue */    ,power_returned_l3
  /* uint16_t */      ,gas_device_type
  /* String */        ,gas_equipment_id
  /* uint8_t */       ,gas_valve_position
  /* TimestampedFixedValue */ ,gas_delivered
#ifdef USE_PRE40_PROTOCOL                          //PRE40
  /* TimestampedFixedValue */ ,gas_delivered2      //PRE40
#endif                                             //PRE40
//  /* uint16_t */      ,thermal_device_type
//  /* String */        ,thermal_equipment_id
//  /* uint8_t */       ,thermal_valve_position
//  /* TimestampedFixedValue */ ,thermal_delivered
//  /* uint16_t */      ,water_device_type
//  /* String */        ,water_equipment_id
//  /* uint8_t */       ,water_valve_position
//  /* TimestampedFixedValue */ ,water_delivered
//  /* uint16_t */      ,slave_device_type
//  /* String */        ,slave_equipment_id
//  /* uint8_t */       ,slave_valve_position
//  /* TimestampedFixedValue */ ,slave_delivered
>;

enum    { TAB_UNKNOWN, TAB_ACTUEEL, TAB_LAST24HOURS, TAB_LAST7DAYS, TAB_LAST24MONTHS, TAB_GRAPHICS, TAB_SYSINFO, TAB_EDITOR };

typedef struct {
    uint32_t  Label;
    float     EDT1;
    float     EDT2;
    float     ERT1;
    float     ERT2;
    float     GDT;
} dataStruct;

static dataStruct hourData;   // 0 + 1-24
static dataStruct dayData;    // 1 - 7 (0=header, 1=sunday)
static dataStruct monthData;  // 0 + year1 1 t/m 12 + year2 1 t/m 12

const char *weekDayName[]  { "Unknown", "Zondag", "Maandag", "Dinsdag", "Woensdag"
                            , "Donderdag", "Vrijdag", "Zaterdag", "Unknown" };
const char *monthName[]    { "00", "Januari", "Februari", "Maart", "April", "Mei", "Juni", "Juli"
                            , "Augustus", "September", "Oktober", "November", "December", "13" };
const char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "Unknown" };

/**
struct FSInfo {
    size_t totalBytes;
    size_t usedBytes;
    size_t blockSize;
    size_t pageSize;
    size_t maxOpenFiles;
    size_t maxPathLength;
};
**/
#ifdef DTR_ENABLE
  P1Reader    slimmeMeter(&Serial, DTR_ENABLE);
#else
  P1Reader    slimmeMeter(&Serial, 0);
#endif



//===========================GLOBAL VAR'S======================================
  WiFiClient  wifiClient;
  MyData      DSMRdata;
  uint32_t    readTimer;
  time_t      actT, newT;
  char        actTimestamp[20] = "";
  char        newTimestamp[20] = "";
  uint32_t    slotErrors = 0;
  uint32_t    nrReboots  = 0;

//----------------- old var's -----(remove as soon as possible)-----------------
int8_t    actTab = 0;
uint32_t  telegramInterval, noMeterWait, telegramCount, telegramErrors, lastOledStatus;
char      cMsg[150], fChar[10];
float     EnergyDelivered, EnergyReturned, prevEnergyDelivered=0.0, prevEnergyReturned=0.0;
float     PowerDelivered, PowerReturned, maxPowerDelivered, maxPowerReturned;
char      maxTimePD[7], maxTimePR[7]; // hh:mm
int32_t   PowerDelivered_l1, PowerDelivered_l2, PowerDelivered_l3;  // Watt in 1 watt resolution
int32_t   PowerReturned_l1,  PowerReturned_l2,  PowerReturned_l3;   // Watt in 1 watt resolution
float     GasDelivered;
String    pTimestamp;
String    P1_Version, Equipment_Id, GasEquipment_Id, ElectricityTariff;
char      Identification[100];  // Sn (0..96)
float     EnergyDeliveredTariff1, EnergyDeliveredTariff2, EnergyReturnedTariff1, EnergyReturnedTariff2;
float     Voltage_l1, Voltage_l2, Voltage_l3;
uint8_t   Current_l1, Current_l2, Current_l3;
uint16_t  GasDeviceType;

String    lastReset = "";
bool      spiffsNotPopulated = false; // v1.0.3b
bool      OTAinProgress = false, doLog = false, Verbose1 = false, Verbose2 = false, showRaw = false;
int8_t    thisHour = -1, prevNtpHour = 0, thisDay = -1, thisMonth = -1, lastMonth, thisYear = 15;
int32_t   thisHourKey = -1;
int8_t    forceMonth = 0, forceDay = 0;
int8_t    showRawCount = 0;
uint32_t  nextSecond, unixTimestamp;
uint64_t  upTimeSeconds;
IPAddress ipDNS, ipGateWay, ipSubnet;
float     settingEDT1, settingEDT2, settingERT1, settingERT2, settingGDT;
float     settingENBK, settingGNBK;
uint8_t   settingInterval;
char      settingBgColor[MAXCOLORNAME], settingFontColor[MAXCOLORNAME];
char      settingMQTTbroker[101], settingMQTTuser[40], settingMQTTpasswd[30], settingMQTTtopTopic[21];
uint32_t  settingMQTTinterval;

struct showValues {
  template<typename Item>
  void apply(Item &i) {
    TelnetStream.print("showValues: ");
    if (i.present()) 
    {
      TelnetStream.print(Item::name);
      TelnetStream.print(F(": "));
      TelnetStream.print(i.val());
      TelnetStream.print(Item::unit());
    } else 
    {
      TelnetStream.print(F("<no value>"));
    }
    TelnetStream.println();
  }
};


//===========================================================================================
void displayStatus() 
{
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  switch(msgMode) {
    case 1:   sprintf(cMsg, "Up:%15.15s", upTime().c_str());
              break;
    case 2:   sprintf(cMsg, "WiFi RSSI:%4d dBm", WiFi.RSSI());
              break;
    case 3:   sprintf(cMsg, "Heap:%7d Bytes", ESP.getFreeHeap());
              break;
    case 4:   sprintf(cMsg, "IP %s", WiFi.localIP().toString().c_str());
              break;
    default:  sprintf(cMsg, "Telgrms:%6d/%3d", telegramCount, telegramErrors);
              msgMode = 0;
  }
  oled_Print_Msg(3, cMsg, 0);
  msgMode++;
#endif
  
} // displayStatus()

void printData()
{  
} // printData()


//===========================================================================================
void setup() 
{
#ifdef USE_PRE40_PROTOCOL                                                         //PRE40
//Serial.begin(115200);                                                           //DEBUG
  Serial.begin(9600, SERIAL_7E1);                                                 //PRE40
#else   // not use_dsmr_30                                                        //PRE40
  Serial.begin(115200, SERIAL_8N1);
#endif  // use_dsmr_30
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FLASH_BUTTON, INPUT);
#ifdef DTR_ENABLE
  pinMode(DTR_ENABLE, OUTPUT);
#endif
  
  Serial.printf("\n\nBooting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oledSleepTimer = millis() + (10 * 60000); // initially 10 minutes on
  oled_Init();
  oled_Clear();  // clear the screen so we can paint the menu.
  oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
  int8_t sPos = String(_FW_VERSION).indexOf(' ');
  sprintf(cMsg, "(c)2019 [%s]", String(_FW_VERSION).substring(0,sPos).c_str());
  oled_Print_Msg(1, cMsg, 0);
  oled_Print_Msg(2, " Willem Aandewiel", 0);
  oled_Print_Msg(3, " >> Have fun!! <<", 1000);
  yield();
#else  // don't blink if oled-screen attatched
  for(int I=0; I<8; I++) 
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(2000);
  }
#endif
  digitalWrite(LED_BUILTIN, LED_OFF);  // HIGH is OFF
  lastReset     = ESP.getResetReason();

  startTelnet();
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
  oled_Print_Msg(3, "telnet (poort 23)", 2500);
#endif  // has_oled_ssd1306

//================ SPIFFS ===========================================
  if (!SPIFFS.begin()) {
    DebugTln(F("SPIFFS Mount failed\r"));   // Serious problem with SPIFFS 
    SPIFFSmounted = false;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
    oled_Print_Msg(3, "SPIFFS FAILED!", 2000);
#endif  // has_oled_ssd1306
    
  } else { 
    DebugTln(F("SPIFFS Mount succesfull\r"));
    SPIFFSmounted = true;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
    oled_Print_Msg(3, "SPIFFS mounted", 1500);
#endif  // has_oled_ssd1306
  }

//------ read status file for last Timestamp --------------------
  //strcpy(actTimestamp, "010101010101X");
  //writeLastStatus();  // only for firsttime initialization
  readLastStatus(); // place it in actTimestamp
  // set the time to actTimestamp!
  actT = epoch(actTimestamp, strlen(actTimestamp), true);
  DebugTf("===>actTimestamp[%s]-> nrReboots[%u]-> Errors[%u]<======\r\n\n", actTimestamp
                                                                    , nrReboots++
                                                                    , slotErrors);

//=============now test if SPIFFS is correct populated!============
  DSMRfileExist("/DSMRindex.html");
  DSMRfileExist("/DSMRindex.js");
  DSMRfileExist("/DSMRindex.css");
  //DSMRfileExist("/DSMRgraphics.js");
  //DSMRfileExist("/DSMReditor.html");
  //DSMRfileExist("/DSMReditor.js");
  DSMRfileExist("/FSexplorer.html");
  DSMRfileExist("/FSexplorer.css");
//=============end SPIFFS =========================================

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Clear();  // clear the screen 
  oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
  oled_Print_Msg(1, "Verbinden met WiFi", 500);
#endif  // has_oled_ssd1306

  digitalWrite(LED_BUILTIN, LED_ON);
  startWiFi();

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
  oled_Print_Msg(1, WiFi.SSID(), 0);
  sprintf(cMsg, "IP %s", WiFi.localIP().toString().c_str());
  oled_Print_Msg(2, cMsg, 1500);
#endif  // has_oled_ssd1306
  digitalWrite(LED_BUILTIN, LED_OFF);
  
  Debugln();
  Debug (F("Connected to " )); Debugln (WiFi.SSID());
  Debug (F("IP address: " ));  Debugln (WiFi.localIP());
  Debugln();

  for (int L=0; L < 10; L++) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(200);
  }
  digitalWrite(LED_BUILTIN, LED_OFF);
  
  startMDNS(_HOSTNAME);
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(3, "mDNS gestart", 1500);
#endif  // has_oled_ssd1306


#if defined(USE_NTP_TIME)                                   //USE_NTP
//================ startNTP =========================================
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )  //USE_NTP
    oled_Print_Msg(3, "setup NTP server", 100);             //USE_NTP
  #endif  // has_oled_ssd1306                               //USE_NTP
                                                            //USE_NTP
  if (!startNTP())                                          //USE_NTP
  {                                                         //USE_NTP
    DebugTln(F("ERROR!!! No NTP server reached!\r\n\r"));   //USE_NTP
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )  //USE_NTP
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);             //USE_NTP
    oled_Print_Msg(2, "geen reactie van", 100);             //USE_NTP
    oled_Print_Msg(2, "NTP server's", 100);                 //USE_NTP 
    oled_Print_Msg(3, "Reboot DSMR-logger", 2000);          //USE_NTP
  #endif  // has_oled_ssd1306                               //USE_NTP
    delay(2000);                                            //USE_NTP
    ESP.restart();                                          //USE_NTP
    delay(3000);                                            //USE_NTP
  }                                                         //USE_NTP
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )  //USE_NTP
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);             //USE_NTP
    oled_Print_Msg(3, "NTP gestart", 1500);                 //USE_NTP
    prevNtpHour = hour();                                   //USE_NTP
  #endif                                                    //USE_NTP
                                                            //USE_NTP
#endif  //USE_NTP_TIME                                      //USE_NTP

  sprintf(cMsg, "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

  Serial.print("\nGebruik 'telnet ");
  Serial.print (WiFi.localIP());
  Serial.println("' voor verdere debugging\r\n");

//===========================================================================================

#if defined(USE_NTP_TIME)                                                           //USE_NTP
  time_t t = now(); // store the current time in time variable t                    //USE_NTP
  sprintf(cMsg, "%02d%02d%02d%02d%02d%02dW\0\0", (year(t) - 2000), month(t), day(t) //USE_NTP
                                               , hour(t), minute(t), second(t));    //USE_NTP
  pTimestamp = cMsg;                                                                //USE_NTP
  DebugTf("Time is set to [%s] from NTP\r\n", cMsg);                                //USE_NTP
#endif  // use_dsmr_30

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  sprintf(cMsg, "DT: %02d%02d%02d%02d0101W", thisYear, thisMonth, thisDay, thisHour);
  oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
  oled_Print_Msg(3, cMsg, 1500);
#endif  // has_oled_ssd1306

  readSettings(false);
  //readColors(false);

#ifdef USE_MQTT                                               //USE_MQTT
  startMQTT();
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )    //USE_MQTT
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);               //USE_MQTT
    oled_Print_Msg(3, "MQTT server set!", 1500);              //USE_MQTT
  #endif  // has_oled_ssd1306                                 //USE_MQTT
#endif                                                        //USE_MQTT

  telegramCount   = 0;
  telegramErrors  = 0;

  if (!spiffsNotPopulated) {
    DebugTln(F("SPIFFS correct populated -> normal operation!\r"));
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0); 
    oled_Print_Msg(1, "OK, SPIFFS correct", 0);
    oled_Print_Msg(2, "Verder met normale", 0);
    oled_Print_Msg(3, "Verwerking ;-)", 2500);
#endif  // has_oled_ssd1306
    httpServer.serveStatic("/",               SPIFFS, "/DSMRindex.html");
    httpServer.serveStatic("/DSMRindex.html", SPIFFS, "/DSMRindex.html");
    httpServer.serveStatic("/index",          SPIFFS, "/DSMRindex.html");
    httpServer.serveStatic("/index.html",     SPIFFS, "/DSMRindex.html");
  } else {
    DebugTln(F("Oeps! not all files found on SPIFFS -> present FSexplorer!\r"));
    spiffsNotPopulated = true;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "!OEPS! niet alle", 0);
    oled_Print_Msg(1, "files op SPIFFS", 0);
    oled_Print_Msg(2, "gevonden! (fout!)", 0);
    oled_Print_Msg(3, "Start FSexplorer", 2000);
#endif  // has_oled_ssd1306
  }

  setupFSexplorer();
  httpServer.serveStatic("/DSMRindex.css",   SPIFFS, "/DSMRindex.css");
  httpServer.serveStatic("/DSMRindex.js",     SPIFFS, "/DSMRindex.js");
  //httpServer.serveStatic("/DSMReditor.html",  SPIFFS, "/DSMReditor.html");
  //httpServer.serveStatic("/DSMReditor.js",    SPIFFS, "/DSMReditor.js");
  //httpServer.serveStatic("/DSMRgraphics.js",  SPIFFS, "/DSMRgraphics.js");
  httpServer.serveStatic("/FSexplorer.png",   SPIFFS, "/FSexplorer.png");

  httpServer.on("/api", HTTP_GET, processAPI);
  // all other api calls are catched in FSexplorer onNotFounD!

  httpServer.begin();
  DebugTln( "HTTP server gestart\r" );
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )    //HAS_OLED
  oled_Clear();                                             //HAS_OLED
  oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);               //HAS_OLED
  oled_Print_Msg(2, "HTTP server ..", 0);                   //HAS_OLED
  oled_Print_Msg(3, "gestart (poort 80)", 0);               //HAS_OLED
#endif  // has_oled_ssd1306                                 //HAS_OLED

  for (int i = 0; i< 10; i++) 
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(250);
  }

  DebugTln(F("Enable slimmeMeter..\r"));
  delay(100);
  slimmeMeter.enable(true);

  //test(); monthTabel

  DebugTf("Startup complete! actTimestamp[%s]\r\n", actTimestamp);  

#ifdef IS_ESP12
  Serial.swap();
#endif // is_esp12

  sprintf(cMsg, "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

  telegramInterval = millis() + 5000;
  noMeterWait      = millis() + 5000;
  upTimeSeconds    = (millis() / 1000) + 50;
  nextSecond       = millis() + 1000;

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
    oled_Print_Msg(1, "Startup complete", 0);
    oled_Print_Msg(2, "Wait for first", 0);
    oled_Print_Msg(3, "telegram .....", 500);
#endif  // has_oled_ssd1306
  
} // setup()


//===========================================================================================
void loop () 
{
  httpServer.handleClient();
  MDNS.update();
  handleKeyInput();
  //handleRefresh();  // webSocket
  handleMQTT();
  handleMindergas();

  // once every second, increment uptime seconds
  if (millis() > nextSecond) 
  {
    nextSecond += 1000; // nextSecond is ahead of millis() so it will "rollover" 
    upTimeSeconds++;    // before millis() and this will probably work just fine
  }
  
#if defined(USE_NTP_TIME)                                                         //USE_NTP
  if (timeStatus() == timeNeedsSync || prevNtpHour != hour())                     //USE_NTP
  {                                                                               //USE_NTP
    prevNtpHour = hour();                                                         //USE_NTP
    setSyncProvider(getNtpTime);                                                  //USE_NTP
    setSyncInterval(600);                                                         //USE_NTP
  }                                                                               //USE_NTP
#endif                                                                            //USE_NTP

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  checkFlashButton();
  if (millis() - lastOledStatus > 5000) 
  {
    lastOledStatus = millis();
    displayStatus();
  }
#endif

  if (!showRaw) 
  {
    slimmeMeter.loop();
    //---- capture new telegram ??
    if (millis() > telegramInterval) 
    {
      telegramInterval = millis() + (settingInterval * 1000);  // test 10 seconden
      slimmeMeter.enable(true);
#ifdef ARDUINO_ESP8266_GENERIC
      digitalWrite(LED_BUILTIN, LED_ON);
#else
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
#endif
    }
  } // !showRaw 
  
  //---- this part is processed in 'normal' operation mode!
  if (!showRaw) 
  {
      if (slimmeMeter.available()) 
      {
        DebugTln(F("\r\n[Time----][FreeHeap/mBlck][Function----(line):\r"));
        // Voorbeeld: [21:00:11][   9880/  8960] loop        ( 997): read telegram [28] => [140307210001S]
        telegramCount++;
        
        DSMRdata = {};
        String    DSMRerror;
        
        if (slimmeMeter.parse(&DSMRdata, &DSMRerror))   // Parse succesful, print result
        {
          if (telegramCount > 1563000000) 
          {
            delay(1000);
            ESP.reset();
            delay(1000);
          }
          digitalWrite(LED_BUILTIN, LED_OFF);
          processTelegram();
          sendMQTTData();

          if (Verbose2) 
          {
            DSMRdata.applyEach(showValues());
            printData();
          }
          
        } else                  // Parser error, print error
        {
          telegramErrors++;
          DebugTf("Parse error\r\n%s\r\n\r\n", DSMRerror.c_str());
          slimmeMeter.enable(true);
        }
        
      } // if (slimmeMeter.available()) 

  }
  else   
  {
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
      if (showRawCount == 0) 
      {
        oled_Print_Msg(0, "<DSMRfirmwareAPI>", 0);
        oled_Print_Msg(1, "-------------------------",0);
        oled_Print_Msg(2, "Raw Format",0);
        sprintf(cMsg, "Raw Count %4d", showRawCount);
        oled_Print_Msg(3, cMsg, 0);
      }
#endif

      while(Serial.available() > 0) 
      {   
        char rIn = Serial.read();       
        if (rIn == '!') 
        {
          showRawCount++;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
          sprintf(cMsg, "Raw Count %4d", showRawCount);
          oled_Print_Msg(3, cMsg, 0);
#endif
        }
        TelnetStream.write((char)rIn);
      }   // while Serial.available()
      
      if (showRawCount > 20) 
      {
        showRaw       = false;
        showRawCount  = 0;
      }
  } 

} // loop()



/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
***************************************************************************/
