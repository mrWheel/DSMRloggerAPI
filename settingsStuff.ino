/*
***************************************************************************  
**  Program  : settingsStuff, part of DSMRloggerAPI
**  Version  : v0.0.7
**
**  Copyright (c) 2019 Willem Aandewiel
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
  DebugT(F("Start writing setting data "));

  file.print("EnergyDeliveredT1 = "); file.println(String(settingEDT1, 5));     Debug(F("."));
  file.print("EnergyDeliveredT2 = "); file.println(String(settingEDT2, 5));     Debug(F("."));
  file.print("EnergyReturnedT1 = ");  file.println(String(settingERT1, 5));     Debug(F("."));
  file.print("EnergyReturnedT2 = ");  file.println(String(settingERT2, 5));     Debug(F("."));
  file.print("GASDeliveredT = ");     file.println(String(settingGDT,  5));     Debug(F("."));
  file.print("EnergyVasteKosten = "); file.println(String(settingENBK, 2));     Debug(F("."));
  file.print("GasVasteKosten = ");    file.println(String(settingGNBK, 2));     Debug(F("."));
  file.print("SleepTime = ");         file.println(settingSleepTime);           Debug(F("."));
  file.print("TelegramInterval = ");  file.println(settingInterval);            Debug(F("."));
  file.print("BackGroundColor = ");   file.println(settingBgColor);             Debug(F("."));
  file.print("FontColor = ");         file.println(settingFontColor);           Debug(F("."));

#ifdef USE_MQTT
  sprintf(settingMQTTbroker, "%s:%d", MQTTbrokerURL, MQTTbrokerPort);
  file.print("MQTTbroker = ");        file.println(settingMQTTbroker);          Debug(F("."));
  file.print("MQTTUser = ");          file.println(settingMQTTuser);            Debug(F("."));
  file.print("MQTTpasswd = ");        file.println(settingMQTTpasswd);          Debug(F("."));
  file.print("MQTTinterval = ");      file.println(settingMQTTinterval);        Debug(F("."));
  file.print("MQTTtopTopic = ");      file.println(settingMQTTtopTopic);        Debug(F("."));
#endif
  
  file.print("MindergasAuthtoken = ");file.println(settingMindergasAuthtoken);  Debug(F("."));
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
    DebugT(F("TelegramInterval = "));  Debugln(settingInterval);            
    DebugT(F("BackGroundColor = "));   Debugln(settingBgColor);             
    DebugT(F("FontColor = "));         Debugln(settingFontColor);   

#ifdef USE_MQTT
    sprintf(settingMQTTbroker, "%s:%d", MQTTbrokerURL, MQTTbrokerPort);
    DebugT(F("MQTTbroker = "));        Debugln(settingMQTTbroker);          
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
    DebugT(F("MindergasAuthtoken = "));Debugln(settingMindergasAuthtoken);  
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

  settingEDT1       = 0.1;
  settingEDT2       = 0.2;
  settingERT1       = 0.3;
  settingERT2       = 0.4;
  settingGDT        = 0.5;
  settingENBK       = 15.15;
  settingGNBK       = 11.11;
  settingInterval   = 10; // seconds
  settingSleepTime  =  0; // infinite
  strCopy(settingBgColor, sizeof(settingBgColor), "deepskyblue");
  strCopy(settingFontColor, sizeof(settingFontColor), "white");
  settingMQTTbroker[0]     = '\0';
  settingMQTTuser[0]       = '\0';
  settingMQTTpasswd[0]     = '\0';
  settingMQTTinterval      = 60;
  sprintf(settingMQTTtopTopic, "%s", _HOSTNAME);
  settingMindergasAuthtoken[0] = '\0';

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
    sTmp                = file.readStringUntil('\n');
    sTmp.replace("\r", "");
    //DebugTf("[%s] (%d)\r\n", sTmp.c_str(), sTmp.length());
    int8_t wc = splitString(sTmp.c_str(), '=', words, 10);
    words[0].toLowerCase();
    nColor = words[1].substring(0,15);

    if (words[0].equalsIgnoreCase("EnergyDeliveredT1")) settingEDT1         = words[1].toFloat();  
    if (words[0].equalsIgnoreCase("EnergyDeliveredT2")) settingEDT2         = words[1].toFloat();  
    if (words[0].equalsIgnoreCase("EnergyReturnedT1"))  settingERT1         = words[1].toFloat();  
    if (words[0].equalsIgnoreCase("EnergyReturnedT2"))  settingERT2         = words[1].toFloat();  
    if (words[0].equalsIgnoreCase("GasDeliveredT"))     settingGDT          = words[1].toFloat();  
    if (words[0].equalsIgnoreCase("EnergyVasteKosten")) settingENBK         = words[1].toFloat();
    if (words[0].equalsIgnoreCase("GasVasteKosten"))    settingGNBK         = words[1].toFloat();

    if (words[0].equalsIgnoreCase("SleepTime"))         settingSleepTime    = words[1].toInt();  
    if (words[0].equalsIgnoreCase("TelegramInterval"))  settingInterval     = words[1].toInt();  

    //if (words[0].equalsIgnoreCase("BackgroundColor")) strncpy(settingBgColor,   String(nColor).substring(0,(MAXCOLORNAME - 1)).c_str());  
    if (words[0].equalsIgnoreCase("BackgroundColor"))   strCopy(settingBgColor, (MAXCOLORNAME - 1),   words[1].c_str());  
    //if (words[0].equalsIgnoreCase("FontColor"))       strncpy(settingFontColor, String(nColor).substring(0,(MAXCOLORNAME - 1)).c_str());  
    if (words[0].equalsIgnoreCase("FontColor"))         strCopy(settingFontColor, (MAXCOLORNAME - 1),  words[1].c_str()); 

    if (words[0].equalsIgnoreCase("MindergasAuthtoken"))  strCopy(settingMindergasAuthtoken, 20, words[1].c_str());  
   
#ifdef USE_MQTT
    if (words[0].equalsIgnoreCase("MQTTbroker"))  {
      memset(settingMQTTbroker, '\0', sizeof(settingMQTTbroker));
      memset(MQTTbrokerURL, '\0', sizeof(MQTTbrokerURL));
      strCopy(settingMQTTbroker, 100, words[1].c_str());
      int cln = String(settingMQTTbroker).indexOf(":",0);
      DebugTf("settingMQTTbroker[%s] => found[:] @[%d]\r\n", settingMQTTbroker, cln);
      if (cln > -1) 
      {
        MQTTbrokerPort = String(settingMQTTbroker).substring((cln+1)).toInt();
        settingMQTTbroker[cln] = '\0';
        strCopy(MQTTbrokerURL, cln, settingMQTTbroker);
        //DebugTf("URL[%s]->Port[%d]\r\n", MQTTbrokerURL, MQTTbrokerPort);
        sprintf(settingMQTTbroker, "%s:%d", MQTTbrokerURL, MQTTbrokerPort);
        //DebugTf("new -> settingMQTTbroker[%s]\r\n", settingMQTTbroker);
      } 
      else 
      {
        strCopy(MQTTbrokerURL, 100, settingMQTTbroker);
        MQTTbrokerPort = 1883;
      }
      DebugTf("[%s] => MQTTbrokerURL[%s], port[%d]\r\n", settingMQTTbroker, MQTTbrokerURL, MQTTbrokerPort);
    }
    if (words[0].equalsIgnoreCase("MQTTuser"))      strCopy(settingMQTTuser    ,35 ,words[1].c_str());  
    if (words[0].equalsIgnoreCase("MQTTpasswd"))    strCopy(settingMQTTpasswd  ,25, words[1].c_str());  
    if (words[0].equalsIgnoreCase("MQTTinterval"))  settingMQTTinterval        = words[1].toInt();  
    if (words[0].equalsIgnoreCase("MQTTtopTopic"))  strCopy(settingMQTTtopTopic, 20, words[1].c_str());  
#endif
    
  } // while available()

  file.close();  
  DebugTln(F(" .. done\r"));

  if (!show) return;
  
  Debugln(F("\r\n==== Settings ===================================================\r"));
  Debugf("   Energy Delivered Tarief 1 : %9.7f\r\n",  settingEDT1);
  Debugf("   Energy Delivered Tarief 2 : %9.7f\r\n",  settingEDT2);
  Debugf("   Energy Delivered Tarief 1 : %9.7f\r\n",  settingERT1);
  Debugf("   Energy Delivered Tarief 2 : %9.7f\r\n",  settingERT2);
  Debugf("        Gas Delivered Tarief : %9.7f\r\n",  settingGDT);
  Debugf("     Energy Netbeheer Kosten : %9.2f\r\n",  settingENBK);
  Debugf("        Gas Netbeheer Kosten : %9.2f\r\n",  settingGNBK);
  Debugf("   Telegram Process Interval : %d\r\n", settingInterval);
  Debugf("OLED Sleep Min. (0=oneindig) : %d\r\n", settingSleepTime);
  Debugf("            BackGround Color : %s\r\n", settingBgColor);
  Debugf("                  Font Color : %s\r\n", settingFontColor);
#ifdef USE_MQTT
  Debugln(F("\r\n==== MQTT settings ==============================================\r"));
  Debugf("          MQTT broker URL/IP : %s:%d", MQTTbrokerURL, MQTTbrokerPort);
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
  Debugf("         Mindergas Authtoken : %s\r\n", settingMindergasAuthtoken);
#endif  
  
  Debugln(F("-\r"));

} // readSettings()


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
