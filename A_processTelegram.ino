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
