/*
***************************************************************************
**  Program  : networkStuff.h, part of DSMRloggerAPI
**  Version  : v3.0
**
**  Copyright (c) 2020 .. 2022 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/


#include <ESP8266WiFi.h>        //ESP8266 Core WiFi Library         
#include <ESP8266WebServer.h>   // Version 1.0.0 - part of ESP8266 Core https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>        // part of ESP8266 Core https://github.com/esp8266/Arduino

#include <WiFiUdp.h>            // part of ESP8266 Core https://github.com/esp8266/Arduino
#ifdef USE_UPDATE_SERVER
  //#include "ESP8266HTTPUpdateServer.h"
  #include "ModUpdateServer.h"  // https://github.com/mrWheel/ModUpdateServer
  #include "UpdateServerHtml.h"
#endif
#include <WiFiManager.h>        // version 0.15.0 - https://github.com/tzapu/WiFiManager
// included in main program: #include <TelnetStream.h>       // Version 0.0.1 - https://github.com/jandrassy/TelnetStream
#include <FS.h>                 // part of ESP8266 Core https://github.com/esp8266/Arduino


ESP8266WebServer        httpServer (80);
#ifdef USE_UPDATE_SERVER
  ESP8266HTTPUpdateServer httpUpdater(true);
#endif


static      FSInfo SPIFFSinfo;
bool        FSYSmounted;
bool        isConnected = false;

//gets called when WiFiManager enters configuration mode
//===========================================================================================
void configModeCallback (WiFiManager *myWiFiManager)
{
  DebugTln(F("Entered config mode\r"));
  DebugTln(WiFi.softAPIP().toString());
  //if you used auto generated SSID, print it
  DebugTln(myWiFiManager->getConfigPortalSSID());
  if (settingOledType > 0)
  {
    oled_Clear();
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "AP mode active", 0);
    oled_Print_Msg(2, "Connect to:", 0);
    oled_Print_Msg(3, myWiFiManager->getConfigPortalSSID(), 0);
  }

} // configModeCallback()


//===========================================================================================
void startWiFi(const char *hostname, int timeOut)
{
  WiFiManager manageWiFi;
  uint32_t lTime = millis();
  String thisAP = String(hostname) + "-" + WiFi.macAddress();

  DebugTln("start ...");

  WiFi.mode(WIFI_STA);

  manageWiFi.setDebugOutput(true);

  //--- set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  manageWiFi.setAPCallback(configModeCallback);

  //--- sets timeout until configuration portal gets turned off
  //--- useful to make it all retry or go to sleep in seconds
  //manageWiFi.setTimeout(240);  // 4 minuten
  manageWiFi.setTimeout(timeOut);  // in seconden ...

  //--- fetches ssid and pass and tries to connect
  //--- if it does not connect it starts an access point with the specified name
  //--- here  "DSMR-WS-<MAC>"
  //--- and goes into a blocking loop awaiting configuration
  if (!manageWiFi.autoConnect(thisAP.c_str()))
  {
    DebugTln(F("failed to connect and hit timeout"));
    if (settingOledType > 0)
    {
      oled_Clear();
      oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
      oled_Print_Msg(1, "Failed to connect", 0);
      oled_Print_Msg(2, "and hit TimeOut", 0);
      oled_Print_Msg(3, "**** NO WIFI ****", 0);
    }

    DebugTf(" took [%d] milli-seconds ==> ERROR!\r\n", (millis() - lTime));
    //return;
  }
  else
  {
    DebugTf("Connected with IP-address [%s]\r\n\r\n", WiFi.localIP().toString().c_str());
  }
  if (settingOledType > 0)
  {
    oled_Clear();
  }

  if (WiFi.softAPdisconnect(true))
        Serial.println("WiFi Access Point disconnected and closed");
  else  Serial.println("Hm.. could not disconnect WiFi Access Point! (maybe there was none?)");

#ifdef USE_UPDATE_SERVER
  httpUpdater.setup(&httpServer);
  httpUpdater.setIndexPage(UpdateServerIndex);
  httpUpdater.setSuccessPage(UpdateServerSuccess);
#endif
  DebugTf(" took [%d] milli-seconds => OK!\r\n", (millis() - lTime));

} // startWiFi()


//===========================================================================================
void startTelnet()
{
  TelnetStream.begin();
  DebugTln(F("\nTelnet server started .."));
  TelnetStream.flush();

} // startTelnet()


//=======================================================================
void startMDNS(const char *Hostname)
{
  DebugTf("[1] mDNS setup as [%s.local]\r\n", Hostname);
  if (MDNS.begin(Hostname))               // Start the mDNS responder for Hostname.local
  {
    DebugTf("[2] mDNS responder started as [%s.local]\r\n", Hostname);
  }
  else
  {
    DebugTln(F("[3] Error setting up MDNS responder!\r\n"));
  }
  MDNS.addService("http", "tcp", 80);

} // startMDNS()

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
