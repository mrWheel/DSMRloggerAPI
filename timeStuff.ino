/*
***************************************************************************  
**  Program  : timeStuff, part of DSMRfirmwareAPI
**  Version  : v0.0.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//static time_t ntpTimeSav;

//===========================================================================================
String buildDateTimeString(const char* timeStamp, int len) 
{
  String tmpTS = String(timeStamp);
  String DateTime = "";
  if (len < 12) return String(timeStamp);
  DateTime   = "20" + tmpTS.substring(0, 2);    // YY
  DateTime  += "-"  + tmpTS.substring(2, 4);    // MM
  DateTime  += "-"  + tmpTS.substring(4, 6);    // DD
  DateTime  += " "  + tmpTS.substring(6, 8);    // HH
  DateTime  += ":"  + tmpTS.substring(8, 10);   // MM
  DateTime  += ":"  + tmpTS.substring(10, 12);  // SS
  return DateTime;
    
} // buildDateTimeString()

//===========================================================================================
void epochToTimestamp(time_t t, char *ts, int8_t len) 
{
  if (len < 12) {
    strcpy(ts, "Error");
    return;
  }
  //------------yy  mm  dd  hh  mm  ss
  sprintf(ts, "%02d%02d%02d%02d%02d%02d", year(t)-2000, month(t), day(t)
                                        , hour(t), minute(t), second(t));
                                               
  //DebugTf("epochToTimestamp() => [%s]\r\n", ts);
  
} // epochToTimestamp()

//===========================================================================================
String getDayName(int weekDayNr) 
{
  /*
  if (weekDayNr >=1 && weekDayNr <= 7)
      return weekDayName[weekDayNr];
      
  return weekDayName[0];    
  */
  return "vandaag";  
} // getDayName()


//===========================================================================================
int8_t SecondFromTimestamp(String timeStamp) 
{
  return timeStamp.substring(10, 12).toInt();
    
} // SecondFromTimestamp()

//===========================================================================================
int8_t MinuteFromTimestamp(String timeStamp) 
{
  return timeStamp.substring(8, 10).toInt();
    
} // MinuteFromTimestamp()

//===========================================================================================
int8_t HourFromTimestamp(String timeStamp) 
{
  return timeStamp.substring(6, 8).toInt();
    
} // HourFromTimestamp()

//===========================================================================================
int8_t DayFromTimestamp(String timeStamp) 
{
  return timeStamp.substring(4, 6).toInt();
    
} // DayFromTimestamp()

//===========================================================================================
int8_t MonthFromTimestamp(String timeStamp) 
{
  return timeStamp.substring(2, 4).toInt();
    
} // MonthFromTimestamp()

//===========================================================================================
int8_t YearFromTimestamp(String timeStamp) 
{
  return timeStamp.substring(0, 2).toInt();
    
} // YearFromTimestamp()

//===========================================================================================
int32_t HoursKeyTimestamp(String timeStamp) 
{
  return timeStamp.substring(0, 8).toInt();
    
} // HourFromTimestamp()

//===========================================================================================
// calculate epoch from timeStamp
// if syncTime is true, set system time to calculated epoch-time
time_t epoch(const char *timeStamp, int8_t len, bool syncTime) 
{
  //DebugTf("epoch(%s)\r\n", timeStamp.c_str());  Serial.flush();

  if (len < 13) return now();
  /*
  DebugTf("DateTime: [%02d]-[%02d]-[%02d] [%02d]:[%02d]:[%02d]\r\n"
                                                                 ,DayFromTimestamp(timeStamp)
                                                                 ,MonthFromTimestamp(timeStamp)
                                                                 ,YearFromTimestamp(timeStamp)
                                                                 ,HourFromTimestamp(timeStamp)
                                                                 ,MinuteFromTimestamp(timeStamp)
                                                                 ,0
                       );
  */ 
 
  time_t nT;
  time_t savEpoch = now();
  
  setTime(HourFromTimestamp(timeStamp)
         ,MinuteFromTimestamp(timeStamp)
         ,SecondFromTimestamp(timeStamp)
         ,DayFromTimestamp(timeStamp)
         ,MonthFromTimestamp(timeStamp)
         ,YearFromTimestamp(timeStamp));

  nT = now();
  if (!syncTime)
  {
    setTime(savEpoch);
    //DebugTln("Restore saved time");
  }
  return nT;

} // epoch()


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
