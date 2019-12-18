/* 
***************************************************************************  
**  Program  : MQTTstuff, part of DSMRfirmwareAPI
**  Version  : v0.0.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

#ifdef USE_MQTT
  #include <PubSubClient.h>       // MQTT client publish and subscribe functionality

  static            PubSubClient MQTTclient(wifiClient);

  int8_t            reconnectAttempts = 0;
  uint32_t          lastMQTTPublish   = 0;

  static bool       MQTTisConnected   = false;
  static uint32_t   MQTTretry;
  static IPAddress  MQTTbrokerIP;
  static char       MQTTbrokerURL[101];
  static uint16_t   MQTTbrokerPort = 1883;
  static char       MQTTbrokerIPchar[20];
#endif

//===========================================================================================
void startMQTT() 
{
#ifdef USE_MQTT
  
  DebugTln(F("Set MQTT broker.. "));  

  WiFi.hostByName(MQTTbrokerURL, MQTTbrokerIP);
  sprintf(MQTTbrokerIPchar, "%d.%d.%d.%d", MQTTbrokerIP[0]
                                         , MQTTbrokerIP[1]
                                         , MQTTbrokerIP[2]
                                         , MQTTbrokerIP[3]);
  if (!isValidIP(MQTTbrokerIP)) 
  {
    DebugTf("ERROR: [%s] => is not a valid URL\r\n", MQTTbrokerURL);
    MQTTisConnected = false;
  } else 
  {
    DebugTf("[%s] => setServer(%s, %d)\r\n", settingMQTTbroker, MQTTbrokerIPchar, MQTTbrokerPort);
    MQTTclient.setServer(MQTTbrokerIPchar, MQTTbrokerPort);         
  }

#endif
} // startMQTT()


//===========================================================================================
void handleMQTT() 
{
#ifdef USE_MQTT
  bool doTry = true;

  if (millis() > MQTTretry) 
  {
    MQTTretry = millis() + 600000;  // tien minuten voor re-connect
    DebugTf("MQTT server is [%s], IP[%s]\r\n", settingMQTTbroker, MQTTbrokerIPchar);
    if (String(settingMQTTbroker).length() < 10)   // not likely a valid server name
    {
      MQTTisConnected = false;
      return;
    }
    if (!isValidIP(MQTTbrokerIP)) 
    {
      MQTTisConnected = false;
      return;
    }
    if (!MQTTclient.connected() && doTry) 
    {
      if (!MQTTreconnect()) 
      {
        doTry = false;
        MQTTisConnected = false;
      } else 
      {
        MQTTisConnected = true;        
      }
    }
  }
  MQTTclient.loop();
  
#endif
} // handleMQTT()


//===========================================================================================
bool MQTTreconnect() 
{
#ifdef USE_MQTT
  String    MQTTclientId  = String(_HOSTNAME) + WiFi.macAddress();
  
  if (!isValidIP(MQTTbrokerIP)) 
  {
       return false;
  }

  reconnectAttempts = 0;
  // Loop until we're reconnected
  while (reconnectAttempts < 2) 
  {
      reconnectAttempts++;
      DebugT(F("Attempting MQTT connection ... "));
      // Attempt to connect
      if (String(settingMQTTuser).length() < 1) 
      {
        Debug(F("without a Username/Password "));
        MQTTisConnected = MQTTclient.connect(MQTTclientId.c_str());
      } else 
      {
        Debugf("Username [%s] ", settingMQTTuser);
        MQTTisConnected = MQTTclient.connect(MQTTclientId.c_str(), settingMQTTuser, settingMQTTpasswd);
      }
      if (MQTTisConnected) 
      {
        Debugln(F(" .. connected\r"));
        return true;
      } else 
      {
        Debugln(F(" .. \r"));
        DebugTf("failed, rc=[%d] ..  try again in 3 seconds\r\n", MQTTclient.state());
        // Wait 3 seconds before retrying
        delay(3000);
      }
  } // while ..

  DebugTln(F("5 attempts have failed.\r"));
  return false;

#endif
}

//===========================================================================================
String trimVal(char *in) 
{
  String Out = in;
  Out.trim();
  return Out;
} // trimVal()




//=======================================================================
struct buildJsonMQTT {
  String jsonString;
  char topicId[100];
  
    template<typename Item>
    void apply(Item &i) {
      if (i.present()) 
      {
        String Name = Item::name;
        String Unit = Item::unit();

        jsonDoc.clear();
        jsonString = "";
        
        sprintf(topicId, "%s/JSON/", settingMQTTtopTopic);
        strConcat(topicId, sizeof(topicId), Name.c_str());
        //DebugTf("topicId[%s]\r\n", topicId);
        
        JsonArray array = jsonDoc.createNestedArray(Name);
        JsonObject nested = array.createNestedObject();
        
        nested["value"] = typecastValue(i.val());
        
        if (Unit.length() > 0)
        {
          nested["unit"]  = Unit;
        }
        serializeJson(jsonDoc, jsonString); 
        //DebugTf("jsonString[%s]\r\n", jsonString.c_str());
        sprintf(cMsg, "%s", jsonString.c_str());
        //DebugTf("topicId[%s] -> [%s]\r\n", topicId, cMsg);
        MQTTclient.publish(topicId, cMsg); 
      }
  }
};

//===========================================================================================
void sendMQTTData() 
{
/*  
* The maximum message size, including header, is 128 bytes by default. 
* This is configurable via MQTT_MAX_PACKET_SIZE in PubSubClient.h.
* Als de json string te lang wordt zal de string niet naar de MQTT server
* worden gestuurd. Vandaar de korte namen als ED en PDl1.
* Mocht je langere, meer zinvolle namen willen gebruiken dan moet je de
* MQTT_MAX_PACKET_SIZE dus aanpassen!!!
*/
//===========================================================================================
#ifdef USE_MQTT
  String dateTime, topicId, json;

  if ((millis() - lastMQTTPublish) >= (settingMQTTinterval * 1000))
        lastMQTTPublish = millis();
  else  return;

  if (!MQTTisConnected || (strcmp(MQTTbrokerIPchar, "0.0.0.0")) == 0) return;

  DebugTf("Sending data to MQTT server [%s]:[%d]\r\n", MQTTbrokerURL, MQTTbrokerPort);

  DSMRdata.applyEach(buildJsonMQTT());

#endif

} // sendMQTTData()
                        
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
