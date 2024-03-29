# DSMRloggerAPI
Firmware for the DSMR-logger using only API call's

[Here](https://willem.aandewiel.nl/index.php/2019/04/09/dsmr-logger-v4-slimme-meter-uitlezer/) and
    [here](https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/)
          you can find information about this project.

Documentation can be found [here](https://mrwheel-docs.gitbook.io/dsmrloggerapi/) (in progress)!

Let op!

Een nieuwe ontwikkeling van de hard- and software staat [hier](https://willem.aandewiel.nl/index.php/2022/11/15/crisis-what-crisis/) beschreven!


There is a new permutation of the hard- and software that you can find [here](https://willem.aandewiel.nl/index.php/2022/11/15/crisis-what-crisis/)!

<table>
  <tr><th>Versie</th><th align="Left">Opmerking</th></tr>
  <tr>
    <td valign="top">3.0.4</td>
    <td>"One Fits All" Release
        <br>Nieuwe optie op DSMR-logger dagelijks te rebooten
        <br>URL mindergas API aangepast
    </td>
  </tr>
  <tr>
    <td valign="top">3.0.3</td>
    <td>"One Fits All" Release
        <br>Zet Wifi mode op "WIFI_STA" in startWiFi()
    </td>
  </tr>
  <tr>
    <td valign="top">3.0.2</td>
    <td>"One Fits All" Release
        <br>In de zeldzame situatie waardoor de Slimme Meter een ObisID 0-0:96.13.0
        <br>melding genereert kan de DSMRlogger crashen. Dit komt omdat dit veld
        <br>een maximum grootte heeft van 2048 chars.
        <br>Het veld "message_long" niet opvraagbaar gemaakt.
        <br>Processing van RAW telegram aangepast zodat ook een telegram met
        <br>bovengenoemde ObisID niet voor problemen zorgt.
        <br>Code formatting aangepast.
    </td>
  </tr>
  <tr>
    <td valign="top">3.0.1</td>
    <td>"One Fits All" Release
        <br>Instelling DSMR protocol nu via settings
        <br>Belgische Slimme Meters werken nu zonder aanpassingen (dsmr2Lib)
        <br>Selectie MBus waar de Slimme Meter op is aangesloten via settings
        <br>SPIFFS is vervangen door LittleFS
    </td>
  </tr>
  <tr>
    <td valign="top">2.0.1</td>
    <td>First Final Release
      <br>Implementing DSMRloggerWS actual api (for backwards compatibility)
      <br>More robust way to write hourly data to the RING-files
      <br>Bugfix PRE40 gasmeter reading
      <br>Remove validity check on meterstanden editor
      <br>Better FieldName translation
      <br>Bugfix mindergas processing
    </td>
  </tr>
  <tr>
    <td valign="top">1.2.1</td>
    <td>Third Official Release
      <br>Instelling SM_HAS_NO_FASE_INFO nu via settings
      <br>Selectie OLED scherm via settings
      <br>Mogelijkheid om het oled scherm 180* te flippen via settings
      <br>Check op volgordelijkheid Uren (in de GUI)
      <br>macaddress in /api/v1/dev/info (Phyxion)
      <br>Bailout some functions on low heap
      <br>Simplification and better tab restAPIs
      <br>Editer Maanden tabel verbetert  (maar nog steeds lastig te gebruiken)
      <br>Betere test of er op github nieuwe firmware beschikbaar is
      <br>bugfix prevent GUI firering multiple restAPI call's
      <br>The Leading Edge type GUI now completely from github. Set
          'index page' to "DSMRindexEDGE.html" to make use of
          the latest development (but be aware, there might be bugs)
      </td>
   </tr>
   <tr>
     <td valign="top">1.1.0</td>
     <td>Second Official Release
       <br>Introduction ESP_SysLogger
       <br>GUI is now even more a Single Page Application (SPA)
       <br>Better coping with WiFi losses (continue operation and try to reconnect)
       <br>Restructure slimmemeter.loop() etc.
       <br>You can find pré compiled binaries at <b>code</b> -> <b><i>releases</i></b>
     </td>
   </tr>
   <tr>
     <td valign="top">1.0.1</td>
     <td>First official release
       <br>You can find pré compiled binaries at <b>code</b> -> <b><i>releases</i></b>
     </td>
   </tr>
   <tr>
     <td valign="top">0.3.5 RC3</td>
     <td>Third Release Candidate
     </td>
   </tr>
</table>
