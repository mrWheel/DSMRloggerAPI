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

int16_t   bytesWritten;

//static    FSInfo SPIFFSinfo;


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

//===========================================================================================
bool buildDataRecord(char *recIn) 
{
  static float GG = 1;
  char record[DATA_RECLEN + 1] = "";
  char key[10] = "";
 
  uint16_t recSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
  strCopy(key, 10, actTimestamp, 0, 8);

  //int HH = HourFromTimestamp(actTimestamp);
  //int DD = DayFromTimestamp(actTimestamp);
  //int MM = MonthFromTimestamp(actTimestamp);
  //int YY = YearFromTimestamp(actTimestamp);
  //GG = GG + 0.1;

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

  if (!isNumericp(record, 8))
  {
    DebugTf("timeStamp [%-13.13s] not valid\r\n", record);
    slotErrors++;
    return;
  }
  
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
void readDataFromFile(int8_t fileType, const char *fileName
                          , int16_t fromSlot, int16_t period
                          , bool doJson, const char *rName) 
{
  uint16_t  slot = 0, maxSlots = 0, periodSlots = 0, offset = 0;
  char      buffer[DATA_RECLEN +2] = "";
  char      recID[10]  = "";
  float     EDT1, EDT2, ERT1, ERT2, GDT;
  JsonArray root;
    
  switch(fileType) {
    case HOURS:   maxSlots    = _NO_HOUR_SLOTS_;
                  periodSlots = HOURS_PER_PERIOD;
                  break;
    case DAYS:    maxSlots    = _NO_DAY_SLOTS_;
                  periodSlots =  DAYS_PER_PERIOD;
                  break;
    case MONTHS:  maxSlots    = _NO_MONTH_SLOTS_;
                  periodSlots = MONTHS_PER_PERIOD;
                  break;
  }


  if (!SPIFFS.exists(fileName))
  {
    DebugTf("File [%s] does not excist!\r\n", fileName);
    return;
  }

  if (doJson)
  {
      root = jsonDoc.createNestedArray("hist");
  }

  File dataFile = SPIFFS.open(fileName, "r+");  // read and write ..
  if (!dataFile) 
  {
    DebugTf("Error opening [%s]\r\n", fileName);
    return;
  }

  uint16_t recNr = (period -1) * periodSlots;
  
  DebugTf("for (s=[%d]; s>[%d]p[%d]; s--)\r\n", maxSlots, 0, periodSlots);
  for(int16_t s = maxSlots; s > 0, periodSlots > 0; s--, periodSlots--)
  { 
    int16_t nextSlot = (s + fromSlot) -1;
    if (nextSlot < 0) nextSlot += maxSlots;
    slot    = (nextSlot % maxSlots) +1;
    //Debugf("Inx[%02d]: ", s);
    offset  = (slot * DATA_RECLEN);
    dataFile.seek(offset, SeekSet); 
    int l = dataFile.readBytesUntil('\n', buffer, sizeof(buffer));
    buffer[l] = 0;
    if (l >= (DATA_RECLEN -1))  // '\n' is skipped by readBytesUntil()
    {
      if (!isNumericp(buffer, 8)) // first 8 bytes is YYMMDDHH
      {
        {
          Debugf("slot[%02d]==>timeStamp [%-13.13s] not valid!!\r\n", slot, buffer);
        }
      }
      else
      {
        if (doJson)
        {
          sscanf(buffer, "%[^;];%f;%f;%f;%f;%f", recID
                                               , &EDT1, &EDT2, &ERT1, &ERT2, &GDT);
          JsonObject nested = root.createNestedObject();
          nested["rec"]  = recNr++;
          nested["date"] = recID;
          nested["edt1"] = (float)EDT1;
          nested["edt2"] = (float)EDT2;
          nested["ert1"] = (float)ERT1;
          nested["ert2"] = (float)ERT2;
          nested["gdt"]  = (float)EDT1;
        }
        else
        {
          Debugf("slot[%02d]->[%s]\r\n", slot, buffer);
        }
      }
    }

    
  }
  dataFile.close();

} // readDatafromFile()


//===========================================================================================
void readDataFromFile(int8_t fileType, const char *fileName, const char *timeStamp
                          , uint8_t period, bool doJson, const char *rName) 
{
  uint16_t recsPerCall = 0, firstSlot = 0;

  DebugTf("timeStamp[%s]\r\n", timeStamp);
  //recNr = 0;  // reset recNr counter!
  
  switch(fileType) {
    case HOURS:   firstSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
                  recsPerCall = HOURS_PER_PERIOD;
                  break;
    case DAYS:    firstSlot = timestampToDaySlot(actTimestamp, strlen(actTimestamp));
                  recsPerCall = DAYS_PER_PERIOD;
                  break;
    case MONTHS:  firstSlot = timestampToMonthSlot(actTimestamp, strlen(actTimestamp));
                  recsPerCall = MONTHS_PER_PERIOD;
                  break;
  }

  firstSlot = firstSlot + (recsPerCall * (period - 1));
  readDataFromFile(fileType, fileName, firstSlot, period, doJson, rName);

} // readDatafromFile()


//===========================================================================================
void readDataFromFile(int8_t fileType, const char *fileName, const char *timeStamp
                          , bool doJson, const char *rName) 
{
  int16_t startSlot, nrSlots, slotsPerPeriod;
  
  switch(fileType) {
    case HOURS:   startSlot = timestampToHourSlot(actTimestamp, strlen(actTimestamp));
                  slotsPerPeriod = HOURS_PER_PERIOD;
                  nrSlots = _NO_HOUR_SLOTS_;
                  break;
    case DAYS:    startSlot = timestampToDaySlot(actTimestamp, strlen(actTimestamp));
                  slotsPerPeriod = DAYS_PER_PERIOD;
                  nrSlots = _NO_DAY_SLOTS_;
                  break;
    case MONTHS:  startSlot = timestampToMonthSlot(actTimestamp, strlen(actTimestamp));
                  slotsPerPeriod = MONTHS_PER_PERIOD;
                  nrSlots = _NO_MONTH_SLOTS_;
                  break;
  }
  DebugTf("start[%d], perPeriod[%d], nrSlots[%d]\r\n", startSlot, slotsPerPeriod, nrSlots);
  for( int p=1; p<=(nrSlots / slotsPerPeriod); p++)
  {
    DebugTf("start[%d], perPeriod[%d], nrSlots[%d]\r\n", startSlot, slotsPerPeriod, nrSlots);
    readDataFromFile(fileType, fileName, startSlot, p, false, "") ;
    startSlot -= slotsPerPeriod;
  }
  
} // readDataFromFile()


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
    sprintf(cMsg, "%02d%02d%02d%02d", 0, 0, 0, 0);
    
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


//====================================================================
uint16_t timestampToHourSlot(const char * TS, int8_t len)
{
  //char      aSlot[5];
  time_t    t1 = epoch((char*)TS, strlen(TS), false);
  uint32_t  nrHours = t1 / SECS_PER_HOUR;
  //sprintf(aSlot, "%d", ((nrDays % KEEP_DAYS_HOURS) *24) + hour(t1));
  //uint8_t   uSlot  = String(aSlot).toInt();
  uint8_t   recSlot = (nrHours % _NO_HOUR_SLOTS_) +1;
  
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
  //char      aSlot[5];
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
  //char      aSlot[5];
  time_t    t1 = epoch((char*)TS, strlen(TS), false);
  uint32_t  nrMonths = ( (year(t1) -1) * 12) + month(t1);    // eg: year(2023) * 12 = 24276 + month(9) = 202309
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
