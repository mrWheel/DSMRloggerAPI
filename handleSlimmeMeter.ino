/*
***************************************************************************  
**  Program  : handleSlimmeMeter - part of DSMRloggerAPI
**  Version  : v2.0.1
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/  

#if !defined(HAS_NO_SLIMMEMETER)
//==================================================================================
void handleSlimmemeter()
{
  //DebugTf("showRaw (%s)\r\n", showRaw ?"true":"false");
  if (showRaw) {
    //-- process telegrams in raw mode
    processSlimmemeterRaw();
  } 
  else
  {
    processSlimmemeter();
  } 

} // handleSlimmemeter()


//==================================================================================
void processSlimmemeterRaw()
{
  char    tlgrm[1200] = "";
   
  DebugTf("handleSlimmerMeter RawCount=[%4d]\r\n", showRawCount);
  showRawCount++;
  showRaw = (showRawCount <= 20);
  if (!showRaw)
  {
    showRawCount  = 0;
    return;
  }
  
  if (settingOledType > 0)
  {
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "-------------------------",0);
    oled_Print_Msg(2, "Raw Format",0);
    snprintf(cMsg, sizeof(cMsg), "Raw Count %4d", showRawCount);
    oled_Print_Msg(3, cMsg, 0);
  }

  slimmeMeter.enable(true);
  Serial.setTimeout(5000);  // 5 seconds must be enough ..
  memset(tlgrm, 0, sizeof(tlgrm));
  int l = 0;
  // The terminator character is discarded from the serial buffer.
  l = Serial.readBytesUntil('/', tlgrm, sizeof(tlgrm));
  // now read from '/' to '!'
  // The terminator character is discarded from the serial buffer.
  l = Serial.readBytesUntil('!', tlgrm, sizeof(tlgrm));
  Serial.setTimeout(1000);  // seems to be the default ..
//  DebugTf("read [%d] bytes\r\n", l);
  if (l == 0) 
  {
    DebugTln(F("RawMode: Timerout - no telegram received within 5 seconds"));
    return;
  }

  tlgrm[l++] = '!';
#if !defined( USE_PRE40_PROTOCOL )
  // next 6 bytes are "<CRC>\r\n"
  for (int i=0; ( i<6 && (i<(sizeof(tlgrm)-7)) ); i++)
  {
    tlgrm[l++] = (char)Serial.read();
  }
#else
  tlgrm[l++]    = '\r';
  tlgrm[l++]    = '\n';
#endif
  tlgrm[(l +1)] = '\0';
  // shift telegram 1 char to the right (make room at pos [0] for '/')
  for (int i=strlen(tlgrm); i>=0; i--) { tlgrm[i+1] = tlgrm[i]; yield(); }
  tlgrm[0] = '/'; 
  //Post result to Debug 
  Debugf("Telegram (%d chars):\r\n/%s\r\n", strlen(tlgrm), tlgrm); 
  return;
  
} // processSlimmemeterRaw()


//==================================================================================
void processSlimmemeter()
{
  slimmeMeter.loop();
  if (slimmeMeter.available()) 
  {
    DebugTf("telegramCount=[%d] telegramErrors=[%d]\r\n", telegramCount, telegramErrors);
    Debugln(F("\r\n[Time----][FreeHeap/mBlck][Function----(line):\r"));
    // Voorbeeld: [21:00:11][   9880/  8960] loop        ( 997): read telegram [28] => [140307210001S]
    telegramCount++;
        
    DSMRdata = {};
    String    DSMRerror;
        
    if (slimmeMeter.parse(&DSMRdata, &DSMRerror))   // Parse succesful, print result
    {
      if (telegramCount > (UINT32_MAX - 10)) 
      {
        delay(1000);
        ESP.reset();
        delay(1000);
      }
      digitalWrite(LED_BUILTIN, LED_OFF);
      if (DSMRdata.identification_present)
      {
        //--- this is a hack! The identification can have a backslash in it
        //--- that will ruin javascript processing :-(
        for(int i=0; i<DSMRdata.identification.length(); i++)
        {
          if (DSMRdata.identification[i] == '\\') DSMRdata.identification[i] = '=';
          yield();
        }
      }
            
      if (DSMRdata.p1_version_be_present)
      {
        DSMRdata.p1_version = DSMRdata.p1_version_be;
        DSMRdata.p1_version_be_present  = false;
        DSMRdata.p1_version_present     = true;
      }

      modifySmFaseInfo();
      /****
      if (!settingSmHasFaseInfo)
      {
        if (DSMRdata.power_delivered_present && !DSMRdata.power_delivered_l1_present)
        {
          DSMRdata.power_delivered_l1 = DSMRdata.power_delivered;
          DSMRdata.power_delivered_l1_present = true;
          DSMRdata.power_delivered_l2_present = true;
          DSMRdata.power_delivered_l3_present = true;
        }
        if (DSMRdata.power_returned_present && !DSMRdata.power_returned_l1_present)
        {
          DSMRdata.power_returned_l1 = DSMRdata.power_returned;
          DSMRdata.power_returned_l1_present = true;
          DSMRdata.power_returned_l2_present = true;
          DSMRdata.power_returned_l3_present = true;
        }
      } // No Fase Info
      ****/
      
#ifdef USE_NTP_TIME
      if (!DSMRdata.timestamp_present)                        //USE_NTP
      {                                                       //USE_NTP
        sprintf(cMsg, "%02d%02d%02d%02d%02d%02dW\0\0"         //USE_NTP
                        , (year() - 2000), month(), day()     //USE_NTP
                        , hour(), minute(), second());        //USE_NTP
        DSMRdata.timestamp         = cMsg;                    //USE_NTP
        DSMRdata.timestamp_present = true;                    //USE_NTP
      }                                                       //USE_NTP
#endif

      //-- handle mbus delivered values
      gasDelivered = modifyMbusDelivered();

      processTelegram();
      if (Verbose2) 
      {
        DSMRdata.applyEach(showValues());
      }
          
    } 
    else                  // Parser error, print error
    {
      telegramErrors++;
      #ifdef USE_SYSLOGGER
        sysLog.writef("Parse error\r\n%s\r\n\r\n", DSMRerror.c_str());
      #endif
      DebugTf("Parse error\r\n%s\r\n\r\n", DSMRerror.c_str());
      //--- set DTR to get a new telegram as soon as possible
      slimmeMeter.enable(true);
      slimmeMeter.loop();
    }

    if ( (telegramCount > 25) && (telegramCount % (2100 / (settingTelegramInterval + 1)) == 0) )
    {
      DebugTf("Processed [%d] telegrams ([%d] errors)\r\n", telegramCount, telegramErrors);
      writeToSysLog("Processed [%d] telegrams ([%d] errors)", telegramCount, telegramErrors);
    }
        
  } // if (slimmeMeter.available()) 
  
} // handleSlimmeMeter()

#endif  // HAS_NO_SLIMMEMETER


//==================================================================================
void modifySmFaseInfo()
{
  if (!settingSmHasFaseInfo)
  {
        if (DSMRdata.power_delivered_present && !DSMRdata.power_delivered_l1_present)
        {
          DSMRdata.power_delivered_l1 = DSMRdata.power_delivered;
          DSMRdata.power_delivered_l1_present = true;
          DSMRdata.power_delivered_l2_present = true;
          DSMRdata.power_delivered_l3_present = true;
        }
        if (DSMRdata.power_returned_present && !DSMRdata.power_returned_l1_present)
        {
          DSMRdata.power_returned_l1 = DSMRdata.power_returned;
          DSMRdata.power_returned_l1_present = true;
          DSMRdata.power_returned_l2_present = true;
          DSMRdata.power_returned_l3_present = true;
        }
  } // No Fase Info
  
} //  modifySmFaseInfo()


//==================================================================================
float modifyMbusDelivered()
{
  float tmpGasDelivered = 0;

      mbus1Delivered =  (float)DSMRdata.mbus1_delivered
                      + (float)DSMRdata.mbus1_delivered_ntc
                      + (float)DSMRdata.mbus1_delivered_dbl;
      if (settingMbus1Type > 0) DebugTf("mbus1Delivered [%.3f]\r\n", mbus1Delivered);
//    DSMRdata.mbus1_delivered.val()       = (TimestampedFixedValue)(mbus1Delivered*1);
      DSMRdata.mbus1_delivered_present     = true;
      DSMRdata.mbus1_delivered_ntc_present = false;
      DSMRdata.mbus1_delivered_dbl_present = false;
      if ( (settingMbus1Type == 3) && (DSMRdata.mbus1_device_type == 3) )
      {
        tmpGasDelivered = mbus1Delivered;
        DebugTf("gasDelivered [%.3f]\r\n", tmpGasDelivered);
      }

      mbus2Delivered =  (float)DSMRdata.mbus2_delivered
                      + (float)DSMRdata.mbus2_delivered_ntc
                      + (float)DSMRdata.mbus2_delivered_dbl;
      if (settingMbus2Type > 0) DebugTf("mbus2Delivered [%.3f]\r\n", mbus2Delivered);
      DSMRdata.mbus2_delivered_present     = true;
      DSMRdata.mbus2_delivered_ntc_present = false;
      DSMRdata.mbus2_delivered_dbl_present = false;
      if ( (settingMbus2Type == 3) && (DSMRdata.mbus2_device_type == 3) )
      {
        tmpGasDelivered = mbus2Delivered;
        DebugTf("gasDelivered [%.3f]\r\n", tmpGasDelivered);
      }

      mbus3Delivered =  (float)DSMRdata.mbus3_delivered
                      + (float)DSMRdata.mbus3_delivered_ntc
                      + (float)DSMRdata.mbus3_delivered_dbl;
      if (settingMbus3Type > 0) DebugTf("mbus3Delivered [%.3f]\r\n", mbus3Delivered);
      DSMRdata.mbus3_delivered_present     = true;
      DSMRdata.mbus3_delivered_ntc_present = false;
      DSMRdata.mbus3_delivered_dbl_present = false;
      if ( (settingMbus3Type == 3) && (DSMRdata.mbus3_device_type == 3) )
      {
        tmpGasDelivered = mbus3Delivered;
        DebugTf("gasDelivered [%.3f]\r\n", tmpGasDelivered);
      }

      mbus4Delivered =  (float)DSMRdata.mbus4_delivered
                      + (float)DSMRdata.mbus4_delivered_ntc
                      + (float)DSMRdata.mbus4_delivered_dbl;
      if (settingMbus4Type > 0) DebugTf("mbus4Delivered [%.3f]\r\n", mbus4Delivered);
      DSMRdata.mbus4_delivered_present     = true;
      DSMRdata.mbus4_delivered_ntc_present = false;
      DSMRdata.mbus4_delivered_dbl_present = false;
      if ( (settingMbus4Type == 3) && (DSMRdata.mbus4_device_type == 3) )
      {
        tmpGasDelivered = mbus4Delivered;
        DebugTf("gasDelivered [%.3f]\r\n", tmpGasDelivered);
      }
    return tmpGasDelivered;
    
} //  modifyMbusDelivered()

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
