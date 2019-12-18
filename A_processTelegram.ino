
// Eigenlijk is dit ook een FSM van maken ;-)
// States: INIT, PROCESS_TIMESTAMP, WAIT_FOR_THIRD_TELEGRAM, WAIT_FOR_CHANGE_HOUR_OR_DAY, WRITE_TO_FILE_AND_STATUS)
// Echter met wat kleine wijzigingen kan het volgens mij ook, veel eenvoudiger.

void processTelegram()
{
  DebugTf("Telegram[%d]=>DSMRdata.timestamp[%s]\r\n", telegramCount
                                                    , DSMRdata.timestamp.c_str());
  strcpy(newTimestamp, DSMRdata.timestamp.c_str()); 

  actT = epoch(actTimestamp, strlen(actTimestamp), false);
  newT = epoch(newTimestamp, strlen(newTimestamp), true); // update system time
  
  // Skip first 3 telegrams .. just to settle down a bit ;-) ==> omdat je anders altijd begint met uur/dag change.
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
