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
  
  DebugTf("===>>>>> HOUR[%02d] => recSlot[%02d]\r\n", hour(t1), recSlot);

  if (recSlot < 1 || recSlot > _NO_HOUR_SLOTS_)
  {
    DebugTf("HOUR: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_HOUR_SLOTS_ + 1;
  }
  return recSlot;
  
} // timestampToHourSlot()


//====================================================================
uint16_t timestampToDaySlot(const char * TS, int8_t len)
{
  char      aSlot[5];
  time_t    t1 = epoch((char*)TS, strlen(TS), false);
  uint32_t  nrDays = t1 / SECS_PER_DAY;
  sprintf(aSlot, "%d", (nrDays % KEEP_DAYS));
  uint8_t   uSlot  = String(aSlot).toInt();
  uint8_t   recSlot = (uSlot % _NO_DAY_SLOTS_) +1;
  
  DebugTf("===>>>>>  DAY[%02d] => recSlot[%02d]\r\n", day(t1), recSlot);

  if (recSlot < 1 || recSlot > _NO_DAY_SLOTS_)
  {
    DebugTf("DAY: Some serious error! Slot is [%d]\r\n", recSlot);
    recSlot = _NO_DAY_SLOTS_ + 1;
  }
  return recSlot;
  
} // timestampToDaySlot()


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
int8_t fileLabel2Rec(int8_t fileType, uint32_t RecKey) 
{
  int16_t recLen, offset, maxRecords = 0;
  int32_t Label;
  File dataFile;
  
  DebugTf("RecKey is [%d]\r\n", RecKey);

  if (fileType == MONTHS) 
  {
    recLen      = MONTHS_RECLEN;
    maxRecords  = MONTHS_RECS;
    dataFile    = SPIFFS.open(MONTHS_FILE, "r");    // open for Read
  } else if (fileType == DAYS) 
  {
    recLen      = DAYS_RECLEN;
    maxRecords  = DAYS_RECS;
    dataFile    = SPIFFS.open(DAYS_FILE, "r");      // open for Read    
  } else if (fileType == HOURS) 
  {
    recLen      = HOURS_RECLEN;
    maxRecords  = HOURS_RECS;
    dataFile    = SPIFFS.open(HOURS_FILE, "r");      // open for Read    
  } else 
  {
    maxRecords  = 0;
    recLen      = 0;
  }
  if (!dataFile) return 0;

  for(int r = 1; r <= maxRecords; r++) 
  {
    offset = r * recLen;
    dataFile.seek(offset, SeekSet);
    Label = (int)dataFile.readStringUntil(';').toInt();
    if (fileType == MONTHS) 
    {
      if (Verbose2) DebugTf("Check record[%02d] for [%04d] -> found [%04d]\r\n", r, RecKey, Label);
    } else if (fileType == DAYS) 
    {
      if (Verbose2) DebugTf("Check record[%02d] for [%06d] -> found [%06d]\r\n", r, RecKey, Label);
    } else if (fileType == HOURS) 
    {
      if (Verbose2) DebugTf("Check record[%02d] for [%08d] -> found [%08d]\r\n", r, RecKey, Label);
    }
    if (Label == RecKey) 
    {
      if (Verbose1) DebugTf("Found Label [%d]\r\n", Label);
      dataFile.close();
      return r;
    } else 
    {
      if (Label < RecKey) 
      {
        DebugTf("NotFound: Label [%d] < [%d]\r\n", Label, RecKey);
        dataFile.close();
        return -1;
      }
    }
    yield();
  }
  dataFile.close();
  return 0;

} // fileLabel2Rec()


//===========================================================================================
bool fileShiftDown(int8_t fileType) 
{
  int16_t recLen, offset, maxRecords;
  String  recData;
  bool    exitState = true;
  File    dataFile;
  
  if (fileType == MONTHS) 
  {
    dataFile    = SPIFFS.open(MONTHS_FILE, "r+");     // open for Read
    recLen      = MONTHS_RECLEN;
    maxRecords  = MONTHS_RECS;
    
  } else if (fileType == DAYS) 
  {
    dataFile    = SPIFFS.open(DAYS_FILE, "r+");     // open for Read
    recLen      = DAYS_RECLEN;
    maxRecords  = DAYS_RECS;
    
  } else if (fileType == HOURS) 
  {
    dataFile    = SPIFFS.open(HOURS_FILE, "r+");     // open for Read
    recLen      = HOURS_RECLEN;
    maxRecords  = HOURS_RECS;
  
  } else recLen = 0;
  
  if (!dataFile) return false;

  if (Verbose2) DebugTf("recLen is [%02d]\r\n", recLen);

  for (int r = maxRecords; r >= 1; r--) 
  {
    offset = r * recLen;
    dataFile.seek(offset, SeekSet);
    recData  = dataFile.readStringUntil('\n');
    if (Verbose2) DebugTf("Move record[%02d] @[%04d] to [%02d] - [%20.20s**]\r\n", r, offset, (r+1), recData.c_str());
    offset = (r+1) * recLen;
    dataFile.seek(offset, SeekSet);
    bytesWritten = dataFile.print(recData);
    if (bytesWritten != (recLen -1)) 
    {
      DebugTf("ERROR! recNo[%02d]: written [%d] bytes but should have been [%d] for Label %8.8s\r\n", r, bytesWritten, recLen, recData.c_str());
      exitState = false;  // save State, still need to close file
    }
    dataFile.print('\n');
    yield();
  }
  dataFile.close();

  if (!exitState) return false;

  return true;
  
} // fileShiftDown()


//===========================================================================================
void fileWriteData(int8_t fileType, dataStruct newDat) 
{
  if      (fileType == MONTHS)  { DebugTf("newDat.label is [%04d]\r\n", newDat.Label); }
  else if (fileType == DAYS)    { DebugTf("newDat.label is [%06d]\r\n", newDat.Label); }
  else if (fileType == HOURS)   { DebugTf("newDat.label is [%08d]\r\n", newDat.Label); }
  else 
  {
    DebugTf("Unknown fileType [%d] .. abort!\r\n", fileType);
    return;
  }
  int16_t recNo = fileLabel2Rec(fileType, newDat.Label);
  fileWriteData(fileType, newDat, recNo);

} // fileWriteData(dataStruct newDat)


//===========================================================================================
void fileWriteData(int8_t fileType, dataStruct newDat, int16_t recNo) 
{
  String  fileName;
  char *fileHeader, *fileFormat;
  int8_t  fileRecLen, fileNoRecs;
  
  DebugTf("----> write recNo[%d]\r\n", recNo);
  
  if (!SPIFFSmounted) 
  {
    DebugTln(F("No SPIFFS filesystem..ABORT!!!\r"));
    return;
  }
  
  if (fileType == MONTHS) 
  {
      fileName    = MONTHS_FILE;
      fileHeader  = (char*)MONTHS_CSV_HEADER;
      fileFormat  = (char*)MONTHS_FORMAT;
      fileRecLen  = MONTHS_RECLEN;
      fileNoRecs  = MONTHS_RECS;
      
  } else if (fileType == DAYS) 
  {
      fileName    = DAYS_FILE;
      fileHeader  = (char*)DAYS_CSV_HEADER;
      fileFormat  = (char*)DAYS_FORMAT;
      fileRecLen  = DAYS_RECLEN;
      fileNoRecs  = DAYS_RECS;
    
  } else if (fileType == HOURS) 
  {
      fileName    = HOURS_FILE;
      fileHeader  = (char*)HOURS_CSV_HEADER;
      fileFormat  = (char*)HOURS_FORMAT;
      fileRecLen  = HOURS_RECLEN;
      fileNoRecs  = HOURS_RECS;
    
  } else 
  {
      DebugTf("Unknown fileType [%d] .. abort!\r\n", fileType);
      return;
  }

  // --- check if the file exists and can be opened ---
  File dataFile  = SPIFFS.open(fileName, "r+");    // open for Read & writing
  if (!dataFile) 
  {
    DebugTf("File [%s] does not exist, create one\r\n", fileName.c_str());
    if (!createFile(fileType, fileName, fileHeader, fileFormat, fileRecLen)) 
    {
      return;
    }
  } // if (!dataFile)

  DebugTf("checkRecordsInFile [%s] ...\r\n", fileName.c_str());
  checkRecordsInFile(fileType, fileName, fileFormat, fileRecLen, fileNoRecs, newDat);

  //--- and now .. add or update the new data --------
  if (!dataFile) 
  {                            // if last open failed, try again after createFile()
    dataFile  = SPIFFS.open(fileName, "r+");  // open for Read & writing
  }
  if (recNo > 0) 
  {
    //---- write new data
    sprintf(cMsg, fileFormat, newDat.Label   , String(newDat.EDT1, 3).c_str()
                                             , String(newDat.EDT2, 3).c_str()
                                             , String(newDat.ERT1, 3).c_str()
                                             , String(newDat.ERT2, 3).c_str()
                                             , String(newDat.GDT, 3).c_str());
    fillRecord(cMsg, fileRecLen);
    dataFile.seek((recNo * fileRecLen), SeekSet);
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != fileRecLen) 
    {
      DebugTf("ERROR!! recNo[%02d]: written [%d] bytes but should have been [%d] for Label[%s]\r\n", recNo, bytesWritten, fileRecLen, cMsg);
    }
    if (Verbose1) DebugTf("recNo[%02d] := %s", recNo, cMsg);
    
  } else if (recNo == -1) 
  {
    DebugTln(F("Need to shift down!\r"));
    fileShiftDown(fileType);
    //---- write new data
    sprintf(cMsg, fileFormat, newDat.Label   , String(newDat.EDT1, 3).c_str()
                                             , String(newDat.EDT2, 3).c_str()
                                             , String(newDat.ERT1, 3).c_str()
                                             , String(newDat.ERT2, 3).c_str()
                                             , String(newDat.GDT,  3).c_str());
    fillRecord(cMsg, fileRecLen);
    dataFile.seek((1 * fileRecLen), SeekSet);
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != fileRecLen) 
    {
      DebugTf("ERROR!! recNo[%d]: written [%02d] bytes but should have been [%d] for Label[%s]\r\n", 1, bytesWritten, fileRecLen, cMsg);
    }

    if (Verbose1) DebugTf("recNo[%02d] Data[%s]", 1, cMsg);
  } else 
  {
    DebugTf("No record with label [%d] found!\r\n", newDat.Label);
  }
  yield();

  dataFile.close();  

  if (Verbose1) DebugTln(F(" ..Done\r"));

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

} // fileWriteData(fileType, dataStruct newDat, int8_t recNo)


//===========================================================================================
bool createFile(int8_t fileType, String fileName, const char *fileHeader, const char *fileFormat, uint16_t fileRecLen) 
{
  int8_t  YY, MM, DD, HH;

    File dataFile  = SPIFFS.open(fileName, "a");  // create File
    // -- first write fileHeader ----------------------------------------
    sprintf(cMsg, "%s", fileHeader);  // you cannot modify *fileHeader!!!
    DebugT(cMsg); Debugln(F("\r"));
    fillRecord(cMsg, fileRecLen);
    DebugT(cMsg); Debugln(F("\r"));
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != fileRecLen) 
    {
      DebugTf("ERROR!! recNo[%d]: written [%d] bytes but should have been [%d] for Header[%s]\r\n", 0, bytesWritten, fileRecLen, cMsg);
    }
    DebugTln(F(".. that went well! Now add next record ..\r"));
    // -- as this file is empty, write one data record ------------
    YY = YearFromTimestamp(pTimestamp);
    MM = MonthFromTimestamp(pTimestamp);
    DD = DayFromTimestamp(pTimestamp);
    HH = HourFromTimestamp(pTimestamp);
    if (fileType == MONTHS)     sprintf(cMsg, "%02d%02d", YY, MM);
    else if (fileType == DAYS)  sprintf(cMsg, "%02d%02d%02d", YY, MM, DD);
    else                        sprintf(cMsg, "%02d%02d%02d%02d", YY, MM, DD, HH);
    
    sprintf(cMsg, fileFormat, String(cMsg).toInt(), String(0,3).c_str(), String(0,3).c_str()
                                                  , String(0,3).c_str(), String(0,3).c_str()
                                                  , String(0,2).c_str() );
    fillRecord(cMsg, fileRecLen);
    DebugTf("Write [%s] Data[%s]\r\n", fileName.c_str(), cMsg);
//  dataFile.seek((1 * fileRecLen), SeekSet);
    bytesWritten = dataFile.print(cMsg);
    if (bytesWritten != fileRecLen) 
    {
      DebugTf("ERROR!! recNo[%d]: written [%d] bytes but should have been [%d] for record [1]\r\n", 0, bytesWritten, fileRecLen);
    }

    dataFile.close();
    dataFile  = SPIFFS.open(fileName, "r+");       // open for Read & writing
    if (!dataFile) 
    {
      DebugTf("Something is very wrong writing to [%s]\r\n", fileName.c_str());
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
  int8_t  YY, MM, DD, HH;

  // -- offSet can only be -1, 0 or +1 ---
  if (offSet < -1) offSet = -1;
  if (offSet >  1) offSet =  1;

  if (Verbose2) DebugTf("offSet[%d], Label IN[%d] ", offSet, Label);
  
  if (fileType == HOURS) 
  {
    label2Fields(Label, YY, MM, DD, HH);
    HH += offSet;

  } else if (fileType == DAYS) 
  {
    label2Fields(Label, YY, MM, DD);
    HH = 0;
    DD += offSet;

  } else if (fileType == MONTHS) 
  {
    label2Fields(Label, YY, MM);
    HH = 0;
    DD = 1;    
    MM += offSet;

  }
  if (offSet >= 1) 
  {
    if (HH > 23) 
    {
      HH = 0;
      DD++;    
    }
    if (MM == 4 || MM == 6 || MM == 9 || MM == 11) 
    {
      if (DD > 30) 
      {
        DD = 1;
        MM++ ;   
      }
    } else if (MM == 2) 
    {
      if (DD > 28) 
      {
        DD = 1;
        MM++;
      }
    } else if (DD > 31) 
    {
      DD = 1;
      MM++;      
    }
    if (MM > 12) 
    {
      MM = 1;
      YY++;
    }
    
  } else if (offSet <= -1) 
  {
    if (HH < 0) 
    {
      HH = 23;
      DD--;
    }
    if (DD < 1) 
    {
        MM--;
        if (MM == 4 || MM == 6 || MM == 9 || MM == 11)
              DD = 30;    
        else if (MM == 2)
              DD = 28;
        else  DD = 31;      
      }
      if (MM < 1) 
      {
        MM = 12;
        YY--;
      }
    } // offSet <= -1    
    if (YY <  0) YY =  0;
    if (YY > 99) YY = 99;
    if (MM <  1) MM =  1;
    if (MM > 12) MM = 12;
    if (DD <  1) DD =  1;
    if (DD > 31) DD = 31;
    if (HH <  0) HH =  0;
    if (HH > 23) HH = 23;

    if (fileType == HOURS) 
    {
      sprintf(cMsg, "%02d%02d%02d%02d", YY, MM, DD, HH);
      if (Verbose2) Debugf(" => Label OUT[%s]\r\n", cMsg);
      return (String(cMsg).toInt());
      
    } else if (fileType == DAYS) 
    {
      sprintf(cMsg, "%02d%02d%02d", YY, MM, DD);
      if (Verbose2) Debugf(" => Label OUT[%s]\r\n", cMsg);
      return (String(cMsg).toInt());
      
    } else if (fileType == MONTHS) 
    {
      sprintf(cMsg, "%02d%02d", YY, MM);
      if (Verbose2) Debugf(" => Label OUT[%s]\r\n", cMsg);
      return (String(cMsg).toInt());
      
    }

    return 0;
    
} // updateLabel()


//===========================================================================================
dataStruct fileReadData(int8_t fileType, uint8_t recNo) 
{
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
    sprintf(cMsg, MONTHS_CSV_HEADER);
    dataFile = SPIFFS.open(MONTHS_FILE, "r");
    if (Verbose2) DebugTf("%s: size(%d) \r\n", MONTHS_FILE, dataFile.size());
    recLen = MONTHS_RECLEN;
  }
  else if (fileType == DAYS) 
  {
    sprintf(cMsg, DAYS_CSV_HEADER);
    dataFile = SPIFFS.open(DAYS_FILE, "r");
    if (Verbose2) DebugTf("%s: size(%d) \r\n", DAYS_FILE, dataFile.size());
    recLen = DAYS_RECLEN;
  }
  else if (fileType == HOURS) 
  {
    sprintf(cMsg, HOURS_CSV_HEADER);
    dataFile = SPIFFS.open(HOURS_FILE, "r");
    if (Verbose2) DebugTf("%s: size(%d) \r\n", HOURS_FILE, dataFile.size());
    recLen = HOURS_RECLEN;
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
