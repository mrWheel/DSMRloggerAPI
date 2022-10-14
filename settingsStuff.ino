/*
***************************************************************************
**  Program  : settingsStuff, part of DSMRloggerAPI
**  Version  : v3.0
**
**  Copyright (c) 2020 .. 2022 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
* 1.0.11 added Mindergas Authtoken setting
*/

//=======================================================================
void writeSettings()
{
  yield();
  DebugT(F("Writing to ["));
  Debug(SETTINGS_FILE);
  Debugln(F("] ..."));
  File file = FSYS.open(SETTINGS_FILE, "w"); // open for reading and writing
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

  file.print("Hostname = ");
  file.println(settingHostname);
  Debug(F("."));
  file.print("preDSMR40 = ");
  file.println(settingPreDSMR40);
  Debug(F("."));
  file.print("dailyReboot = ");
  file.println(settingDailyReboot);
  Debug(F("."));
  file.print("EnergyDeliveredT1 = ");
  file.println(String(settingEDT1, 5));
  Debug(F("."));
  file.print("EnergyDeliveredT2 = ");
  file.println(String(settingEDT2, 5));
  Debug(F("."));
  file.print("EnergyReturnedT1 = ");
  file.println(String(settingERT1, 5));
  Debug(F("."));
  file.print("EnergyReturnedT2 = ");
  file.println(String(settingERT2, 5));
  Debug(F("."));
  file.print("mBus1Type = ");
  file.println(settingMbus1Type);
  Debug(F("."));
  file.print("mBus2Type = ");
  file.println(settingMbus2Type);
  Debug(F("."));
  file.print("mBus3Type = ");
  file.println(settingMbus3Type);
  Debug(F("."));
  file.print("mBus4Type = ");
  file.println(settingMbus4Type);
  Debug(F("."));
  file.print("GASDeliveredT = ");
  file.println(String(settingGDT,  5));
  Debug(F("."));
  file.print("EnergyVasteKosten = ");
  file.println(String(settingENBK, 2));
  Debug(F("."));
  file.print("GasVasteKosten = ");
  file.println(String(settingGNBK, 2));
  Debug(F("."));
  file.print("OledType = ");
  file.println(settingOledType);
  Debug(F("."));
  file.print("OledSleep = ");
  file.println(settingOledSleep);
  Debug(F("."));
  file.print("OledFlip = ");
  file.println(settingOledFlip);
  Debug(F("."));
  file.print("SmHasFaseInfo = ");
  file.println(settingSmHasFaseInfo);
  Debug(F("."));

  file.print("TelegramInterval = ");
  file.println(settingTelegramInterval);
  Debug(F("."));
  file.print("IndexPage = ");
  file.println(settingIndexPage);
  Debug(F("."));

#ifdef USE_MQTT
  //sprintf(settingMQTTbroker, "%s:%d", MQTTbroker, MQTTbrokerPort);
  file.print("MQTTbroker = ");
  file.println(settingMQTTbroker);
  Debug(F("."));
  file.print("MQTTbrokerPort = ");
  file.println(settingMQTTbrokerPort);
  Debug(F("."));
  file.print("MQTTUser = ");
  file.println(settingMQTTuser);
  Debug(F("."));
  file.print("MQTTpasswd = ");
  file.println(settingMQTTpasswd);
  Debug(F("."));
  file.print("MQTTinterval = ");
  file.println(settingMQTTinterval);
  Debug(F("."));
  file.print("MQTTtopTopic = ");
  file.println(settingMQTTtopTopic);
  Debug(F("."));
#endif

#ifdef USE_MINDERGAS
  file.print("MindergasAuthtoken = ");
  file.println(settingMindergasToken);
  Debug(F("."));
#endif

  file.close();

  Debugln(F(" done"));
  if (Verbose1)
  {
    DebugTln(F("Wrote this:"));
    DebugT(F("EnergyDeliveredT1 = "));
    Debugln(String(settingEDT1, 5));
    DebugT(F("EnergyDeliveredT2 = "));
    Debugln(String(settingEDT2, 5));
    DebugT(F("EnergyReturnedT1 = "));
    Debugln(String(settingERT1, 5));
    DebugT(F("EnergyReturnedT2 = "));
    Debugln(String(settingERT2, 5));
    DebugT(F("mBus1Type = "));
    Debugln(settingMbus1Type);
    DebugT(F("mBus2Type = "));
    Debugln(settingMbus2Type);
    DebugT(F("mBus3Type = "));
    Debugln(settingMbus3Type);
    DebugT(F("mBus4Type = "));
    Debugln(settingMbus4Type);
    DebugT(F("GASDeliveredT = "));
    Debugln(String(settingGDT,  5));
    DebugT(F("EnergyVasteKosten = "));
    Debugln(String(settingENBK, 2));
    DebugT(F("GasVasteKosten = "));
    Debugln(String(settingGNBK, 2));
    DebugT(F("OledType = "));
    if (settingOledType == 1)          Debugln("SDD1306");
    else if (settingOledType == 2)     Debugln("SH1306");
    else                               Debugln("None");
    DebugT(F("OledSleep = "));
    Debugln(settingOledSleep);
    DebugT(F("OledFlip = "));
    if (settingOledFlip)  Debugln(F("Yes"));
    else                  Debugln(F("No"));

    DebugT(F("SmHasFaseInfo"));
    if (settingSmHasFaseInfo == 1)     Debugln("Yes");
    else                               Debugln("No");
    DebugT(F("TelegramInterval = "));
    Debugln(settingTelegramInterval);
    DebugT(F("IndexPage = "));
    Debugln(settingIndexPage);

#ifdef USE_MQTT
    DebugT(F("MQTTbroker = "));
    Debugln(settingMQTTbroker);
    DebugT(F("MQTTbrokerPort = "));
    Debugln(settingMQTTbrokerPort);
    DebugT(F("MQTTUser = "));
    Debugln(settingMQTTuser);
#ifdef SHOW_PASSWRDS
    DebugT(F("MQTTpasswd = "));
    Debugln(settingMQTTpasswd);
#else
    DebugTln(F("MQTTpasswd = ********"));
#endif
    DebugT(F("MQTTinterval = "));
    Debugln(settingMQTTinterval);
    DebugT(F("MQTTtopTopic = "));
    Debugln(settingMQTTtopTopic);
#endif

#ifdef USE_MINDERGAS
#ifdef SHOW_PASSWRDS
    DebugT(F("MindergasAuthtoken = "));
    Debugln(settingMindergasToken);
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
  settingPreDSMR40          = 0;
  settingDailyReboot        = 0;
  settingMbus1Type          = 3;
  settingMbus2Type          = 0;
  settingMbus3Type          = 0;
  settingMbus4Type          = 0;
  settingEDT1               = 0.1;
  settingEDT2               = 0.2;
  settingERT1               = 0.3;
  settingERT2               = 0.4;
  settingGDT                = 0.5;
  settingENBK               = 15.15;
  settingGNBK               = 11.11;
  settingSmHasFaseInfo      =  1; // default: it does
  settingTelegramInterval   = 10; // seconds
  settingOledType           =  1; // 0=None, 1=SDD1306, 2=SH1106
  settingOledSleep          =  0; // infinite
  settingOledFlip           =  0; // Don't flip
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

  if (!FSYS.exists(SETTINGS_FILE))
  {
    DebugTln(F(" .. file not found! --> created file!"));
    writeSettings();
  }

  for (int T = 0; T < 2; T++)
  {
    file = FSYS.open(SETTINGS_FILE, "r");
    if (!file)
    {
      if (T == 0) DebugTf(" .. something went wrong opening [%s]\r\n", SETTINGS_FILE);
      else        DebugTln(T);
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
    nColor    = words[1].substring(0, 15);

    if (words[0].equalsIgnoreCase("Hostname"))            strCopy(settingHostname, 29, words[1].c_str());
    if (words[0].equalsIgnoreCase("preDSMR40"))           settingPreDSMR40    = words[1].toInt();
    if (words[0].equalsIgnoreCase("dailyReboot"))         settingDailyReboot  = words[1].toInt();
    if (words[0].equalsIgnoreCase("EnergyDeliveredT1"))   settingEDT1         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("EnergyDeliveredT2"))   settingEDT2         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("EnergyReturnedT1"))    settingERT1         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("EnergyReturnedT2"))    settingERT2         = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("mBus1Type"))           settingMbus1Type    = words[1].toInt();
    if (words[0].equalsIgnoreCase("mBus2Type"))           settingMbus2Type    = words[1].toInt();
    if (words[0].equalsIgnoreCase("mBus3Type"))           settingMbus3Type    = words[1].toInt();
    if (words[0].equalsIgnoreCase("mBus4Type"))           settingMbus4Type    = words[1].toInt();
    if (words[0].equalsIgnoreCase("GasDeliveredT"))       settingGDT          = strToFloat(words[1].c_str(), 5);
    if (words[0].equalsIgnoreCase("EnergyVasteKosten"))   settingENBK         = strToFloat(words[1].c_str(), 2);
    if (words[0].equalsIgnoreCase("GasVasteKosten"))      settingGNBK         = strToFloat(words[1].c_str(), 2);
    if (words[0].equalsIgnoreCase("SmHasFaseInfo"))
    {
      settingSmHasFaseInfo = words[1].toInt();
      if (settingSmHasFaseInfo != 0)  settingSmHasFaseInfo = 1;
      else                            settingSmHasFaseInfo = 0;
    }

    if (words[0].equalsIgnoreCase("OledType"))
    {
      settingOledType = words[1].toInt();
      if (settingOledType > 2) settingOledType = 1;
    }
    if (words[0].equalsIgnoreCase("OledSleep"))
    {
      settingOledSleep    = words[1].toInt();
      CHANGE_INTERVAL_MIN(oledSleepTimer, settingOledSleep);
    }
    if (words[0].equalsIgnoreCase("OledFlip"))    settingOledFlip = words[1].toInt();
    if (settingOledFlip != 0) settingOledFlip = 1;
    else                      settingOledFlip = 0;

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
    if (words[0].equalsIgnoreCase("MQTTuser"))            strCopy(settingMQTTuser, 35, words[1].c_str());
    if (words[0].equalsIgnoreCase("MQTTpasswd"))          strCopy(settingMQTTpasswd, 35, words[1].c_str());
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
  if (settingTelegramInterval  < 2) settingTelegramInterval = 10;
  if (settingMQTTbrokerPort    < 1) settingMQTTbrokerPort   = 1883;

  if (!show) return;

  Debugln(F("\r\n==== Settings ===================================================\r"));
  Debugf("                    Hostname : %s\r\n",     settingHostname);
  Debugf("   Pre DSMR 40 (0=No, 1=Yes) : %s\r\n",     settingPreDSMR40 ? "Yes":"No");
  Debugf("  Daily Reboot (0=No, 1=Yes) : %s\r\n",     settingDailyReboot ? "Yes":"No");
  Debugf("   Energy Delivered Tarief 1 : %9.7f\r\n",  settingEDT1);
  Debugf("   Energy Delivered Tarief 2 : %9.7f\r\n",  settingEDT2);
  Debugf("   Energy Delivered Tarief 1 : %9.7f\r\n",  settingERT1);
  Debugf("   Energy Delivered Tarief 2 : %9.7f\r\n",  settingERT2);
  Debugf("        Gas Delivered Tarief : %9.7f\r\n",  settingGDT);
  Debugf("     Energy Netbeheer Kosten : %9.2f\r\n",  settingENBK);
  Debugf("        Gas Netbeheer Kosten : %9.2f\r\n",  settingGNBK);
  Debugf("                 MBus 1 Type : %d\r\n",     settingMbus1Type);
  Debugf("                 MBus 2 Type : %d\r\n",     settingMbus2Type);
  Debugf("                 MBus 3 Type : %d\r\n",     settingMbus3Type);
  Debugf("                 MBus 4 Type : %d\r\n",     settingMbus4Type);
  Debugf("  SM Fase Info (0=No, 1=Yes) : %s\r\n",     settingSmHasFaseInfo ? "Yes":"No");
  Debugf("   Telegram Process Interval : %d\r\n",     settingTelegramInterval);
  Debugf("         OLED Type (0, 1, 2) : %d\r\n",     settingOledType);
  Debugf("OLED Sleep Min. (0=oneindig) : %d\r\n",     settingOledSleep);
  Debugf("     Flip Oled (0=No, 1=Yes) : %d\r\n",     settingOledFlip);
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

  if (!stricmp(field, "Hostname"))
  {
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
  if (!stricmp(field, "pre_DSMR40"))        settingPreDSMR40    = String(newValue).toInt();
  if (!stricmp(field, "dailyreboot"))       settingDailyReboot  = String(newValue).toInt();
  if (!stricmp(field, "ed_tariff1"))        settingEDT1         = String(newValue).toFloat();
  if (!stricmp(field, "ed_tariff2"))        settingEDT2         = String(newValue).toFloat();
  if (!stricmp(field, "er_tariff1"))        settingERT1         = String(newValue).toFloat();
  if (!stricmp(field, "er_tariff2"))        settingERT2         = String(newValue).toFloat();
  if (!stricmp(field, "electr_netw_costs")) settingENBK         = String(newValue).toFloat();

  if (!stricmp(field, "mbus1_type"))        settingMbus1Type    = String(newValue).toInt();
  if (!stricmp(field, "mbus2_type"))        settingMbus2Type    = String(newValue).toInt();
  if (!stricmp(field, "mbus3_type"))        settingMbus3Type    = String(newValue).toInt();
  if (!stricmp(field, "mbus4_type"))        settingMbus4Type    = String(newValue).toInt();
  if (!stricmp(field, "gd_tariff"))         settingGDT          = String(newValue).toFloat();
  if (!stricmp(field, "gas_netw_costs"))    settingGNBK         = String(newValue).toFloat();

  if (!stricmp(field, "sm_has_fase_info"))
  {
    settingSmHasFaseInfo = String(newValue).toInt();
    if (settingSmHasFaseInfo != 0)  settingSmHasFaseInfo = 1;
    else                            settingSmHasFaseInfo = 0;
  }

  if (!stricmp(field, "oled_type"))
  {
    settingOledType     = String(newValue).toInt();
    if (settingOledType > 2)  settingOledType = 1;
    oled_Init();
  }
  if (!stricmp(field, "oled_screen_time"))
  {
    settingOledSleep    = String(newValue).toInt();
    CHANGE_INTERVAL_MIN(oledSleepTimer, settingOledSleep)
  }
  if (!stricmp(field, "oled_flip_screen"))
  {
    settingOledFlip     = String(newValue).toInt();
    if (settingOledFlip != 0) settingOledFlip = 1;
    else                      settingOledFlip = 0;
    oled_Init();
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
  if (!stricmp(field, "mqtt_broker"))
  {
    DebugT("settingMQTTbroker! to : ");
    memset(settingMQTTbroker, '\0', sizeof(settingMQTTbroker));
    strCopy(settingMQTTbroker, 100, newValue);
    Debugf("[%s]\r\n", settingMQTTbroker);
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_broker_port"))
  {
    settingMQTTbrokerPort = String(newValue).toInt();
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_user"))
  {
    strCopy(settingMQTTuser, 35, newValue);
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_passwd"))
  {
    strCopy(settingMQTTpasswd, 35, newValue);
    mqttIsConnected = false;
    CHANGE_INTERVAL_MS(reconnectMQTTtimer, 100); // try reconnecting cyclus timer
  }
  if (!stricmp(field, "mqtt_interval"))
  {
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
