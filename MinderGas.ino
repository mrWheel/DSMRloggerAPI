/**************************************************************************
**  Program  : MinderGas.ino
**  Version  : v0.1.7
**
**  Copyright (c) 2020 Robert van den Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
* Created by Robert van den Breemen (8 feb 2020)
*
*/
#define MG_FILENAME         "/Mindergas.post"

DECLARE_TIMER_MIN(minderGasTimer, 1); 

//=======================================================================
void handleMindergas()
{
  #ifdef USE_MINDERGAS

  if (DUE(minderGasTimer) )
  {
    processMindergas();
  }
  #endif

} // handleMindergas()


#ifdef USE_MINDERGAS

DECLARE_TIMER_MIN(MGcountDownTimer, 1); //do not change 

enum states_of_MG { MG_INIT, MG_WAIT_FOR_FIRST_TELEGRAM, MG_WAIT_FOR_NEXT_DAY
                           , MG_WRITE_TO_FILE, MG_DO_COUNTDOWN
                           , MG_SEND_MINDERGAS, MG_NO_AUTHTOKEN, MG_ERROR };
                           
enum states_of_MG stateMindergas = MG_INIT;

int8_t    MG_Day                    = -1;
bool      validToken                = false;
bool      handleMindergasSemaphore  = false;
int8_t    MGminuten                 = 0;

//=======================================================================
//force mindergas update, by skipping states
void forceMindergasUpdate()
{
  sprintf(timeLastResponse, "@%02d|%02d:%02d -> ", day(), hour(), minute());

  validToken = true;

  if (SPIFFS.exists(MG_FILENAME))
  {
    MG_Day = day();   // make it thisDay...
    strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "force Mindergas countdown");
    DebugTln(F("Force send data to mindergas.nl in ~1 minute"));
    MGminuten=1;
    stateMindergas = MG_DO_COUNTDOWN;
    processMindergas();
  }
  else
  {
    strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Force Write Mindergas.post");
    DebugTln(F("Force Write data to post file now!"));
    stateMindergas = MG_WRITE_TO_FILE;  // write file is next state
    processMindergas();
  }
  
} // forceMindergasUpdate()

//=======================================================================
// handle finite state machine of mindergas
void processMindergas()
{
  time_t t;
  File   minderGasFile;

  if (handleMindergasSemaphore) // if already running ? then return...
  {
    DebugTln(F("already running .. bailing out!"));
    return; // you may only enter this once
  } 
  //signal that we are busy...
  handleMindergasSemaphore = true;
  
  yield(); 
  
  switch(stateMindergas) {
    case MG_INIT:  // only after reboot
      DebugTln(F("Mindergas State: MG_INIT"));
      sprintf(timeLastResponse, "@%02d|%02d:%02d -> ", day() , hour(), minute());
      if (SPIFFS.exists(MG_FILENAME))
      {
        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "found Mindergas.post");
        validToken     = true;
        stateMindergas = MG_SEND_MINDERGAS;
        DebugTln(F("Next State: MG_SEND_MINDERGAS"));
        break;
      } 
      // check to see if there is a authtoken
      validToken = (strlen(settingMindergasToken) > 5); // Assume there is a valid token, if there is a string. To be proven later.
      if  (validToken) 
      {
        //Next state is wait for first telegram
        stateMindergas = MG_WAIT_FOR_FIRST_TELEGRAM; 
      }
      else
      {
        // No AuthToken
        DebugTln(F("MinderGas Authtoken is not set, no update can be done."));
        //DebugTln(F("Next State: MG_NO_AUTHTOKEN"));
        stateMindergas = MG_NO_AUTHTOKEN; // no token, no mindergas
      } // end-if 
      break;
      
    case MG_WAIT_FOR_FIRST_TELEGRAM:
      DebugTln(F("Mindergas State: MG_WAIT_FOR_FIRST_TELEGRAM"));
      // if you received at least one telegram, then wait for midnight
      if ((telegramCount - telegramErrors) > 1) 
      {
        // Now you know what day it is, do setup MG_Day. This to enable day change detection.
        MG_Day = day(); 
        stateMindergas = MG_WAIT_FOR_NEXT_DAY;
      }
      break;
      
    case MG_WAIT_FOR_NEXT_DAY:
      DebugTln(F("Mindergas State: MG_WAIT_FOR_NEXT_DAY"));
      // Detect day change at midnight, then...
      if (MG_Day != day())                  // It is no longer the same day, so it must be past midnight
      {
        MG_Day = day();                     // make it thisDay...
        //DebugTln(F("Next State: MG_WRITE_TO_FILE"));
        stateMindergas = MG_WRITE_TO_FILE;  // write file is next state
      }
      break;
      
    case MG_WRITE_TO_FILE:
      DebugTln(F("Mindergas State: MG_WRITE_TO_FILE"));
      // create POST and write to file, so it will survive a reset within the countdown period
      DebugTf("Writing to [%s] ..\r\n", MG_FILENAME);
      minderGasFile = SPIFFS.open(MG_FILENAME, "a"); //  create File
      if (!minderGasFile) 
      {
        // cannot create file, thus error
        DebugTf("open(%s, 'w') FAILED!!! --> Bailout\r\n", MG_FILENAME);
        // now in failure mode
        //DebugTln(F("Next State: MG_ERROR"));
        stateMindergas = MG_ERROR;
        break;
      } 
      // write POST respons into file
      yield();
      DebugTln(F("Start writing POST data "));
      t = now() - SECS_PER_DAY;  // we want to upload the gas usage of yesterday so rewind the clock for 1 day
      char dataString[80];
      sprintf(dataString,"{ \"date\": \"%04d-%02d-%02d\", \"reading\": \"%.3f\" }"
                                                          , year(t)
                                                          , month(t)
                                                          , day(t)
                                                          , DSMRdata.gas_delivered.val());
      // write the POST to a file...
      minderGasFile.println(F("POST /api/gas_meter_readings HTTP/1.1"));
      minderGasFile.print(F("AUTH-TOKEN:")); minderGasFile.println(settingMindergasToken);
      minderGasFile.println(F("Host: mindergas.nl"));
      minderGasFile.println(F("User-Agent: DSMRWS"));
      minderGasFile.println(F("Content-Type: application/json"));
      minderGasFile.println(F("Accept: application/json"));
      minderGasFile.print(F("Content-Length: ")); minderGasFile.println(strlen(dataString));
      minderGasFile.println();
      minderGasFile.println(dataString);        

      minderGasFile.close();
      sprintf(timeLastResponse, "@%02d|%02d:%02d -> ", day(), hour(), minute());
      strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Mindergas.post aangemaakt");

      // start countdown
      MGminuten = random(10,120);
      DebugTf("MinderGas update in [%d] minute(s)\r\n", MGminuten);
      // Lets'do the countdown
      stateMindergas = MG_DO_COUNTDOWN;
      break;
      
    case MG_DO_COUNTDOWN:
      DebugTf("Mindergas State: MG_DO_COUNTDOWN (%d minuten te gaan)\r\n", MGminuten);
      sprintf(timeLastResponse, "@%02d|%02d:%02d -> ", day(), hour(), minute());
      strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "countdown for sending");
      if ( DUE(MGcountDownTimer) ) //this timer need to run every minute to let this work
      { 
        MGminuten--; 
        DebugTf("MinderGas update in less than [%d] minutes\r\n", MGminuten);
        intStatuscodeMindergas = MGminuten;
      }
      if (MGminuten <= 0)
      {
        //--- when waitime is done, it's time to send the POST string
        intStatuscodeMindergas = 0;
        stateMindergas = MG_SEND_MINDERGAS;
      }
      break;
      
    case MG_SEND_MINDERGAS:
      DebugTln(F("Mindergas State: MG_SEND_MINDERGAS"));
      strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "try to send Mindergas.post");

      // if POST response for Mindergas exists, then send it... btw it should exist by now :)
      if ((validToken) && SPIFFS.exists(MG_FILENAME)) 
      {  
          WiFiClient MGclient;   
          // try to connect to minderGas
          DebugTln(F("Connecting to Mindergas..."));
          //connect over http with mindergas
          if (MGclient.connect((char*)"mindergas.nl",80)) 
          {
            // create a string with the date and the meter value
            minderGasFile = SPIFFS.open(MG_FILENAME, "r");
            String sBuffer;
            sBuffer = "";
            while(minderGasFile.available()) 
            { 
              char ltr = minderGasFile.read();
              sBuffer += ltr;
            }
            minderGasFile.close();
            // then post to mindergas...
            DebugTln(F("Reading POST from file:"));
            Debugln(sBuffer);
            DebugTln(F("Send to Mindergas.nl..."));
            MGclient.println(sBuffer);
            // read response from mindergas.nl
            sprintf(timeLastResponse, "@%02d|%02d:%02d >> ", day() , hour(), minute());
            DebugTf("[%s] Mindergas response: ", timeLastResponse);
            bool bDoneResponse = false;
            while (!bDoneResponse && (MGclient.connected() || MGclient.available())) 
            {
              if (MGclient.available()) 
              {
                  // read the status code the response
                  if (MGclient.find("HTTP/1.1"))
                  {
                    // skip to find HTTP/1.1
                    // then parse response code
                    intStatuscodeMindergas = MGclient.parseInt(); // parse status code
                    //Debugln();
                    Debugf("Statuscode: [%d]\r\n", intStatuscodeMindergas);
                    switch (intStatuscodeMindergas) {
                      case 401:
                        validToken = false;
                        strCopy(settingMindergasToken, sizeof(settingMindergasToken), "Invalid token"); 
                        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Unauthorized, token invalid!"); // report error back to see in settings page
                        DebugTln(F("Invalid Mindergas Authenication Token"));
                        stateMindergas = MG_NO_AUTHTOKEN;
                        break;
                        
                      case 422:
                        validToken = true;
                        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Unprocessed entity"); // report error back to see in settings page
                        DebugTln(F("Unprocessed entity, goto website mindergas for more information")); 
                        stateMindergas = MG_WAIT_FOR_NEXT_DAY;              
                        break;
                        
                      case 201:  
                        validToken = true;
                        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Created entry"); // report error back to see in settings page
                        DebugTln(F("Succes, the gas delivered has been added to your mindergas.nl account"));
                        DebugTln(F("Next State: MG_WAIT_FOR_NEXT_DAY"));
                        stateMindergas = MG_WAIT_FOR_NEXT_DAY;               
                        break;
                        
                      default:
                        validToken = true;
                        strCopy(txtResponseMindergas, sizeof(txtResponseMindergas), "Unknown response code"); // report error back to see in settings page
                        DebugTln(F("Unknown responsecode, goto mindergas for information"));
                        stateMindergas = MG_WAIT_FOR_NEXT_DAY;           
                        break;
                    } // end switch-case             
                  }  // end-if find HTTP/1.1
                  
                  //close HTTP connection
                  MGclient.stop();
                  DebugTln(F("Disconnected from mindergas.nl"));
                  // delete POST file from SPIFFS
                  if (SPIFFS.remove(MG_FILENAME)) 
                  {
                    DebugTln(F("POST Mindergas file succesfully deleted!"));
                  } 
                  else 
                  {
                    // help, this should just not happen, but if it does, it will not influence behaviour in a negative way
                    DebugTln(F("Failed to delete POST Mindergas file"));
                  } 
                  bDoneResponse = true;    
              } // end-if client.available() 
              else 
              {
                // wait for connections, just keep trying...
                delay(100); 
              } // end-else
            } // end-while
        } // sending done
      } // end-if file exists
      break;
      
    case MG_NO_AUTHTOKEN:
      if (Verbose2) DebugTln(F("Mindergas State: MG_NO_AUTHTOKEN"));
      if (validToken)
      {
        stateMindergas = MG_INIT;   
      }
      // Do not update mindergas when a failing token is detected
      break;
      
    case MG_ERROR:
      DebugTln(F("Mindergas State: MG_ERROR"));
      break;
      
    default:
      DebugTln(F("Mindergas State: Impossible, default state!")); 
      break;  
          
  } // switch(..)

  //on exit, allow next handle state event
  handleMindergasSemaphore = false;
  
} // processMindergas()

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
