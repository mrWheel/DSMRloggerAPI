/* 
***************************************************************************  
**  Program  : restAPI, part of DSMRfirmwareAPI
**  Version  : v0.0.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

// global vars
static  DynamicJsonDocument jsonDoc(4000);  // generic doc to return, clear() before use!
char    rootName[20] = "";
char    fieldName[40] = "";


//=======================================================================
void processAPI() 
{
  char *URI = (char*)httpServer.uri().c_str();
  char fName[40] = "";
  
  strToLower(URI);
  DebugTf("incomming URL is [%s] \r\n", URI);
  
  if (!strcasecmp(URI, "/api/v1/dev/info") )
  {
    sendDeviceInfo();
  }
  else if (!strcasecmp(URI, "/api/v1/sm/actual") )
  {
    sendActual();
  }
  else if (!strncmp(URI, "/api/v1/sm/fields", strlen("/api/v1/sm/fields")) )
  {
    if (strlen(URI) > strlen("/api/v1/sm/fields") ) 
    {
      strnCopy(fName, sizeof(fName), URI, strlen("/api/v1/sm/fields/"), strlen(URI));
      DebugTf("fName is [%s]\r\n", fName);
    }
    sendJson("fields", fName);
  }
  else 
  {
    sendApiInfo();
  }
  
} // processAPI()

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
    <ul>\
    <li><b>/api/v1/dev/info</b> - Device information in JSON format</li>\
    <li><b>/api/v1/sm/actual</b> - Actual data from Slimme Meter in JSON format</li>\
    <li><b>/api/v1/sm/fields</b> - All available fields from the Slimme Meter in JSON format</li>\
    <li><b>/api/v1/sm/fields/{fieldName}</b> - Only the requested field from the Slimme Meter in JSON format</li>\
    </ul>\
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
void _returnJSON(JsonObject obj);
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
  jsonDoc.clear();
    
//-Slimme Meter Info----------------------------------------------------------
  
  jsonDoc["Identification"]      = DSMRdata.identification;
  jsonDoc["P1_Version"]          = DSMRdata.p1_version;
  jsonDoc["Equipment_Id"]        = DSMRdata.equipment_id;
  jsonDoc["Electricity_Tariff"]  = DSMRdata.electricity_tariff;
  jsonDoc["Gas_Device_Type"]     = DSMRdata.gas_device_type;
  jsonDoc["Gas_Equipment_Id"]    = DSMRdata.gas_equipment_id;
  
//-Device Info-----------------------------------------------------------------
  jsonDoc["Author"]              = "Willem Aandewiel (www.aandewiel.nl)";
  jsonDoc["FwVersion"]           = String( _FW_VERSION );
  jsonDoc["Compiled"]            = String( __DATE__ ) 
                                      + String( "  " )
                                      + String( __TIME__ );
  jsonDoc["FreeHeap"]            = ESP.getFreeHeap();
  jsonDoc["maxFreeBlock"]        = ESP.getMaxFreeBlockSize();
  jsonDoc["ChipID"]              = String( ESP.getChipId(), HEX );
  jsonDoc["CoreVersion"]         = String( ESP.getCoreVersion() );
  jsonDoc["SdkVersion"]          = String( ESP.getSdkVersion() );
  jsonDoc["CpuFreqMHz"]          = ESP.getCpuFreqMHz();
  jsonDoc["SketchSize"]          = formatFloat( (ESP.getSketchSize() / 1024.0), 3);
  jsonDoc["FreeSketchSpace"]     = formatFloat( (ESP.getFreeSketchSpace() / 1024.0), 3);

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  jsonDoc["FlashChipID"]         = cMsg;  // flashChipId
  jsonDoc["FlashChipSize_MB"]    = formatFloat((ESP.getFlashChipSize() / 1024.0 / 1024.0), 3);
  jsonDoc["FlashChipRealSize_MB"]= formatFloat((ESP.getFlashChipRealSize() / 1024.0 / 1024.0), 3);
  jsonDoc["FlashChipSpeed_MHz"]  = formatFloat((ESP.getFlashChipSpeed() / 1000.0 / 1000.0), 0);

  FlashMode_t ideMode = ESP.getFlashChipMode();
  jsonDoc["FlashChipMode"]       = flashMode[ideMode];
  jsonDoc["BoardType"] = 
#ifdef ARDUINO_ESP8266_NODEMCU
     "ESP8266_NODEMCU";
#endif
#ifdef ARDUINO_ESP8266_GENERIC
     "ESP8266_GENERIC";
#endif
#ifdef ESP8266_ESP01
     "ESP8266_ESP01";
#endif
#ifdef ESP8266_ESP12
     "ESP8266_ESP12";
#endif
  jsonDoc["SSID"]                = WiFi.SSID();
//jsonDoc["PskKey"]              = WiFi.psk();   // uncomment if you want to see this
  jsonDoc["IpAddress"]           = WiFi.localIP().toString();
  jsonDoc["WiFiRSSI"]            = WiFi.RSSI();
  jsonDoc["Hostname"]            = _HOSTNAME;
  jsonDoc["upTime"]              = upTime();
  jsonDoc["TelegramCount"]       = telegramCount;
  jsonDoc["TelegramErrors"]      = telegramErrors;
  jsonDoc["lastReset"]           = lastReset;
  
  _returnJSON( jsonDoc.as<JsonObject>() );

} // sendDeviceInfo()


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
void sendJson(const char *Name, const char *fName) 
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

} // sendJson()

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
