/* 
***************************************************************************  
**  Program  : restAPI, part of DSMRfirmwareAPI
**  Version  : v0.0.7
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

// global vars
static  DynamicJsonDocument jsonDoc(4000);  // generic doc to return, clear() before use!
JsonObject root;

char    rootName[20] = "";
char    fieldName[40] = "";
volatile bool histApiSemafore = false;

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
  DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (strcmp(word4, "info") == 0)
  {
    sendDeviceInfo();
  }
  else if (strcmp(word4, "time") == 0)
  {
    sendDeviceTime();
  }
  else sendApiInfo();
  
} // handleDevApi()


//====================================================
void handleHistApi(const char *word4, const char *word5, const char *word6)
{
  int8_t    fileType = 0;
  uint32_t  overruleSemafore = millis() + 2000;
  char      fileName[20] = "";
  
  //while (histApiSemafore && ((millis() - overruleSemafore) < 0))
  //{
  //  yield();
  //}
  histApiSemafore = true;

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
    histApiSemafore = false;
    return;
  }
  sendJsonHist(fileType, fileName);

  histApiSemafore = false;


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
    sendSmInfo();
  }
  else if (strcmp(word4, "actual") == 0)
  {
    sendActual();
  }
  else if (strcmp(word4, "fields") == 0)
  {
    sendJsonFields(word4, word5);
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
    <tr><td><b>/api/v1/hist/hours/</b></td><td>Readings from hours table in JSON format</td></tr>\
    <tr><td><b>/api/v1/hist/days/</b></td><td>Readings from days table in JSON format</td></tr>\
    <tr><td><b>/api/v1/hist/months/</b></td><td>Readings from months table in JSON format</td></tr>\
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
    See the documenatation @weet ik veel \
  </body>\
</html>\r\n";

  httpServer.send ( 200, "text/html", APIhelp );
  
} // sendApiInfo()


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
void sendSmInfo() 
{
  char objSprtr[10] = "";
  
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", "{\"sminfo\":[\r\n");
    
//-Slimme Meter Info----------------------------------------------------------
  sendNestedJsonObject(objSprtr, "Identification",      DSMRdata.identification.c_str());
  sprintf(objSprtr, ",\r\n");
  sendNestedJsonObject(objSprtr, "P1_Version",          DSMRdata.p1_version.c_str());
  sendNestedJsonObject(objSprtr, "Equipment_Id",        DSMRdata.equipment_id.c_str());
  sendNestedJsonObject(objSprtr, "Electricity_Tariff",  DSMRdata.electricity_tariff.c_str());
  sendNestedJsonObject(objSprtr, "Gas_Device_Type",     DSMRdata.gas_device_type);
  sendNestedJsonObject(objSprtr, "Gas_Equipment_Id",    DSMRdata.gas_equipment_id.c_str());
  
  httpServer.sendContent("\r\n]}\r\n");

} // sendSmInfo()


//=======================================================================
void sendDeviceInfo() 
{
  char objSprtr[10] = "";
  
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", "{\"devinfo\":[\r\n");

//-Device Info-----------------------------------------------------------------
  sendNestedJsonObject(objSprtr, "Author", "Willem Aandewiel (www.aandewiel.nl)");
  sprintf(objSprtr, ",\r\n");
  sendNestedJsonObject(objSprtr, "FwVersion", _FW_VERSION);

  sprintf(cMsg, "%s %s", __DATE__, __TIME__);
  sendNestedJsonObject(objSprtr, "Compiled", cMsg);
  //String(__DATE__).c_str()
  //                                 + String( "  " ).c_str()
  //                                 + String( __TIME__ ).c_str());
  sendNestedJsonObject(objSprtr, "maxFreeBlock", ESP.getMaxFreeBlockSize());
  sendNestedJsonObject(objSprtr, "ChipID", String( ESP.getChipId(), HEX ).c_str());
  sendNestedJsonObject(objSprtr, "CoreVersion", String( ESP.getCoreVersion() ).c_str() );
  sendNestedJsonObject(objSprtr, "SdkVersion", String( ESP.getSdkVersion() ).c_str());
  sendNestedJsonObject(objSprtr, "CpuFreq", ESP.getCpuFreqMHz(), "MHz");
  sendNestedJsonObject(objSprtr, "SketchSize", formatFloat( (ESP.getSketchSize() / 1024.0), 3), "Bytes");
  sendNestedJsonObject(objSprtr, "FreeSketchSpace", formatFloat( (ESP.getFreeSketchSpace() / 1024.0), 3), "Bytes");

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  sendNestedJsonObject(objSprtr, "FlashChipID", cMsg);  // flashChipId
  sendNestedJsonObject(objSprtr, "FlashChipSize", formatFloat((ESP.getFlashChipSize() / 1024.0 / 1024.0), 3), "MB");
  sendNestedJsonObject(objSprtr, "FlashChipRealSize", formatFloat((ESP.getFlashChipRealSize() / 1024.0 / 1024.0), 3), "MB");
  sendNestedJsonObject(objSprtr, "FlashChipSpeed", formatFloat((ESP.getFlashChipSpeed() / 1000.0 / 1000.0), 0), "MHz");

  FlashMode_t ideMode = ESP.getFlashChipMode();
  sendNestedJsonObject(objSprtr, "FlashChipMode", flashMode[ideMode]);
  sendNestedJsonObject(objSprtr, "BoardType",
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
  sendNestedJsonObject(objSprtr, "SSID", WiFi.SSID().c_str());
//sendNestedJsonObject(objSprtr, "PskKey", WiFi.psk());   // uncomment if you want to see this
  sendNestedJsonObject(objSprtr, "IpAddress", WiFi.localIP().toString().c_str());
  sendNestedJsonObject(objSprtr, "WiFiRSSI", WiFi.RSSI());
  sendNestedJsonObject(objSprtr, "Hostname", _HOSTNAME);
  sendNestedJsonObject(objSprtr, "upTime", upTime().c_str());
  sendNestedJsonObject(objSprtr, "TelegramCount", telegramCount);
  sendNestedJsonObject(objSprtr, "TelegramErrors", telegramErrors);
  sendNestedJsonObject(objSprtr, "lastReset", lastReset.c_str());

  httpServer.sendContent("\r\n]}\r\n");

} // sendDeviceInfo()


//=======================================================================
void sendDeviceTime() 
{
  char objSprtr[10] = "";
  
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", "{\"devtime\":[\r\n");

//-Device Info-----------------------------------------------------------------
  sendNestedJsonObject(objSprtr, "time", buildDateTimeString(actTimestamp, sizeof(actTimestamp)).c_str()); 
  sprintf(objSprtr, ",\r\n");
  sendNestedJsonObject(objSprtr, "epoch", now());

  httpServer.sendContent("\r\n]}\r\n");

} // sendDeviceTime()

//=======================================================================
void sendActual() 
{
  jsonDoc.clear();

//-Totalen----------------------------------------------------------

  jsonDoc["Timestamp"]                 = pTimestamp;
  jsonDoc["Energy_Delivered"]          = formatFloat(EnergyDelivered, 3);
  jsonDoc["Energy_Returned"]           = formatFloat(EnergyReturned, 3);
  jsonDoc["Gas_Delivered"]             = formatFloat(GasDelivered, 2);
  jsonDoc["Energy_Delivered_Tariff1"]  = formatFloat(EnergyDeliveredTariff1, 3);
  jsonDoc["Energy_Delivered_Tariff2"]  = formatFloat(EnergyDeliveredTariff2, 3);
  jsonDoc["Energy_Returned_Tariff1"]   = formatFloat(EnergyReturnedTariff1, 3);
  jsonDoc["Energy_Returned_Tariff2"]   = formatFloat(EnergyReturnedTariff2, 3);
  jsonDoc["Energy_Tariff"]             = ElectricityTariff;
  jsonDoc["Power_Delivered"]           = formatFloat(PowerDelivered, 3);
  jsonDoc["Power_Returned"]            = formatFloat(PowerReturned, 3);
  jsonDoc["Voltage_l1"]                = formatFloat(Voltage_l1, 1);
  jsonDoc["Current_l1"]                = Current_l1;
  jsonDoc["Voltage_l2"]                = formatFloat(Voltage_l2, 1);
  jsonDoc["Current_l2"]                = Current_l2;
  jsonDoc["Voltage_l3"]                = formatFloat(Voltage_l3, 1);
  jsonDoc["Current_l3"]                = Current_l3;
  jsonDoc["Power_Delivered_l1"]        = PowerDelivered_l1;
  jsonDoc["Power_Returned_l1"]         = PowerReturned_l1;
  jsonDoc["Power_Delivered_l2"]        = PowerDelivered_l2;
  jsonDoc["Power_Returned_l2"]         = PowerReturned_l2;
  jsonDoc["Power_Delivered_l3"]        = PowerDelivered_l3;
  jsonDoc["Power_Returned_l3"]         = PowerReturned_l3;
  
  _returnJSON( jsonDoc.as<JsonObject>() );

} // sendActual()


//=======================================================================
struct buildJsonApi {
//  {
//    "apiName": [
//      {
//        "name": "identification",
//        "value":"XMX5LGBBLB2410065887"
//      },
//      {
//        "name":"energy_returned_tariff2",
//        "value":79.141,
//        "unit":"kWh"}
//      }
//    ]
//  }

    JsonArray root = jsonDoc.createNestedArray(rootName);
    bool  skip = false;
    
    template<typename Item>
    void apply(Item &i) {
      skip = false;
      String Name = Item::name;
      if (strlen(fieldName) > 1 && Name != String(fieldName))
      {
        skip = true;
      }
      if (!skip)
      {
        JsonObject nested = root.createNestedObject();

        nested["name"]  = Name;
        if (i.present()) 
        {
          String Unit = Item::unit();
          nested["value"] = typecastValue(i.val());
        
          if (Unit.length() > 0)
          {
            nested["unit"]  = Unit;
          }
        }
        else
        {
          nested["value"] = "-";
        }
      }
  }

};

//=======================================================================
void sendJsonFields(const char *Name, const char *fName) 
{
  jsonDoc.clear();

  strcpy(rootName, Name);
  if (strlen(fName) > 0)
  {
        strCopy(fieldName, sizeof(fieldName), fName);
  }
  else fieldName[0] = '\0';

  DSMRdata.applyEach(buildJsonApi());

  _returnJSON( jsonDoc.as<JsonObject>() );

} // sendJsonFields()


//=======================================================================
void sendJsonHist(int8_t fileType, const char *fileName) 
{
  uint8_t slotNr = timestampToHourSlot(actTimestamp, strlen(actTimestamp));  
  uint8_t startSlot, nrSlots, recNr  = 0;
    
  switch(fileType) {
    case HOURS:   startSlot       = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
                  nrSlots         = _NO_HOUR_SLOTS_;
                  break;
    case DAYS:    startSlot       = timestampToDaySlot(actTimestamp, strlen(actTimestamp));
                  nrSlots         = _NO_DAY_SLOTS_;
                  break;
    case MONTHS:  startSlot       = timestampToMonthSlot(actTimestamp, strlen(actTimestamp));
                  nrSlots         = _NO_MONTH_SLOTS_;
                  break;
  }

  String jsonString;
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);

  jsonDoc.clear();
  httpServer.send(200, "application/json", "{\"hist\":[\r\n");

  slotNr += nrSlots +1; // <==== voorbij actuele slot!

  DebugTf("sendJsonHist startSlot[%02d]\r\n", (slotNr % nrSlots));

  for (uint8_t s = 0; s < nrSlots; s++)
  {
    jsonDoc.clear();

    if (s == 0)
          jsonString = "";    
    else  jsonString = ",\r\n";    
    
    root = jsonDoc.to<JsonObject>();
    readOneSlot(fileType, fileName, s, s+slotNr, true, "hist") ;
    serializeJson(jsonDoc, jsonString);         // machine readable
    httpServer.sendContent(jsonString);
  }
  httpServer.sendContent("\r\n]}\r\n");
  
  //httpServer.sendHeader( "Content-Length", "0");
  //httpServer.send ( 200, "application/json", "");
  
} // sendJsonHist()


//=======================================================================
void sendNestedJsonObject(const char *objSprtr, const char *fName, const char *fValue, const char *fUnit)
{
  char jsonBuff[200] = "";
  
  if (strlen(fUnit) == 0)
  {
    sprintf(jsonBuff, "%s{\"name\": \"%s\", \"value\": \"%s\"}"
                                      , objSprtr, fName, fValue);
  }
  else
  {
    sprintf(jsonBuff, "%s{\"name\": \"%s\", \"value\": \"%s\", \"unit\": \"%s\"}"
                                      , objSprtr, fName, fValue, fUnit);
  }

  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObject(a, b, c)

void sendNestedJsonObject(const char *objSprtr, const char *fName, const char *fValue)
{
  char noUnit[] = {'\0'};

  sendNestedJsonObject(objSprtr, fName, fValue, noUnit);
  
} // sendNestedJsonObject(a, b)


void sendNestedJsonObject(const char *objSprtr, const char *fName, int32_t fValue, const char *fUnit)
{
  char jsonBuff[200] = "";
  
  if (strlen(fUnit) == 0)
  {
    sprintf(jsonBuff, "%s{\"name\": \"%s\", \"value\": %d}"
                                      , objSprtr, fName, fValue);
  }
  else
  {
    sprintf(jsonBuff, "%s{\"name\": \"%s\", \"value\": %d, \"unit\": \"%s\"}"
                                      , objSprtr, fName, fValue, fUnit);
  }

  httpServer.sendContent(jsonBuff);

} // sendNestedJsonObject(a, n, c)

void sendNestedJsonObject(const char *objSprtr, const char *fName, int32_t fValue)
{
  char noUnit[] = {'\0'};

  sendNestedJsonObject(objSprtr, fName, fValue, noUnit);
  
} // sendNestedJsonObject(a, n)

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
