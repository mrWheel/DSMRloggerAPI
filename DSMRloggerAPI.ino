/*
***************************************************************************  
**  Program  : DSMRloggerAPI (restAPI)
*/
#define _FW_VERSION "v1.1.1 (21-03-2020)"
/*
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      
  Arduino-IDE settings for DSMR-logger Version 4 (ESP-12):

    - Board: "Generic ESP8266 Module"
    - Flash mode: "DOUT" | "DIO"    // change only after power-off and on again!
    - Flash size: "4MB (FS: 2MB OAT:~1019KB)"  << LET OP! 2MB SPIFFS
    - DebugT port: "Disabled"
    - DebugT Level: "None"
    - IwIP Variant: "v2 Lower Memory"
    - Reset Method: "none"   // but will depend on the programmer!
    - Crystal Frequency: "26 MHz" 
    - VTables: "Flash"
    - Flash Frequency: "40MHz"
    - CPU Frequency: "80 MHz"
    - Buildin Led: "2"  // GPIO02 for Wemos and ESP-12
    - Upload Speed: "115200"                                                                                                                                                                                                                                                 
    - Erase Flash: "Only Sketch"
    - Port: <select correct port>
*/
/*
**  You can find more info in the following links (all in Dutch): 
**   https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/
**   https://mrwheel-docs.gitbook.io/dsmrloggerapi/
**   https://mrwheel.github.io/DSMRloggerWS/
*/
/******************** compiler options  ********************************************/
#define USE_REQUEST_PIN           // define if it's a esp8266 with GPIO 12 connected to SM DTR pin
#define USE_UPDATE_SERVER         // define if there is enough memory and updateServer to be used
#define HAS_OLED_SSD1306          // define if a 0.96" OLED display is present
//  #define HAS_OLED_SH1106           // define if a 1.3" OLED display is present
//  #define USE_BELGIUM_PROTOCOL      // define if Slimme Meter is a Belgium Smart Meter
//  #define USE_PRE40_PROTOCOL        // define if Slimme Meter is pre DSMR 4.0 (2.2 .. 3.0)
//  #define USE_NTP_TIME              // define to generate Timestamp from NTP (Only Winter Time for now)
//  #define SM_HAS_NO_FASE_INFO       // if your SM does not give fase info use total delivered/returned
//  #define HAS_NO_SLIMMEMETER        // define for testing only!
#define USE_MQTT                  // define if you want to use MQTT (configure through webinterface)
#define USE_MINDERGAS             // define if you want to update mindergas (configure through webinterface)
//  #define USE_SYSLOGGER             // define if you want to use the sysLog library for debugging
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
    //} else 
    //{
    //  TelnetStream.print(F("<no value>"));
    }
    TelnetStream.println();
  }
};


//===========================================================================================
void displayStatus() 
{
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  switch(msgMode) { 
    case 1:   snprintf(cMsg, sizeof(cMsg), "Up:%15.15s", upTime().c_str());
              break;
    case 2:   snprintf(cMsg, sizeof(cMsg), "WiFi RSSI:%4d dBm", WiFi.RSSI());
              break;
    case 3:   snprintf(cMsg, sizeof(cMsg), "Heap:%7d Bytes", ESP.getFreeHeap());
              break;
    case 4:   if (WiFi.status() != WL_CONNECTED)
                    snprintf(cMsg, sizeof(cMsg), "**** NO  WIFI ****");
              else  snprintf(cMsg, sizeof(cMsg), "IP %s", WiFi.localIP().toString().c_str());
              break;
    default:  snprintf(cMsg, sizeof(cMsg), "Telgrms:%6d/%3d", telegramCount, telegramErrors);
              break;
  }
  oled_Print_Msg(3, cMsg, 0);
  msgMode= (msgMode+1) % 5; //modular 5 = number of message displayed (hence it cycles thru the messages
#endif
  
} // displayStatus()


#ifdef USE_SYSLOGGER
//===========================================================================================
void openSysLog(bool empty)
{
  if (sysLog.begin(500, 100, empty))  // 500 lines use existing sysLog file
  {   
    DebugTln("Succes opening sysLog!");
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "Syslog OK!", 500);
#endif  // has_oled_ssd1306
  }
  else
  {
    DebugTln("Error opening sysLog!");
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "Error Syslog", 1500);
#endif  // has_oled_ssd1306
  }

  sysLog.setDebugLvl(1);
  sysLog.setOutput(&TelnetStream);
  sysLog.status();
  sysLog.write("\r\n");
  for (int q=0;q<3;q++)
  {
    sysLog.write("******************************************************************************************************");
  }
  writeToSysLog("Last Reset Reason [%s]", ESP.getResetReason().c_str());
  writeToSysLog("actTimestamp[%s], nrReboots[%u], Errors[%u]", actTimestamp
                                                             , nrReboots
                                                             , slotErrors);

  sysLog.write(" ");

} // openSysLog()
#endif

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
  
  //--- setup randomseed the right way
  //--- This is 8266 HWRNG used to seed the Random PRNG
  //--- Read more: https://config9.com/arduino/getting-a-truly-random-number-in-arduino/
  randomSeed(RANDOM_REG32); 
  snprintf(settingHostname, sizeof(settingHostname), "%s", _DEFAULT_HOSTNAME);
  Serial.printf("\n\nBooting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Init();
  oled_Clear();  // clear the screen so we can paint the menu.
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  int8_t sPos = String(_FW_VERSION).indexOf(' ');
  snprintf(cMsg, sizeof(cMsg), "(c)2020 [%s]", String(_FW_VERSION).substring(0,sPos).c_str());
  oled_Print_Msg(1, cMsg, 0);
  oled_Print_Msg(2, " Willem Aandewiel", 0);
  oled_Print_Msg(3, " >> Have fun!! <<", 1000);
  yield();
#else  // don't blink if oled-screen attatched
  for(int I=0; I<8; I++) 
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
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
  if (SPIFFS.begin()) 
  {
    DebugTln(F("SPIFFS Mount succesfull\r"));
    SPIFFSmounted = true;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "SPIFFS mounted", 1500);
#endif  // has_oled_ssd1306
    
  } else { 
    DebugTln(F("SPIFFS Mount failed\r"));   // Serious problem with SPIFFS 
    SPIFFSmounted = false;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "SPIFFS FAILED!", 2000);
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

//=============start Networkstuff==================================
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Clear();  // clear the screen 
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  oled_Print_Msg(1, "Verbinden met WiFi", 500);
#endif  // has_oled_ssd1306

  digitalWrite(LED_BUILTIN, LED_ON);
  startWiFi(settingHostname, 240);  // timeout 4 minuten

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  oled_Print_Msg(1, WiFi.SSID(), 0);
  snprintf(cMsg, sizeof(cMsg), "IP %s", WiFi.localIP().toString().c_str());
  oled_Print_Msg(2, cMsg, 1500);
#endif  // has_oled_ssd1306
  digitalWrite(LED_BUILTIN, LED_OFF);
  
  Debugln();
  Debug (F("Connected to " )); Debugln (WiFi.SSID());
  Debug (F("IP address: " ));  Debugln (WiFi.localIP());
  Debug (F("IP gateway: " ));  Debugln (WiFi.gatewayIP());
  Debugln();

  for (int L=0; L < 10; L++) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(200);
  }
  digitalWrite(LED_BUILTIN, LED_OFF);

//-----------------------------------------------------------------
#ifdef USE_SYSLOGGER
  openSysLog(false);
  snprintf(cMsg, sizeof(cMsg), "SSID:[%s],  IP:[%s], Gateway:[%s]", WiFi.SSID().c_str()
                                                                  , WiFi.localIP().toString().c_str()
                                                                  , WiFi.gatewayIP().toString().c_str());
  writeToSysLog("%s", cMsg);

#endif

  startMDNS(settingHostname);
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(3, "mDNS gestart", 1500);
#endif  // has_oled_ssd1306

//=============end Networkstuff======================================

#if defined(USE_NTP_TIME)                                   //USE_NTP
//================ startNTP =========================================
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )  
    oled_Print_Msg(3, "setup NTP server", 100);             //USE_NTP
  #endif  // has_oled_ssd1306                               //USE_NTP
                                                            //USE_NTP
  if (!startNTP())                                          //USE_NTP
  {                                                         //USE_NTP
    DebugTln(F("ERROR!!! No NTP server reached!\r\n\r"));   //USE_NTP
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 ) 
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);                //USE_NTP
    oled_Print_Msg(2, "geen reactie van", 100);             //USE_NTP
    oled_Print_Msg(2, "NTP server's", 100);                 //USE_NTP 
    oled_Print_Msg(3, "Reboot DSMR-logger", 2000);          //USE_NTP
  #endif  // has_oled_ssd1306                               //USE_NTP
    delay(2000);                                            //USE_NTP
    ESP.restart();                                          //USE_NTP
    delay(3000);                                            //USE_NTP
  }                                                         //USE_NTP
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 ) 
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);                //USE_NTP
    oled_Print_Msg(3, "NTP gestart", 1500);                 //USE_NTP
    prevNtpHour = hour();                                   //USE_NTP
  #endif                                                    //USE_NTP
                                                            //USE_NTP
#endif  //USE_NTP_TIME                                      //USE_NTP
//================ end NTP =========================================

  snprintf(cMsg, sizeof(cMsg), "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

  Serial.print("\nGebruik 'telnet ");
  Serial.print (WiFi.localIP());
  Serial.println("' voor verdere debugging\r\n");

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
    else  hasAlternativeIndex    = false;
  }
  if (!hasAlternativeIndex && !DSMRfileExist("/DSMRindex.html", false) )
  {
    spiffsNotPopulated = true;
  }
  if (!hasAlternativeIndex)    //--- there's no alternative index.html
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
#ifdef USE_SYSLOGGER
  if (spiffsNotPopulated)
  {
    sysLog.write("SPIFFS is not correct populated (files are missing)");
  }
#endif
  
//=============now test if "convertPRD" file exists================

  if (SPIFFS.exists("/!PRDconvert") )
  {
    convertPRD2RING();
  }

//=================================================================

#if defined(USE_NTP_TIME)                                                           //USE_NTP
  time_t t = now(); // store the current time in time variable t                    //USE_NTP
  snprintf(cMsg, sizeof(cMsg), "%02d%02d%02d%02d%02d%02dW\0\0"                      //USE_NTP
                                               , (year(t) - 2000), month(t), day(t) //USE_NTP
                                               , hour(t), minute(t), second(t));    //USE_NTP
  pTimestamp = cMsg;                                                                //USE_NTP
  DebugTf("Time is set to [%s] from NTP\r\n", cMsg);                                //USE_NTP
#endif  // use_dsmr_30

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  snprintf(cMsg, sizeof(cMsg), "DT: %02d%02d%02d%02d0101W", thisYear, thisMonth, thisDay, thisHour);
  oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
  oled_Print_Msg(3, cMsg, 1500);
#endif  // has_oled_ssd1306

//================ Start MQTT  ======================================

#ifdef USE_MQTT                                                 //USE_MQTT
  connectMQTT();                                                //USE_MQTT
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 ) //USE_MQTT
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);                    //USE_MQTT
    oled_Print_Msg(3, "MQTT server set!", 1500);                //USE_MQTT
  #endif  // has_oled_ssd1306                                   //USE_MQTT
#endif                                                          //USE_MQTT

//================ End of Start MQTT  ===============================


//================ Start HTTP Server ================================

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
      httpServer.serveStatic("/DSMRindex.html",   SPIFFS, settingIndexPage);
    }
    else
    {
      httpServer.serveStatic("/",                 SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/DSMRindex.html",   SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/index",            SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/index.html",       SPIFFS, "/DSMRindex.html");
      httpServer.serveStatic("/DSMRindex.css",    SPIFFS, "/DSMRindex.css");
      httpServer.serveStatic("/DSMRindex.js",     SPIFFS, "/DSMRindex.js");
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
//================ Start HTTP Server ================================

  //test(); monthTabel
  
#ifdef USE_MINDERGAS
    handleMindergas();
#endif

  DebugTf("Startup complete! actTimestamp[%s]\r\n", actTimestamp);  
  writeToSysLog("Startup complete! actTimestamp[%s]", actTimestamp);  

//================ End of Slimmer Meter ============================


//================ The final part of the Setup =====================

  snprintf(cMsg, sizeof(cMsg), "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "Startup complete", 0);
    oled_Print_Msg(2, "Wait for first", 0);
    oled_Print_Msg(3, "telegram .....", 500);
#endif  // has_oled_ssd1306

//================ Start Slimme Meter ===============================

  DebugTln(F("Enable slimmeMeter..\r"));

#if defined( USE_REQUEST_PIN ) && !defined( HAS_NO_SLIMMEMETER )
    DebugTf("Swapping serial port to Smart Meter, debug output will continue on telnet\r\n");
    DebugFlush();
    Serial.swap();
#endif // is_esp12

  delay(100);
  slimmeMeter.enable(true);

} // setup()


//===[ no-blocking delay with running background tasks in ms ]============================
DECLARE_TIMER_MS(timer_delay_ms, 1);
void delayms(unsigned long delay_ms)
{
  CHANGE_INTERVAL_MS(timer_delay_ms, delay_ms);
  RESTART_TIMER(timer_delay_ms);
  while (!DUE(timer_delay_ms))
  {
    doSystemTasks();
  }
    
} // delayms()

//========================================================================================

//==[ Do Telegram Processing ]===============================================================
void doTaskTelegram()
{
  if (Verbose1) DebugTln("doTaskTelegram");
  #if defined(HAS_NO_SLIMMEMETER)
    handleTestdata();
  #else
    //-- enable DTR to read a telegram from the Slimme Meter
    slimmeMeter.enable(true); 
    slimmeMeter.loop();
    handleSlimmemeter();
  #endif
  if (WiFi.status() != WL_CONNECTED)
  {
    for(int b=0; b<10; b++) { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); delay(75);}
  }
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//===[ Do System tasks ]=============================================================
void doSystemTasks()
{
  #ifndef HAS_NO_SLIMMEMETER
    slimmeMeter.loop();
  #endif
  #ifdef USE_MQTT
    MQTTclient.loop();
  #endif
  httpServer.handleClient();
  MDNS.update();
  handleKeyInput();
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  checkFlashButton();
#endif

  yield();

} // doSystemTasks()

  
void loop () 
{  
  //--- do the tasks that has to be done 
  //--- as often as possible
  doSystemTasks();

  loopCount++;

  //--- verwerk volgend telegram
  if DUE(nextTelegram)
  {
    doTaskTelegram();
  }

  //--- update upTime counter
  if DUE(updateSeconds)
  {
    upTimeSeconds++;
  }

//--- if an OLED screen attached, display the status
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  if DUE(updateDisplay)
  {
    displayStatus();
  }
#endif

//--- if mindergas then check
#ifdef USE_MINDERGAS
  if ( DUE(minderGasTimer) )
  {
    handleMindergas();
  }
#endif

  //--- if connection lost, try to reconnect to WiFi
  if ( DUE(reconnectWiFi) && (WiFi.status() != WL_CONNECTED) )
  {
    writeToSysLog("Restart wifi with [%s]...", settingHostname);
    startWiFi(settingHostname, 10);
    if (WiFi.status() != WL_CONNECTED)
          writeToSysLog("%s", "Wifi still not connected!");
    else {
          snprintf(cMsg, sizeof(cMsg), "IP:[%s], Gateway:[%s]", WiFi.localIP().toString().c_str()
                                                              , WiFi.gatewayIP().toString().c_str());
          writeToSysLog("%s", cMsg);
    }
  }

//--- if NTP set, see if it needs synchronizing
#if defined(USE_NTP_TIME)                                           //USE_NTP
  if DUE(synchrNTP)                                                 //USE_NTP
  {
    if (timeStatus() == timeNeedsSync || prevNtpHour != hour())     //USE_NTP
    {
      prevNtpHour = hour();                                         //USE_NTP
      setSyncProvider(getNtpTime);                                  //USE_NTP
      setSyncInterval(600);                                         //USE_NTP
    }
  }
#endif                                                              //USE_NTP
  
  yield();
  
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
