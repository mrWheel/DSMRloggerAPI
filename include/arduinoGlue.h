/*** Last Changed: 2026-02-28 - 17:12 ***/
#ifndef ARDUINOGLUE_H
#define ARDUINOGLUE_H

//============ Includes ====================
#include <Arduino.h>
#include <WiFiUdp.h>
#include <list>
#include <tuple>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <FS.h>
#include <LittleFS.h>
#include <TimeLib.h>
#include <TelnetStream.h>
#include <dsmr2.h>
#include <PubSubClient.h>

//============ Defines & Macros====================
#define MAXLINELENGTH 128   // longest normal line is 47 char (+3 for \r\n\0)
#define TELEGRAM_INTERVAL 5 // seconds
#define MG_FILENAME "/Mindergas.post"
extern const char* PROG_VERSION;
#define USE_LITTLEFS      // if not #defined: use SPIFFS
#define USE_UPDATE_SERVER // define if there is enough memory and updateServer to be used
#define USE_MQTT          // define if you want to use MQTT (configure through webinterface)
#define USE_MINDERGAS     // define if you want to update mindergas (configure through webinterface)
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
#define Debug(...) ({              \
  Serial.print(__VA_ARGS__);       \
  TelnetStream.print(__VA_ARGS__); \
})
#define Debugln(...) ({              \
  Serial.println(__VA_ARGS__);       \
  TelnetStream.println(__VA_ARGS__); \
})
#define Debugf(...) ({              \
  Serial.printf(__VA_ARGS__);       \
  TelnetStream.printf(__VA_ARGS__); \
})
#define DebugFlush() ({ \
  Serial.flush();       \
  TelnetStream.flush(); \
})
// #define DebugT(...)     ({ _debugBOL(__FUNCTION__, __LINE__);
#define DebugT(...) ({ \
  Debug(__VA_ARGS__);  \
})
#define DebugTln(...) ({ \
  Debugln(__VA_ARGS__);  \
})
#define DebugTf(...) ({ \
  Debugf(__VA_ARGS__);  \
})
#define FSYS LittleFS
#define writeToSysLog(...) ({ sysLog.writeDbg(sysLog.buildD("[%02d:%02d:%02d][%7d][%-12.12s] ", hour(), minute(), second(), ESP.getFreeHeap(), __FUNCTION__), __VA_ARGS__); })
#define writeToSysLog(...) // nothing
#define _DEFAULT_HOSTNAME "DSMR-API"
#define DTR_ENABLE 12
#define SETTINGS_FILE "/DSMRsettings.ini"
#define LED_ON LOW
#define LED_OFF HIGH
#define FLASH_BUTTON 0
#define MAXCOLORNAME 15
#define JSON_BUFF_MAX 255
#define MQTT_BUFF_MAX 200
#define MAX_TLGRM_LENGTH 1200
#define DATA_FORMAT "%-8.8s;%10.3f;%10.3f;%10.3f;%10.3f;%10.3f;\n"
#define DATA_CSV_HEADER "YYMMDDHH;      EDT1;      EDT2;      ERT1;      ERT2;       GDT;"
#define DATA_RECLEN 75
#define HOURS_FILE "/RINGhours.csv"
#define _NO_HOUR_SLOTS_ (48 + 1)
#define DAYS_FILE "/RINGdays.csv"
#define _NO_DAY_SLOTS_ (14 + 1)
#define MONTHS_FILE "/RINGmonths.csv"
#define _NO_MONTH_SLOTS_ (24 + 1)
#define SKIP_MISSED_TICKS 0
#define SKIP_MISSED_TICKS_WITH_SYNC 1
#define CATCH_UP_MISSED_TICKS 2
#define DECLARE_TIMER_EXTERN(timerName, ...) \
  extern uint32_t timerName##_interval;      \
  extern uint32_t timerName##_due;           \
  extern byte timerName##_type;

#define DECLARE_TIMER_MIN(timerName, ...)                                                        \
  uint32_t timerName##_interval = (getParam(0, __VA_ARGS__, 0) * 1000 * 60),                     \
           timerName##_due = millis() + timerName##_interval + random(timerName##_interval / 3); \
  byte timerName##_type = getParam(1, __VA_ARGS__, 0);

#define DECLARE_TIMER_SEC(timerName, ...)                                                        \
  uint32_t timerName##_interval = (getParam(0, __VA_ARGS__, 0) * 1000),                          \
           timerName##_due = millis() + timerName##_interval + random(timerName##_interval / 3); \
  byte timerName##_type = getParam(1, __VA_ARGS__, 0);

#define DECLARE_TIMER_MS(timerName, ...)                                                         \
  uint32_t timerName##_interval = (getParam(0, __VA_ARGS__, 0)),                                 \
           timerName##_due = millis() + timerName##_interval + random(timerName##_interval / 3); \
  byte timerName##_type = getParam(1, __VA_ARGS__, 0);

#define DECLARE_TIMER DECLARE_TIMER_MS
#define CHANGE_INTERVAL_MIN(timerName, ...)                         \
  timerName##_interval = (getParam(0, __VA_ARGS__, 0) * 60 * 1000); \
  timerName##_due = millis() + timerName##_interval;
#define CHANGE_INTERVAL_SEC(timerName, ...)                    \
  timerName##_interval = (getParam(0, __VA_ARGS__, 0) * 1000); \
  timerName##_due = millis() + timerName##_interval;
#define CHANGE_INTERVAL_MS(timerName, ...)              \
  timerName##_interval = (getParam(0, __VA_ARGS__, 0)); \
  timerName##_due = millis() + timerName##_interval;
#define CHANGE_INTERVAL CHANGE_INTERVAL_MS
#define TIME_LEFT(timerName) (__TimeLeft__(timerName##_due))
#define TIME_LEFT_MS(timerName) ((TIME_LEFT(timerName)))
#define TIME_LEFT_MIN(timerName) ((TIME_LEFT(timerName)) / (60 * 1000))
#define TIME_LEFT_SEC(timerName) ((TIME_LEFT(timerName)) / 1000)
#define TIME_PAST(timerName) ((timerName##_interval - TIME_LEFT(timerName)))
#define TIME_PAST_MS(timerName)       ( (TIME_PAST(timerName) )
#define TIME_PAST_SEC(timerName) ((TIME_PAST(timerName) / 1000))
#define TIME_PAST_MIN(timerName) ((TIME_PAST(timerName) / (60 * 1000)))
#define RESTART_TIMER(timerName) (timerName##_due = millis() + timerName##_interval);
#define DUE(timerName) (__Due__(timerName##_due, timerName##_interval, timerName##_type))

/**
 * Define the DSMRdata we're interested in, as well as the DSMRdatastructure to
 * hold the parsed DSMRdata. This list shows all supported fields, remove
 * any fields you are not using from the below list to make the parsing
 * and printing code smaller.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
    /* String */ identification
    /* String */,
    p1_version
    /* String */,
    p1_version_be
    /* String */,
    timestamp
    /* String */,
    equipment_id
    /* FixedValue */,
    energy_delivered_tariff1
    /* FixedValue */,
    energy_delivered_tariff2
    /* FixedValue */,
    energy_returned_tariff1
    /* FixedValue */,
    energy_returned_tariff2
    /* String */,
    electricity_tariff
    /* FixedValue */,
    power_delivered
    /* FixedValue */,
    power_returned
    /* FixedValue */,
    electricity_threshold
    /* uint8_t */,
    electricity_switch_position
    /* uint32_t */,
    electricity_failures
    /* uint32_t */,
    electricity_long_failures
    /* String */,
    electricity_failure_log
    /* uint32_t */,
    electricity_sags_l1
    /* uint32_t */,
    electricity_sags_l2
    /* uint32_t */,
    electricity_sags_l3
    /* uint32_t */,
    electricity_swells_l1
    /* uint32_t */,
    electricity_swells_l2
    /* uint32_t */,
    electricity_swells_l3
    /* String */,
    message_short
    /* String */ //         ,message_long // this one is too big and will crash the MCU
    /* FixedValue */,
    voltage_l1
    /* FixedValue */,
    voltage_l2
    /* FixedValue */,
    voltage_l3
    /* FixedValue */,
    current_l1
    /* FixedValue */,
    current_l2
    /* FixedValue */,
    current_l3
    /* FixedValue */,
    power_delivered_l1
    /* FixedValue */,
    power_delivered_l2
    /* FixedValue */,
    power_delivered_l3
    /* FixedValue */,
    power_returned_l1
    /* FixedValue */,
    power_returned_l2
    /* FixedValue */,
    power_returned_l3
    /* uint16_t */,
    mbus1_device_type
    /* String */,
    mbus1_equipment_id_tc
    /* String */,
    mbus1_equipment_id_ntc
    /* uint8_t */,
    mbus1_valve_position
    /* TimestampedFixedValue */,
    mbus1_delivered
    /* TimestampedFixedValue */,
    mbus1_delivered_ntc
    /* TimestampedFixedValue */,
    mbus1_delivered_dbl
    /* uint16_t */,
    mbus2_device_type
    /* String */,
    mbus2_equipment_id_tc
    /* String */,
    mbus2_equipment_id_ntc
    /* uint8_t */,
    mbus2_valve_position
    /* TimestampedFixedValue */,
    mbus2_delivered
    /* TimestampedFixedValue */,
    mbus2_delivered_ntc
    /* TimestampedFixedValue */,
    mbus2_delivered_dbl
    /* uint16_t */,
    mbus3_device_type
    /* String */,
    mbus3_equipment_id_tc
    /* String */,
    mbus3_equipment_id_ntc
    /* uint8_t */,
    mbus3_valve_position
    /* TimestampedFixedValue */,
    mbus3_delivered
    /* TimestampedFixedValue */,
    mbus3_delivered_ntc
    /* TimestampedFixedValue */,
    mbus3_delivered_dbl
    /* uint16_t */,
    mbus4_device_type
    /* String */,
    mbus4_equipment_id_tc
    /* String */,
    mbus4_equipment_id_ntc
    /* uint8_t */,
    mbus4_valve_position
    /* TimestampedFixedValue */,
    mbus4_delivered
    /* TimestampedFixedValue */,
    mbus4_delivered_ntc
    /* TimestampedFixedValue */,
    mbus4_delivered_dbl>;

extern MyData DSMRdata;

//============ Structs, Unions & Enums ============
//-- from DSMRloggerAPI.h
enum
{
  PERIOD_UNKNOWN,
  HOURS,
  DAYS,
  MONTHS,
  YEARS
};

//-- from DSMRloggerAPI.h
enum
{
  TAB_UNKNOWN,
  TAB_ACTUEEL,
  TAB_LAST24HOURS,
  TAB_LAST7DAYS,
  TAB_LAST24MONTHS,
  TAB_GRAPHICS,
  TAB_SYSINFO,
  TAB_EDITOR
};

//-- from DSMRloggerAPI.h
typedef struct
{
  uint32_t Label;
  float EDT1;
  float EDT2;
  float ERT1;
  float ERT2;
  float GDT;
} dataStruct;

//-- from DSMRloggerAPI.ino
// Item& typecastValue(Item &i);
//=======================================================================
template <typename Item>
Item& typecastValue(Item& i)
{
  return i;
}

float typecastValue(TimestampedFixedValue i);
float typecastValue(FixedValue i);
float typecastValue(uint16_t i); // Added for uint16_t
float typecastValue(uint8_t i);  // Optional, in case uint8_t is used
float typecastValue(int i);      // Optional, in case int is used
float typecastValue(float i);    // For float, in case it's already float
float typecastValue(String& i);  // Added for String

struct showValues
{
  template <typename Item>
  void apply(Item& i)
  {
    if (i.present())
    {
      DebugT(Item::name);
      Debug(F(": "));
      Debug(i.val());
      Debug(Item::unit());
    }
    Debugln();
  }
};

//-- from MinderGas.ino
enum states_of_MG
{
  MG_INIT,
  MG_WAIT_FOR_FIRST_TELEGRAM,
  MG_WAIT_FOR_NEXT_DAY,
  MG_WRITE_TO_FILE,
  MG_DO_COUNTDOWN,
  MG_SEND_MINDERGAS,
  MG_NO_AUTHTOKEN,
  MG_ERROR
};

//-- from handleTestdata.ino
enum runStates
{
  SInit,
  SMonth,
  SDay,
  SHour,
  SNormal
};

//-- from MQTTstuff.ino
enum states_of_MQTT
{
  MQTT_STATE_INIT,
  MQTT_STATE_TRY_TO_CONNECT,
  MQTT_STATE_IS_CONNECTED,
  MQTT_STATE_ERROR
};

//-- from MQTTstuff.ino
extern char settingMQTTtopTopic[21]; //-- from DSMRloggerAPI
extern PubSubClient MQTTclient;      //-- from DSMRloggerAPI
extern bool Verbose1;                //-- from DSMRloggerAPI
extern bool Verbose2;                //-- from DSMRloggerAPI
extern char mqttBuff[100];           //-- from MQTTstuff
void strConcat(char* dest, int maxLen, const char* src);
void strConcat(char* dest, int maxLen, float v, int dec);
void strConcat(char* dest, int maxLen, int32_t v);
void processTelegram();
//-- from jsonStuff.ino -----------
void sendStartJsonObj(const char* objName);
void sendEndJsonObj();
void sendNestedJsonObj(uint8_t recNr, const char* recID, uint8_t slot, float EDT1, float EDT2, float ERT1, float ERT2, float GDT);
void sendNestedJsonObj(const char* cName, const char* cValue, const char* cUnit);
void sendNestedJsonObj(const char* cName, const char* cValue);
void sendNestedJsonObj(const char* cName, String sValue, const char* cUnit);
void sendNestedJsonObj(const char* cName, String sValue);
void sendNestedJsonObj(const char* cName, int32_t iValue, const char* cUnit);
void sendNestedJsonObj(const char* cName, int32_t iValue);
void sendNestedJsonObj(const char* cName, uint32_t uValue, const char* cUnit);
void sendNestedJsonObj(const char* cName, uint32_t uValue);
void sendNestedJsonObj(const char* cName, float fValue, const char* cUnit);
void sendNestedJsonObj(const char* cName, float fValue);
void sendNestedJsonV0Obj(const char* cName, uint32_t uValue);
void sendNestedJsonV0Obj(const char* cName, float fValue);
void sendNestedJsonV0Obj(const char* cName, int32_t iValue);
void sendNestedJsonV0Obj(const char* cName, String sValue);
void sendJsonSettingObj(const char* cName, float fValue, const char* fType, int minValue, int maxValue);
void sendJsonSettingObj(const char* cName, float fValue, const char* fType, int minValue, int maxValue, int decPlaces);
void sendJsonSettingObj(const char* cName, int iValue, const char* iType, int minValue, int maxValue);
void sendJsonSettingObj(const char* cName, const char* cValue, const char* sType, int maxLen);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, const char* cValue, const char* cUnit);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, const char* cValue);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, String sValue, const char* cUnit);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, String sValue);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, int32_t iValue, const char* cUnit);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, int32_t iValue);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, uint32_t uValue, const char* cUnit);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, uint32_t uValue);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, float fValue, const char* cUnit);
void createMQTTjsonMessage(char* mqttBuff, const char* cName, float fValue);

struct buildJsonMQTT
{
#ifdef USE_MQTT

  char topicId[100];

  template <typename Item>
  void apply(Item& i)
  {
    if (i.present())
    {
      String Name = String(Item::name);
      String Unit = Item::unit();

      if (settingMQTTtopTopic[strlen(settingMQTTtopTopic) - 1] == '/')
        snprintf(topicId, sizeof(topicId), "%s", settingMQTTtopTopic);
      else
        snprintf(topicId, sizeof(topicId), "%s/", settingMQTTtopTopic);
      strConcat(topicId, sizeof(topicId), Name.c_str());
      if (Verbose2)
        DebugTf("topicId[%s]\r\n", topicId);

      if (Unit.length() > 0)
      {
        createMQTTjsonMessage(mqttBuff, Name.c_str(), typecastValue(i.val()), Unit.c_str());
      }
      else
      {
        createMQTTjsonMessage(mqttBuff, Name.c_str(), typecastValue(i.val()));
      }

      // snprintf(cMsg, sizeof(cMsg), "%s", jsonString.c_str());
      // DebugTf("topicId[%s] -> [%s]\r\n", topicId, mqttBuff);
      if (!MQTTclient.publish(topicId, mqttBuff))
      {
        DebugTf("Error publish(%s) [%s] [%d bytes]\r\n", topicId, mqttBuff, (strlen(topicId) + strlen(mqttBuff)));
      }
    }
  }
#endif
};

//============ Extern Variables ============
// extern MyData          DSMRdata;                          		//-- from DSMRloggerAPI
extern bool DSTactive;                  //-- from DSMRloggerAPI
extern bool FSYSnotPopulated;           //-- from DSMRloggerAPI
extern time_t actT;                     //-- from DSMRloggerAPI
extern char actTimestamp[20];           //-- from DSMRloggerAPI
extern char cMsg[150];                  //-- from DSMRloggerAPI
extern dataStruct dayData;              //-- from DSMRloggerAPI
extern bool doLog;                      //-- from DSMRloggerAPI
extern char fChar[10];                  //-- from DSMRloggerAPI
extern const char* flashMode[];         //-- from DSMRloggerAPI
extern float gasDelivered;              //-- from DSMRloggerAPI
extern bool hasAlternativeIndex;        //-- from DSMRloggerAPI
extern dataStruct hourData;             //-- from DSMRloggerAPI
extern uint16_t intStatuscodeMindergas; //-- from DSMRloggerAPI
extern IPAddress ipDNS;                 //-- from DSMRloggerAPI
extern IPAddress ipGateWay;             //-- from DSMRloggerAPI
extern IPAddress ipSubnet;              //-- from DSMRloggerAPI
extern int8_t lastMonth;                //-- from DSMRloggerAPI
extern String lastReset;                //-- from DSMRloggerAPI
extern uint32_t loopCount;              //-- from DSMRloggerAPI
extern dataStruct monthData;            //-- from DSMRloggerAPI
extern const char* monthName[];         //-- from DSMRloggerAPI
extern bool mqttIsConnected;            //-- from DSMRloggerAPI
extern time_t newT;                     //-- from DSMRloggerAPI
extern char newTimestamp[20];           //-- from DSMRloggerAPI
extern uint32_t nrReboots;              //-- from DSMRloggerAPI
extern time_t ntpTime;                  //-- from DSMRloggerAPI
extern String pTimestamp;               //-- from DSMRloggerAPI
extern int8_t prevNtpHour;              //-- from DSMRloggerAPI
extern uint32_t readTimer;              //-- from DSMRloggerAPI
extern uint8_t settingDailyReboot;      //-- from DSMRloggerAPI
extern float settingEDT1;               //-- from DSMRloggerAPI
extern float settingEDT2;               //-- from DSMRloggerAPI
extern float settingENBK;               //-- from DSMRloggerAPI
extern float settingERT1;               //-- from DSMRloggerAPI
extern float settingERT2;               //-- from DSMRloggerAPI
extern float settingGDT;                //-- from DSMRloggerAPI
extern float settingGNBK;               //-- from DSMRloggerAPI
extern char settingHostname[30];        //-- from DSMRloggerAPI
extern char settingIndexPage[50];       //-- from DSMRloggerAPI
extern int32_t settingMQTTbrokerPort;   //-- from DSMRloggerAPI
extern char settingMQTTbroker[101];     //-- from DSMRloggerAPI
extern int32_t settingMQTTinterval;     //-- from DSMRloggerAPI
extern char settingMQTTpasswd[40];      //-- from DSMRloggerAPI
extern char settingMQTTuser[40];        //-- from DSMRloggerAPI
extern uint8_t settingMbus1Type;        //-- from DSMRloggerAPI
extern uint8_t settingMbus2Type;        //-- from DSMRloggerAPI
extern uint8_t settingMbus3Type;        //-- from DSMRloggerAPI
extern uint8_t settingMbus4Type;        //-- from DSMRloggerAPI
extern char settingMindergasToken[21];  //-- from DSMRloggerAPI
extern uint8_t settingPreDSMR40;        //-- from DSMRloggerAPI
extern uint8_t settingSmHasFaseInfo;    //-- from DSMRloggerAPI
extern uint8_t settingTelegramInterval; //-- from DSMRloggerAPI
extern bool showRaw;                    //-- from DSMRloggerAPI
extern int8_t showRawCount;             //-- from DSMRloggerAPI
extern P1Reader slimmeMeter;            //-- from DSMRloggerAPI
extern uint32_t slotErrors;             //-- from DSMRloggerAPI
// extern int             strcicmp;                          		//-- from DSMRloggerAPI
// extern ESPSL           sysLog;                            		//-- from DSMRloggerAPI
extern uint32_t telegramCount;           //-- from DSMRloggerAPI
extern uint32_t telegramErrors;          //-- from DSMRloggerAPI
extern int8_t thisDay;                   //-- from DSMRloggerAPI
extern int8_t thisHour;                  //-- from DSMRloggerAPI
extern int8_t thisMonth;                 //-- from DSMRloggerAPI
extern int8_t thisYear;                  //-- from DSMRloggerAPI
extern char timeLastResponse[16];        //-- from DSMRloggerAPI
extern char txtResponseMindergas[30];    //-- from DSMRloggerAPI
extern uint32_t unixTimestamp;           //-- from DSMRloggerAPI
extern uint64_t upTimeSeconds;           //-- from DSMRloggerAPI
extern const char* weekDayName[];        //-- from DSMRloggerAPI
extern WiFiClient wifiClient;            //-- from DSMRloggerAPI
extern char _bol[128];                   //-- from Debug
extern int16_t bytesWritten;             //-- from FSYSstuff
extern IPAddress MQTTbrokerIP;           //-- from MQTTstuff
extern char MQTTbrokerIPchar[20];        //-- from MQTTstuff
extern String MQTTclientId;              //-- from MQTTstuff
extern char lastMQTTtimestamp[15];       //-- from MQTTstuff
extern int8_t reconnectAttempts;         //-- from MQTTstuff
extern int8_t MG_Day;                    //-- from MinderGas
extern int8_t MGminuten;                 //-- from MinderGas
extern bool bDoneResponse;               //-- from MinderGas
extern bool handleMindergasSemaphore;    //-- from MinderGas
extern int8_t retryCounter;              //-- from MinderGas
extern bool validToken;                  //-- from MinderGas
extern float CUR_l1;                     //-- from handleTestdata
extern float CUR_l2;                     //-- from handleTestdata
extern float CUR_l3;                     //-- from handleTestdata
extern double C_l1;                      //-- from handleTestdata
extern double C_l2;                      //-- from handleTestdata
extern double C_l3;                      //-- from handleTestdata
extern double ED_T1;                     //-- from handleTestdata
extern double ED_T2;                     //-- from handleTestdata
extern double ER_T1;                     //-- from handleTestdata
extern double ER_T2;                     //-- from handleTestdata
extern uint8_t ETariffInd;               //-- from handleTestdata
extern float GDelivered;                 //-- from handleTestdata
extern float IPD_l1;                     //-- from handleTestdata
extern float IPD_l2;                     //-- from handleTestdata
extern float IPD_l3;                     //-- from handleTestdata
extern float IPR_l1;                     //-- from handleTestdata
extern float IPR_l2;                     //-- from handleTestdata
extern float IPR_l3;                     //-- from handleTestdata
extern float PDelivered;                 //-- from handleTestdata
extern float PReturned;                  //-- from handleTestdata
extern int8_t State;                     //-- from handleTestdata
extern double V_l1;                      //-- from handleTestdata
extern double V_l2;                      //-- from handleTestdata
extern double V_l3;                      //-- from handleTestdata
extern char actDSMR[3];                  //-- from handleTestdata
extern int16_t actDay;                   //-- from handleTestdata
extern int16_t actHour;                  //-- from handleTestdata
extern uint32_t actInterval;             //-- from handleTestdata
extern int16_t actMinute;                //-- from handleTestdata
extern int16_t actMonth;                 //-- from handleTestdata
extern int16_t actSec;                   //-- from handleTestdata
extern int16_t actSpeed;                 //-- from handleTestdata
extern int16_t actYear;                  //-- from handleTestdata
extern int16_t calcCRC;                  //-- from handleTestdata
extern uint16_t currentCRC;              //-- from handleTestdata
extern int16_t forceBuildRecs;           //-- from handleTestdata
extern bool forceBuildRingFiles;         //-- from handleTestdata
extern uint32_t nextESPcheck;            //-- from handleTestdata
extern uint32_t nextGuiUpdate;           //-- from handleTestdata
extern uint32_t nextMinute;              //-- from handleTestdata
extern char savDSMR[3];                  //-- from handleTestdata
extern char telegramLine[MAXLINELENGTH]; //-- from handleTestdata
extern char telegram[MAX_TLGRM_LENGTH];  //-- from handleTestdata
extern int16_t testTlgrmLines;           //-- from handleTestdata
extern char noUnit[];                    //-- from jsonStuff
extern bool FSYSmounted;                 //-- from networkStuff
// extern FSInfo          SPIFFSinfo;                        		//-- from networkStuff
extern ESP8266WebServer httpServer; //-- from networkStuff
// extern ESP8266HTTPUpdateServer httpUpdater;                       		//-- from networkStuff
extern bool isConnected; //-- from networkStuff
// extern const int       NTP_PACKET_SIZE;                   		//-- from ntpStuff
extern int8_t UTCtime; //-- from ntpStuff
extern WiFiUDP Udp;    //-- from ntpStuff
// extern bool            externalNtpTime;                   		//-- from ntpStuff
// extern int             ntpPoolIndx;                       		//-- from ntpStuff
// extern IPAddress       ntpServerIP;                       		//-- from ntpStuff
extern char ntpServerName[50]; //-- from ntpStuff
// extern int             ntpServerNr;                       		//-- from ntpStuff
extern byte packetBuffer[]; //-- from ntpStuff
extern bool boolDisplay;    //-- from oledStuff
extern uint8_t charHeight;  //-- from oledStuff
extern uint8_t lineHeight;  //-- from oledStuff
extern uint8_t msgMode;     //-- from oledStuff
// extern SSD1306AsciiWire oled;                              		//-- from oledStuff
// extern void            oled_Print_Msg;                    		//-- from oledStuff
extern uint8_t settingOledFlip;   //-- from oledStuff
extern uint16_t settingOledSleep; //-- from oledStuff
extern uint8_t settingOledType;   //-- from oledStuff
extern int actualElements;        //-- from restAPI
extern uint32_t antiWearTimer;    //-- from restAPI
extern char fieldName[40];        //-- from restAPI
extern char fieldsArray[50][35];  //-- from restAPI
extern int fieldsElements;        //-- from restAPI
extern int infoElements;          //-- from restAPI
extern bool onlyIfPresent;        //-- from restAPI

//============ Function Prototypes =========
//-- from ntpStuff.ino -----------
bool startNTP();
time_t getNtpTime();
void check4DST(time_t t);
void sendNTPpacket(IPAddress& address);
time_t dateTime2Epoch(char const* date, char const* time);
//-- from MQTTstuff.ino -----------
void connectMQTT();
bool connectMQTT_FSM();
String trimVal(char* in);
void sendMQTTData();
//-- from handleTestdata.ino -----------
void handleTestdata();
int16_t buildTelegram(int16_t line, char telegramLine[]);
int16_t buildTelegram30(int16_t line, char telegramLine[]);
void updateMeterValues(uint8_t period);
String Format(double x, int len, int d);
int FindCharInArrayRev(unsigned char array[], char c, int len);
int16_t decodeTelegram(int len);
unsigned int CRC16(unsigned int crc, unsigned char* buf, int len);
//-- from handleSlimmeMeter.ino -----------
void handleSlimmemeter();
void processSlimmemeterRaw();
void processSlimmemeter();
void modifySmFaseInfo();
float modifyMbusDelivered();
//-- from timeStuff.ino -----------
String buildDateTimeString(const char* timeStamp, int len);
void epochToTimestamp(time_t t, char* ts, int8_t len);
int8_t SecondFromTimestamp(const char* timeStamp);
int8_t MinuteFromTimestamp(const char* timeStamp);
int8_t HourFromTimestamp(const char* timeStamp);
int8_t DayFromTimestamp(const char* timeStamp);
int8_t MonthFromTimestamp(const char* timeStamp);
int8_t YearFromTimestamp(const char* timeStamp);
int32_t HoursKeyTimestamp(const char* timeStamp);
time_t epoch(const char* timeStamp, int8_t len, bool syncTime);
//-- from convertPRD2RING.ino -----------
void convertPRD2RING();
void convertPRDfile(int8_t fileType);
void writeToRINGfile(int8_t fileType, const char* key, float EDT1, float EDT2, float ERT1, float ERT2, float GDT);
//-- from FSmanager.ino -----------
void setupFsManager();
bool handleList();
void deleteRecursive(const String& path);
bool handleFile(String&& path);
void handleUpload();
void formatFS();
void listFS();
void sendResponce();
const String formatBytes(size_t const& bytes);
const String& contentType(String& filename);
void updateFirmware();
void reBootESP();
void doRedirect(String msg, int wait, const char* URL, bool reboot);
//-- from MinderGas.ino -----------
void handleMindergas();
void forceMindergasUpdate();
void processMindergas_FSM();
boolean sendMindergasPostFile();
void writePostToFile();
//-- from DSMRloggerAPI.ino -----------
void displayStatus();
void openSysLog(bool empty);
void delayms(unsigned long delay_ms);
void doTaskTelegram();
void doSystemTasks();
// if DUE(nextTelegram);
// if DUE(updateSeconds);
// if DUE(updateDisplay);
// if DUE(synchrNTP);
//-- from processTelegram.ino -----------
//-- from settingsStuff.ino -----------
void writeSettings();
void readSettings(bool show);
void updateSetting(const char* field, const char* newValue);
//-- from restAPI.ino -----------
void processAPI();
void handleDevV1Api(const char* URI, const char* word4, const char* word5, const char* word6);
void handleHistV1Api(const char* URI, const char* word4, const char* word5, const char* word6);
void handleSmV1Api(const char* URI, const char* word4, const char* word5, const char* word6);
void sendDeviceInfo();
void sendDeviceTime();
void sendDeviceSettings();
void sendDeviceDebug(const char* URI, String tail);
//-- from arduinoGlue.h -----------
// void apply(Item &i);
//-- from restAPI.ino -----------
void sendJsonV0Fields();
void sendJsonV1Fields(const char* Name);
void sendJsonHist(int8_t fileType, const char* fileName, const char* timeStamp, bool desc);
bool isInFieldsArray(const char* lookUp, int elemts);
void copyToFieldsArray(const char inArray[][35], int elemts);
void listFieldsArray(char inArray[][35]);
void sendApiNotFound(const char* URI);
//-- from helperStuff.ino -----------
int readSerialUntil(Stream& uartOut, char cEnd, uint32_t waitT, bool doEcho);
bool compare(String x, String y);
boolean isValidIP(IPAddress ip);
bool isNumericp(const char* timeStamp, int8_t len);
int8_t splitString(String inStrng, char delimiter, String wOut[], uint8_t maxWords);
String upTime();
void strToLower(char* src);
void strCopy(char* dest, int maxLen, const char* src, uint8_t frm, uint8_t to);
void strCopy(char* dest, int maxLen, const char* src);
int stricmp(const char* a, const char* b);
char* intToStr(int32_t v);
char* floatToStr(float v, int dec);
float formatFloat(float v, int dec);
float strToFloat(const char* s, int dec);
//--Item &typecastValue(Item &i);
float typecastValue(TimestampedFixedValue i);
float typecastValue(FixedValue i);
//-- from menuStuff.ino -----------
void displayHoursHist(bool Telnet = true);
void displayDaysHist(bool Telnet = true);
void displayMonthsHist(bool Telnet = true);
void displayBoardInfo();
void handleKeyInput();
//-- from FSYSstuff.ino -----------
void readLastStatus();
void writeLastStatus();
void buildDataRecordFromSM(char* recIn);
uint16_t buildDataRecordFromJson(char* recIn, String jsonIn);
void writeDataToFile(const char* fileName, const char* record, uint16_t slot, int8_t fileType);
void writeDataToFiles();
void readOneSlot(int8_t fileType, const char* fileName, uint8_t recNr, uint8_t readSlot, bool doJson, const char* rName);
void readSlotFromTimestamp(int8_t fileType, const char* fileName, const char* timeStamp, bool doJson, const char* rName);
void readAllSlots(int8_t fileType, const char* fileName, const char* timeStamp, bool doJson, const char* rName);
bool createFile(const char* fileName, uint16_t noSlots);
void fillRecord(char* record, int8_t len);
uint16_t timestampToHourSlot(const char* TS, int8_t len);
uint16_t timestampToDaySlot(const char* TS, int8_t len);
uint16_t timestampToMonthSlot(const char* TS, int8_t len);
int32_t freeSpace();
void listFSYS();
void eraseFile();
bool DSMRfileExist(const char* fileName, bool doDisplay);
//-- from oledStuff.h -----------
void checkFlashButton();
void oled_Init();
void oled_Clear();
void oled_Print_Msg(uint8_t line, String message, uint16_t wait);
//-- from Debug.h -----------
// void _debugBOL(const char *fn, int line);
//-- from networkStuff.h -----------
void configModeCallback(WiFiManager* myWiFiManager);
void startWiFi(const char* hostname, int timeOut);
void startTelnet();
void startMDNS(const char* Hostname);
//-- from safeTimers.h -----------
uint32_t __Due__(uint32_t& timer_due, uint32_t timer_interval, byte timerType);
uint32_t __TimeLeft__(uint32_t timer_due);
uint32_t getParam(int i, ...);

#endif // ARDUINOGLUE_H
