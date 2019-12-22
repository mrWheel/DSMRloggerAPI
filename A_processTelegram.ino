

void processTelegram()
{
  bool updateStatus = false;
  
  DebugTf("Telegram[%d]=>DSMRdata.timestamp[%s]\r\n", telegramCount
                                                    , DSMRdata.timestamp.c_str());
  strcpy(newTimestamp, DSMRdata.timestamp.c_str()); 

  actT = epoch(actTimestamp, strlen(actTimestamp), false);
  newT = epoch(newTimestamp, strlen(newTimestamp), true); // update system time
  // Skip first 3 telegrams .. just to settle doen a bit ;-)
  if ((telegramCount - telegramErrors) < 3) 
  {
    return;
  }
  // has the hout changed (or the day)
  if (hour(actT) != hour(newT) || day(actT) != day(newT) )
  {
    updateStatus = true;
  }

  sprintf(actTimestamp, "%s", newTimestamp);
  if (updateStatus)
  {
    writeDataToFiles();
    writeLastStatus();
    updateStatus = false;
  }

} // processTelegram()
