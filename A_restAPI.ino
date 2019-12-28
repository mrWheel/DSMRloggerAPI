/* 
***************************************************************************  
**  Program  : restAPI, part of DSMRloggerAPI
**  Version  : v0.0.7
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

// ******* Global Vars *******
uint32_t  antiWearTimer = 0;

char fieldName[40] = "";

char fieldsArray[50][35] = {{0}}; // to lookup fields 
int  fieldsElements      = 0;

int  actualElements = 8;
char actualArray[][35] = { "timestamp","energy_delivered_tariff1","energy_delivered_tariff2"
                          ,"energy_returned_tariff1","energy_returned_tariff2"
                          ,"power_delivered","power_returned"
                          ,"\0"};
int  infoElements = 7;
char infoArray[][35]   = { "identification","p1_version","equipment_id","electricity_tariff"
                          ,"gas_device_type","gas_equipment_id"
                          , "\0" };
  

//=======================================================================
void processAPI() 
{
  char *URI = (char*)httpServer.uri().c_str();
  char fName[40] = "";
  String words[10];

  strToLower(URI);
  DebugTf("incomming URI is [%s] \r\n", URI);

  int8_t wc = splitString(URI, '/', words, 10);
  
  if (words[1] != "api" || words[2] != "v1")
  {
    sendApiInfo();
  }

  if (words[3] == "dev")
  {
    handleDevApi(words[4].c_str(), words[5].c_str(), words[6].c_str());
  }
  else if (words[3] == "hist")
  {
    handleHistApi(words[4].c_str(), words[5].c_str(), words[6].c_str());
  }
  else if (words[3] == "sm")
  {
    handleSmApi(words[4].c_str(), words[5].c_str(), words[6].c_str());
  }
  else sendApiInfo();
  
} // processAPI()


//====================================================
void handleDevApi(const char *word4, const char *word5, const char *word6)
{
  //DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (strcmp(word4, "info") == 0)
  {
    sendDeviceInfo();
  }
  else if (strcmp(word4, "time") == 0)
  {
    sendDeviceTime();
  }
  else if (strcmp(word4, "settings") == 0)
  {
    sendDeviceSettings();
  }
  else sendApiInfo();
  
} // handleDevApi()


//====================================================
void handleHistApi(const char *word4, const char *word5, const char *word6)
{
  int8_t    fileType = 0;
  char      fileName[20] = "";
  
  DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (   strcmp(word4, "hours") == 0 )
  {
    fileType = HOURS;
    strCopy(fileName, sizeof(fileName), HOURS_FILE);
  }
  else if (strcmp(word4, "days") == 0 )
  {
    fileType = DAYS;
    strCopy(fileName, sizeof(fileName), DAYS_FILE);
  }
  else if (strcmp(word4, "months") == 0)
  {
    fileType = MONTHS;
    strCopy(fileName, sizeof(fileName), MONTHS_FILE);
  }
  else 
  {
    sendApiInfo();
    return;
  }
  if (strcmp(word5, "desc") == 0)
        sendJsonHist(fileType, fileName, actTimestamp, true);
  else  sendJsonHist(fileType, fileName, actTimestamp, false);

} // handleHistApi()


//====================================================
void handleSmApi(const char *word4, const char *word5, const char *word6)
{
  char    tlgrm[1200] = "";
  uint8_t p=0;  
  bool    stopParsingTelegram = false;

  DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (strcmp(word4, "info") == 0)
  {
    //sendSmInfo();
    copyToFieldsArray(infoArray, infoElements);
    sendJsonFields(word4);
  }
  else if (strcmp(word4, "actual") == 0)
  {
    //sendSmActual();
    copyToFieldsArray(actualArray, actualElements);
    sendJsonFields(word4);
  }
  else if (strcmp(word4, "fields") == 0)
  {
    fieldsElements = 0;

    if (strlen(word5) > 0)
    {
       memset(fieldsArray,0,sizeof(fieldsArray));
       strCopy(fieldsArray[0], 34,"timestamp");
       strCopy(fieldsArray[1], 34, word5);
       fieldsElements = 2;
    }
    sendJsonFields(word4);
  }
  else if (strcmp(word4, "telegram") == 0)
  {
    slimmeMeter.enable(true);

    Serial.setTimeout(5000);
    // The terminator character is discarded from the serial buffer.
    int l = Serial.readBytesUntil('/', tlgrm, sizeof(tlgrm));
    // now read from '/' to '!'
    // The terminator character is discarded from the serial buffer.
    l = Serial.readBytesUntil('!', tlgrm, sizeof(tlgrm));
    DebugTf("read [%d] bytes\r\n", l);
    tlgrm[l++] = '!';
    // next 6 bytes are "<CRC>\r\n"
    for (int i=l; (i<l+6) && (i<sizeof(tlgrm)); i++)
    {
      tlgrm[i] = (char)Serial.read();
    }
    for (int i=strlen(tlgrm); i>=0; i--) tlgrm[i+1] = tlgrm[i];
    tlgrm[0] = '/'; 
    Serial.setTimeout(1000);  // seems to be the default ..
    if (Verbose1) Debugf("Telegram (%d chars):\r\n/%s", strlen(tlgrm), tlgrm);
    httpServer.send(200, "application/plain", tlgrm);

  }
  else sendApiInfo();
  
} // handleSmApi()


//=======================================================================
// some helper functions
//void _returnJSON(JsonObject obj);
void _returnJSON(JsonObject obj)
{
  String jsonString;

  serializeJson(obj, jsonString);         // machine readable
  //serializeJsonPretty(obj, jsonString); // human readable for testing
  DebugTf("JSON String is %d chars\r\n", jsonString.length());
  //DebugTln(jsonString);
  httpServer.send(200, "application/json", jsonString);
  
} // _returnJSON()

void _returnJSON400(const char * message)
{
  httpServer.send(400, "application/json", message);

} // _returnJSON400()


//=======================================================================
void sendDeviceInfo() 
{
  sendStartJsonObj("devinfo");

  sendNestedJsonObj("Author", "Willem Aandewiel (www.aandewiel.nl)");
  sendNestedJsonObj("FwVersion", _FW_VERSION);

  sprintf(cMsg, "%s %s", __DATE__, __TIME__);
  sendNestedJsonObj("Compiled", cMsg);
  sendNestedJsonObj("maxFreeBlock", ESP.getMaxFreeBlockSize());
  sendNestedJsonObj("ChipID", String( ESP.getChipId(), HEX ).c_str());
  sendNestedJsonObj("CoreVersion", String( ESP.getCoreVersion() ).c_str() );
  sendNestedJsonObj("SdkVersion", String( ESP.getSdkVersion() ).c_str());
  sendNestedJsonObj("CpuFreq", ESP.getCpuFreqMHz(), "MHz");
  sendNestedJsonObj("SketchSize", formatFloat( (ESP.getSketchSize() / 1024.0), 3), "kB");
  sendNestedJsonObj("FreeSketchSpace", formatFloat( (ESP.getFreeSketchSpace() / 1024.0), 3), "kB");

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  sendNestedJsonObj("FlashChipID", cMsg);  // flashChipId
  sendNestedJsonObj("FlashChipSize", formatFloat((ESP.getFlashChipSize() / 1024.0 / 1024.0), 3), "MB");
  sendNestedJsonObj("FlashChipRealSize", formatFloat((ESP.getFlashChipRealSize() / 1024.0 / 1024.0), 3), "MB");
  sendNestedJsonObj("FlashChipSpeed", formatFloat((ESP.getFlashChipSpeed() / 1000.0 / 1000.0), 0), "MHz");

  FlashMode_t ideMode = ESP.getFlashChipMode();
  sendNestedJsonObj("FlashChipMode", flashMode[ideMode]);
  sendNestedJsonObj("BoardType",
#ifdef ARDUINO_ESP8266_NODEMCU
     "ESP8266_NODEMCU"
#endif
#ifdef ARDUINO_ESP8266_GENERIC
     "ESP8266_GENERIC"
#endif
#ifdef ESP8266_ESP01
     "ESP8266_ESP01"
#endif
#ifdef ESP8266_ESP12
     "ESP8266_ESP12"
#endif
  );
  sendNestedJsonObj("SSID", WiFi.SSID().c_str());
//sendNestedJsonObj("PskKey", WiFi.psk());   // uncomment if you want to see this
  sendNestedJsonObj("IpAddress", WiFi.localIP().toString().c_str());
  sendNestedJsonObj("WiFiRSSI", WiFi.RSSI());
  sendNestedJsonObj("Hostname", _HOSTNAME);
  sendNestedJsonObj("upTime", upTime());
  sendNestedJsonObj("TelegramCount", (int)telegramCount);
  sendNestedJsonObj("TelegramErrors", (int)telegramErrors);
  sendNestedJsonObj("lastReset", lastReset);

  httpServer.sendContent("\r\n]}\r\n");

} // sendDeviceInfo()


//=======================================================================
void sendDeviceTime() 
{
  sendStartJsonObj("devtime");
  sendNestedJsonObj("time", buildDateTimeString(actTimestamp, sizeof(actTimestamp)).c_str()); 
  sendNestedJsonObj("epoch", (int)now());

  sendEndJsonObj();

} // sendDeviceTime()


//=======================================================================
void sendDeviceSettings() 
{
  DebugTln("sending device settings ...\r");

  sendStartJsonObj("settings");
  
  sendNestedJsonObj("settingEDT1",          settingEDT1);
  sendNestedJsonObj("settingEDT2",          settingEDT2);
  sendNestedJsonObj("settingERT1",          settingERT1);
  sendNestedJsonObj("settingERT2",          settingERT2);
  sendNestedJsonObj("settingGDT",           settingGDT);
  sendNestedJsonObj("settingENBK",          settingENBK);
  sendNestedJsonObj("settingGNBK",          settingGNBK);
  sendNestedJsonObj("settingInterval",      settingInterval);
  sendNestedJsonObj("settingMQTTbroker",    settingMQTTbroker);
  sendNestedJsonObj("settingMQTTuser",      settingMQTTuser);
  sendNestedJsonObj("settingMQTTpasswd",    settingMQTTpasswd);
  sendNestedJsonObj("settingMQTTtopTopic",  settingMQTTtopTopic);
  sendNestedJsonObj("settingMQTTinterval",  settingMQTTinterval);

  sendEndJsonObj();

} // sendDeviceSettings()


//=======================================================================
struct buildJsonApi {
    bool  skip = false;
    
    template<typename Item>
    void apply(Item &i) {
      skip = false;
      String Name = Item::name;

      if (!isInFieldsArray(Name.c_str(), fieldsElements))
      {
        skip = true;
      }
      if (!skip)
      {
        if (i.present()) 
        {
          String Unit = Item::unit();
        
          if (Unit.length() > 0)
          {
            sendNestedJsonObj(Name.c_str(), typecastValue(i.val()), Unit.c_str());
          }
          else 
          {
            sendNestedJsonObj(Name.c_str(), typecastValue(i.val()));
          }
        }
        else
        {
          sendNestedJsonObj(Name.c_str(), "-");
        }
      }
  }

};

//=======================================================================
void sendJsonFields(const char *Name) 
{
  sendStartJsonObj(Name);
  DSMRdata.applyEach(buildJsonApi());
  sendEndJsonObj();

} // sendJsonFields()


//=======================================================================
void sendJsonHist(int8_t fileType, const char *fileName, const char *timeStamp, bool desc) 
{
  uint8_t startSlot, nrSlots, recNr  = 0;
  char    typeApi[10];

  if (millis() - antiWearTimer > 61000)
  {
    antiWearTimer = millis();
    writeDataToFiles();
    writeLastStatus();
  }
    
  switch(fileType) {
    case HOURS:   startSlot       = timestampToHourSlot(timeStamp, strlen(timeStamp));
                  nrSlots         = _NO_HOUR_SLOTS_;
                  strCopy(typeApi, 9, "hours");
                  break;
    case DAYS:    startSlot       = timestampToDaySlot(timeStamp, strlen(timeStamp));
                  nrSlots         = _NO_DAY_SLOTS_;
                  strCopy(typeApi, 9, "days");
                  break;
    case MONTHS:  startSlot       = timestampToMonthSlot(timeStamp, strlen(timeStamp));
                  nrSlots         = _NO_MONTH_SLOTS_;
                  strCopy(typeApi, 9, "months");
                  break;
  }

  sendStartJsonObj(typeApi);

  if (desc)
        startSlot += nrSlots +1; // <==== voorbij actuele slot!
  else  startSlot += nrSlots;    // <==== start met actuele slot!

  DebugTf("sendJsonHist startSlot[%02d]\r\n", (startSlot % nrSlots));

  for (uint8_t s = 0; s < nrSlots; s++)
  {
    if (desc)
          readOneSlot(fileType, fileName, s, (s +startSlot), true, "hist") ;
    else  readOneSlot(fileType, fileName, s, (startSlot -s), true, "hist") ;
  }
  sendEndJsonObj();
  
} // sendJsonHist()


bool isInFieldsArray(const char* lookUp, int elemts)
{
  if (elemts == 0) return true;

  for (int i=0; i<elemts; i++)
  {
    if (Verbose2) DebugTf("[%2d] Looking for [%s] in array[%s]\r\n", i, lookUp, fieldsArray[i]); 
    if (strcmp(lookUp, fieldsArray[i]) == 0) return true;
  }
  return false;
  
} // isInFieldsArray()


void copyToFieldsArray(const char inArray[][35], int elemts)
{
  int i = 0;
  memset(fieldsArray,0,sizeof(fieldsArray));
  DebugTln("start copying ....");
  
  for ( i=0; i<elemts; i++)
  {
    strncpy(fieldsArray[i], inArray[i], 34);
    DebugTf("[%2d] => inArray[%s] fieldsArray[%s]\r\n", i, inArray[i], fieldsArray[i]); 

  }
  fieldsElements = i;
  
} // copyToFieldsArray()


bool listFieldsArray(char inArray[][35])
{
  int i = 0;

  for ( i=0; strlen(inArray[i]) == 0; i++)
  {
    DebugTf("[%2d] => inArray[%s]\r\n", i, inArray[i]); 
  }
  
} // listFieldsArray()


//====================================================
void sendApiInfo()
{
  const char *APIhelp = \
"<html>\
  <head>\
    <title>API reference</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>API Reference</h1>\
    <table>\
    <tr><td><b>/api/v1/dev/info</b></td><td>Device information in JSON format</td></tr>\
    <tr><td><b>/api/v1/dev/time</b></td><td>Device time (epoch) in JSON format</td></tr>\
    <tr><td><b>/api/v1/dev/settings</b></td><td>Device settings in JSON format</td></tr>\
    <tr><td><b>/api/v1/hist/hours/{desc|asc}</b></td><td>Readings from hours table in JSON format</td></tr>\
    <tr><td><b>/api/v1/hist/days/{desc|asc}</b></td><td>Readings from days table in JSON format</td></tr>\
    <tr><td><b>/api/v1/hist/months/{desc|asc}</b></td><td>Readings from months table in JSON format</td></tr>\
    <tr><td><b>/api/v1/sm/info</b></td><td>Information about the Slimme Meter in JSON format</td></tr>\
    <tr><td><b>/api/v1/sm/actual</b></td><td>Actual data from Slimme Meter in JSON format</td></tr>\
    <tr><td><b>/api/v1/sm/fields</b></td><td>All available fields from the Slimme Meter in JSON format</td></tr>\
    <tr><td><b>/api/v1/sm/fields/{fieldName}</b></td><td>Only the requested field from the Slimme Meter in JSON format</td></tr>\
    <tr><td><b>/api/v1/sm/telegram</b></td><td>raw telegram including all \\r\\n line endings</td></tr>\
    </table>\
    <br>\
    JSON format: {\"fields\":[{\"name\":\"&lt;fieldName&gt;\",\"value\":&lt;value&gt;,\"unit\":\"&lt;unit&gt;\"}]}\
    <br>\
    <br>\
    <a href='/'>terug</a>\
  </body>\
</html>\r\n";

  httpServer.send ( 200, "text/html", APIhelp );
  
} // sendApiInfo()



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
