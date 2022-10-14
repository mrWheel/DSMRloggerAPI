/*
**************************************************************************
**  Program  : MinderGas.ino
**  Version  : v3.0
**
**  Copyright (c) 2020 .. 2022 Robert van den Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.
***************************************************************************
* Created by Robert van den Breemen (8 feb 2020)
*
*/
#define MG_FILENAME         "/Mindergas.post"

//=======================================================================
void handleMindergas()
{
#ifdef USE_MINDERGAS
  processMindergas_FSM();
#endif

} // handleMindergas()


#ifdef USE_MINDERGAS

enum states_of_MG { MG_INIT, MG_WAIT_FOR_FIRST_TELEGRAM, MG_WAIT_FOR_NEXT_DAY
, MG_WRITE_TO_FILE, MG_DO_COUNTDOWN
, MG_SEND_MINDERGAS, MG_NO_AUTHTOKEN, MG_ERROR
                  };

enum  states_of_MG stateMindergas   = MG_INIT;
bool  sendMindergasPostFile();
void  writePostToFile();

int8_t    MG_Day                    = -1;
bool      validToken                = false;
bool      handleMindergasSemaphore  = false;
int8_t    MGminuten                 = 0;
int8_t    retryCounter = 0;
bool      bDoneResponse             = false;

//=======================================================================
//force mindergas update, by skipping states
void forceMindergasUpdate()
{
  snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());

  validToken = true;

  if (FSYS.exists(MG_FILENAME))
  {
    writeToSysLog("found [%s] at day#[%d]", MG_FILENAME, day());
    MG_Day = day();   // make it thisDay...
    strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "force Mindergas countdown");
    DebugTln(F("Force send data to mindergas.nl in ~1 minute"));
    MGminuten=1;
    CHANGE_INTERVAL_MIN(minderGasTimer, 1);
    stateMindergas = MG_DO_COUNTDOWN;
    processMindergas_FSM();
  }
  else
  {
    strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Force Write Mindergas.post");
    DebugTln(F("Force Write data to post file now!"));
    writeToSysLog("Force Write Data to [%s]", MG_FILENAME);
    CHANGE_INTERVAL_MIN(minderGasTimer, 1);
    stateMindergas = MG_WRITE_TO_FILE;  // write file is next state
    processMindergas_FSM();
  }

} // forceMindergasUpdate()


//=======================================================================
// handle finite state machine of mindergas
void processMindergas_FSM()
{
  if (handleMindergasSemaphore)   // if already running ? then return...
  {
    DebugTln(F("already running .. bailing out!"));
    writeToSysLog("already running .. bailing out!");
    return; //--- you may only enter this once
  }
  //signal that we are busy...
  handleMindergasSemaphore = true;

  yield();

  switch(stateMindergas)
  {
    case MG_INIT: //--- only after reboot
      DebugTln(F("Mindergas State: MG_INIT"));
      writeToSysLog("Mindergas State: MG_INIT");
      snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());
      if (intStatuscodeMindergas == 0)
      {
        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "INITIAL STATE");
      }
      if (FSYS.exists(MG_FILENAME))
      {
        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "found Mindergas.post");
        writeToSysLog(txtResponseMindergas);
        validToken     = true;
        stateMindergas = MG_SEND_MINDERGAS;
        CHANGE_INTERVAL_MIN(minderGasTimer, 1);
        break;
      }
      //--- check to see if there is a authtoken
      //--- Assume there is a valid token, if there is a string. To be proven later.
      validToken = (strlen(settingMindergasToken) > 5);
      if  (validToken)
      {
        CHANGE_INTERVAL_MIN(minderGasTimer, 1);
        //--- Next state is wait for first telegram
        stateMindergas = MG_WAIT_FOR_FIRST_TELEGRAM;
      }
      else
      {
        //--- No AuthToken
        DebugTln(F("MinderGas Authtoken is not set, no update can be done."));
        writeToSysLog("MinderGas Authtoken is not set, no update can be done.");
        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "NO_AUTHTOKEN");
        stateMindergas = MG_NO_AUTHTOKEN; // no token, no mindergas
      } // end-if
      break;

    case MG_WAIT_FOR_FIRST_TELEGRAM:
      DebugTln(F("Mindergas State: MG_WAIT_FOR_FIRST_TELEGRAM"));
      writeToSysLog("Mindergas State: MG_WAIT_FOR_FIRST_TELEGRAM");
      //--- if you received at least one telegram, then wait for midnight
      if ((telegramCount - telegramErrors) > 1)
      {
        //--- Now you know what day it is, do setup MG_Day. This to enable day change detection.
        MG_Day = day();
        stateMindergas = MG_WAIT_FOR_NEXT_DAY;
      }
      break;

    case MG_WAIT_FOR_NEXT_DAY:
      DebugTln(F("Mindergas State: MG_WAIT_FOR_NEXT_DAY"));
      writeToSysLog("Mindergas State: MG_WAIT_FOR_NEXT_DAY");
      if (intStatuscodeMindergas == 0)
      {
        snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());
        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "WAIT_FOR_NEXT_DAY");
      }
      CHANGE_INTERVAL_MIN(minderGasTimer, 30);
      //--- Detect day change at midnight, then...
      if (MG_Day != day())                  // It is no longer the same day, so it must be past midnight
      {
        MG_Day = day();                     // make it thisDay...
        writeToSysLog("a new day has become .. next: Write to file");
        snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());
        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "WRITE_TO_FILE");
        CHANGE_INTERVAL_MIN(minderGasTimer, 1);
        stateMindergas = MG_WRITE_TO_FILE;  // write file is next state
      }
      break;

    case MG_WRITE_TO_FILE:
      DebugTln(F("Mindergas State: MG_WRITE_TO_FILE"));
      writeToSysLog("Mindergas State: MG_WRITE_TO_FILE");
      writePostToFile();
      CHANGE_INTERVAL_MIN(minderGasTimer, 1);
      break;

    case MG_DO_COUNTDOWN:
      DebugTf("Mindergas State: MG_DO_COUNTDOWN (%d minuten te gaan)\r\n", MGminuten);
      writeToSysLog("Mindergas State: MG_DO_COUNTDOWN (%d minuten te gaan)", MGminuten);
      snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());
      snprintf(txtResponseMindergas, sizeof(txtResponseMindergas), "Send DATA in %d minutes", MGminuten );
      DebugTf("MinderGas update in less than [%d] minutes\r\n", MGminuten );
      MGminuten--;
      intStatuscodeMindergas = MGminuten;

      if (MGminuten <= 0)
      {
        //--- when waitime is done, it's time to send the POST string
        intStatuscodeMindergas = 0;
        stateMindergas = MG_SEND_MINDERGAS;
      }
      break;

    case MG_SEND_MINDERGAS:
      DebugTln(F("Mindergas State: MG_SEND_MINDERGAS"));
      writeToSysLog("Mindergas State: MG_SEND_MINDERGAS");

      snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());
      strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "SEND_MINDERGAS");

      //--- if POST response for Mindergas exists, then send it... btw it should exist by now :)
      if ((validToken) && FSYS.exists(MG_FILENAME))
      {
        if (!sendMindergasPostFile())
        {
          Debugln();
          retryCounter++;
          if (retryCounter < 3)
          {
            //--- start countdown -- again
            MGminuten = random(1, 10);
            DebugTf("Try[%d] MinderGas update in [%d] minute(s)\r\n", retryCounter, MGminuten);
            writeToSysLog("Try[%d] MinderGas update in [%d] minute(s)", retryCounter, MGminuten);
            //--- Lets'do the countdown
            strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "DO_COUNTDOWN");
            stateMindergas = MG_DO_COUNTDOWN;
            break;
          }
        }
        Debugln();
        //--- delete POST file from FSYS
        if (FSYS.remove(MG_FILENAME))
        {
          DebugTln(F("POST Mindergas file succesfully deleted!"));
          writeToSysLog("Deleted Mindergas.post !");
        }
        else
        {
          //--- help, this should just not happen, but if it does, it
          //--- will not influence behaviour in a negative way
          DebugTln(F("Failed to delete POST Mindergas file"));
          writeToSysLog("Failed to delete Mindergas.post");
        }
        //          bDoneResponse = true;
      }
      CHANGE_INTERVAL_MIN(minderGasTimer, 30);
      break;

    case MG_NO_AUTHTOKEN:
      if (Verbose2) DebugTln(F("Mindergas State: MG_NO_AUTHTOKEN"));
      if (validToken)
      {
        stateMindergas = MG_INIT;
      }
      CHANGE_INTERVAL_MIN(minderGasTimer, 60);
      break;

    case MG_ERROR:
      DebugTln(F("Mindergas State: MG_ERROR"));
      writeToSysLog("Mindergas State: MG_ERROR");
      CHANGE_INTERVAL_MIN(minderGasTimer, 30);
      break;

    default:
      DebugTln(F("Mindergas State: Impossible, default state!"));
      writeToSysLog("Mindergas State: Impossible, default state!");
      CHANGE_INTERVAL_MIN(minderGasTimer, 10);
      stateMindergas = MG_INIT;
      break;

  } // switch(..)

  //on exit, allow next handle state event
  handleMindergasSemaphore = false;

} // processMindergas_FSM()


//=======================================================================
boolean sendMindergasPostFile()
{
  WiFiClient  MGclient;
  const char *MGhost = "www.mindergas.nl";
  File        minderGasFile;
  boolean     bMgClientReply = false;

  bDoneResponse = false;

  //--- create a string with the date and the meter value
  DebugTln(F("Reading POST from file:"));
  minderGasFile = FSYS.open(MG_FILENAME, "r");
  String sBuffer;
  sBuffer = "";
  while(minderGasFile.available())
  {
    char ltr = minderGasFile.read();
    sBuffer += ltr;
  }
  minderGasFile.close();
  Debugln(sBuffer);

  //--- try to connect to minderGas
  DebugTf("Connecting to %s ...\r\n", MGhost);
  //--- connect over http with mindergas
  if (MGclient.connect(MGhost, 80))
  {
    //--- then post to mindergas...
    DebugTln(F("Send to Mindergas.nl..."));
    writeToSysLog("Send to Mindergas.nl...");
    MGclient.println(sBuffer);
    //--- read response from mindergas.nl
    snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d >> ", day(), hour(), minute());
    DebugTf("[%s] Mindergas response: ", timeLastResponse);

    MGclient.setTimeout(1000);

    while (!bDoneResponse && (MGclient.connected() || MGclient.available()))
    {
      Debug(".");
      if (MGclient.available())
      {
        Debug(".");
        //--- skip to find HTTP/1.1
        //--- then parse response code
        if (MGclient.find("HTTP/1.1"))
        {
          intStatuscodeMindergas = MGclient.parseInt(); // parse status code
          //Debugln();
          writeToSysLog("Mindergas response: [%d]", intStatuscodeMindergas);
          Debugf("Statuscode: [%d]\r\n", intStatuscodeMindergas);
          switch (intStatuscodeMindergas)
          {
            case 201:
              validToken = true;
              bMgClientReply = true;
              //--- report error back to see in settings page
              strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Created entry");
              Debugln(F("Succes, the gas delivered has been added to your mindergas.nl account"));
              writeToSysLog("Succes, the gas delivered has been added to your mindergas.nl account");
              DebugTln(F("Next State: MG_WAIT_FOR_NEXT_DAY"));
              stateMindergas = MG_WAIT_FOR_NEXT_DAY;
              break;

            case 401:
              validToken = false;
              bMgClientReply = true;
              strCopy(settingMindergasToken, sizeof(settingMindergasToken), "Invalid token");
              strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Unauthorized, token invalid!"); // report error back to see in settings page
              Debugln(F("Invalid Mindergas Authenication Token"));
              writeToSysLog("Invalid Mindergas Authenication Token");
              stateMindergas = MG_NO_AUTHTOKEN;
              break;

            case 422:
              validToken = true;
              bMgClientReply = true;
              //--- report error back to see in settings page
              strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Unprocessed entity");
              Debugln(F("Unprocessed entity, goto website mindergas for more information"));
              writeToSysLog("Unprocessed entity, goto website mindergas for more information");
              stateMindergas = MG_WAIT_FOR_NEXT_DAY;
              break;

            default:
              validToken = true;
              bMgClientReply = true;
              //--- report error back to see in settings page
              strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Unknown response code");
              Debugln(F("Unknown responsecode, goto mindergas for information"));
              stateMindergas = MG_WAIT_FOR_NEXT_DAY;
              break;
          } // end switch-case
        }  // end-if find HTTP/1.1

        //--- close HTTP connection
        MGclient.stop();
        DebugTln(F("Disconnected from mindergas.nl"));
        writeToSysLog("Disconnected from mindergas.nl");
        bDoneResponse = true;

      } // end-if client.available()
      else
      {
        //--- wait for connections, just keep trying...
        delay(100);
      } // end-else

    } // while ..

  } //   connect to mindergas.nl
  else
  {
    DebugTln(".. not connected (ERROR!)");
  }
  return bMgClientReply;

} // sendMindergasPostFile()



//=======================================================================
void writePostToFile()
{
  //--- create POST and write to file, so it will survive a reset within the countdown period
  DebugTf("Writing to [%s] ..\r\n", MG_FILENAME);
  writeToSysLog("Writing to [%s] ..", MG_FILENAME);
  File minderGasFile = FSYS.open(MG_FILENAME, "a"); //  create File
  if (!minderGasFile)
  {
    //--- cannot create file, thus error
    DebugTf("open(%s, 'w') FAILED!!! --> Bailout\r\n", MG_FILENAME);
    //--- now in failure mode
    //DebugTln(F("Next State: MG_ERROR"));
    strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "ERROR CREATE FILE");
    writeToSysLog(txtResponseMindergas);
    stateMindergas = MG_ERROR;
    return;
  }
  //--- write POST respons into file
  yield();
  DebugTln(F("Start writing POST data "));
  //--- we want to upload the gas usage of yesterday so rewind the clock for 1 day
  time_t t = now() - SECS_PER_DAY;
  char dataString[80];
  snprintf(dataString, sizeof(dataString), "{ \"date\": \"%04d-%02d-%02d\", \"reading\": \"%.3f\" }"
           , year(t)
           , month(t)
           , day(t)
           , gasDelivered);
  //                                                        , DSMRdata.mbus1_delivered.val());
  //--- write the POST to a file...
  //-- api aanroep is veranderd per 1 jan. 2021 (??)
  //-- was: minderGasFile.println(F("POST /api/gas_meter_readings HTTP/1.1"));
  //-- hieronder de nieuwe URL
  minderGasFile.println(F("POST /api/meter_readings HTTP/1.1"));
  minderGasFile.print(F("AUTH-TOKEN:"));
  minderGasFile.println(settingMindergasToken);
  minderGasFile.println(F("Host: mindergas.nl"));
  minderGasFile.println(F("User-Agent: DSMRAPI"));
  minderGasFile.println(F("Content-Type: application/json"));
  minderGasFile.println(F("Accept: application/json"));
  minderGasFile.print(F("Content-Length: "));
  minderGasFile.println(strlen(dataString));
  minderGasFile.println();
  minderGasFile.println(dataString);

  minderGasFile.close();
  snprintf(timeLastResponse, sizeof(timeLastResponse), "@%02d|%02d:%02d -> ", day(), hour(), minute());
  strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Mindergas.post aangemaakt");
  writeToSysLog(txtResponseMindergas);

  //--- start countdown
  MGminuten = random(10, 120);
  DebugTf("MinderGas update in [%d] minute(s)\r\n", MGminuten);
  writeToSysLog("MinderGas update in [%d] minute(s)", MGminuten);
  //--- Lets'do the countdown
  strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "DO_COUNTDOWN");
  stateMindergas = MG_DO_COUNTDOWN;

} // writePostToFile()

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
