/*
***************************************************************************  
**  Program  : processTelegram - part of DSMRloggerAPI
**  Version  : v0.1.2
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/  

#if !defined(HAS_NO_SLIMMEMETER)
//==================================================================================
void handleSlimmemeter(){
  DebugTf("showRaw (%s)", showRaw ?"true":"false");
  if (!showRaw) {
    //-- process telegrams in raw mode
    processSlimmemeterRaw();
  } 
  else
  {
    //-- process telegrams in normal mode
    processSlimmemeter();
  } 
}
void processSlimmemeterRaw(){
  DebugTf("handleSlimmerMeter RawCount=[%4d]\r\n", showRawCount);
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    if (showRawCount == 0) 
    {
      oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
      oled_Print_Msg(1, "-------------------------",0);
      oled_Print_Msg(2, "Raw Format",0);
      sprintf(cMsg, "Raw Count %4d", showRawCount);
      oled_Print_Msg(3, cMsg, 0);
    }
  #endif

  while(Serial.available() > 0) 
  {   
    char rIn = Serial.read();       
    if (rIn == '!') 
    {
      showRawCount++;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
      sprintf(cMsg, "Raw Count %4d", showRawCount);
      oled_Print_Msg(3, cMsg, 0);
#endif
    }
    TelnetStream.write((char)rIn);
  }   // while Serial.available()
      
  if (showRawCount > 20) 
  {
    showRaw       = false;
    showRawCount  = 0;
  }
  return;
}
//==================================================================================
void processSlimmemeter(){
  DebugTf("handleSlimmerMeter telegramCount=[%4d] telegramErrors=[%4d]\r\n", telegramCount, telegramErrors);
  slimmeMeter.loop();
  if (slimmeMeter.available()) 
  {
    DebugTln(F("\r\n[Time----][FreeHeap/mBlck][Function----(line):\r"));
    // Voorbeeld: [21:00:11][   9880/  8960] loop        ( 997): read telegram [28] => [140307210001S]
    telegramCount++;
        
    DSMRdata = {};
    String    DSMRerror;
        
    if (slimmeMeter.parse(&DSMRdata, &DSMRerror))   // Parse succesful, print result
    {
      if (telegramCount > 1563000000) 
      {
        delay(1000);
        ESP.reset();
        delay(1000);
      }
      digitalWrite(LED_BUILTIN, LED_OFF);
      processTelegram();

      if (Verbose2) 
      {
        DSMRdata.applyEach(showValues());
        printData();
      }
          
    } 
    else                  // Parser error, print error
    {
      telegramErrors++;
      DebugTf("Parse error\r\n%s\r\n\r\n", DSMRerror.c_str());
      slimmeMeter.enable(true);
    }
        
  } // if (slimmeMeter.available()) 
      
} // handleSlimmeMeter()

#endif

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
