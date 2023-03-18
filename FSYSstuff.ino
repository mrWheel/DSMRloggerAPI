/*
***************************************************************************
**  Program  : FSYSstuff, part of DSMRloggerAPI
**  Version  : v3.0
**
**  Copyright (c) 2020 .. 2022 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
*/

int16_t   bytesWritten;

//static    FSInfo SPIFFSinfo;

//====================================================================
void readLastStatus()
{
  char buffer[100]  = "";
  char dummy[50]    = "";
  char spiffsTimestamp[20] = "";

  File _file = FSYS.open("/DSMRstatus.csv", "r");
  if (!_file)
  {
    DebugTln("read(): No /DSMRstatus.csv found ..");
  }
  if(_file.available())
  {
    int l = _file.readBytesUntil('\n', buffer, sizeof(buffer));
    buffer[l] = 0;
    DebugTf("read lastUpdate[%s]\r\n", buffer);
    sscanf(buffer, "%[^;]; %u; %u; %u; %[^;]", spiffsTimestamp
                                          , &nrReboots
                                          , &slotErrors
                                          , &telegramCount
                                          , dummy);
    DebugTf("values timestamp[%s], nrReboots[%u], slotErrors[%u], telegramCount[%u], dummy[%s]\r\n"
                                          , spiffsTimestamp
                                          , nrReboots
                                          , slotErrors
                                          , telegramCount
                                          , dummy);
    yield();
  }
  _file.close();
  if (strlen(spiffsTimestamp) != 13)
  {
    strcpy(spiffsTimestamp, "010101010101X");
  }
  snprintf(actTimestamp, sizeof(actTimestamp), "%s", spiffsTimestamp);

}  // readLastStatus()


//====================================================================
void writeLastStatus()
{
  if (ESP.getFreeHeap() < 8500)   // to prevent firmware from crashing!
  {
    DebugTf("Bailout due to low heap (%d bytes)\r\n", ESP.getFreeHeap());
    writeToSysLog("Bailout low heap (%d bytes)", ESP.getFreeHeap());
    return;
  }
  char buffer[100] = "";
  DebugTf("writeLastStatus() => %s; %u; %u; %u;\r\n", actTimestamp
                                                    , nrReboots
                                                    , slotErrors
                                                    , telegramCount);
  writeToSysLog("writeLastStatus() => %s; %u; %u; %u;", actTimestamp
                                                    , nrReboots
                                                    , slotErrors
                                                    , telegramCount);
  File _file = FSYS.open("/DSMRstatus.csv", "w");
  if (!_file)
  {
    DebugTln("write(): No /DSMRstatus.csv found ..");
  }
  snprintf(buffer, sizeof(buffer), "%-13.13s; %010u; %010u; %010u; %s;\n"
                                                    , actTimestamp
                                                    , nrReboots
                                                    , slotErrors
                                                    , telegramCount
                                                    , "meta data");
  _file.print(buffer);
  _file.flush();
  _file.close();

} // writeLastStatus()

//===========================================================================================
void buildDataRecordFromSM(char *recIn)
{
  static float GG = 1;
  char record[DATA_RECLEN + 1] = "";
  char key[10] = "";

  uint16_t recSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
  strCopy(key, 10, actTimestamp, 0, 8);

  snprintf(record, sizeof(record), (char *)DATA_FORMAT, key, (float)DSMRdata.energy_delivered_tariff1
           , (float)DSMRdata.energy_delivered_tariff2
           , (float)DSMRdata.energy_returned_tariff1
           , (float)DSMRdata.energy_returned_tariff2
           , (float)gasDelivered);
  // DATA + \n + \0
  fillRecord(record, DATA_RECLEN);

  strcpy(recIn, record);

} // buildDataRecordFromSM()

//===========================================================================================
uint16_t buildDataRecordFromJson(char *recIn, String jsonIn)
{
  //static float GG = 1;
  char      record[DATA_RECLEN + 1] = "";
  String    wOut[10];
  String    wPair[5];
  char      uKey[15]  = "";
  float     uEDT1     = 0.0;
  float     uEDT2     = 0.0;
  float     uERT1     = 0.0;
  float     uERT2     = 0.0;
  float     uGDT      = 0.0;
  uint16_t  recSlot;

  DebugTln(jsonIn);

  jsonIn.replace("{", "");
  jsonIn.replace("}", "");
  jsonIn.replace("\"", "");
  int8_t wp = splitString(jsonIn.c_str(), ',',  wOut, 9) ;
  for(int f=0; f<wp; f++)
  {
    splitString(wOut[f].c_str(), ':', wPair, 4);
    if (Verbose2) DebugTf("[%d] -> [%s]\r\n", f, wOut[f].c_str());
    if (wPair[0].indexOf("recid") == 0)  strCopy(uKey, 10, wPair[1].c_str());
    if (wPair[0].indexOf("edt1")  == 0)  uEDT1 = wPair[1].toFloat();
    if (wPair[0].indexOf("edt2")  == 0)  uEDT2 = wPair[1].toFloat();
    if (wPair[0].indexOf("ert1")  == 0)  uERT1 = wPair[1].toFloat();
    if (wPair[0].indexOf("ert2")  == 0)  uERT2 = wPair[1].toFloat();
    if (wPair[0].indexOf("gdt")   == 0)  uGDT  = wPair[1].toFloat();
  }
  strConcat(uKey, 15, "0101X");
  recSlot = timestampToMonthSlot(uKey, strlen(uKey));

  DebugTf("MONTHS: Write [%s] to slot[%02d] in %s\r\n", uKey, recSlot, MONTHS_FILE);
  snprintf(record, sizeof(record), (char *)DATA_FORMAT, uKey, (float)uEDT1
           , (float)uEDT2
           , (float)uERT1
           , (float)uERT2
           , (float)uGDT);

  // DATA + \n + \0
  fillRecord(record, DATA_RECLEN);

  strcpy(recIn, record);

  return recSlot;

} // buildDataRecordFromJson()


//===========================================================================================
void writeDataToFile(const char *fileName, const char *record, uint16_t slot, int8_t fileType)
{
  uint16_t offset = 0;

  if (!isNumericp(record, 8))
  {
    DebugTf("timeStamp [%-13.13s] not valid\r\n", record);
    slotErrors++;
    return;
  }

  if (!FSYS.exists(fileName))
  {
    switch(fileType)
    {
      case HOURS:
        createFile(fileName, _NO_HOUR_SLOTS_);
        break;
      case DAYS:
        createFile(fileName, _NO_DAY_SLOTS_);
        break;
      case MONTHS:
        createFile(fileName, _NO_MONTH_SLOTS_);
        break;
    }
  }

  File dataFile = FSYS.open(fileName, "r+");  // read and write ..
  if (!dataFile)
  {
    DebugTf("Error opening [%s]\r\n", fileName);
    return;
  }
  // slot goes from 0 to _NO_OF_SLOTS_
  // we need to add 1 to slot to skip header record!
  offset = ((slot + 1) * DATA_RECLEN);
  dataFile.seek(offset, SeekSet);
  int32_t bytesWritten = dataFile.print(record);
  if (bytesWritten != DATA_RECLEN)
  {
    DebugTf("ERROR! slot[%02d]: written [%d] bytes but should have been [%d]\r\n", slot, bytesWritten, DATA_RECLEN);
    writeToSysLog("ERROR! slot[%02d]: written [%d] bytes but should have been [%d]", slot, bytesWritten, DATA_RECLEN);
  }
  dataFile.close();

} // writeDataToFile()


//===========================================================================================
void writeDataToFiles()
{
  char record[DATA_RECLEN + 1] = "";
  uint16_t recSlot;

  buildDataRecordFromSM(record);
  DebugTf(">%s\r\n", record); // record ends in a \n

  // update HOURS
  recSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
  if (Verbose1) DebugTf("HOURS:  Write to slot[%02d] in %s\r\n", recSlot, HOURS_FILE);
  writeDataToFile(HOURS_FILE, record, recSlot, HOURS);
  writeToSysLog("HOURS: actTimestamp[%s], recSlot[%d]", actTimestamp, recSlot);

  // update DAYS
  recSlot = timestampToDaySlot(actTimestamp, strlen(actTimestamp));
  if (Verbose1) DebugTf("DAYS:   Write to slot[%02d] in %s\r\n", recSlot, DAYS_FILE);
  writeDataToFile(DAYS_FILE, record, recSlot, DAYS);

  // update MONTHS
  recSlot = timestampToMonthSlot(actTimestamp, strlen(actTimestamp));
  if (Verbose1) DebugTf("MONTHS: Write to slot[%02d] in %s\r\n", recSlot, MONTHS_FILE);
  writeDataToFile(MONTHS_FILE, record, recSlot, MONTHS);

} // writeDataToFiles(fileType, dataStruct newDat, int8_t slotNr)


//===========================================================================================
void readOneSlot(int8_t fileType, const char *fileName, uint8_t recNr
                 , uint8_t readSlot, bool doJson, const char *rName)
{
  uint16_t  slot, maxSlots = 0, offset;
  char      buffer[DATA_RECLEN +2] = "";
  char      recID[10]  = "";
  float     EDT1, EDT2, ERT1, ERT2, GDT;

  switch(fileType)
  {
    case HOURS:
      maxSlots    = _NO_HOUR_SLOTS_;
      break;
    case DAYS:
      maxSlots    = _NO_DAY_SLOTS_;
      break;
    case MONTHS:
      maxSlots    = _NO_MONTH_SLOTS_;
      break;
  }

  if (!FSYS.exists(fileName))
  {
    DebugTf("File [%s] does not excist!\r\n", fileName);
    return;
  }

  File dataFile = FSYS.open(fileName, "r+");  // read and write ..
  if (!dataFile)
  {
    DebugTf("Error opening [%s]\r\n", fileName);
    return;
  }

  slot    = (readSlot % maxSlots);
  // slot goes from 0 to _NO_OF_SLOTS_
  // we need to add 1 to slot to skip header record!
  offset  = ((slot +1) * DATA_RECLEN);
  dataFile.seek(offset, SeekSet);
  int l = dataFile.readBytesUntil('\n', buffer, sizeof(buffer));
  buffer[l] = 0;
  if (l >= (DATA_RECLEN -1))   // '\n' is skipped by readBytesUntil()
  {
    if (!isNumericp(buffer, 8))   // first 8 bytes is YYMMDDHH
    {
      {
        Debugf("slot[%02d]==>timeStamp [%-13.13s] not valid!!\r\n", slot, buffer);
        writeToSysLog("slot[%02d]==>timeStamp [%-13.13s] not valid!!", slot, buffer);
      }
    }
    else
    {
      if (doJson)
      {
        sscanf(buffer, "%[^;];%f;%f;%f;%f;%f", recID
               , &EDT1, &EDT2, &ERT1, &ERT2, &GDT);
        sendNestedJsonObj(recNr++, recID, slot, EDT1, EDT2, ERT1, ERT2, GDT);

      }
      else
      {
        Debugf("slot[%02d]->[%s]\r\n", slot, buffer);
      }
    }

  }
  dataFile.close();

} // readOneSlot()


//===========================================================================================
void readSlotFromTimestamp(int8_t fileType, const char *fileName, const char *timeStamp
                           , bool doJson, const char *rName)
{
  uint16_t firstSlot = 0, maxSlots = 0;

  DebugTf("timeStamp[%s]\r\n", timeStamp);

  switch(fileType)
  {
    case HOURS:
      firstSlot   = timestampToHourSlot(timeStamp, strlen(timeStamp));
      maxSlots    = _NO_HOUR_SLOTS_;
      break;
    case DAYS:
      firstSlot   = timestampToDaySlot(timeStamp, strlen(timeStamp));
      maxSlots    = _NO_DAY_SLOTS_;
      break;
    case MONTHS:
      firstSlot   = timestampToMonthSlot(timeStamp, strlen(timeStamp));
      maxSlots    = _NO_MONTH_SLOTS_;
      break;
  }

  firstSlot += maxSlots;
  DebugTf("firstSlot[%d] -> slot[%d]\r\n", firstSlot, (firstSlot % maxSlots));
  readOneSlot(fileType, fileName, firstSlot, 0, doJson, rName);

} // readSlotFromTimestamp()


//===========================================================================================
void readAllSlots(int8_t fileType, const char *fileName, const char *timeStamp
                  , bool doJson, const char *rName)
{
  int16_t startSlot, endSlot, nrSlots, recNr = 0;

  switch(fileType)
  {
    case HOURS:
      startSlot       = timestampToHourSlot(timeStamp, strlen(timeStamp));
      nrSlots         = _NO_HOUR_SLOTS_;
      break;
    case DAYS:
      startSlot       = timestampToDaySlot(timeStamp, strlen(timeStamp));
      nrSlots         = _NO_DAY_SLOTS_;
      break;
    case MONTHS:
      startSlot       = timestampToMonthSlot(timeStamp, strlen(timeStamp));
      nrSlots         = _NO_MONTH_SLOTS_;
      break;
  }

  endSlot   = nrSlots + startSlot;
  //startSlot += nrSlots;
  DebugTf("start[%02d], endSlot[%02d]\r\n", (startSlot%nrSlots), endSlot);
  for( uint16_t s=startSlot; s<endSlot; s++ )
  {
    readOneSlot(fileType, fileName, s, recNr++, false, "");
  }

} // readAllSlots()


//===========================================================================================
bool createFile(const char *fileName, uint16_t noSlots)
{
  DebugTf("fileName[%s], fileRecLen[%d]\r\n", fileName, DATA_RECLEN);

  File dataFile  = FSYS.open(fileName, "a");  // create File
  // -- first write fileHeader ----------------------------------------
  snprintf(cMsg, sizeof(cMsg), "%s", DATA_CSV_HEADER);  // you cannot modify *fileHeader!!!
  fillRecord(cMsg, DATA_RECLEN);
  DebugT(cMsg);
  Debugln(F("\r"));
  bytesWritten = dataFile.print(cMsg);
  if (bytesWritten != DATA_RECLEN)
  {
    DebugTf("ERROR!! slotNr[%d]: written [%d] bytes but should have been [%d] for Header\r\n", 0, bytesWritten, DATA_RECLEN);
  }
  DebugTln(F(".. that went well! Now add next record ..\r"));
  // -- as this file is empty, write one data record ------------
  snprintf(cMsg, sizeof(cMsg), "%02d%02d%02d%02d", 0, 0, 0, 0);

  snprintf(cMsg, sizeof(cMsg), DATA_FORMAT, cMsg, 0.000, 0.000, 0.000, 0.000, 0.000);

  fillRecord(cMsg, DATA_RECLEN);
  for(int r = 1; r <= noSlots; r++)
  {
    DebugTf("Write [%s] Data[%-9.9s]\r\n", fileName, cMsg);
    dataFile.seek((r * DATA_RECLEN), SeekSet);
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != DATA_RECLEN)
    {
      DebugTf("ERROR!! recNo[%d]: written [%d] bytes but should have been [%d] \r\n", r, bytesWritten, DATA_RECLEN);
    }
  } // for ..

  dataFile.close();
  dataFile  = FSYS.open(fileName, "r+");       // open for Read & writing
  if (!dataFile)
  {
    DebugTf("Something is very wrong writing to [%s]\r\n", fileName);
    return false;
  }
  dataFile.close();

  return true;

} //  createFile()


//===========================================================================================
void fillRecord(char *record, int8_t len)
{
  int8_t s = 0, l = 0;
  while (record[s] != '\0' && record[s]  != '\n')
  {
    s++;
  }
  if (Verbose1) DebugTf("Length of record is [%d] bytes\r\n", s);
  for (l = s; l < (len - 2); l++)
  {
    record[l] = ' ';
  }
  record[l]   = ';';
  record[l+1] = '\n';
  record[len] = '\0';

  while (record[l] != '\0')
  {
    l++;
  }
  if (Verbose1) DebugTf("Length of record is now [%d] bytes\r\n", l);

} // fillRecord()


//====================================================================
uint16_t timestampToHourSlot(const char *TS, int8_t len)
{
  //char      aSlot[5];
  time_t    t1 = epoch((char *)TS, strlen(TS), false);
  uint32_t  nrHours = t1 / SECS_PER_HOUR;
  //sprintf(aSlot, "%d", ((nrDays % KEEP_DAYS_HOURS) *24) + hour(t1));
  //uint8_t   uSlot  = String(aSlot).toInt();
  uint8_t   recSlot = (nrHours % _NO_HOUR_SLOTS_);

  if (Verbose1) DebugTf("===>>>>>  HOUR[%02d] => recSlot[%02d]\r\n", hour(t1), recSlot);

  if (recSlot < 0 || recSlot >= _NO_HOUR_SLOTS_)
  {
    DebugTf("HOUR: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_HOUR_SLOTS_;
    slotErrors++;
  }
  return recSlot;

} // timestampToHourSlot()


//====================================================================
uint16_t timestampToDaySlot(const char *TS, int8_t len)
{
  //char      aSlot[5];
  time_t    t1 = epoch((char *)TS, strlen(TS), false);
  uint32_t  nrDays = t1 / SECS_PER_DAY;
  uint16_t  recSlot = (nrDays % _NO_DAY_SLOTS_);

  if (Verbose1) DebugTf("===>>>>>   DAY[%02d] => recSlot[%02d]\r\n", day(t1), recSlot);

  if (recSlot < 0 || recSlot >= _NO_DAY_SLOTS_)
  {
    DebugTf("DAY: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_DAY_SLOTS_;
    slotErrors++;
  }
  return recSlot;

} // timestampToDaySlot()


//====================================================================
uint16_t timestampToMonthSlot(const char *TS, int8_t len)
{
  //char      aSlot[5];
  time_t    t1 = epoch((char *)TS, strlen(TS), false);
  uint32_t  nrMonths = ( (year(t1) -1) * 12) + month(t1);    // eg: year(2023) * 12 = 24276 + month(9) = 202309
  uint16_t  recSlot = (nrMonths % _NO_MONTH_SLOTS_); // eg: 24285 % _NO_MONTH_SLOT_

  if (Verbose1) DebugTf("===>>>>> MONTH[%02d] => recSlot[%02d]\r\n", month(t1), recSlot);

  if (recSlot < 0 || recSlot >= _NO_MONTH_SLOTS_)
  {
    DebugTf("MONTH: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_MONTH_SLOTS_;
    slotErrors++;
  }
  return recSlot;

} // timestampToMonthSlot()


//===========================================================================================
int32_t freeSpace()
{
  int32_t space;

  FSYS.info(SPIFFSinfo);

  space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);

  return space;

} // freeSpace()

//===========================================================================================
void listFSYS()
{
  typedef struct _fileMeta
  {
    char    Name[20];
    int32_t Size;
  } fileMeta;

  _fileMeta dirMap[30];
  int fileNr = 0;

  Dir dir = FSYS.openDir("/");         // List files on SPIFFS
  while (dir.next())
  {
    dirMap[fileNr].Name[0] = '\0';
#if defined( USE_LITTLEFS )
    strncat(dirMap[fileNr].Name, dir.fileName().substring(0).c_str(), 19);
#else // SPIFFS
    strncat(dirMap[fileNr].Name, dir.fileName().substring(1).c_str(), 19); // remove leading '/'
#endif
    dirMap[fileNr].Size = dir.fileSize();
    fileNr++;
  }

  // -- bubble sort dirMap op .Name--
  for (int8_t y = 0; y < fileNr; y++)
  {
    yield();
    for (int8_t x = y + 1; x < fileNr; x++)
    {
      //DebugTf("y[%d], x[%d] => seq[x][%s] ", y, x, dirMap[x].Name);
      if (compare(String(dirMap[x].Name), String(dirMap[y].Name)))
      {
        fileMeta temp = dirMap[y];
        dirMap[y]     = dirMap[x];
        dirMap[x]     = temp;
      } /* end if */
      //Debugln();
    } /* end for */
  } /* end for */

  DebugTln(F("\r\n"));
  for(int f=0; f<fileNr; f++)
  {
    Debugf("%-25s %6d bytes \r\n", dirMap[f].Name, dirMap[f].Size);
    yield();
  }

  FSYS.info(SPIFFSinfo);

  Debugln(F("\r"));
  if (freeSpace() < (10 * SPIFFSinfo.blockSize))
    Debugf("Available FSYS space [%6d]kB (LOW ON SPACE!!!)\r\n", (freeSpace() / 1024));
  else  Debugf("Available FSYS space [%6d]kB\r\n", (freeSpace() / 1024));
  Debugf("           FSYS Size [%6d]kB\r\n", (SPIFFSinfo.totalBytes / 1024));
  Debugf("     FSYS block Size [%6d]bytes\r\n", SPIFFSinfo.blockSize);
  Debugf("      FSYS page Size [%6d]bytes\r\n", SPIFFSinfo.pageSize);
  Debugf(" FSYS max.Open Files [%6d]\r\n\r\n", SPIFFSinfo.maxOpenFiles);


} // listFSYS()


//===========================================================================================
void eraseFile()
{
  char eName[30] = "";

  //--- erase buffer
  while (TelnetStream.available() > 0)
  {
    yield();
    (char)TelnetStream.read();
  }

  Debug("Enter filename to erase: ");
  TelnetStream.setTimeout(10000);
  TelnetStream.readBytesUntil('\n', eName, sizeof(eName));
  TelnetStream.setTimeout(1000);

  //--- remove control chars like \r and \n ----
  //--- and shift all char's one to the right --
  for(int i=strlen(eName); i>0; i--)
  {
    eName[i] = eName[i-1];
    if (eName[i] < ' ') eName[i] = '\0';
  }
  //--- add leading slash on position 0
  eName[0] = '/';

  if (FSYS.exists(eName))
  {
    Debugf("\r\nErasing [%s] from FSYS\r\n\n", eName);
    FSYS.remove(eName);
  }
  else
  {
    Debugf("\r\nfile [%s] not found..\r\n\n", eName);
  }
  //--- empty buffer ---
  while (TelnetStream.available() > 0)
  {
    yield();
    (char)TelnetStream.read();
  }

} // eraseFile()


//===========================================================================================
bool DSMRfileExist(const char *fileName, bool doDisplay)
{
  char fName[30] = "";
  if (fileName[0] != '/')
  {
    strConcat(fName, 5, "/");
  }
  strConcat(fName, 29, fileName);

  DebugTf("check if [%s] exists .. ", fName);
  if (settingOledType > 0)
  {
    oled_Print_Msg(1, "Bestaat:", 10);
    oled_Print_Msg(2, fName, 10);
    oled_Print_Msg(3, "op FSYS?", 250);
  }

  if (!FSYS.exists(fName) )
  {
    if (doDisplay)
    {
      Debugln(F("NO! Error!!"));
      if (settingOledType > 0)
      {
        oled_Print_Msg(3, "Nee! FOUT!", 6000);
      }
      writeToSysLog("Error! File [%s] not found!", fName);
      return false;
    }
    else
    {
      Debugln(F("NO! "));
      if (settingOledType > 0)
      {
        oled_Print_Msg(3, "Nee! ", 6000);
      }
      writeToSysLog("File [%s] not found!", fName);
      return false;
    }
  }
  else
  {
    Debugln(F("Yes! OK!"));
    if (settingOledType > 0)
    {
      oled_Print_Msg(3, "JA! (OK!)", 250);
    }
  }
  return true;

} //  DSMRfileExist()


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
