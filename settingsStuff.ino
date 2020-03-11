/*
***************************************************************************  
**  Program  : settingsStuff, part of DSMRloggerAPI
**  Version  : v1.0.1
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
* 1.0.11 added Mindergas Authtoken setting
*/

//=======================================================================
void writeSettings() 
{
  yield();
  DebugT(F("Writing to [")); Debug(SETTINGS_FILE); Debugln(F("] ..."));
  File file = SPIFFS.open(SETTINGS_FILE, "w"); // open for reading and writing
  if (!file) 
  {
    DebugTf("open(%s, 'w') FAILED!!! --> Bailout\r\n", SETTINGS_FILE);
    return;
  }
  yield();

  if (strlen(settingIndexPage) < 7) strCopy(settingIndexPage, (sizeof(settingIndexPage) -1), "DSMRindex.html");
  if (settingTelegramInterval < 2)  settingTelegramInterval = 10;
  if (settingMQTTbrokerPort < 1)    settingMQTTbrokerPort = 1883;
    
  DebugT(F("Start writing setting data "));

  file.print("Hostname = ");          file.println(settingHostname);            Debug(F("."));
  file.print("EnergyDeliveredT1 = "); file.println(String(settingEDT1, 5));     Debug(F("."));
  file.print("EnergyDeliveredT2 = "); file.println(String(settingEDT2, 5));     Debug(F("."));
  file.print("EnergyReturnedT1 = ");  file.println(String(settingERT1, 5));     Debug(F("."));
  file.print("EnergyReturnedT2 = ");  file.println(String(settingERT2, 5));     Debug(F("."));
  file.print("GASDeliveredT = ");     file.println(String(settingGDT,  5));     Debug(F("."));
  file.print("EnergyVasteKosten = "); file.println(String(settingENBK, 2));     Debug(F("."));
  file.print("GasVasteKosten = ");    file.println(String(settingGNBK, 2));     Debug(F("."));
  file.print("SleepTime = ");         file.println(settingSleepTime);           Debug(F("."));
  file.print("TelegramInterval = ");  file.println(settingTelegramInterval);    Debug(F("."));
  file.print("IndexPage = ");         file.println(settingIndexPage);           Debug(F("."));

#ifdef USE_MQTT
  //sprintf(settingMQTTbroker, "%s:%d", MQTTbroker, MQTTbrokerPort);
  file.print("MQTTbroker = ");        file.println(settingMQTTbroker);          Debug(F("."));
  file.print("MQTTbrokerPort = ");    file.println(settingMQTTbrokerPort);      Debug(F("."));
  file.print("MQTTUser = ");          file.println(settingMQTTuser);            Debug(F("."));
  file.print("MQTTpasswd = ");        file.println(settingMQTTpasswd);          Debug(F("."));
  file.print("MQTTinterval = ");      file.println(settingMQTTinterval);        Debug(F("."));
  file.print("MQTTtopTopic = ");      file.println(settingMQTTtopTopic);        Debug(F("."));
#endif
  
#ifdef USE_MINDERGAS
  file.print("MindergasAuthtoken = ");file.println(settingMindergasToken);  Debug(F("."));
#endif

file.close();  
  
  Debugln(F(" done"));
  if (Verbose1) 
  {
    DebugTln(F("Wrote this:"));
    DebugT(F("EnergyDeliveredT1 = ")); Debugln(String(settingEDT1, 5));     
    DebugT(F("EnergyDeliveredT2 = ")); Debugln(String(settingEDT2, 5));     
    DebugT(F("EnergyReturnedT1 = "));  Debugln(String(settingERT1, 5));     
    DebugT(F("EnergyReturnedT2 = "));  Debugln(String(settingERT2, 5));     
    DebugT(F("GASDeliveredT = "));     Debugln(String(settingGDT,  5));     
    DebugT(F("EnergyVasteKosten = ")); Debugln(String(settingENBK, 2));    
    DebugT(F("GasVasteKosten = "));    Debugln(String(settingGNBK, 2));    
    DebugT(F("SleepTime = "));         Debugln(settingSleepTime);           
    DebugT(F("TelegramInterval = "));  Debugln(settingTelegramInterval);            
    DebugT(F("IndexPage = "));         Debugln(settingIndexPage);             

#ifdef USE_MQTT
    DebugT(F("MQTTbroker = "));        Debugln(settingMQTTbroker);          
    DebugT(F("MQTTbrokerPort = "));    Debugln(settingMQTTbrokerPort);          
    DebugT(F("MQTTUser = "));          Debugln(settingMQTTuser);     
  #ifdef SHOW_PASSWRDS       
      DebugT(F("MQTTpasswd = "));        Debugln(settingMQTTpasswd);  
  #else 
      DebugTln(F("MQTTpasswd = ********"));  
  #endif       
    DebugT(F("MQTTinterval = "));      Debugln(settingMQTTinterval);        
    DebugT(F("MQTTtopTopic = "));      Debugln(settingMQTTtopTopic);   
#endif
  
#ifdef USE_MINDERGAS
  #ifdef SHOW_PASSWRDS   
    DebugT(F("MindergasAuthtoken = "));Debugln(settingMindergasToken);
  #else
    DebugTln(F("MindergasAuthtoken = ********")); 
  #endif
#endif
  } // Verbose1
  
} // writeSettings()


//=======================================================================
void readSettings(bool show) 
{
  String sTmp, nColor;
  String words[10];
  
  File file;
  
  DebugTf(" %s ..\r\n", SETTINGS_FILE);

  snprintf(settingHostname, sizeof(settingHostname), "%s", _DEFAULT_HOSTNAME);
  settingEDT1               = 0.1;
  settingEDT2               = 0.2;
  settingERT1               = 0.3;
  settingERT2               = 0.4;
  settingGDT                = 0.5;
  settingENBK               = 15.15;
  settingGNBK               = 11.11;
  settingTelegramInterval   = 10; // seconds
  settingSleepTime          =  0; // infinite
  strCopy(settingIndexPage, sizeof(settingIndexPage), "DSMRindex.html");
  settingMQTTbroker[0]     = '\0';
  settingMQTTbrokerPort    = 1883;
  settingMQTTuser[0]       = '\0';
  settingMQTTpasswd[0]     = '\0';
  settingMQTTinterval      =  0;
  snprintf(settingMQTTtopTopic, sizeof(settingMQTTtopTopic), "%s", settingHostname);

#ifdef USE_MINDERGAS
  settingMindergasToken[0] = '\0';
#endif

  if (!SPIFFS.exists(SETTINGS_FILE)) 
  {
    DebugTln(F(" .. file not found! --> created file!"));
    writeSettings();
  }

  for (int T = 0; T < 2; T++) 
  {
    file = SPIFFS.open(SETTINGS_FILE, "r");
    if (!file) 
    {
      if (T == 0) DebugTf(" .. something went wrong opening [%s]\r\n", SETTINGS_FILE);
      else        DebugT(T);
      delay(100);
    }
  } // try T times ..

  DebugTln(F("Reading settings:\r"));
  while(file.available()) 
  {
    sTmp      = file.readStringUntil('\n');
    sTmp.replace("\r", "");
    //DebugTf("[%s] (%d)\r\n", sTmp.c_str(), sTmp.length());
    int8_t wc = splitString(sTmp.c_str(), '=', words, 10);
    words[0].toLowerCase();
    nColor    = words[1].substring(0,15);

    if (words[0].equalsIgnoreCase("Hostname"))            strCopy(settingHostname, 29, words[1].c_str());
    if (words[0].equalsIgnoreCase("EnergyDeliveredT1"))   settingEDT1         = strToFloat(words[1].c_str(), 5);  
    if (words[0].equalsIgnoreCase("EnergyDeliveredT2"))   settingEDT2         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("EnergyReturnedT1"))    settingERT1         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("EnergyReturnedT2"))    settingERT2         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("GasDeliveredT"))       settingGDT          = strToFloat(words[1].c_str(), 5); 
    if (words[0].equalsIgnoreCase("EnergyVasteKosten"))   settingENBK         = strToFloat(words[1].c_str(), 2);
    if (words[0].equalsIgnoreCase("GasVasteKosten"))      settingGNBK         = strToFloat(words[1].c_str(), 2);

    if (words[0].equalsIgnoreCase("SleepTime"))           
    {
      settingSleepTime    = words[1].toInt();    
      #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
        CHANGE_INTERVAL_MIN(oledSleepTimer, settingSleepTime);
      #endif
    }
    
    if (words[0].equalsIgnoreCase("TelegramInterval"))   
    {
      settingTelegramInterval     = words[1].toInt(); 
      CHANGE_INTERVAL_SEC(nextTelegram, settingTelegramInterval); 
    }

    if (words[0].equalsIgnoreCase("IndexPage"))           strCopy(settingIndexPage, (sizeof(settingIndexPage) -1), words[1].c_str());  

#ifdef USE_MINDERGAS
    if (words[0].equalsIgnoreCase("MindergasAuthtoken"))  strCopy(settingMindergasToken, 20, words[1].c_str());  
#endif

#ifdef USE_MQTT
    if (words[0].equalsIgnoreCase("MQTTbroker"))  
    {
      memset(settingMQTTbroker, '\0', sizeof(settingMQTTbroker));
      strCopy(settingMQTTbroker, 100, words[1].c_str());
    }
    if (words[0].equalsIgnoreCase("MQTTbrokerPort"))      settingMQTTbrokerPort    = words[1].toInt();  
    if (words[0].equalsIgnoreCase("MQTTuser"))            strCopy(settingMQTTuser    ,35 ,words[1].c_str());  
    if (words[0].equalsIgnoreCase("MQTTpasswd"))          strCopy(settingMQTTpasswd  ,25, words[1].c_str());  
    if (words[0].equalsIgnoreCase("MQTTinterval"))        settingMQTTinterval        = words[1].toInt(); 
    if (words[0].equalsIgnoreCase("MQTTtopTopic"))        strCopy(settingMQTTtopTopic, 20, words[1].c_str());  
    
    CHANGE_INTERVAL_SEC(publishMQTTtimer, settingMQTTinterval);
    CHANGE_INTERVAL_MIN(reconnectMQTTtimer, 1);
#endif
    
  } // while available()
  
  file.close();  

  //--- this will take some time to settle in
  //--- probably need a reboot before that to happen :-(
  MDNS.setHostname(settingHostname);    // start advertising with new(?) settingHostname

  DebugTln(F(" .. done\r"));


  if (strlen(settingIndexPage) < 7) strCopy(settingIndexPage, (sizeof(settingIndexPage) -1), "DSMRindex.html");
  if (settingTelegramInterval < 3)  settingTelegramInterval = 10;
  if (settingMQTTbrokerPort < 1)    settingMQTTbrokerPort = 1883;

  if (!show) return;
  
  Debugln(F("\r\n==== Settings ===================================================\r"));
  Debugf("                    Hostname : %s\r\n",   settingHostname);
  Debugf("   Energy Delivered Tarief 1 : %9.7f\r\n",  settingEDT1);
  Debugf("   Energy Delivered Tarief 2 : %9.7f\r\n",  settingEDT2);
  Debugf("   Energy Delivered Tarief 1 : %9.7f\r\n",  settingERT1);
  Debugf("   Energy Delivered Tarief 2 : %9.7f\r\n",  settingERT2);
  Debugf("        Gas Delivered Tarief : %9.7f\r\n",  settingGDT);
  Debugf("     Energy Netbeheer Kosten : %9.2f\r\n",  settingENBK);
  Debugf("        Gas Netbeheer Kosten : %9.2f\r\n",  settingGNBK);
  Debugf("   Telegram Process Interval : %d\r\n",     settingTelegramInterval);
  Debugf("OLED Sleep Min. (0=oneindig) : %d\r\n",     settingSleepTime);
  Debugf("                  Index Page : %s\r\n",     settingIndexPage);

#ifdef USE_MQTT
  Debugln(F("\r\n==== MQTT settings ==============================================\r"));
  Debugf("          MQTT broker URL/IP : %s:%d", settingMQTTbroker, settingMQTTbrokerPort);
  if (MQTTclient.connected()) Debugln(F(" (is Connected!)\r"));
  else                 Debugln(F(" (NOT Connected!)\r"));
  Debugf("                   MQTT user : %s\r\n", settingMQTTuser);
#ifdef SHOW_PASSWRDS
  Debugf("               MQTT password : %s\r\n", settingMQTTpasswd);
#else
  Debug( "               MQTT password : *************\r\n");
#endif
  Debugf("          MQTT send Interval : %d\r\n", settingMQTTinterval);
  Debugf("              MQTT top Topic : %s\r\n", settingMQTTtopTopic);
#endif  // USE_MQTT
#ifdef USE_MINDERGAS
  Debugln(F("\r\n==== Mindergas settings ==============================================\r"));
  Debugf("         Mindergas Authtoken : %s\r\n", settingMindergasToken);
#endif  
  
  Debugln(F("-\r"));

} // readSettings()


//=======================================================================
void updateSetting(const char *field, const char *newValue)
{
  DebugTf("-> field[%s], newValue[%s]\r\n", field, newValue);

  if (!stricmp(field, "Hostname")) {
    strCopy(settingHostname, 29, newValue); 
    if (strlen(settingHostname) < 1) strCopy(settingHostname, 29, _DEFAULT_HOSTNAME); 
    char *dotPntr = strchr(settingHostname, '.') ;
    if (dotPntr != NULL)
    {
      byte dotPos = (dotPntr-settingHostname);
      if (dotPos > 0)  settingHostname[dotPos] = '\0';
    }
    Debugln();
    DebugTf("Need reboot before new %s.local will be available!\r\n\n", settingHostname);
  }
  if (!stricmp(field, "ed_tariff1"))        settingEDT1         = String(newValue).toFloat();  
  if (!stricmp(field, "ed_tariff2"))        settingEDT2         = String(newValue).toFloat();  
  if (!stricmp(field, "er_tariff1"))        settingERT1         = String(newValue).toFloat();  
  if (!stricmp(field, "er_tariff2"))        settingERT2         = String(newValue).toFloat();  
  if (!stricmp(field, "electr_netw_costs")) settingENBK         = String(newValue).toFloat();

  if (!stricmp(field, "gd_tariff"))         settingGDT          = String(newValue).toFloat();  
  if (!stricmp(field, "gas_netw_costs"))    settingGNBK         = String(newValue).toFloat();

  if (!stricmp(field, "oled_screen_time")) 
  {
    settingSleepTime    = String(newValue).toInt();  
    #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
       CHANGE_INTERVAL_MIN(oledSleepTimer, settingSleepTime)
    #endif
  }
  
  if (!stricmp(field, "tlgrm_interval"))    
  {
    settingTelegramInterval     = String(newValue).toInt();  
    CHANGE_INTERVAL_SEC(nextTelegram, settingTelegramInterval)
  }

  if (!stricmp(field, "index_page"))        strCopy(settingIndexPage, (sizeof(settingIndexPage) -1), newValue);  

#ifdef USE_MINDERGAS
  if (!stricmp(field, "MindergasToken"))    strCopy(settingMindergasToken, 20, newValue);  
#endif //USE_MINDERGAS

#ifdef USE_MQTT
  if (!stricmp(field, "mqtt_broker"))  {
    DebugT("settingMQTTbroker! to : ");
    memset(settingMQTTbroker, '\0', sizeof(settingMQTTbroker));
    strCopy(settingMQTTbroker, 100, newValue);
    Debugf("[%s]\r\n", settingMQTTbroker);
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_broker_port")) {
    settingMQTTbrokerPort = String(newValue).toInt();  
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_user")) {
    strCopy(settingMQTTuser    ,35, newValue);  
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_passwd")) {
    strCopy(settingMQTTpasswd  ,25, newValue);  
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_interval")) {
    settingMQTTinterval   = String(newValue).toInt();  
    CHANGE_INTERVAL_SEC(publishMQTTtimer, settingMQTTinterval);
  }
  if (!stricmp(field, "mqtt_toptopic"))     strCopy(settingMQTTtopTopic, 20, newValue);  
#endif

  writeSettings();
  
} // updateSetting()


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
