/* 
***************************************************************************  
**  Program  : FSmanager, part of DSMRloggerAPI
**  Version  : v2.0.1
**
**  Mostly stolen from: https://fipsok.de/Esp8266-Webserver/littlefs-esp8266-270.tab
*/  
// ****************************************************************
// Sketch Esp8266 Filesystem Manager spezifisch sortiert Modular(Tab)
// created: Jens Fleischer, 2020-06-08
// last mod: Jens Fleischer, 2020-09-02
// For more information visit: https://fipsok.de/Esp8266-Webserver/littlefs-esp8266-270.tab
// ****************************************************************
// Hardware: Esp8266
// Software: Esp8266 Arduino Core 2.7.0 - 2.7.4
// Geprüft: von 1MB bis 2MB Flash
// Getestet auf: Nodemcu
/******************************************************************
  Copyright (c) 2020 Jens Fleischer. All rights reserved.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*******************************************************************/
// Diese Version von LittleFS sollte als Tab eingebunden werden.
// #include <FSYS.h> #include <ESP8266WebServer.h> müssen im Haupttab aufgerufen werden
// Die Funktionalität des ESP8266 Webservers ist erforderlich.
// "httpServer.onNotFound()" darf nicht im Setup des ESP8266 Webserver stehen.
// Die Funktion "setupFS();" muss im Setup aufgerufen werden.
/**************************************************************************************/

#include <list>
#include <tuple>

const char WARNING[] PROGMEM = R"(<h2>Check! Sketch is compiled with "FS:none"!)";
const char HELPER[]  PROGMEM = R"(
  <br>You first need to upload these two files:
  <ul>
    <li>FSmanager.html</li>
    <li>FSmanager.css</li>
  </ul>
  <hr>
  <form method="POST" action="/upload" enctype="multipart/form-data">
    <input type="file" name="upload">
    <input type="submit" value="Upload">
  </form>
  <hr>
  <br/><b>or</b> you can use the <i>Flash Utility</i> to flash firmware or litleFS!
  <form action='/update' method='GET'>
    <input type='submit' name='SUBMIT' value='Flash Utility'/>
  </form>
)";


//=====================================================================================
void setupFsManager() 
{                                                                       // Funktionsaufruf "setupFS();" muss im Setup eingebunden werden
  FSYS.begin();
  httpServer.on("/format", formatFS);
  httpServer.on("/listFS", listFS);
  httpServer.on("/ReBoot", reBootESP);
  httpServer.on("/upload", HTTP_POST, sendResponce, handleUpload);
  httpServer.on("/update", updateFirmware);
  httpServer.onNotFound([]() 
  {
    if (Verbose1) DebugTf("in 'onNotFound()'!! [%s] => \r\n", String(httpServer.uri()).c_str());
    /**
    if (httpServer.uri() == "/main") 
    {
      if (Verbose1) DebugTf("next: index.html\r\n", String(httpServer.uri()).c_str());
      doRedirect("Back in .. ", 0, "/", false);
    }
    if (httpServer.uri() == "/update") 
    {
      if (Verbose1) DebugTf("next: update\r\n", String(httpServer.uri()).c_str());
      doRedirect("Back in .. ", 0, "/updateIndex", false);
    }
    **/
    if (httpServer.uri().indexOf("/api/") == 0) 
    {
      if (Verbose1) DebugTf("next: processAPI(%s)\r\n", String(httpServer.uri()).c_str());
      processAPI();
    }
    else
    {
      DebugTf("next: handleFile(%s)\r\n"
                      , String(httpServer.urlDecode(httpServer.uri())).c_str());
      if (!handleFile(httpServer.urlDecode(httpServer.uri())))
      {
        httpServer.send(404, "text/plain", "FileNotFound\r\n");
      }
    }
  });
  
} //  setupFsManager()


//=====================================================================================
bool handleList() 
{ // Senden aller Daten an den Client
  // Füllt FSInfo Struktur mit Informationen über das Dateisystem
#if defined( USE_LITTLEFS )
  #warning LittleFS selected!!!
  FSInfo fs_info;  FSYS.info(fs_info);
  Dir dir = FSYS.openDir("/");
  using namespace std;
  typedef tuple<String, String, int> records;
  list<records> dirList;
  while (dir.next()) 
  { 
    yield();  
    if (dir.isDirectory()) // Ordner und Dateien zur Liste hinzufügen
    {
      uint8_t ran {0};
      Dir fold = FSYS.openDir(dir.fileName());
      while (fold.next())  
      {
        yield();
        ran++;
        dirList.emplace_back(dir.fileName(), fold.fileName(), fold.fileSize());
      }
      if (!ran) dirList.emplace_back(dir.fileName(), "", 0);
    }
    else {
      dirList.emplace_back("", dir.fileName(), dir.fileSize());
    }
  }
  dirList.sort([](const records & f, const records & l) 
  {                              // Dateien sortieren
    if (httpServer.arg(0) == "1") 
    {
      return get<2>(f) > get<2>(l);
    } else 
    {
      for (uint8_t i = 0; i < 31; i++) 
      {
        if (tolower(get<1>(f)[i]) < tolower(get<1>(l)[i])) return true;
        else if (tolower(get<1>(f)[i]) > tolower(get<1>(l)[i])) return false;
      }
      return false;
    }
  });
  dirList.sort([](const records & f, const records & l) 
  {                              // Ordner sortieren
    if (get<0>(f)[0] != 0x00 || get<0>(l)[0] != 0x00) 
    {
      for (uint8_t i = 0; i < 31; i++) 
      {
          if (tolower(get<0>(f)[i]) < tolower(get<0>(l)[i])) return true;
          else if (tolower(get<0>(f)[i]) > tolower(get<0>(l)[i])) return false;
      }
    }
    return false;
  });
  
  String temp = "[";
  for (auto& t : dirList) 
  {
    if (temp != "[") temp += ',';
    temp += "{\"folder\":\"" + get<0>(t) + "\",\"name\":\"" + get<1>(t) + "\",\"size\":\"" + formatBytes(get<2>(t)) + "\"}";
  }
              // Berechnet den verwendeten Speicherplatz
  temp += ",{\"usedBytes\":\"" + formatBytes(fs_info.usedBytes) +   
              // Zeigt die Größe des Speichers                  
          "\",\"totalBytes\":\"" + formatBytes(fs_info.totalBytes) +                  
              // Berechnet den freien Speicherplatz 
          "\",\"freeBytes\":\"" + (fs_info.totalBytes - fs_info.usedBytes) + "\"}]"; 

#else
  #warning SPIFFS selected!!!
  typedef struct _fileMeta {
    char    Name[30];     
    int32_t Size;
  } fileMeta;

  _fileMeta dirMap[30];
  int fileNr = 0;
  
  File root = SPIFFS.open("/", "r");         // List files on SPIFFS
  if(!root){
      DebugTln("- failed to open directory");
      return false;
  }
  if(!root.isDirectory())
  {
      DebugTln(" - not a directory");
      return false;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
        DebugT("  DIR : ");
        DebugTln(file.name());
        // directory is skipped
    } else 
    {
      dirMap[fileNr].Name[0] = '\0';
      strncat(dirMap[fileNr].Name, file.name(), 29); // first copy file.name() to dirMap
      memmove(dirMap[fileNr].Name, dirMap[fileNr].Name+1, strlen(dirMap[fileNr].Name)); // remove leading '/'
      dirMap[fileNr].Size = file.size();
    }
    file = root.openNextFile();
    fileNr++;
  }

  // -- bubble sort dirMap op .Name--
  for (int8_t y = 0; y < fileNr; y++) {
    yield();
    for (int8_t x = y + 1; x < fileNr; x++)  {
      //DebugTf("y[%d], x[%d] => seq[y][%s] / seq[x][%s] ", y, x, dirMap[y].Name, dirMap[x].Name);
      if (compare(String(dirMap[x].Name), String(dirMap[y].Name)))  
      {
        //Debug(" !switch!");
        fileMeta temp = dirMap[y];
        dirMap[y] = dirMap[x];
        dirMap[x] = temp;
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
  
  String temp = "[";
  for (int f=0; f < fileNr; f++)  
  {
    if (temp != "[") temp += ",";
    temp += R"({"name":")" + String(dirMap[f].Name) + R"(","size":")" + formatBytes(dirMap[f].Size) + R"("})";
  }
  //SPIFFS.info(SPIFFSinfo);
  //--- UITZOEKEN 
//  temp += R"(,{"usedBytes":")" + formatBytes(FSYS.usedBytes() * 1.05) + R"(",)" +       // Berechnet den verwendeten Speicherplatz + 5% Sicherheitsaufschlag
//          R"("totalBytes":")" + formatBytes(FSYS.totalBytes()) + R"(","freeBytes":")" + // Zeigt die Größe des Speichers
//          (FSYS.totalBytes() - (FSYS.usedBytes() * 1.05)) + R"("}])";                 // Berechnet den freien Speicherplatz + 5% Sicherheitsaufschlag

#endif

  httpServer.send(200, "application/json", temp);
  return true;
  
} //  handleList() 


#if defined( USE_LITTLEFS)
//=====================================================================================
  void deleteRecursive(const String &path) 
  {
    if (FSYS.remove(path)) 
    {
      FSYS.open(path.substring(0, path.lastIndexOf('/')) + "/", "w");
      return;
    }
    Dir dir = FSYS.openDir(path);
    while (dir.next()) 
    {
      yield();
      deleteRecursive(path + '/' + dir.fileName());
    }
    FSYS.rmdir(path);
    
  } //  deleteRecursive()
#endif

//=====================================================================================
bool handleFile(String &&path) 
{
  if (httpServer.hasArg("new")) 
  {
    String folderName {httpServer.arg("new")};
    for (auto& c : {34, 37, 38, 47, 58, 59, 92}) for (auto& e : folderName) if (e == c) e = 95;    // Ersetzen der nicht erlaubten Zeichen
    FSYS.mkdir(folderName);
  }
  if (httpServer.hasArg("sort")) return handleList();
  if (httpServer.hasArg("delete")) 
  {
    #if defined( USE_LITTLEFS )
      deleteRecursive(httpServer.arg("delete"));
    #else
      FSYS.remove(httpServer.arg("delete"));    // Datei löschen
    #endif
    sendResponce();
    return true;
  }
  if (!FSYS.exists("/FSmanager.html")) 
  {
    // ermöglicht das hochladen der FSmanager.html
    httpServer.send(200, "text/html", FSYS.begin() ? HELPER : WARNING);     
  }
  if (path.endsWith("/")) path += "/index.html";
  // Vorrübergehend für den Admin Tab
  //if (path == "/FSmanager.html") sendResponce(); 
  if (path == "/admin.html") sendResponce(); 

#if defined( USE_LITTLEFS)
  return FSYS.exists(path) ? ({File f = FSYS.open(path, "r"); httpServer.streamFile(f, mime::getContentType(path)); f.close(); true;}) : false;

#else
  return FSYS.exists(path) ? ({File f = FSYS.open(path, "r"); httpServer.streamFile(f, contentType(path)); f.close(); true;}) : false;

#endif
} //  handleFile()


//=====================================================================================
void handleUpload() 
{                            // Dateien ins Filesystem schreiben
  static File fsUploadFile;
  
  HTTPUpload& upload = httpServer.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    if (upload.filename.length() > 31) 
    {  // Dateinamen kürzen
      upload.filename = upload.filename.substring(upload.filename.length() - 31, upload.filename.length());
    }
    printf(PSTR("handleFileUpload Name: /%s\r\n"), upload.filename.c_str());
    fsUploadFile = FSYS.open(httpServer.arg(0) + "/" + httpServer.urlDecode(upload.filename), "w");
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    printf(PSTR("handleFileUpload Data: %u\r\n"), upload.currentSize);
    fsUploadFile.write(upload.buf, upload.currentSize);
  } 
  else if (upload.status == UPLOAD_FILE_END) 
  {
    printf(PSTR("handleFileUpload Size: %u\r\n"), upload.totalSize);
    fsUploadFile.close();
  }
  
} //  handleUpload()


//=====================================================================================
void formatFS()      // Formatiert das Filesystem
{
  DebugTln("formatting littleFS ..");
  //FSYS.format();
  sendResponce();

} //  formatFS()

//=====================================================================================
void listFS() 
{
  DebugTln("send littleFS data ..");
  sendResponce();
  
} //  listFS()


//=====================================================================================
void sendResponce() 
{
  httpServer.sendHeader("Location", "/FSmanager.html");
  httpServer.send(303, "message/http");

} //  sendResponce()


//=====================================================================================
const String formatBytes(size_t const& bytes) 
{                                        // lesbare Anzeige der Speichergrößen
  return bytes < 1024 ? static_cast<String>(bytes) + " Byte" : bytes < 1048576 ? static_cast<String>(bytes / 1024.0) + " KB" : static_cast<String>(bytes / 1048576.0) + " MB";

} //  formatBytes()


#if !defined( USE_LITTLEFS )
  //=====================================================================================
  const String &contentType(String& filename) 
  {       
    if (filename.endsWith(".htm") || filename.endsWith(".html")) filename = "text/html";
    else if (filename.endsWith(".css"))   filename = "text/css";
    else if (filename.endsWith(".js"))    filename = "application/javascript";
    else if (filename.endsWith(".json"))  filename = "application/json";
    else if (filename.endsWith(".png"))   filename = "image/png";
    else if (filename.endsWith(".gif"))   filename = "image/gif";
    else if (filename.endsWith(".jpg"))   filename = "image/jpeg";
    else if (filename.endsWith(".ico"))   filename = "image/x-icon";
    else if (filename.endsWith(".xml"))   filename = "text/xml";
    else if (filename.endsWith(".pdf"))   filename = "application/x-pdf";
    else if (filename.endsWith(".zip"))   filename = "application/x-zip";
    else if (filename.endsWith(".gz"))    filename = "application/x-gzip";
    else                                  filename = "text/plain";
    return filename;
  
  } // &contentType()
#endif


//=====================================================================================
void updateFirmware()
{
#ifdef USE_UPDATE_SERVER
  DebugTln(F("Redirect to updateIndex .."));
  doRedirect("wait ... ", 0, "/updateIndex", false);
#else
  doRedirect("UpdateServer not available", 10, "/", false);
#endif
      
} // updateFirmware()

//=====================================================================================
void reBootESP()
{
  DebugTln(F("Redirect and ReBoot .."));
  doRedirect("Reboot DSMR-logger ..", 30, "/", true);
      
} // reBootESP()

//=====================================================================================
void doRedirect(String msg, int wait, const char* URL, bool reboot)
{
  String redirectHTML = 
    "<!DOCTYPE HTML><html lang='en-US'>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<style type='text/css'>"
    "body {background-color: lightblue;}"
    "</style>"
    "<title>Redirect to Main Program</title>"
    "</head>"
    "<body>";

  if (wait > 0)
  {
    redirectHTML += 
      "<h1>"+String(settingHostname)+"</h1>"
      "<h3>"+msg+"</h3>"
      "<br><div style='width: 500px; position: relative; font-size: 25px;'>"
      "  <div style='float: left;'>Redirect over &nbsp;</div>"
      "  <div style='float: left;' id='counter'>"+String(wait)+"</div>"
      "  <div style='float: left;'>&nbsp; seconden ...</div>"
      "  <div style='float: right;'>&nbsp;</div>"
      "</div>"
      "<!-- Note: don't tell people to `click` the link, just tell them that it is a link. -->"
      "<br><br><hr>If you are not redirected automatically, click this <a href='/'>Main Program</a>."
      "  <script>"
      "      setInterval(function() {"
      "          var div = document.querySelector('#counter');"
      "          var count = div.textContent * 1 - 1;"
      "          div.textContent = count;"
      "          if (count <= 0) {"
      "              window.location.replace('"+String(URL)+"'); "
      "          } "
      "      }, 1000); "
      "  </script> "
      "</body></html>\r\n";
  }
  else  
  {
    redirectHTML += 
      "  <div style='position:absolute; bottom:0; right:0;' id='counter'>3</div>"
      "  <script>"
      "      setInterval(function() {"
      "          var div = document.querySelector('#counter');"
      "          var count =  div.textContent * 1 - 1;"
      "          div.textContent = count;"
      "          if (count <= 0) {"
      "              window.location.replace('"+String(URL)+"'); "
      "          } "
      "      }, 500); "
      "  </script> "
      "</body></html>\r\n";
  }
  
  DebugTln(msg);
  Debugln(redirectHTML);
  
  httpServer.send(200, "text/html", redirectHTML);
  if (reboot) 
  {
    delay(5000);
    ESP.restart();
    delay(5000);
  }
  
} // doRedirect()
/* eof */
