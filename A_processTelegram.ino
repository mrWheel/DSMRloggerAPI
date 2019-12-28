/*
***************************************************************************  
**  Program  : processTelegram - part of DSMRfirmwareAPI
**  Version  : v0.0.7
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/  

void processTelegram()
{
  DebugTf("Telegram[%d]=>DSMRdata.timestamp[%s]\r\n", telegramCount
                                                    , DSMRdata.timestamp.c_str());
  strcpy(newTimestamp, DSMRdata.timestamp.c_str()); 

  actT = epoch(actTimestamp, strlen(actTimestamp), false);
  newT = epoch(newTimestamp, strlen(newTimestamp), true); // update system time
  
  // Skip first 3 telegrams .. just to settle down a bit ;-)
  
  if ((telegramCount - telegramErrors) < 3) 
  {
    return;
  }
  
  sprintf(actTimestamp, "%s", newTimestamp);

  // has the hour changed (or the day)  
  if (hour(actT) != hour(newT) || day(actT) != day(newT) )
  {
    writeDataToFiles();
    writeLastStatus();
  }

} // processTelegram()


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
