/*
***************************************************************************
**  Program  : DSMRloggerAPI (restAPI)
*/
#define _FW_VERSION "v3.0.5 (18-03-2023)"
/*
**  Copyright (c) 2020, 2021, 2022, 2023 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*
  Arduino-IDE settings for DSMR-logger Version 4 (ESP-12):

    - Board: "Generic ESP8266 Module"
    - Builtin Led: "2"  // GPIO02 for Wemos and ESP-12
    - Flash mode: "DOUT" | "DIO"    // changes only after power-off and on again!
    - Flash size: "4MB (FS: 2MB OAT:~1019KB)"  << LET OP! 2MB FSYS
    - DebugT port: "Disabled"
    - DebugT Level: "None"
    - IwIP Variant: "v2 Lower Memory"
    - Reset Method: "none"   // but will depend on the programmer!
    - Crystal Frequency: "26 MHz"
    - VTables: "Flash"
    - Flash Frequency: "40MHz"
    - CPU Frequency: "80 MHz" (or 160MHz)
    - Upload Speed: "115200"
    - Erase Flash: "Only Sketch"
    - Port: <select correct port>


  Coding Style  ( http://astyle.sourceforge.net/astyle.html#_Quick_Start )
   - Allman style (-A1)
   - tab 2 spaces (-s2)
   - Indent 'switch' blocks (-S)
   - Indent preprocessor blocks (-xW)
   - Indent multi-line preprocessor definitions ending with a backslash (-w)
   - Indent C++ comments beginning in column one (-Y)
   - Insert space padding after commas (-xg)
   - Attach a pointer or reference operator (-k3)

  use:  astyle -A1 -s2 -S -xW -w -Y -xg- k3 *.{ino|h}

  remove <filename>.orig afterwards
    
*/

/*
**  You can find more info in the following links (all in Dutch):
**   https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/
**   https://mrwheel-docs.gitbook.io/dsmrloggerapi/
**   https://mrwheel.github.io/DSMRloggerWS/
*/
/******************** compiler options  ********************************************/
#define USE_LITTLEFS              // if not #defined: use SPIFFS
#define USE_UPDATE_SERVER         // define if there is enough memory and updateServer to be used
#define USE_MQTT                  // define if you want to use MQTT (configure through webinterface)
#define USE_MINDERGAS             // define if you want to update mindergas (configure through webinterface)
//  #define USE_SYSLOGGER             // define if you want to use the sysLog library for debugging
//  #define SHOW_PASSWRDS             // well .. show the PSK key and MQTT password, what else?
//  #define HAS_NO_SLIMMEMETER        // define for testing only!
/******************** don't change anything below this comment **********************/

#include "DSMRloggerAPI.h"

struct showValues
{
  template<typename Item>
  void apply(Item &i)
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


//===========================================================================================
void displayStatus()
{
  if (settingOledType > 0)
  {
    switch(msgMode)
    {
      case 1:
        snprintf(cMsg, sizeof(cMsg), "Up:%-15.15s", upTime().c_str());
        break;
      case 2:
        snprintf(cMsg, sizeof(cMsg), "WiFi RSSI:%4d dBm", WiFi.RSSI());
        break;
      case 3:
        snprintf(cMsg, sizeof(cMsg), "Heap:%7d Bytes", ESP.getFreeHeap());
        break;
      case 4:
        if (WiFi.status() != WL_CONNECTED)
          snprintf(cMsg, sizeof(cMsg), "**** NO  WIFI ****");
        else  snprintf(cMsg, sizeof(cMsg), "IP %s", WiFi.localIP().toString().c_str());
        break;
      default:
        snprintf(cMsg, sizeof(cMsg), "Telgrms:%6d/%3d", telegramCount, telegramErrors);
        break;
    }

    oled_Print_Msg(3, cMsg, 0);
    msgMode= (msgMode+1) % 5; //modular 5 = number of message displayed (hence it cycles thru the messages
  }
} // displayStatus()


#ifdef USE_SYSLOGGER
//===========================================================================================
void openSysLog(bool empty)
{
  if (sysLog.begin(500, 100, empty))   // 500 lines use existing sysLog file
  {
    DebugTln("Succes opening sysLog!");
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
      oled_Print_Msg(3, "Syslog OK!", 500);
    }
  }
  else
  {
    DebugTln("Error opening sysLog!");
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
      oled_Print_Msg(3, "Error Syslog", 1500);
    }
  }

  sysLog.setDebugLvl(1);
  sysLog.setOutput(&TelnetStream);
  sysLog.status();
  sysLog.write("\r\n");
  for (int q=0; q<3; q++)
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
  Serial.begin(115200, SERIAL_8N1); // for now. Look at end of setup()
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FLASH_BUTTON, INPUT);
  pinMode(DTR_ENABLE, OUTPUT);

  //--- setup randomseed the right way
  //--- This is 8266 HWRNG used to seed the Random PRNG
  //--- Read more: https://config9.com/arduino/getting-a-truly-random-number-in-arduino/
  randomSeed(RANDOM_REG32);
  snprintf(settingHostname, sizeof(settingHostname), "%s", _DEFAULT_HOSTNAME);
  Serial.printf("\n\nBooting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());

  if (settingOledType > 0)
  {
    oled_Init();
    oled_Clear();  // clear the screen so we can paint the menu.
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    int8_t sPos = String(_FW_VERSION).indexOf(' ');
    snprintf(cMsg, sizeof(cMsg), "(c)2020 [%s]", String(_FW_VERSION).substring(0, sPos).c_str());
    oled_Print_Msg(1, cMsg, 0);
    oled_Print_Msg(2, " Willem Aandewiel", 0);
    oled_Print_Msg(3, " >> Have fun!! <<", 1000);
    yield();
  }
  else     // don't blink if oled-screen attatched
  {
    for(int I=0; I<8; I++)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(500);
    }
  }
  digitalWrite(LED_BUILTIN, LED_OFF);  // HIGH is OFF
  //-- Press [Reset] -> "External System"
  //-- Software reset -> "Software/System restart"
  lastReset     = ESP.getResetReason();

  startTelnet();
  if (settingOledType > 0)
  {
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "telnet (poort 23)", 2500);
  }

  //================ FSYS ===========================================
#if defined( USE_LITTLEFS )
  LittleFSConfig cfg;
#else
  SPIFFSConfig cfg;
#endif
  cfg.setAutoFormat(false);
  FSYS.setConfig(cfg);

  if (FSYS.begin())
  {
#if defined( USE_LITTLEFS )
    DebugTln(F("LittleFS Mount succesfull\r"));
#else
    DebugTln(F("SPIFFS Mount succesfull\r"));
#endif
    File nF = FSYS.open("/!doNotFormat", "w");
    nF.close();
    FSYSmounted = true;
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
      oled_Print_Msg(3, "FSYS mounted", 1500);
    }
  }
  else
  {
#if defined( USE_LITTLEFS )
    DebugTln(F("LittleFS Mount failed\r")); // Serious problem with LittleFS
#else
    DebugTln(F("SPIFFS Mount failed\r"));   // Serious problem with SPIFFS
#endif
    FSYSmounted = false;
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
      oled_Print_Msg(3, "FSYS FAILED!", 2000);
    }
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
  oled_Init();

  if (settingDailyReboot)
  {
    if (!lastReset.equals("Software/System restart")) telegramCount = 0;
  }

  //=============start Networkstuff==================================
  if (settingOledType > 0)
  {
    if (settingOledFlip)  oled_Init();  // only if true restart(init) oled screen
    oled_Clear();                       // clear the screen
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "Verbinden met WiFi", 500);
  }
  digitalWrite(LED_BUILTIN, LED_ON);
  startWiFi(settingHostname, 240);  // timeout 4 minuten

  if (settingOledType > 0)
  {
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    oled_Print_Msg(1, WiFi.SSID(), 0);
    snprintf(cMsg, sizeof(cMsg), "IP %s", WiFi.localIP().toString().c_str());
    oled_Print_Msg(2, cMsg, 1500);
  }
  digitalWrite(LED_BUILTIN, LED_OFF);

  Debugln();
  Debug (F("Connected to " ));
  Debugln (WiFi.SSID());
  Debug (F("IP address: " ));
  Debugln (WiFi.localIP());
  Debug (F("IP gateway: " ));
  Debugln (WiFi.gatewayIP());
  Debugln();

  for (int L=0; L < 10; L++)
  {
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
  if (settingOledType > 0)
  {
    oled_Print_Msg(3, "mDNS gestart", 1500);
  }

  //=============end Networkstuff======================================

  //================ startNTP =========================================
  if (settingOledType > 0)
  {
    oled_Print_Msg(3, "setup NTP server", 100);
  }

  if (!startNTP())
  {
    DebugTln(F("ERROR!!! No NTP server reached!\r\n\r"));
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
      oled_Print_Msg(2, "geen reactie van", 100);
      oled_Print_Msg(2, "NTP server's", 100);
      oled_Print_Msg(3, "Reboot DSMR-logger", 2000);
    }
    delay(2000);
    ESP.restart();
    delay(3000);
  }

  setSyncProvider(getNtpTime);
  snprintf(cMsg, sizeof(cMsg), "%02d-%02d-%02d %02d:%02d:%02d (%s)"
           , year(ntpTime), month(ntpTime), day(ntpTime)
           , hour(ntpTime), minute(ntpTime), second(ntpTime)
           , DSTactive ? "CEST":"CET");
  DebugTf("NTP time is [%s]\r\n", cMsg);

  if (settingOledType > 0)
  {
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "NTP gestart", 1500);
  }
  prevNtpHour = hour();

  //================ end NTP =========================================

  snprintf(cMsg, sizeof(cMsg), "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

  Serial.print("\nGebruik 'telnet ");
  Serial.print (WiFi.localIP());
  Serial.println("' voor verdere debugging\r\n");

  //=============now test if FS is correct populated!============
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
    FSYSnotPopulated = true;
  }
  if (!hasAlternativeIndex)    //--- there's no alternative index.html
  {
    DSMRfileExist("/DSMRindex.js",    false);
    DSMRfileExist("/DSMRindex.css",   false);
    DSMRfileExist("/DSMRgraphics.js", false);
  }
  if (!DSMRfileExist("/FSmanager.html", true))
  {
    FSYSnotPopulated = true;
  }
  if (!DSMRfileExist("/FSmanager.css", true))
  {
    FSYSnotPopulated = true;
  }
  //=============end FSYS =========================================
#ifdef USE_SYSLOGGER
  if (FSYSnotPopulated)
  {
    sysLog.write("FSYS is not correct populated (files are missing)");
  }
#endif

  //=============now test if "convertPRD" file exists================

  if (FSYS.exists("/!PRDconvert") )
  {
    convertPRD2RING();
  }

  //=================================================================

  time_t t = now(); // store the current time in time variable t
  check4DST(t);
  snprintf(cMsg, sizeof(cMsg), "%02d%02d%02d%02d%02d%02d\0\0"                      //USE_NTP
           , (year(t) - 2000), month(t), day(t) //USE_NTP
           , hour(t), minute(t), second(t));    //USE_NTP
  if (DSTactive)  strConcat(cMsg, 15, "S");
  else            strConcat(cMsg, 15, "W");
  pTimestamp = cMsg;                                                                //USE_NTP
  DebugTf("Time is set to [%s] from NTP\r\n", cMsg);                                //USE_NTP

  if (settingOledType > 0)
  {
    snprintf(cMsg, sizeof(cMsg), "DT: %02d%02d%02d%02d0101x", thisYear
             , thisMonth, thisDay, thisHour);
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    oled_Print_Msg(3, cMsg, 1500);
  }

  //================ Start MQTT  ======================================

#ifdef USE_MQTT                                                 //USE_MQTT
  connectMQTT();                                                //USE_MQTT
  if (settingOledType > 0)                                      //USE_MQTT
  {
    //USE_MQTT
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);                   //USE_MQTT
    oled_Print_Msg(3, "MQTT server set!", 1500);                //USE_MQTT
  }                                                             //USE_MQTT
#endif                                                          //USE_MQTT

  //================ End of Start MQTT  ===============================


  //================ Start HTTP Server ================================

  if (!FSYSnotPopulated)
  {
    DebugTln(F("FSYS correct populated -> normal operation!\r"));
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
      oled_Print_Msg(1, "OK, FSYS correct", 0);
      oled_Print_Msg(2, "Verder met normale", 0);
      oled_Print_Msg(3, "Verwerking ;-)", 2500);
    }
    if (hasAlternativeIndex)
    {
      httpServer.serveStatic("/",                 FSYS, settingIndexPage);
      httpServer.serveStatic("/index",            FSYS, settingIndexPage);
      httpServer.serveStatic("/index.html",       FSYS, settingIndexPage);
      httpServer.serveStatic("/DSMRindex.html",   FSYS, settingIndexPage);
    }
    else
    {
      httpServer.serveStatic("/",                 FSYS, "/DSMRindex.html");
      httpServer.serveStatic("/DSMRindex.html",   FSYS, "/DSMRindex.html");
      httpServer.serveStatic("/index",            FSYS, "/DSMRindex.html");
      httpServer.serveStatic("/index.html",       FSYS, "/DSMRindex.html");
      httpServer.serveStatic("/DSMRindex.css",    FSYS, "/DSMRindex.css");
      httpServer.serveStatic("/DSMRindex.js",     FSYS, "/DSMRindex.js");
      httpServer.serveStatic("/DSMRgraphics.js",  FSYS, "/DSMRgraphics.js");
    }
  }
  else
  {
    DebugTln(F("Oeps! not all files found on FSYS -> Start FSmanager!\r"));
    FSYSnotPopulated = true;
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, "!OEPS! niet alle", 0);
      oled_Print_Msg(1, "files op FSYS", 0);
      oled_Print_Msg(2, "gevonden! (fout!)", 0);
      oled_Print_Msg(3, "Start FSmanager", 2000);
    }
  }

  setupFsManager();
  //httpServer.serveStatic("/FSexplorer.png",   FSYS, "/FSexplorer.png");

  httpServer.on("/api", HTTP_GET, processAPI);
  // all other api calls are catched in FSmanager onNotFounD!

  httpServer.begin();
  DebugTln( "HTTP server gestart\r" );
  if (settingOledType > 0)                                  //HAS_OLED
  {
    //HAS_OLED
    oled_Clear();                                           //HAS_OLED
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);                //HAS_OLED
    oled_Print_Msg(2, "HTTP server ..", 0);                 //HAS_OLED
    oled_Print_Msg(3, "gestart (poort 80)", 0);             //HAS_OLED
  }                                                         //HAS_OLED

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

  if (settingOledType > 0)
  {
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "Startup complete", 0);
    oled_Print_Msg(2, "Wait for first", 0);
    oled_Print_Msg(3, "telegram .....", 500);
  }

  //================ Start Slimme Meter ===============================

  DebugTln(F("Enable slimmeMeter..\r"));

#if !defined( HAS_NO_SLIMMEMETER )
  DebugTf("Swapping serial port to Smart Meter, debug output will continue on telnet\r\n");
  DebugFlush();
  if (settingPreDSMR40 == 0)
  {
    DebugTln("Serial will be set to 115200 baud / 7N1");
    DebugFlush();
    Serial.end();
    delay(100);
    Serial.begin(115200, SERIAL_8N1);
    slimmeMeter.doChecksum(true);
  }
  else
  {
    //PRE40
    DebugTln("Serial will be set to 9600 baud / 7N1");
    DebugFlush();
    Serial.end();
    delay(100);
    Serial.begin(9600, SERIAL_7E1);
    slimmeMeter.doChecksum(false);
  }
  Serial.swap();

#endif // HAS_NO_SLIMME_METER

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
    for(int b=0; b<10; b++)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(75);
    }
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
  if (settingOledType > 0)
  {
    checkFlashButton();
  }

  yield();

} // doSystemTasks()


//========================================================================================
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
  if (settingOledType > 0)
  {
    if DUE(updateDisplay)
    {
      displayStatus();
    }
  }

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
    else
    {
      snprintf(cMsg, sizeof(cMsg), "IP:[%s], Gateway:[%s]", WiFi.localIP().toString().c_str()
               , WiFi.gatewayIP().toString().c_str());
      writeToSysLog("%s", cMsg);
    }
  }

  //--- see if NTP needs synchronizing
  if DUE(synchrNTP)
  {
    setSyncProvider(getNtpTime);
    setSyncInterval(3600);
    check4DST(ntpTime);
  }

#ifndef HAS_NO_SLIMMEMETER
  //-- hier moet nog even over worden nagedacht
  //-- via een setting in- of uit-schakelen
  if (settingDailyReboot && (hour() == 4) && (minute() == 5))
  {
    slotErrors      = 0;
    nrReboots       = 0;
    writeLastStatus();
    //--  skip to next minute (6)
    delay(60000);
    ESP.restart();
    delay(3000);
  }
#endif

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
