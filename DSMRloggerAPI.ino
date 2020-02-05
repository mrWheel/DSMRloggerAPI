/*
***************************************************************************  
**  Program  : DSMRloggerAPI (restAPI)
*/
#define _FW_VERSION "v0.3.0 (04-02-2020)"
/*
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      
  Arduino-IDE settings for DSMR-logger Version 4 (ESP-12):

    - Board: "Generic ESP8266 Module"
    - Flash mode: "DOUT" | "DIO"    // if you change from one to the other OTA may fail!
    - Flash size: "4MB (FS: 2MB OAT:~1019KB)"  
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
//  #define HAS_OLED_SSD1306          // define if a 0.96" OLED display is present
#define HAS_OLED_SH1106           // define if a 1.3" OLED display is present
//  #define HAS_NO_SLIMMEMETER        // define for testing only!
//  #define USE_PRE40_PROTOCOL        // define if Slimme Meter is pre DSMR 4.0 (2.2 .. 3.0)
//  #define USE_NTP_TIME              // define to generate Timestamp from NTP (Only Winter Time for now)
//  #define SM_HAS_NO_FASE_INFO       // if your SM does not give fase info use total delevered/returned
#define USE_MQTT                  // define if you want to use MQTT
#define USE_MINDERGAS             // define if you want to update mindergas (also add token down below)
//  #define SHOW_PASSWRDS             // well .. show the PSK key and MQTT password, what else?
/******************** don't change anything below this comment **********************/

#include "DSMRloggerAPI.h"

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
              break;
  }
  oled_Print_Msg(3, cMsg, 0);
  msgMode= (msgMode+1) % 5; //modular 5 = number of message displayed (hence it cycles thru the messages
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
  
  //setup randomseed the right way
  randomSeed(RANDOM_REG32); //This is 8266 HWRNG used to seed the Random PRNG: Read more: https://config9.com/arduino/getting-a-truly-random-number-in-arduino/
  Serial.printf("\n\nBooting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  DECLARE_TIMER_MIN(oledSleepTimer, 10);
  oled_Init();
  oled_Clear();  // clear the screen so we can paint the menu.
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
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
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  oled_Print_Msg(3, "telnet (poort 23)", 2500);
#endif  // has_oled_ssd1306

//================ SPIFFS ===========================================
  if (!SPIFFS.begin()) {
    DebugTln(F("SPIFFS Mount failed\r"));   // Serious problem with SPIFFS 
    SPIFFSmounted = false;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "SPIFFS FAILED!", 2000);
#endif  // has_oled_ssd1306
    
  } else { 
    DebugTln(F("SPIFFS Mount succesfull\r"));
    SPIFFSmounted = true;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "SPIFFS mounted", 1500);
#endif  // has_oled_ssd1306
  }

//------ read status file for last Timestamp --------------------
  strcpy(actTimestamp, "040302010101X");
  //==========================================================//
  // writeLastStatus();  // only for firsttime initialization //
  //==========================================================//
  readLastStatus(); // place it in actTimestamp
  // set the time to actTimestamp!
  actT = epoch(actTimestamp, strlen(actTimestamp), true);
  DebugTf("===> actTimestamp[%s]-> nrReboots[%u] - Errors[%u]\r\n\n", actTimestamp
                                                                    , nrReboots++
                                                                    , slotErrors);
                                                                    
  readSettings(true);

//=============now test if SPIFFS is correct populated!============
  if (DSMRfileExist(settingIndexPage, false) )
  {
    if (strcmp(settingIndexPage, "DSMRindex.html") != 0)
    {
      if (settingIndexPage[0] != '/')
      {
        char tempPage[50] = "/";
        strConcat(tempPage, 49, settingIndexPage);
        strCopy(settingIndexPage, sizeof(settingIndexPage), tempPage);
      }
      hasAlternativeIndex        = true;
    }
    else  hasAlternativeIndex        = false;
  }
  if (!hasAlternativeIndex && !DSMRfileExist("/DSMRindex.html", false) )
  {
    spiffsNotPopulated = true;
  }
  if (!hasAlternativeIndex)
  {
    DSMRfileExist("/DSMRindex.js",    false);
    DSMRfileExist("/DSMRindex.css",   false);
    DSMRfileExist("/DSMRgraphics.js", false);
    DSMRfileExist("/DSMReditor.html", false);
    DSMRfileExist("/DSMReditor.js",   false);
  }
  if (!DSMRfileExist("/FSexplorer.html", true))
  {
    spiffsNotPopulated = true;
  }
  if (!DSMRfileExist("/FSexplorer.css", true))
  {
    spiffsNotPopulated = true;
  }

//=============end SPIFFS =========================================

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Clear();  // clear the screen 
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  oled_Print_Msg(1, "Verbinden met WiFi", 500);
#endif  // has_oled_ssd1306

  digitalWrite(LED_BUILTIN, LED_ON);
  startWiFi();

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
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
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);             //USE_NTP
    oled_Print_Msg(2, "geen reactie van", 100);             //USE_NTP
    oled_Print_Msg(2, "NTP server's", 100);                 //USE_NTP 
    oled_Print_Msg(3, "Reboot DSMR-logger", 2000);          //USE_NTP
  #endif  // has_oled_ssd1306                               //USE_NTP
    delay(2000);                                            //USE_NTP
    ESP.restart();                                          //USE_NTP
    delay(3000);                                            //USE_NTP
  }                                                         //USE_NTP
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )  //USE_NTP
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);             //USE_NTP
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
  
//=============now test if "convertPRD" file exists================
  if (DSMRfileExist("!PRDconvert", false) )
  {
    convertPRD2RING();
  }

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
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  oled_Print_Msg(3, cMsg, 1500);
#endif  // has_oled_ssd1306

#ifdef USE_MQTT                                               //USE_MQTT
  startMQTT();
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )    //USE_MQTT
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);               //USE_MQTT
    oled_Print_Msg(3, "MQTT server set!", 1500);              //USE_MQTT
  #endif  // has_oled_ssd1306                                 //USE_MQTT
#endif                                                        //USE_MQTT

  telegramCount   = 0;
  telegramErrors  = 0;

  if (!spiffsNotPopulated) {
    DebugTln(F("SPIFFS correct populated -> normal operation!\r"));
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0); 
    oled_Print_Msg(1, "OK, SPIFFS correct", 0);
    oled_Print_Msg(2, "Verder met normale", 0);
    oled_Print_Msg(3, "Verwerking ;-)", 2500);
#endif  // has_oled_ssd1306
    if (hasAlternativeIndex)
    {
      httpServer.serveStatic("/",                 SPIFFS, settingIndexPage);
      httpServer.serveStatic("/index",            SPIFFS, settingIndexPage);
      httpServer.serveStatic("/index.html",       SPIFFS, settingIndexPage);
    }
    else
    {
      httpServer.serveStatic("/",                 SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/DSMRindex.html",   SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/index",            SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/index.html",       SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/DSMRindex.css",    SPIFFS, "/DSMRindex.css");
      httpServer.serveStatic("/DSMRindex.js",     SPIFFS, "/DSMRindex.js");
      httpServer.serveStatic("/DSMReditor.html",  SPIFFS, "/DSMReditor.html");
      httpServer.serveStatic("/DSMReditor.js",    SPIFFS, "/DSMReditor.js");
      httpServer.serveStatic("/DSMRgraphics.js",  SPIFFS, "/DSMRgraphics.js");
    }
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
  httpServer.serveStatic("/FSexplorer.png",   SPIFFS, "/FSexplorer.png");

  httpServer.on("/api", HTTP_GET, processAPI);
  // all other api calls are catched in FSexplorer onNotFounD!

  httpServer.begin();
  DebugTln( "HTTP server gestart\r" );
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )    //HAS_OLED
  oled_Clear();                                             //HAS_OLED
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);               //HAS_OLED
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

#if defined( IS_ESP12 ) && !defined( HAS_NO_SLIMMEMETER )
    Serial.swap();
#endif // is_esp12

  sprintf(cMsg, "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "Startup complete", 0);
    oled_Print_Msg(2, "Wait for first", 0);
    oled_Print_Msg(3, "telegram .....", 500);
#endif  // has_oled_ssd1306


} // setup()


//===========================================================================================
void loop () 
{
  // setup timers
  DECLARE_TIMER_MS(timer100ms, 100)
  DECLARE_TIMER_MS(timer1s, 1000)
  DECLARE_TIMER_MS(timer5s, 5000)
  DECLARE_TIMER_MS(timer30s, 30000)
  DECLARE_TIMER_MIN(timer10min, 10)
  DECLARE_TIMER_MIN(timer60min, 60)
  DECLARE_TIMER_SEC(timerTelegram, settingInterval)
  CHANGE_INTERVAL_MS(timerTelegram, settingInterval)
  
  // do the loop...
  loopCount++;

  // check the timers?
  if DUE(timer100ms)
    doTaskEvery100ms();

  if DUE(timer1s)
    doTaskEvery1s();

  if DUE(timer5s)
    doTaskEvery5s();

  if DUE(timer30s)
    doTaskEvery30s();

  if DUE(timerTelegram) 
    doTaskTelegram();

  // do the background tasks
  doBackgroundTasks();
  
} // loop()

//==[ Do Telegram Processing ]==
void doTaskTelegram(){
  if (showRaw) {
    //-- process telegrams in raw mode
    handleSlimmemeterRaw();
  } 
  else 
  {
    //---- this part is processed in 'normal' operation mode!
    slimmeMeter.enable(true); // enable a telegram processing from slimme meter
    blinkLEDnow();
    #if defined(HAS_NO_SLIMMEMETER)
      handleTestdata();
    #else
      handleSlimmeMeter();
    #endif
  }
}

//===[ Do task every 100ms ]===
void doTaskEvery100ms(){
  //== do tasks ==
}

//===[ Do task every 1s ]===
void doTaskEvery1s(){
  //== do tasks ==
  upTimeSeconds++;
}

//===[ Do task every 5s ]===
void doTaskEvery5s(){
  //== do tasks ==
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    displayStatus();
  #endif
}

//===[ Do task every 30s ]===
void doTaskEvery30s(){
  //== do tasks ==
  #if defined(USE_NTP_TIME)                                                         //USE_NTP
  if (timeStatus() == timeNeedsSync || prevNtpHour != hour())                     //USE_NTP
  {                                                                               //USE_NTP
    prevNtpHour = hour();                                                         //USE_NTP
    setSyncProvider(getNtpTime);                                                  //USE_NTP
    setSyncInterval(600);                                                         //USE_NTP
  }                                                                               //USE_NTP
  #endif                                                                            //USE_NTP
}

//===[ Do the background tasks ]===
void doBackgroundTasks()
{
  httpServer.handleClient();
  MDNS.update();
  handleKeyInput();
  handleMQTT();                 // MQTT transmissions
  handleMindergas();            // Mindergas update 
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  checkFlashButton();
#endif
  blinkLEDms(1000);               // 'blink' the status led every x ms
  yield();

}

//===[ blink status led in ms ]===
void blinkLEDms(uint32_t iDelay){
  //blink the statusled, when time passed... #non-blocking blink
  DECLARE_TIMER_MS(timerBlink, iDelay);
  CHANGE_INTERVAL_MS(timer, delay_ms);
  if (DUE(timerBlink))
    blinkLEDnow();
}

//===[ blink status now ]===
void blinkLEDnow(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//===[ no-blocking delay with running background tasks in ms ]===
void delayms(unsigned long delay_ms)
{
  DECLARE_TIMER_MS(timer, delay_ms);
  CHANGE_INTERVAL_MS(timer, delay_ms);
  while (!DUE(timer))
    doBackgroundTasks();
}
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
