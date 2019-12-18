/* 
***************************************************************************  
**  Program  : SPIFFSstuff, part of DSMRfirmwareAPI
**  Version  : v0.0.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

int16_t  bytesWritten;


//====================================================================
void readLastStatus()
{
  char buffer[50] = "";
  char dummy[50] = "";
  char spiffsTimestamp[20] = "";
  
  File _file = SPIFFS.open("/DSMRstatus.csv", "r");
  if (!_file)
  {
    DebugTln("read(): No /DSMRstatus.csv found ..");
  }
  if(_file.available()) {
    int l = _file.readBytesUntil('\n', buffer, sizeof(buffer));
    buffer[l] = 0;
    DebugTf("read lastUpdate[%s]\r\n", buffer);
    sscanf(buffer, "%[^;]; %u; %u; %[^;]", spiffsTimestamp, &nrReboots, &slotErrors, dummy);
    DebugTf("values timestamp[%s], nrReboots[%u], slotErrors[%u], dummy[%s]\r\n"
                                                    , spiffsTimestamp
                                                    , nrReboots
                                                    , slotErrors
                                                    , dummy);
    yield();
  }
  _file.close();
  if (strlen(spiffsTimestamp) != 13) {
    strcpy(spiffsTimestamp, "010101010101X");
  }
  sprintf(actTimestamp, "%s", spiffsTimestamp);
  
}  // readLastStatus()


//====================================================================
void writeLastStatus()
{
  char buffer[50] = "";
  DebugTf("writeLastStatus() => %s; %u; %u;\r\n", actTimestamp, nrReboots, slotErrors);
  File _file = SPIFFS.open("/DSMRstatus.csv", "w");
  if (!_file)
  {
    DebugTln("write(): No /DSMRstatus.csv found ..");
  }
  sprintf(buffer, "%-13.13s; %010u; %010u; %s;\n", actTimestamp
                                          , nrReboots
                                          , slotErrors
                                          , "meta data");
  _file.print(buffer);
  _file.flush();
  _file.close();
  
} // writeLastStatus()

//====================================================================
uint16_t timestampToHourSlot(const char * TS, int8_t len)
{
  char      aSlot[5];
  time_t    t1 = epoch((char*)TS, strlen(TS), false);
  uint32_t  nrDays = t1 / SECS_PER_DAY;
  sprintf(aSlot, "%d", ((nrDays % KEEP_DAYS_HOURS) *24) + hour(t1));
  uint8_t   uSlot  = String(aSlot).toInt();
  uint8_t   recSlot = (uSlot % _NO_HOUR_SLOTS_) +1;
  
  DebugTf("===>>>>>  HOUR[%02d] => recSlot[%02d]\r\n", hour(t1), recSlot);

  if (recSlot < 1 || recSlot > _NO_HOUR_SLOTS_)
  {
    DebugTf("HOUR: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_HOUR_SLOTS_ + 1;
    slotErrors++;
  }
  return recSlot;
  
} // timestampToHourSlot()


//====================================================================
uint16_t timestampToDaySlot(const char * TS, int8_t len)
{
  char      aSlot[5];
  time_t    t1 = epoch((char*)TS, strlen(TS), false);
  uint32_t  nrDays = t1 / SECS_PER_DAY;
  //sprintf(aSlot, "%d", (nrDays % KEEP_WEEK_DAYS));
  //uint8_t   uSlot  = String(aSlot).toInt();
  uint16_t  recSlot = (nrDays % _NO_DAY_SLOTS_) +1;
  
  DebugTf("===>>>>>   DAY[%02d] => recSlot[%02d]\r\n", day(t1), recSlot);

  if (recSlot < 1 || recSlot > _NO_DAY_SLOTS_)
  {
    DebugTf("DAY: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_DAY_SLOTS_ + 1;
    slotErrors++;
  }
  return recSlot;
  
} // timestampToDaySlot()


//====================================================================
uint16_t timestampToMonthSlot(const char * TS, int8_t len)
{
  char      aSlot[5];
  time_t    t1 = epoch((char*)TS, strlen(TS), false);
  uint32_t  nrMonths = (year(t1) * 12) + month(t1);    // eg: year(2023) * 12 = 24276 + month(9) = 202309
  uint16_t  recSlot = (nrMonths % _NO_MONTH_SLOTS_) +1; // eg: 24285 % _NO_MONTH_SLOT_
  
  DebugTf("===>>>>> MONTH[%02d] => recSlot[%02d]\r\n", month(t1), recSlot);

  if (recSlot < 1 || recSlot > _NO_MONTH_SLOTS_)
  {
    DebugTf("MONTH: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_MONTH_SLOTS_ + 1;
    slotErrors++;
  }
  return recSlot;
  
} // timestampToMonthSlot()


//===========================================================================================
int32_t freeSpace() 
{
  int32_t space;
  
  SPIFFS.info(SPIFFSinfo);

  space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);

  return space;
  
} // freeSpace()

//===========================================================================================
void listSPIFFS() 
{
   typedef struct _fileMeta {
    char    Name[20];     
    int32_t Size;
  } fileMeta;

  _fileMeta dirMap[30];
  int fileNr = 0;
  
  Dir dir = SPIFFS.openDir("/");         // List files on SPIFFS
  while (dir.next())  
  {
    dirMap[fileNr].Name[0] = '\0';
    strncat(dirMap[fileNr].Name, dir.fileName().substring(1).c_str(), 19); // remove leading '/'
    dirMap[fileNr].Size = dir.fileSize();
    fileNr++;
  }

  // -- bubble sort dirMap op .Name--
  for (int8_t y = 0; y < fileNr; y++) {
    yield();
    for (int8_t x = y + 1; x < fileNr; x++)  {
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

  SPIFFS.info(SPIFFSinfo);

  Debugln(F("\r"));
  if (freeSpace() < (10 * SPIFFSinfo.blockSize))
        Debugf("Available SPIFFS space [%6d]kB (LOW ON SPACE!!!)\r\n", (freeSpace() / 1024));
  else  Debugf("Available SPIFFS space [%6d]kB\r\n", (freeSpace() / 1024));
  Debugf("           SPIFFS Size [%6d]kB\r\n", (SPIFFSinfo.totalBytes / 1024));
  Debugf("     SPIFFS block Size [%6d]bytes\r\n", SPIFFSinfo.blockSize);
  Debugf("      SPIFFS page Size [%6d]bytes\r\n", SPIFFSinfo.pageSize);
  Debugf(" SPIFFS max.Open Files [%6d]\r\n\r\n", SPIFFSinfo.maxOpenFiles);


} // listSPIFFS()

//===========================================================================================
void fillRecord(char *record, int8_t len) 
{
  int8_t s = 0, l = 0;
  while (record[s] != '\0' && record[s]  != '\n') {s++;}
  if (Verbose1) DebugTf("Length of record is [%d] bytes\r\n", s);
  for (l = s; l < (len - 2); l++) {
    record[l] = ' ';
  }
  record[l]   = ';';
  record[l+1] = '\n';
  record[len] = '\0';

  while (record[l] != '\0') {l++;}
  if (Verbose1) DebugTf("Length of record is now [%d] bytes\r\n", l);
  
} // fillRecord()


//===========================================================================================
bool buildDataRecord(char *recIn) 
{
  char record[DATA_RECLEN + 1] = "";
  char key[10] = "";
 
  uint16_t recSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
  strnCopy(key, 10, actTimestamp, 0, 8);


    sprintf(record, (char*)DATA_FORMAT, key , (float)DSMRdata.energy_delivered_tariff1
                                            , (float)DSMRdata.energy_delivered_tariff2
                                            , (float)DSMRdata.energy_returned_tariff1
                                            , (float)DSMRdata.energy_returned_tariff2
                                            , (float)DSMRdata.gas_delivered);
    // DATA + \n + \0                                        
    fillRecord(record, DATA_RECLEN);

    strcpy(recIn, record);

} // buildDataRecord()


//===========================================================================================
void writeDataToFile(const char *fileName, const char *record, uint16_t slot, int8_t fileType) 
{
  uint16_t offset = 0;
  
  if (!SPIFFS.exists(fileName))
  {
    switch(fileType) {
      case HOURS:   createFile(fileName, _NO_HOUR_SLOTS_);
                    break;
      case DAYS:    createFile(fileName, _NO_DAY_SLOTS_);
                    break;
      case MONTHS:  createFile(fileName, _NO_MONTH_SLOTS_);
                    break;
    }
  }

  File dataFile = SPIFFS.open(fileName, "r+");  // read and write ..
  if (!dataFile) 
  {
    DebugTf("Error opening [%s]\r\n", fileName);
    return;
  }
  offset = (slot * DATA_RECLEN);
  dataFile.seek(offset, SeekSet); 
  int32_t bytesWritten = dataFile.print(record);
  if (bytesWritten != DATA_RECLEN) 
  {
    DebugTf("ERROR! slot[%02d]: written [%d] bytes but should have been [%d]\r\n", slot, bytesWritten, DATA_RECLEN);
  }
  dataFile.close();

} // writeDataToFile()


//===========================================================================================
void writeDataToFiles() 
{
  char record[DATA_RECLEN + 1] = "";
  uint16_t recSlot;

  buildDataRecord(record);
  DebugTf(">%s\r", record); // record ends in a \n

  // update HOURS
  recSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
  DebugTf("HOURS:  Write to slot[%02d] in %s\r\n", recSlot, HOURS_FILE);
  writeDataToFile(HOURS_FILE, record, recSlot, HOURS);

  // update DAYS
  recSlot = timestampToDaySlot(actTimestamp, strlen(actTimestamp));
  DebugTf("DAYS:   Write to slot[%02d] in %s\r\n", recSlot, DAYS_FILE);
  writeDataToFile(DAYS_FILE, record, recSlot, DAYS);

  // update MONTHS
  recSlot = timestampToMonthSlot(actTimestamp, strlen(actTimestamp));
  DebugTf("MONTHS: Write to slot[%02d] in %s\r\n", recSlot, MONTHS_FILE);
  writeDataToFile(MONTHS_FILE, record, recSlot, MONTHS);
  


} // writeDataToFiles(fileType, dataStruct newDat, int8_t slotNr)

//===========================================================================================
bool createFile(const char *fileName, uint16_t noSlots) 
{
    DebugTf("fileName[%s], fileRecLen[%d]\r\n", fileName, DATA_RECLEN);

    File dataFile  = SPIFFS.open(fileName, "a");  // create File
    // -- first write fileHeader ----------------------------------------
    sprintf(cMsg, "%s", DATA_CSV_HEADER);  // you cannot modify *fileHeader!!!
    fillRecord(cMsg, DATA_RECLEN);
    DebugT(cMsg); Debugln(F("\r"));
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != DATA_RECLEN) 
    {
      DebugTf("ERROR!! slotNr[%d]: written [%d] bytes but should have been [%d] for Header\r\n", 0, bytesWritten, DATA_RECLEN);
    }
    DebugTln(F(".. that went well! Now add next record ..\r"));
    // -- as this file is empty, write one data record ------------
    sprintf(cMsg, "%02d%02d%02d%02d", 0, month(), day(), hour());
    
    sprintf(cMsg, DATA_FORMAT, cMsg, 0.000, 0.000, 0.000, 0.000, 0.000);

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
    dataFile  = SPIFFS.open(fileName, "r+");       // open for Read & writing
    if (!dataFile) 
    {
      DebugTf("Something is very wrong writing to [%s]\r\n", fileName);
      return false;
    }
    dataFile.close();

    return true;
  
} //  createFile()


//=====================================================================================================
bool checkRecordsInFile(int8_t fileType, String fileName, const char *fileFormat
                                       , uint16_t fileRecLen, int8_t fileNoRecs, dataStruct newDat) 
{
  dataStruct lastRec;
  
  File dataFile  = SPIFFS.open(fileName, "a");        // add to File
  int8_t recsInFile  = dataFile.size() / fileRecLen;  // records in file
  if (Verbose1) DebugTf("[%s] needs [%d] records. Found [%02d] records\r\n", fileName.c_str(), fileNoRecs, recsInFile);
  lastRec = fileReadData(fileType, recsInFile); 
  
  if (recsInFile >= fileNoRecs) return true; // all records are there!
  
  if (Verbose1) DebugTf("Now adding records from [%d]\r\n", newDat.Label);
  for (int r = recsInFile; r <= fileNoRecs; r++) 
  {
    yield();
    lastRec.Label = updateLabel(fileType, lastRec.Label, -1);
    sprintf(cMsg, fileFormat, lastRec.Label, String(lastRec.EDT1, 3).c_str()
                                           , String(lastRec.EDT2, 3).c_str()
                                           , String(lastRec.ERT1, 3).c_str()
                                           , String(lastRec.ERT2, 3).c_str()
                                           , String(lastRec.GDT, 3).c_str());
    fillRecord(cMsg, fileRecLen);
    dataFile.seek((r * fileRecLen), SeekSet);
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != fileRecLen) 
    {
      DebugTf("ERROR!! recNo[%02d]: written [%d] bytes but should have been [%d] for Data[%s]\r\n", r, bytesWritten, fileRecLen, cMsg);
      return false;
    }
    if (Verbose2) DebugTf("Add dummy record[%02d] @pos[%d] := %s", r, (r * fileRecLen), cMsg);
  } // for ..

  return true;
  
} // checkRecordsInFile()


//=======================================================================================================================
int32_t updateLabel(int8_t fileType, int32_t Label, int8_t offSet) 
{
    
} // updateLabel()


//===========================================================================================
dataStruct fileReadData(int8_t fileType, uint8_t recNo) 
{
  /**
  int16_t  recLen, offset;
  dataStruct tmpRec;
  File dataFile;
  
  tmpRec.Label = 0;
  tmpRec.EDT1  = 0;
  tmpRec.EDT2  = 0;
  tmpRec.ERT1  = 0;
  tmpRec.ERT2  = 0;
  tmpRec.GDT   = 0;

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  if (Verbose1) DebugTf("fileReadData(%02d) ... \r\n", recNo);

  if (!SPIFFSmounted) 
  {
    DebugTln(F("No SPIFFS filesystem..\r"));
    return tmpRec;
  }

  if (fileType == MONTHS) 
  {
    sprintf(cMsg, DATA_CSV_HEADER);
    dataFile = SPIFFS.open(DATA_FILE, "r");
    if (Verbose2) DebugTf("%s: size(%d) \r\n", DATA_FILE, dataFile.size());
    recLen = DATA_RECLEN;
  }
  else if (fileType == DAYS) 
  {
    sprintf(cMsg, DATA_CSV_HEADER);
    dataFile = SPIFFS.open(DAYS_FILE, "r");
    if (Verbose2) DebugTf("%s: size(%d) \r\n", DAYS_FILE, dataFile.size());
    recLen = DATA_RECLEN;
  }
  else if (fileType == HOURS) 
  {
    sprintf(cMsg, DATA_RECLEN);
    dataFile = SPIFFS.open(HOURS_FILE, "r");
    if (Verbose2) DebugTf("%s: size(%d) \r\n", HOURS_FILE, dataFile.size());
    recLen = DATA_RECLEN;
  }
  else recLen = 0;
  
  if (dataFile.size() == 0) return tmpRec;

//-- seek() gives strange results ..  
  offset = recNo * recLen;
  dataFile.seek(offset, SeekSet); // skip header
  
  if (dataFile.available() > 0) 
  {
    tmpRec.Label = (int)dataFile.readStringUntil(';').toInt();
    tmpRec.EDT1  = (float)dataFile.readStringUntil(';').toFloat();
    tmpRec.EDT2  = (float)dataFile.readStringUntil(';').toFloat();
    tmpRec.ERT1  = (float)dataFile.readStringUntil(';').toFloat();
    tmpRec.ERT2  = (float)dataFile.readStringUntil(';').toFloat();
    tmpRec.GDT   = (float)dataFile.readStringUntil(';').toFloat();
    String n = dataFile.readStringUntil('\n');
  }
  if (Verbose2) DebugTf("recNo[%02d] Label[%08d], EDT1[%s], EDT2[%s], ERT1[%s], ERT2[%s], GD[%s]\r\n"
                                        , recNo, tmpRec.Label
                                        ,    String(tmpRec.EDT1, 3).c_str()
                                        ,    String(tmpRec.EDT2, 3).c_str()
                                        ,    String(tmpRec.ERT1, 3).c_str()
                                        ,    String(tmpRec.ERT2, 3).c_str()
                                        ,    String(tmpRec.GDT,  3).c_str() );
  
  dataFile.close();  

  if (Verbose1) DebugTln(F(" ..Done\r"));
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  return tmpRec;
  **/
} // fileReadData()

//===========================================================================================
void DSMRfileExist(const char* fileName) 
{

  DebugTf("check if [%s] exists .. ", fileName);
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
  oled_Print_Msg(1, "Bestaat:", 10);
  oled_Print_Msg(2, fileName, 10);
  oled_Print_Msg(3, "op SPIFFS?", 250);
#endif
  if (!SPIFFS.exists(fileName)) 
  {
    Debugln(F("NO! Error!!"));
    spiffsNotPopulated = true;
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(3, "Nee! FOUT!", 6000);
#endif
  } 
  else 
  {
    Debugln(F("Yes! OK!"));
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Print_Msg(3, "JA! (OK!)", 250);
#endif

  }

} //  DSMRfileExist()


//===========================================================================================
int8_t getLastMonth() 
{
  int16_t yearMonth, lastMonth;
  dataStruct tmpDat;

  tmpDat    = fileReadData(MONTHS, 1);
  yearMonth = tmpDat.Label;
  
  lastMonth = (yearMonth % 100);
  
  if (Verbose1) DebugTf(" ==> [%02d]\r\n", lastMonth);

  return lastMonth;

} // getLastMonth()


//===========================================================================================
int8_t getLastYear() 
{
  int16_t yearMonth, lastYear;
  dataStruct tmpDat;

  tmpDat = fileReadData(MONTHS, 1);
  yearMonth = tmpDat.Label;
  
  lastYear = (yearMonth / 100);
  
  if (Verbose1) DebugTf(" ==> [20%02d]\r\n", lastYear);

  return lastYear;

} // getLastYear()


//=======================================================================
uint32_t label2Fields(uint32_t Label, int8_t &YY, int8_t &MM, int8_t &DD, int8_t &HH) 
{
  char cKey[20];
  
  sprintf(cKey, "%08d", Label);
  if (Verbose1) DebugTf("Hours: Label in [%s] \r\n", cKey);
  YY = String(cKey).substring(0,2).toInt();
  MM = String(cKey).substring(2,4).toInt();
  DD = String(cKey).substring(4,6).toInt();
  HH = String(cKey).substring(6,8).toInt();

  if (YY <  0) YY =  0;
  if (YY > 99) YY = 99;
  if (MM <  1) MM =  1;
  if (MM > 12) MM = 12;
  if (DD <  1) DD =  1;
  if (DD > 31) DD = 31;
  if (HH <  0) HH =  0;
  if (HH > 24) HH = 24;

  sprintf(cKey, "%02d%02d%02d%02d", YY, MM, DD, HH);
  if (Verbose1) DebugTf("Label Out[%s] => YY[%2d], MM[%2d], DD[%2d], HH[%2d]\r\n", cKey, YY, MM, DD, HH);

  return String(cKey).toInt();
  
} // label2Fields()


//=======================================================================
uint32_t label2Fields(uint32_t Label, int8_t &YY, int8_t &MM, int8_t &DD) 
{
  char cKey[20];

  sprintf(cKey, "%06d", Label);
  if (Verbose1) DebugTf("Days: Label in [%s] \r\n", cKey);
  YY = String(cKey).substring(0,2).toInt();
  MM = String(cKey).substring(2,4).toInt();
  DD = String(cKey).substring(4,6).toInt();

  if (YY <  0) YY =  0;
  if (YY > 99) YY = 99;
  if (MM <  1) MM =  1;
  if (MM > 12) MM = 12;
  if (DD <  1) DD =  1;
  if (DD > 31) DD = 31;

  sprintf(cKey, "%02d%02d%02d", YY, MM, DD);
  if (Verbose1) DebugTf("Days: Label Out[%s] => YY[%2d], MM[%2d], DD[%2d]\r\n", cKey, YY, MM, DD);

  return String(cKey).toInt();
  
} // label2Fields()


//=======================================================================
uint32_t label2Fields(uint32_t Label, int8_t &YY, int8_t &MM) 
{
  char cKey[20];

  sprintf(cKey, "%04d", Label);
  if (Verbose1) DebugTf("Days: Label in [%s] \r\n", cKey);
  YY = String(cKey).substring(0,2).toInt();
  MM = String(cKey).substring(2,4).toInt();

  if (YY <  0) YY =  0;
  if (YY > 99) YY = 99;
  if (MM <  1) MM =  1;
  if (MM > 12) MM = 12;

  sprintf(cKey, "%02d%02d", YY, MM);
  if (Verbose1) DebugTf("Months: Label Out[%s] => YY[%2d], MM[%2d]\r\n", cKey, YY, MM);

  return String(cKey).toInt();
  
} // label2Fields()

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
