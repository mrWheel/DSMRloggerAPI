
static const char UpdateServerIndex[] PROGMEM =
  R"(<html charset="UTF-8">
     <style type='text/css'>
        body {background-color: lightblue;}
     </style>
     <body>
     <h1>DSMR-logger Flash utility</h1>
<<<<<<< HEAD
     <form method='POST' action='?cmd=0' enctype='multipart/form-data'>
          Selecteer een "<b>.ino.bin</b>" bestand<br/>
          <input type='file' accept='ino.bin' name='firmware'>
          <input type='submit' value='Flash Firmware'>
      </form>
      <form method='POST' action='?cmd=100' enctype='multipart/form-data'> 
          Selecteer een "<b>.spiffs.bin</b>" bestand<br/>
          <input type='file' accept='spiffs.bin' name='filesystem'>
          <input type='submit' value='Flash Spiffs'>
=======
     <form method='POST' action='' enctype='multipart/form-data'>
        Selecteer een "<b>.ino.bin</b>" bestand<br/>
        <input type='file' accept='ino.bin' name='firmware'>
        <input type='submit' value='Flash Firmware'>
      </form>
      <form method='POST' action='' enctype='multipart/form-data'> 
        Selecteer een "<b>.spiffs.bin</b>" bestand<br/>
        <input type='file' accept='spiffs.bin' name='filesystem'>
        <input type='submit' value='Flash Spiffs'>
>>>>>>> f537b35f0c5bfeba86ab88c2576c3f70a894eae4
      </form>
      <br/>Wacht nog <span style='font-size: 1.3em;' id="waitSeconds">120</span> seconden ..
      <br>Als het lijkt of er niets gebeurd, wacht dan tot de teller
           op 'nul' staat en klik daarna <span style='font-size:1.3em;'><b><a href="/">hier</a></b></span>!
     </body>
     <script>
         var seconds = document.getElementById("waitSeconds").textContent;
         var countdown = setInterval(function() {
           seconds--;
           document.getElementById('waitSeconds').textContent = seconds;
           if (seconds <= 0) {
              clearInterval(countdown);
              window.location.assign("/")
           }
         }, 1000);
     </script>
     </html>)";

static const char UpdateServerSuccess[] PROGMEM = 
  R"(<html charset="UTF-8">
     <style type='text/css'>
        body {background-color: lightgray;}
     </style>
     <body>
     <h1>DSMR-logger Flash utility</h1>
     <br/>
     <h2>Update successfull!</h2>
     <br/>
     <br/>Wait for the DSMR-logger to reboot and start the HTTP server
     <br/>
     <br>
     <br/>Wacht nog <span style='font-size: 1.3em;' id="waitSeconds">35</span> seconden ..
     <br/>Als het lijkt of er niets gebeurd, wacht dan tot de teller
          op 'nul' staat en klik daarna <span style='font-size:1.3em;'><b><a href="/">hier</a></b></span>!
     </body>
     <script>
         var seconds = document.getElementById("waitSeconds").textContent;
         var countdown = setInterval(function() {
           seconds--;
           document.getElementById('waitSeconds').textContent = seconds;
           if (seconds <= 0) {
              clearInterval(countdown);
              window.location.assign("/")
           }
         }, 1000);
     </script>
     </html>)";