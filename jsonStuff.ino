/*
***************************************************************************
**  Program  : jsonStuff, part of DSMRloggerAPI
**  Version  : v3.0
**
**  Copyright (c) 2020 .. 2022 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/
static char objSprtr[10] = "";

//=======================================================================
void sendStartJsonObj(const char *objName)
{
  char sBuff[50] = "";
  objSprtr[0]    = '\0';

  snprintf(sBuff, sizeof(sBuff), "{\"%s\":[\r\n", objName);
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", sBuff);

} // sendStartJsonObj()


//=======================================================================
void sendEndJsonObj()
{
  httpServer.sendContent("\r\n]}\r\n");

  //httpServer.sendHeader( "Content-Length", "0");
  //httpServer.send ( 200, "application/json", "");

} // sendEndJsonObj()

//=======================================================================
void sendNestedJsonObj(uint8_t recNr, const char *recID, uint8_t slot, float EDT1, float EDT2, float ERT1, float ERT2, float GDT)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"recnr\": %d, \"recid\": \"%s\", \"slot\": %d,"
           "\"edt1\": %.3f, \"edt2\": %.3f,"
           "\"ert1\": %.3f, \"ert2\": %.3f,"
           "\"gdt\": %.3f}"
           , objSprtr, recNr, recID, slot, EDT1, EDT2, ERT1, ERT2, GDT);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonObj(int, *char, int, float, float, float, float, float)


//=======================================================================
void sendNestedJsonObj(const char *cName, const char *cValue, const char *cUnit)
{
  char jsonBuff[JSON_BUFF_MAX] = "";

  if (strlen(cUnit) == 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": \"%s\"}"
             , objSprtr, cName, cValue);
  }
  else
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": \"%s\", \"unit\": \"%s\"}"
             , objSprtr, cName, cValue, cUnit);
  }

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonObj(*char, *char, *char)

//---------------------------------------------------------------
void sendNestedJsonObj(const char *cName, const char *cValue)
{
  char noUnit[] = {'\0'};

  sendNestedJsonObj(cName, cValue, noUnit);

} // sendNestedJsonObj(*char, *char)


//=======================================================================
void sendNestedJsonObj(const char *cName, String sValue, const char *cUnit)
{
  char jsonBuff[JSON_BUFF_MAX] = "";

  if (sValue.length() > (JSON_BUFF_MAX - 65) )
  {
    DebugTf("[2] sValue.length() [%d]\r\n", sValue.length());
  }

  if (strlen(cUnit) == 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": \"%s\"}"
             , objSprtr, cName, sValue.c_str());
  }
  else
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": \"%s\", \"unit\": \"%s\"}"
             , objSprtr, cName, sValue.c_str(), cUnit);
  }

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonObj(*char, String, *char)

//---------------------------------------------------------------
void sendNestedJsonObj(const char *cName, String sValue)
{
  char noUnit[] = {'\0'};

  if (sValue.length() > (JSON_BUFF_MAX - 60) )
  {
    sendNestedJsonObj(cName, sValue.substring(0, (JSON_BUFF_MAX - (strlen(cName) + 30))), noUnit);
  }
  else
  {
    sendNestedJsonObj(cName, sValue, noUnit);
  }

} // sendNestedJsonObj(*char, String)


//=======================================================================
void sendNestedJsonObj(const char *cName, int32_t iValue, const char *cUnit)
{
  char jsonBuff[200] = "";

  if (strlen(cUnit) == 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %d}"
             , objSprtr, cName, iValue);
  }
  else
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %d, \"unit\": \"%s\"}"
             , objSprtr, cName, iValue, cUnit);
  }

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonObj(*char, int, *char)

//---------------------------------------------------------------
void sendNestedJsonObj(const char *cName, int32_t iValue)
{
  char noUnit[] = {'\0'};

  sendNestedJsonObj(cName, iValue, noUnit);

} // sendNestedJsonObj(*char, int)


//=======================================================================
void sendNestedJsonObj(const char *cName, uint32_t uValue, const char *cUnit)
{
  char jsonBuff[200] = "";

  if (strlen(cUnit) == 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %u}"
             , objSprtr, cName, uValue);
  }
  else
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %u, \"unit\": \"%s\"}"
             , objSprtr, cName, uValue, cUnit);
  }

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonObj(*char, uint, *char)

//---------------------------------------------------------------
void sendNestedJsonObj(const char *cName, uint32_t uValue)
{
  char noUnit[] = {'\0'};

  sendNestedJsonObj(cName, uValue, noUnit);

} // sendNestedJsonObj(*char, uint)


//=======================================================================
void sendNestedJsonObj(const char *cName, float fValue, const char *cUnit)
{
  char jsonBuff[200] = "";

  if (strlen(cUnit) == 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %.3f}"
             , objSprtr, cName, fValue);
  }
  else
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %.3f, \"unit\": \"%s\"}"
             , objSprtr, cName, fValue, cUnit);
  }

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonObj(*char, float, *char)

//---------------------------------------------------------------
void sendNestedJsonObj(const char *cName, float fValue)
{
  char noUnit[] = {'\0'};

  sendNestedJsonObj(cName, fValue, noUnit);

} // sendNestedJsonObj(*char, float)


//=======================================================================
//----- v0 api ----------------------------------------------------------
//=======================================================================
void sendNestedJsonV0Obj(const char *cName, uint32_t uValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s \"%s\": %u"
           , objSprtr, cName, uValue);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonV0Obj(*char, uint)

//---------------------------------------------------------------
void sendNestedJsonV0Obj(const char *cName, float fValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s \"%s\": %.3f"
           , objSprtr, cName, fValue);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonV0Obj(*char, float)

//---------------------------------------------------------------
void sendNestedJsonV0Obj(const char *cName, int32_t iValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s \"%s\": %u"
           , objSprtr, cName, iValue);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedV0Obj(*char, int)

//---------------------------------------------------------------
void sendNestedJsonV0Obj(const char *cName, String sValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s \"%s\": \"%s\""
           , objSprtr, cName
           , sValue.substring(0, (150 - (strlen(cName)))).c_str());

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendNestedJsonV0Obj(*char, String)


//=======================================================================
// ************ function to build Json Settings string ******************
//=======================================================================
void sendJsonSettingObj(const char *cName, float fValue, const char *fType, int minValue, int maxValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %.3f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
           , objSprtr, cName, fValue, fType, minValue, maxValue);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendJsonSettingObj(*char, float, *char, int, int)


//=======================================================================
void sendJsonSettingObj(const char *cName, float fValue, const char *fType, int minValue, int maxValue, int decPlaces)
{
  char jsonBuff[200] = "";

  switch(decPlaces)
  {
    case 0:
      snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %.0f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
               , objSprtr, cName, fValue, fType, minValue, maxValue);
      break;
    case 2:
      snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %.2f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
               , objSprtr, cName, fValue, fType, minValue, maxValue);
      break;
    case 5:
      snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %.5f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
               , objSprtr, cName, fValue, fType, minValue, maxValue);
      break;
    default:
      snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %f, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
               , objSprtr, cName, fValue, fType, minValue, maxValue);

  }

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendJsonSettingObj(*char, float, *char, int, int, int)


//=======================================================================
void sendJsonSettingObj(const char *cName, int iValue, const char *iType, int minValue, int maxValue)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\": %d, \"type\": \"%s\", \"min\": %d, \"max\": %d}"
           , objSprtr, cName, iValue, iType, minValue, maxValue);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendJsonSettingObj(*char, int, *char, int, int)


//=======================================================================
void sendJsonSettingObj(const char *cName, const char *cValue, const char *sType, int maxLen)
{
  char jsonBuff[200] = "";

  snprintf(jsonBuff, sizeof(jsonBuff), "%s{\"name\": \"%s\", \"value\":\"%s\", \"type\": \"%s\", \"maxlen\": %d}"
           , objSprtr, cName, cValue, sType, maxLen);

  httpServer.sendContent(jsonBuff);
  sprintf(objSprtr, ",\r\n");

} // sendJsonSettingObj(*char, *char, *char, int, int)



//=========================================================================
// function to build MQTT Json string ** max message size is 128 bytes!! **
//=========================================================================
void createMQTTjsonMessage(char *mqttBuff, const char *cName, const char *cValue, const char *cUnit)
{
  if (strlen(cUnit) == 0)
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": \"%s\"}]}"
             , cName, cValue);
  }
  else
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": \"%s\", \"unit\": \"%s\"}]}"
             , cName, cValue, cUnit);
  }

} // createMQTTjsonMessage(*char, *char, *char)

//---------------------------------------------------------------
void createMQTTjsonMessage(char *mqttBuff, const char *cName, const char *cValue)
{
  char noUnit[] = {'\0'};

  createMQTTjsonMessage(mqttBuff, cName, cValue, noUnit);

} // createMQTTjsonMessage(*char, *char)


//=======================================================================
void createMQTTjsonMessage(char *mqttBuff, const char *cName, String sValue, const char *cUnit)
{
  uint16_t hdrSize = (strlen(cName) * 2) +strlen(settingMQTTtopTopic) + 30;

  if (strlen(cUnit) == 0)
  {
    //DebugTf("sValue.lenght()[%d], strlen(%s)[%d]\r\n", sValue.length(), cName, strlen(cName));
    if ((hdrSize + sValue.length()) >= 128)
    {
      String tmp = sValue.substring(0, (128 - hdrSize));
      snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": \"%s\"}]}"
               , cName, tmp.c_str());
    }
    else
    {
      snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": \"%s\"}]}"
               , cName, sValue.c_str());
    }
  }
  else
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": \"%s\", \"unit\": \"%s\"}]}"
             , cName, sValue.c_str(), cUnit);
  }

} // createMQTTjsonMessage(*char, String, *char)

//---------------------------------------------------------------
void createMQTTjsonMessage(char *mqttBuff, const char *cName, String sValue)
{
  char noUnit[] = {'\0'};

  createMQTTjsonMessage(mqttBuff, cName, sValue, noUnit);

} // createMQTTjsonMessage(*char, String)


//=======================================================================
void createMQTTjsonMessage(char *mqttBuff, const char *cName, int32_t iValue, const char *cUnit)
{
  if (strlen(cUnit) == 0)
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": %d}]}"
             , cName, iValue);
  }
  else
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": %d, \"unit\": \"%s\"}]}"
             , cName, iValue, cUnit);
  }

} // createMQTTjsonMessage(*char, int, *char)

//---------------------------------------------------------------
void createMQTTjsonMessage(char *mqttBuff, const char *cName, int32_t iValue)
{
  char noUnit[] = {'\0'};

  createMQTTjsonMessage(mqttBuff, cName, iValue, noUnit);

} // createMQTTjsonMessage(char *mqttBuff, *char, int)


//=======================================================================
void createMQTTjsonMessage(char *mqttBuff, const char *cName, uint32_t uValue, const char *cUnit)
{
  if (strlen(cUnit) == 0)
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": %u}]}"
             , cName, uValue);
  }
  else
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": %u, \"unit\": \"%s\"}]}"
             , cName, uValue, cUnit);
  }

} // createMQTTjsonMessage(*char, uint, *char)

//---------------------------------------------------------------
void createMQTTjsonMessage(char *mqttBuff, const char *cName, uint32_t uValue)
{
  char noUnit[] = {'\0'};

  createMQTTjsonMessage(mqttBuff, cName, uValue, noUnit);

} // createMQTTjsonMessage(char *mqttBuff, *char, uint)


//=======================================================================
void createMQTTjsonMessage(char *mqttBuff, const char *cName, float fValue, const char *cUnit)
{
  if (strlen(cUnit) == 0)
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": %.3f}]}"
             , cName, fValue);
  }
  else
  {
    snprintf(mqttBuff, MQTT_BUFF_MAX, "{\"%s\": [{\"value\": %.3f, \"unit\": \"%s\"}]}"
             , cName, fValue, cUnit);
  }

} // createMQTTjsonMessage(*char, float, *char)

//---------------------------------------------------------------
void createMQTTjsonMessage(char *mqttBuff, const char *cName, float fValue)
{
  char noUnit[] = {'\0'};

  createMQTTjsonMessage(mqttBuff, cName, fValue, noUnit);

} // createMQTTjsonMessage(char *mqttBuff, *char, float)


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
