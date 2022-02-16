/*
***************************************************************************
**  Program  : handleSlimmeMeter - part of DSMRloggerAPI
**  Version  : v3.0
**
**  Copyright (c) 2020 .. 2022 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

#if !defined(HAS_NO_SLIMMEMETER)
//==================================================================================
void handleSlimmemeter()
{
  //DebugTf("showRaw (%s)\r\n", showRaw ?"true":"false");
  if (showRaw)
  {
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
  char    tlgrm[MAX_TLGRM_LENGTH] = "";
  char    checkSum[10]            = "";

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
    oled_Print_Msg(1, "-------------------------", 0);
    oled_Print_Msg(2, "Raw Format", 0);
    snprintf(cMsg, sizeof(cMsg), "Raw Count %4d", showRawCount);
    oled_Print_Msg(3, cMsg, 0);
  }

  slimmeMeter.enable(true);

  int l = 0, lc = 0;

  if (l=readSerialUntil(TelnetStream, '/', 5000, false))
  {
    TelnetStream.print("\r\n/");
    l  = readSerialUntil(TelnetStream, '!', 500, true);
    lc = readSerialUntil(TelnetStream, '\n', 500, true) +l;

  }
  else
  {
    DebugTln(F("RawMode: Timed-out - no telegram received within 5 seconds"));
    return;
  }

  Debugf("Telegram has %d chars\r\n\n", lc);
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

      if (!DSMRdata.timestamp_present)
      {
        sprintf(cMsg, "%02d%02d%02d%02d%02d%02d\0\0"
                , (year() - 2000), month(), day()
                , hour(), minute(), second());
        if (DSTactive)  strConcat(cMsg, 15, "S");
        else            strConcat(cMsg, 15, "W");
        DSMRdata.timestamp         = cMsg;
        DSMRdata.timestamp_present = true;
      }

      //-- handle mbus delivered values
      gasDelivered = modifyMbusDelivered();

      processTelegram();
      if (Verbose2)
      {
        DebugTln("showValues: ");
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

  if (DSMRdata.mbus1_delivered_ntc_present)
    DSMRdata.mbus1_delivered = DSMRdata.mbus1_delivered_ntc;
  else if (DSMRdata.mbus1_delivered_dbl_present)
    DSMRdata.mbus1_delivered = DSMRdata.mbus1_delivered_dbl;
  DSMRdata.mbus1_delivered_present     = true;
  DSMRdata.mbus1_delivered_ntc_present = false;
  DSMRdata.mbus1_delivered_dbl_present = false;
  if (settingMbus1Type > 0) DebugTf("mbus1_delivered [%.3f]\r\n", (float)DSMRdata.mbus1_delivered);
  if ( (settingMbus1Type == 3) && (DSMRdata.mbus1_device_type == 3) )
  {
    tmpGasDelivered = (float)(DSMRdata.mbus1_delivered * 1.0);
    DebugTf("gasDelivered .. [%.3f]\r\n", tmpGasDelivered);
  }

  if (DSMRdata.mbus2_delivered_ntc_present)
    DSMRdata.mbus2_delivered = DSMRdata.mbus2_delivered_ntc;
  else if (DSMRdata.mbus2_delivered_dbl_present)
    DSMRdata.mbus2_delivered = DSMRdata.mbus2_delivered_dbl;
  DSMRdata.mbus2_delivered_present     = true;
  DSMRdata.mbus2_delivered_ntc_present = false;
  DSMRdata.mbus2_delivered_dbl_present = false;
  if (settingMbus2Type > 0) DebugTf("mbus2_delivered [%.3f]\r\n", (float)DSMRdata.mbus2_delivered);
  if ( (settingMbus2Type == 3) && (DSMRdata.mbus2_device_type == 3) )
  {
    tmpGasDelivered = (float)(DSMRdata.mbus2_delivered * 1.0);
    DebugTf("gasDelivered .. [%.3f]\r\n", tmpGasDelivered);
  }

  if (DSMRdata.mbus3_delivered_ntc_present)
    DSMRdata.mbus3_delivered = DSMRdata.mbus3_delivered_ntc;
  else if (DSMRdata.mbus3_delivered_dbl_present)
    DSMRdata.mbus3_delivered = DSMRdata.mbus3_delivered_dbl;
  DSMRdata.mbus3_delivered_present     = true;
  DSMRdata.mbus3_delivered_ntc_present = false;
  DSMRdata.mbus3_delivered_dbl_present = false;
  if (settingMbus3Type > 0) DebugTf("mbus3_delivered [%.3f]\r\n", (float)DSMRdata.mbus3_delivered);
  if ( (settingMbus3Type == 3) && (DSMRdata.mbus3_device_type == 3) )
  {
    tmpGasDelivered = (float)(DSMRdata.mbus3_delivered * 1.0);
    DebugTf("gasDelivered .. [%.3f]\r\n", tmpGasDelivered);
  }

  if (DSMRdata.mbus4_delivered_ntc_present)
    DSMRdata.mbus4_delivered = DSMRdata.mbus4_delivered_ntc;
  else if (DSMRdata.mbus4_delivered_dbl_present)
    DSMRdata.mbus4_delivered = DSMRdata.mbus4_delivered_dbl;
  DSMRdata.mbus4_delivered_present     = true;
  DSMRdata.mbus4_delivered_ntc_present = false;
  DSMRdata.mbus4_delivered_dbl_present = false;
  if (settingMbus4Type > 0) DebugTf("mbus4_delivered [%.3f]\r\n", (float)DSMRdata.mbus4_delivered);
  if ( (settingMbus4Type == 3) && (DSMRdata.mbus4_device_type == 3) )
  {
    tmpGasDelivered = (float)(DSMRdata.mbus4_delivered * 1.0);
    DebugTf("gasDelivered .. [%.3f]\r\n", tmpGasDelivered);
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
