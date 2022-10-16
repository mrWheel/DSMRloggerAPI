/*
***************************************************************************  
**  Program  : DSMRindex.js, part of DSMRfirmwareAPI
**  Version  : v3.0.4
**
**  Copyright (c) 2021, 2022, 2023 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
  const APIGW=window.location.protocol+'//'+window.location.host+'/api/';

  "use strict";

  let needBootsTrapMain     = true;
  let needBootsTrapSettings = true;
  let activePage            = "mainPage";
  let activeTab             = "none";
  let presentationType      = "TAB";
  let tabTimer              = 0;
  let actualTimer           = 0;
  let timeTimer             = 0;
  var GitHubVersion         = 0;
  var GitHubVersion_dspl    = "-";
  var firmwareVersion       = 0;
  var firmwareVersion_dspl  = "-";
  var newVersionMsg         = "";
  
  var tlgrmInterval         = 10;
  var ed_tariff1            = 0;
  var ed_tariff2            = 0;
  var er_tariff1            = 0;
  var er_tariff2            = 0;
  var gd_tariff             = 0;
  var electr_netw_costs     = 0;
  var gas_netw_costs        = 0;
  var hostName              =  "-";
  var pre_dsmr40            = 0;
  var dailyreboot           = 0;
  var mbus_nr_gas           = 1;
  
  var data       = [];
                  
  let monthType        = "ED";
  let settingBgColor   = 'deepskyblue';
  let settingFontColor = 'white'
                    
  var monthNames = [ "indxNul","Januari","Februari","Maart","April","Mei","Juni"
                    ,"Juli","Augustus","September","Oktober","November","December"
                    ,"\0"
                   ];
  
  window.onload=bootsTrapMain;
  /*
  window.onfocus = function() {
    if (needBootsTrapMain) {
      window.location.reload(true);
    }
  };
  */
    
  //============================================================================  
  function bootsTrapMain() {
    console.log("bootsTrapMain()");
    needBootsTrapMain = false;
    
    document.getElementById('bActualTab').addEventListener('click',function()
                                                {openTab('ActualTab');});
    document.getElementById('bHoursTab').addEventListener('click',function() 
                                                {openTab('HoursTab');});
    document.getElementById('bDaysTab').addEventListener('click',function() 
                                                {openTab('DaysTab');});
    document.getElementById('bMonthsTab').addEventListener('click',function() 
                                                {openTab('MonthsTab');});
    document.getElementById('bFieldsTab').addEventListener('click',function() 
                                                {openTab('FieldsTab');});
    document.getElementById('bTelegramTab').addEventListener('click',function() 
                                                {openTab('TelegramTab');});
    document.getElementById('bSysInfoTab').addEventListener('click',function() 
                                                {openTab('SysInfoTab');});
    document.getElementById('bAPIdocTab').addEventListener('click',function() 
                                                {openTab('APIdocTab');});
    document.getElementById('FSmanager').addEventListener('click',function() 
                                                { console.log("newTab: goFSmanager");
                                                  location.href = "/FSmanager.html";
                                                });
    document.getElementById('Settings').addEventListener('click',function() 
                                                {openPage('settingsPage');});
    
    document.getElementById('mCOST').checked = false;
    setMonthTableType();
    refreshDevTime();
    getDevSettings();
    refreshDevInfo();
    
    clearInterval(timeTimer);  
    timeTimer = setInterval(refreshDevTime, 10 * 1000); // repeat every 10s

    openPage("mainPage");
    initActualGraph();
    openTab("ActualTab");
    setPresentationType('TAB');
    readGitHubVersion();
    
    console.log("..exit bootsTrapMain()!");
      
  } // bootsTrapMain()
  
    
  function bootsTrapSettings() {
    console.log("bootsTrapSettings()");
    needBootsTrapSettings = false;
    
    document.getElementById('bTerug').addEventListener('click',function()
                                                {openPage('mainPage');});
    document.getElementById('bEditMonths').addEventListener('click',function()
                                                {openTab('tabEditMonths');});
    document.getElementById('bEditSettings').addEventListener('click',function()
                                                {openTab('tabEditSettings');});
    document.getElementById('bUndo').addEventListener('click',function() 
                                                {undoReload();});
    document.getElementById('bSave').addEventListener('click',function() 
                                                {saveData();});
    refreshDevTime();
    refreshDevInfo();
    
    openPage("settingsPage");

    //openTab("tabEditSettings");
    
    //---- update buttons in navigation bar ---
    let x = document.getElementsByClassName("editButton");
    for (var i = 0; i < x.length; i++) {
      x[i].style.background     = settingBgColor;
      x[i].style.border         = 'none';
      x[i].style.textDecoration = 'none';  
      x[i].style.outline        = 'none';  
      x[i].style.boxShadow      = 'none';
    }

  } // bootsTrapSettings()
  

  //============================================================================  
  function openTab(tabName) {
        
    activeTab = tabName;

    clearInterval(tabTimer);  
    clearInterval(actualTimer);  
    
    let bID = "b" + tabName;
    let i;
    //---- update buttons in navigation bar ---
    let x = document.getElementsByClassName("tabButton");
    for (i = 0; i < x.length; i++) {
      x[i].style.background     = 'deepskyblue';
      x[i].style.border         = 'none';
      x[i].style.textDecoration = 'none';  
      x[i].style.outline        = 'none';  
      x[i].style.boxShadow      = 'none';
    }
    //--- hide canvas -------
    document.getElementById("dataChart").style.display = "none";
    document.getElementById("gasChart").style.display  = "none";
    //--- hide all tab's -------
    x = document.getElementsByClassName("tabName");
    for (i = 0; i < x.length; i++) {
      x[i].style.display    = "none";  
    }
    //--- and set active tab to 'block'
    console.log("now set ["+bID+"] to block ..");
    //document.getElementById(bID).style.background='lightgray';
    document.getElementById(tabName).style.display = "block";  
    if (tabName != "ActualTab") {
      clearInterval(actualTimer);
      actualTimer = setInterval(refreshSmActual, 60 * 1000);                  // repeat every 60s
    }
    
    if (tabName == "ActualTab") {
      console.log("newTab: ActualTab");
      refreshSmActual();
      clearInterval(actualTimer);
      if (tlgrmInterval < 10)
            actualTimer = setInterval(refreshSmActual, 10 * 1000);            // repeat every 10s
      else  actualTimer = setInterval(refreshSmActual, tlgrmInterval * 1000); // repeat every tlgrmInterval seconds

    } else if (tabName == "HoursTab") {
      console.log("newTab: HoursTab");
      refreshHours();
      clearInterval(tabTimer);
      tabTimer = setInterval(refreshHours, 58 * 1000); // repeat every 58s

    } else if (tabName == "DaysTab") {
      console.log("newTab: DaysTab");
      refreshDays();
      clearInterval(tabTimer);
      tabTimer = setInterval(refreshDays, 58 * 1000); // repeat every 58s

    } else if (tabName == "MonthsTab") {
      console.log("newTab: MonthsTab");
      refreshMonths();
      clearInterval(tabTimer);
      tabTimer = setInterval(refreshMonths, 118 * 1000); // repeat every 118s
    
    } else if (tabName == "SysInfoTab") {
      console.log("newTab: SysInfoTab");
      refreshDevInfo();
      clearInterval(tabTimer);
      tabTimer = setInterval(refreshDevInfo, 58 * 1000); // repeat every 58s

    } else if (tabName == "FieldsTab") {
      console.log("newTab: FieldsTab");
      refreshSmFields();
      clearInterval(tabTimer);
      tabTimer = setInterval(refreshSmFields, 58 * 1000); // repeat every 58s

    } else if (tabName == "TelegramTab") {
      console.log("newTab: TelegramTab");
      refreshSmTelegram();
      clearInterval(tabTimer); // do not repeat!

    } else if (tabName == "APIdocTab") {
      console.log("newTab: APIdocTab");
      showAPIdoc();
      
    } else if (tabName == "tabEditMonths") {
      console.log("newTab: tabEditMonths");
      document.getElementById('tabMaanden').style.display = 'block';
      getMonths();

    } else if (tabName == "tabEditSettings") {
      console.log("newTab: tabEditSettings");
      document.getElementById('tabEditSettings').style.display = 'block';
      refreshSettings();
    
    }

  } // openTab()
  
  
  //============================================================================  
  function openPage(pageName) {
        
    console.log("openPage("+pageName+")");
    activePage = pageName;
    if (pageName == "mainPage") {
      document.getElementById("settingsPage").style.display = "none";
      data = {};
      needBootsTrapSettings = true;
      openTab("ActualTab");
      if (needBootsTrapMain)       bootsTrapMain();
    }
    else if (pageName == "settingsPage") {
      document.getElementById("mainPage").style.display = "none";  
      data = {};
      needBootsTrapMain = true;
      openTab('tabEditSettings');
      if (needBootsTrapSettings)   bootsTrapSettings();
    }
    document.getElementById(pageName).style.display = "block";  

  } // openPage()
    
  
  //============================================================================  
  function refreshDevInfo()
  {
    fetch(APIGW+"v1/dev/info")
      .then(response => response.json())
      .then(json => {
        //console.log("parsed .., data is ["+ JSON.stringify(json)+"]");
        data = json.devinfo;
        for( let i in data )
        {
            var tableRef = document.getElementById('devInfoTable').getElementsByTagName('tbody')[0];
            //data[i].humanName = translateToHuman(data[i].name);
            bName = translateToHuman(data[i].name);
            data[i].humanName = bName.replace("<2>", "<br>");

            if( ( document.getElementById("devInfoTable_"+data[i].name)) == null )
            {
              //console.log("data["+i+"] => name["+data[i].name+"]");
              var newRow   = tableRef.insertRow();
              newRow.setAttribute("id", "devInfoTable_"+data[i].name, 0);
              // Insert a cell in the row at index 0
              var newCell  = newRow.insertCell(0);
              var newText  = document.createTextNode('');
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(1);
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(2);
              newCell.appendChild(newText);
            }
            tableCells = document.getElementById("devInfoTable_"+data[i].name).cells;
            //tableCells[0].innerHTML = data[i].name;
            tableCells[0].innerHTML = data[i].humanName;
            tableCells[1].innerHTML = data[i].value;
            if (data[i].hasOwnProperty('unit'))
            {
              tableCells[1].style.textAlign = "right";
              tableCells[2].innerHTML = data[i].unit;
            }
            
            if (data[i].name == "fwversion")
            {
              document.getElementById('devVersion').innerHTML = json.devinfo[i].value;
              var tmpFW = json.devinfo[i].value;
              firmwareVersion_dspl = tmpFW;
              tmpX = tmpFW.substring(1, tmpFW.indexOf(' '));
              tmpN = tmpX.split(".");
              firmwareVersion = tmpN[0]*10000 + tmpN[1]*1;
              console.log("firmwareVersion["+firmwareVersion+"] >= GitHubVersion["+GitHubVersion+"]");
              if (GitHubVersion == 0 || firmwareVersion >= GitHubVersion)
                    newVersionMsg = "";
              else  newVersionMsg = firmwareVersion_dspl + " nieuwere versie ("+GitHubVersion_dspl+") beschikbaar";
              document.getElementById('message').innerHTML = newVersionMsg;
              console.log(newVersionMsg);

            } else if (data[i].name == 'hostname')
            {
              document.getElementById('devName').innerHTML = data[i].value;
            } else if (data[i].name == 'tlgrm_interval')
            {
              tlgrmInterval = data[i].value;
            } else if (data[i].name == "compileoptions" && data[i].value.length > 50) 
            {
              tableCells[1].innerHTML = data[i].value.substring(0,50);
              var lLine = data[i].value.substring(50);
              while (lLine.length > 50)
              {
                tableCells[1].innerHTML += "<br>" + lLine.substring(0,50);
                lLine = lLine.substring(50);
              }
              tableCells[1].innerHTML += "<br>" + lLine;
              tableCells[0].setAttribute("style", "vertical-align: top");
            }

          }
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });     
  } // refreshDevInfo()

  
  //============================================================================  
  function refreshDevTime()
  {
    //console.log("Refresh api/v1/dev/time ..");
    fetch(APIGW+"v1/dev/time")
      .then(response => response.json())
      .then(json => {
        //console.log("parsed .., data is ["+ JSON.stringify(json)+"]");
        for( let i in json.devtime ){
            if (json.devtime[i].name == "time")
            {
              //console.log("Got new time ["+json.devtime[i].value+"]");
              document.getElementById('theTime').innerHTML = json.devtime[i].value;
            }
          }
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });     
      
    document.getElementById('message').innerHTML = newVersionMsg;

  } // refreshDevTime()
  
  
  //============================================================================  
  function refreshSmActual()
  {
    fetch(APIGW+"v1/sm/actual")
      .then(response => response.json())
      .then(json => {
          console.log("parsed .., fields is ["+ JSON.stringify(json)+"]");
          data = json.actual;
          copyActualToChart(data);
          if (presentationType == "TAB")
                showActualTable(data);
          else  showActualGraph(data);
          //console.log("-->done..");
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      }); 
  };  // refreshSmActual()
  
  
  //============================================================================  
  function refreshSmFields()
  {
    fetch(APIGW+"v1/sm/fields")
      .then(response => response.json())
      .then(json => {
          //console.log("parsed .., fields is ["+ JSON.stringify(json)+"]");
          data = json.fields;
          for (var i in data) 
          {
            //data[i].humanName = translateToHuman(data[i].name);
            bName = translateToHuman(data[i].name);
            data[i].humanName = bName.replace("<2>", "<br>");

            var tableRef = document.getElementById('fieldsTable').getElementsByTagName('tbody')[0];
            if( ( document.getElementById("fieldsTable_"+data[i].name)) == null )
            {
              var newRow   = tableRef.insertRow();
              newRow.setAttribute("id", "fieldsTable_"+data[i].name, 0);
              // Insert a cell in the row at index 0
              var newCell  = newRow.insertCell(0);                  // name
              var newText  = document.createTextNode('');
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(1);                      // humanName
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(2);                      // value
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(3);                      // unit
              newCell.appendChild(newText);
            }
            tableCells = document.getElementById("fieldsTable_"+data[i].name).cells;
            tableCells[0].innerHTML = data[i].name;
            tableCells[1].innerHTML = data[i].humanName;
            if (data[i].name == "electricity_failure_log" && data[i].value.length > 50) 
            {
              tableCells[2].innerHTML = data[i].value.substring(0,50);
              var lLine = data[i].value.substring(50);
              while (lLine.length > 50)
              {
                tableCells[2].innerHTML += "<br>" + lLine.substring(0,50);
                lLine = lLine.substring(50);
              }
              tableCells[2].innerHTML += "<br>" + lLine;
              tableCells[0].setAttribute("style", "vertical-align: top");
              tableCells[1].setAttribute("style", "vertical-align: top");
            }
            else
            {
              tableCells[2].innerHTML = data[i].value;
            }
            if (data[i].hasOwnProperty('unit'))
            {
              tableCells[2].style.textAlign = "right";              // value
              tableCells[3].style.textAlign = "center";             // unit
              tableCells[3].innerHTML = data[i].unit;
            }
          }
          //console.log("-->done..");
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      }); 
  };  // refreshSmFields()
  
  
  //============================================================================  
  function refreshHours()
  {
    console.log("fetch("+APIGW+"v1/hist/hours/asc)");
    fetch(APIGW+"v1/hist/hours/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
        //console.log(json);
        data = json.hours;
        expandData(data);
        if (presentationType == "TAB")
              showHistTable(data, "Hours");
        else  showHistGraph(data, "Hours");
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      }); 
  } // resfreshHours()
  
  
  //============================================================================  
  function refreshDays()
  {
    console.log("fetch("+APIGW+"v1/hist/days/asc)");
    fetch(APIGW+"v1/hist/days/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
        data = json.days;
        expandData(data);
        if (presentationType == "TAB")
              showHistTable(data, "Days");
        else  showHistGraph(data, "Days");
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });
  } // resfreshDays()
  
  
  //============================================================================  
  function refreshMonths()
  {
    console.log("fetch("+APIGW+"v1/hist/months/asc)");
    fetch(APIGW+"v1/hist/months/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
        //console.log(response);
        data = json.months;
        expandData(data);
        if (presentationType == "TAB")
        {
          if (document.getElementById('mCOST').checked)
                showMonthsCosts(data);
          else  showMonthsHist(data);
        }
        else  showMonthsGraph(data);
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });
  } // resfreshMonths()

    
  //============================================================================  
  function refreshSmTelegram()
  {
    fetch(APIGW+"v1/sm/telegram")
      .then(response => response.text())
      .then(response => {
        //console.log("parsed .., data is ["+ response+"]");
        //console.log('-------------------');
        var divT = document.getElementById('rawTelegram');
        if ( document.getElementById("TelData") == null )
        {
            console.log("CreateElement(pre)..");
            var preT = document.createElement('pre');
            preT.setAttribute("id", "TelData", 0);
            preT.setAttribute('class', 'telegram');
            preT.textContent = response;
            divT.appendChild(preT);
        }
        preT = document.getElementById("TelData");
        preT.textContent = response;
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });     
  } // refreshSmTelegram()

    
  //============================================================================  
  function expandData(data)
  {
    //--- first check op volgordelijkheid ------    
    if (activeTab == "HoursTab") {  
    for (let i=0; i<(data.length -1); i++)
    {
      if (data[i].edt1 < data[i+1].edt1 || data[i].edt2 < data[i+1].edt2)
      {
        console.log("["+(i)+"] ["+data[i].recid+"] := ["+(i+1)+"]["+data[i+1].recid+"]"); 
        data[i].edt1 = data[i+1].edt1 * 1.0;
        data[i].edt2 = data[i+1].edt2 * 1.0;
        data[i].ert1 = data[i+1].ert1 * 1.0;
        data[i].ert2 = data[i+1].ert2 * 1.0;
        data[i].gdt  = data[i+1].gdt  * 1.0;
      }
    } // for ...
    }
    for (let i=0; i<data.length; i++)
    {
      var     costs     = 0;
      data[i].p_ed      = {};
      data[i].p_edw     = {};
      data[i].p_er      = {};
      data[i].p_erw     = {};
      data[i].p_gd      = {};
      data[i].costs_e   = {};
      data[i].costs_g   = {};
      data[i].costs_nw  = {};
      data[i].costs_tt  = {};

      if (i < (data.length -1))
      {
        data[i].p_ed  = ((data[i].edt1 +data[i].edt2)-(data[i+1].edt1 +data[i+1].edt2)).toFixed(3);
        data[i].p_edw = (data[i].p_ed * 1000).toFixed(0);
        data[i].p_er  = ((data[i].ert1 +data[i].ert2)-(data[i+1].ert1 +data[i+1].ert2)).toFixed(3);
        data[i].p_erw = (data[i].p_er * 1000).toFixed(0);
        data[i].p_gd  = (data[i].gdt  -data[i+1].gdt).toFixed(3);
        //-- calculate Energy Delivered costs
        costs = ( (data[i].edt1 - data[i+1].edt1) * ed_tariff1 );
        costs = costs + ( (data[i].edt2 - data[i+1].edt2) * ed_tariff2 );
        //-- subtract Energy Returned costs
        costs = costs - ( (data[i].ert1 - data[i+1].ert1) * er_tariff1 );
        costs = costs - ( (data[i].ert2 - data[i+1].ert2) * er_tariff2 );
        data[i].costs_e = costs;
        //-- add Gas Delivered costs
        data[i].costs_g = ( (data[i].gdt  - data[i+1].gdt)  * gd_tariff );
        //-- compute network costs
        data[i].costs_nw = (electr_netw_costs + gas_netw_costs);
        //-- compute total costs
        data[i].costs_tt = ( (data[i].costs_e + data[i].costs_g + data[i].costs_nw) * 1.0);
      }
      else
      {
        costs             = 0;
        data[i].p_ed      = (data[i].edt1 +data[i].edt2).toFixed(3);
        data[i].p_edw     = (data[i].p_ed * 1000).toFixed(0);
        data[i].p_er      = (data[i].ert1 +data[i].ert2).toFixed(3);
        data[i].p_erw     = (data[i].p_er * 1000).toFixed(0);
        data[i].p_gd      = (data[i].gdt).toFixed(3);
        data[i].costs_e   = 0.0;
        data[i].costs_g   = 0.0;
        data[i].costs_nw  = 0.0;
        data[i].costs_tt  = 0.0;
      }
    } // for i ..
    //console.log("leaving expandData() ..");

  } // expandData()

    
  //============================================================================  
  function showActualTable(data)
  { 
    if (activeTab != "ActualTab") return;

    console.log("showActual()");

    for (var i in data) 
    {
      data[i].humanName = translateToHuman(data[i].name);
      var tableRef = document.getElementById('actualTable').getElementsByTagName('tbody')[0];
      if( ( document.getElementById("actualTable_"+data[i].name)) == null )
      {
        var newRow   = tableRef.insertRow();
        newRow.setAttribute("id", "actualTable_"+data[i].name, 0);
        // Insert a cell in the row at index 0
        var newCell  = newRow.insertCell(0);            // (short)name
        var newText  = document.createTextNode('');
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(1);                // value
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(2);                // unit
        newCell.appendChild(newText);
      }
      tableCells = document.getElementById("actualTable_"+data[i].name).cells;
      tableCells[0].innerHTML = data[i].humanName;
      tableCells[1].innerHTML = data[i].value;
      if (data[i].hasOwnProperty('unit'))
      {
        tableCells[1].style.textAlign = "right";        // value
        tableCells[2].style.textAlign = "center";       // unit
        tableCells[2].innerHTML = data[i].unit;
      }
    }

    //--- hide canvas
    document.getElementById("dataChart").style.display = "none";
    document.getElementById("gasChart").style.display  = "none";
    //--- show table
    document.getElementById("actual").style.display    = "block";

  } // showActualTable()
  
    
  //============================================================================  
  function showHistTable(data, type)
  { 
    console.log("showHistTable("+type+")");
    // the last element has the metervalue, so skip it
    for (let i=0; i<(data.length -1); i++)
    {
      //console.log("showHistTable("+type+"): data["+i+"] => data["+i+"]name["+data[i].recid+"]");

      var tableRef = document.getElementById('last'+type+'Table').getElementsByTagName('tbody')[0];
      if( ( document.getElementById(type +"Table_"+type+"_R"+i)) == null )
      {
        var newRow   = tableRef.insertRow();
        //newRow.setAttribute("id", type+"Table_"+data[i].recid, 0);
        newRow.setAttribute("id", type+"Table_"+type+"_R"+i, 0);
        // Insert a cell in the row at index 0
        var newCell  = newRow.insertCell(0);
        var newText  = document.createTextNode('-');
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(1);
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(2);
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(3);
        newCell.appendChild(newText);
        if (type == "Days")
        {
          newCell  = newRow.insertCell(4);
          newCell.appendChild(newText);
        }
      }

      tableCells = document.getElementById(type+"Table_"+type+"_R"+i).cells;
      tableCells[0].style.textAlign = "right";
      tableCells[0].innerHTML = formatDate(type, data[i].recid);
      tableCells[1].style.textAlign = "right";
      if (data[i].p_edw >= 0)
            tableCells[1].innerHTML = data[i].p_edw;
      else  tableCells[1].innerHTML = "-";
      tableCells[2].style.textAlign = "right";
      if (data[i].p_erw >= 0)
            tableCells[2].innerHTML = data[i].p_erw;
      else  tableCells[2].innerHTML = "-";
      tableCells[3].style.textAlign = "right";
      if (data[i].p_gd >= 0)
            tableCells[3].innerHTML = data[i].p_gd;
      else  tableCells[3].innerHTML = "-";
      if (type == "Days")
      {
        tableCells[4].style.textAlign = "right";
        tableCells[4].innerHTML = ( (data[i].costs_e + data[i].costs_g) * 1.0).toFixed(2);
      }
    };

    //--- hide canvas
    document.getElementById("dataChart").style.display = "none";
    document.getElementById("gasChart").style.display  = "none";
    //--- show table
    document.getElementById("lastHours").style.display = "block";
    document.getElementById("lastDays").style.display  = "block";

  } // showHistTable()

    
  //============================================================================  
  function showMonthsHist(data)
  { 
    console.log("now in showMonthsHist() ..");
    var showRows = 0;
    if (data.length > 24) showRows = 12;
    else                  showRows = data.length / 2;
    //console.log("showRows is ["+showRows+"]");
    for (let i=0; i<showRows; i++)
    {
      //console.log("showMonthsHist(): data["+i+"] => data["+i+"].name["+data[i].recid+"]");
      var tableRef = document.getElementById('lastMonthsTable').getElementsByTagName('tbody')[0];
      if( ( document.getElementById("lastMonthsTable_R"+i)) == null )
      {
        var newRow   = tableRef.insertRow();
        newRow.setAttribute("id", "lastMonthsTable_R"+i, 0);
        // Insert a cell in the row at index 0
        var newCell  = newRow.insertCell(0);          // maand
        var newText  = document.createTextNode('-');
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(1);              // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(2);              // verbruik
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(3);              // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(4);              // verbruik
        newCell.appendChild(newText);

        newCell  = newRow.insertCell(5);              // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(6);              // opgewekt
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(7);              // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(8);             // opgewekt
        newCell.appendChild(newText);
        
        newCell  = newRow.insertCell(9);             // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(10);             // gas
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(11);             // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(12);             // gas
        newCell.appendChild(newText);
      }
      var mmNr = parseInt(data[i].recid.substring(2,4), 10);

      tableCells = document.getElementById("lastMonthsTable_R"+i).cells;
      tableCells[0].style.textAlign = "right";
      tableCells[0].innerHTML = monthNames[mmNr];                           // maand
      
      tableCells[1].style.textAlign = "center";
      tableCells[1].innerHTML = "20"+data[i].recid.substring(0,2);          // jaar
      tableCells[2].style.textAlign = "right";
      if (data[i].p_ed >= 0)
            tableCells[2].innerHTML = data[i].p_ed;                         // verbruik
      else  tableCells[2].innerHTML = "-";     
      tableCells[3].style.textAlign = "center";
      tableCells[3].innerHTML = "20"+data[i+12].recid.substring(0,2);       // jaar
      tableCells[4].style.textAlign = "right";
      if (data[i+12].p_ed >= 0)
            tableCells[4].innerHTML = data[i+12].p_ed;                      // verbruik
      else  tableCells[4].innerHTML = "-";     

      tableCells[5].style.textAlign = "center";
      tableCells[5].innerHTML = "20"+data[i].recid.substring(0,2);          // jaar
      tableCells[6].style.textAlign = "right";
      if (data[i].p_er >= 0)
            tableCells[6].innerHTML = data[i].p_er;                         // opgewekt
      else  tableCells[6].innerHTML = "-";     
      tableCells[7].style.textAlign = "center";
      tableCells[7].innerHTML = "20"+data[i+12].recid.substring(0,2);       // jaar
      tableCells[8].style.textAlign = "right";
      if (data[i+12].p_er >= 0)
            tableCells[8].innerHTML = data[i+12].p_er;                      // opgewekt
      else  tableCells[8].innerHTML = "-";     

      tableCells[9].style.textAlign = "center";
      tableCells[9].innerHTML = "20"+data[i].recid.substring(0,2);          // jaar
      tableCells[10].style.textAlign = "right";
      if (data[i].p_gd >= 0)
            tableCells[10].innerHTML = data[i].p_gd;                        // gas
      else  tableCells[10].innerHTML = "-";     
      tableCells[11].style.textAlign = "center";
      tableCells[11].innerHTML = "20"+data[i+12].recid.substring(0,2);      // jaar
      tableCells[12].style.textAlign = "right";
      if (data[i+12].p_gd >= 0)
            tableCells[12].innerHTML = data[i+12].p_gd;                     // gas
      else  tableCells[12].innerHTML = "-";     

    };
    
    //--- hide canvas
    document.getElementById("dataChart").style.display  = "none";
    document.getElementById("gasChart").style.display   = "none";
    //--- show table
    document.getElementById("lastMonths").style.display = "block";

  } // showMonthsHist()

    
  //============================================================================  
  function showMonthsCosts(data)
  { 
    console.log("now in showMonthsCosts() ..");
    var totalCost   = 0;
    var totalCost_1 = 0;
    var showRows    = 0;
    if (data.length > 24) showRows = 12;
    else                  showRows = data.length / 2;
    //console.log("showRows is ["+showRows+"]");
    for (let i=0; i<showRows; i++)
    {
      //console.log("showMonthsHist(): data["+i+"] => data["+i+"].name["+data[i].recid+"]");
      var tableRef = document.getElementById('lastMonthsTableCosts').getElementsByTagName('tbody')[0];
      if( ( document.getElementById("lastMonthsTableCosts_R"+i)) == null )
      {
        var newRow   = tableRef.insertRow();
        newRow.setAttribute("id", "lastMonthsTableCosts_R"+i, 0);
        // Insert a cell in the row at index 0
        var newCell  = newRow.insertCell(0);          // maand
        var newText  = document.createTextNode('-');
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(1);              // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(2);              // kosten electra
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(3);              // kosten gas
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(4);              // vast recht
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(5);              // kosten totaal
        newCell.appendChild(newText);

        newCell  = newRow.insertCell(6);              // jaar
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(7);              // kosten electra
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(8);              // kosten gas
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(9);              // vast recht
        newCell.appendChild(newText);
        newCell  = newRow.insertCell(10);              // kosten totaal
        newCell.appendChild(newText);
      }
      var mmNr = parseInt(data[i].recid.substring(2,4), 10);

      tableCells = document.getElementById("lastMonthsTableCosts_R"+i).cells;
      tableCells[0].style.textAlign = "right";
      tableCells[0].innerHTML = monthNames[mmNr];                           // maand
      
      tableCells[1].style.textAlign = "center";
      tableCells[1].innerHTML = "20"+data[i].recid.substring(0,2);          // jaar
      tableCells[2].style.textAlign = "right";
      tableCells[2].innerHTML = (data[i].costs_e *1).toFixed(2);            // kosten electra
      tableCells[3].style.textAlign = "right";
      tableCells[3].innerHTML = (data[i].costs_g *1).toFixed(2);            // kosten gas
      tableCells[4].style.textAlign = "right";
      tableCells[4].innerHTML = (data[i].costs_nw *1).toFixed(2);           // netw kosten
      tableCells[5].style.textAlign = "right";
      tableCells[5].style.fontWeight = 'bold';
      tableCells[5].innerHTML = "€ " + (data[i].costs_tt *1).toFixed(2);    // kosten totaal
      //--- omdat de actuele maand net begonnen kan zijn tellen we deze
      //--- niet mee, maar tellen we de laatste maand van de voorgaand periode
      if (i > 0)
            totalCost += data[i].costs_tt;
      else  totalCost += data[i+12].costs_tt;

      tableCells[6].style.textAlign = "center";
      tableCells[6].innerHTML = "20"+data[i+12].recid.substring(0,2);         // jaar
      tableCells[7].style.textAlign = "right";
      tableCells[7].innerHTML = (data[i+12].costs_e *1).toFixed(2);           // kosten electra
      tableCells[8].style.textAlign = "right";
      tableCells[8].innerHTML = (data[i+12].costs_g *1).toFixed(2);           // kosten gas
      tableCells[9].style.textAlign = "right";
      tableCells[9].innerHTML = (data[i+12].costs_nw *1).toFixed(2);          // netw kosten
      tableCells[10].style.textAlign = "right";
      tableCells[10].style.fontWeight = 'bold';
      tableCells[10].innerHTML = "€ " + (data[i+12].costs_tt *1).toFixed(2);  // kosten totaal
      totalCost_1 += data[i+12].costs_tt;

    };

    if( ( document.getElementById("periodicCosts")) == null )
    {
      var newRow   = tableRef.insertRow();                                // voorschot regel
      newRow.setAttribute("id", "periodicCosts", 0);
      // Insert a cell in the row at index 0
      var newCell  = newRow.insertCell(0);                                // maand
      var newText  = document.createTextNode('-');
      newCell.appendChild(newText);
      newCell  = newRow.insertCell(1);              // description
      newCell.setAttribute("colSpan", "4");
      newCell.appendChild(newText);
      newCell  = newRow.insertCell(2);              // voorschot
      newCell.appendChild(newText);
      newCell  = newRow.insertCell(3);              // description
      newCell.setAttribute("colSpan", "4");
      newCell.appendChild(newText);
      newCell  = newRow.insertCell(4);              // voorschot
      newCell.appendChild(newText);
    }
    tableCells = document.getElementById("periodicCosts").cells;
    tableCells[1].style.textAlign = "right";
    tableCells[1].innerHTML = "Voorschot Bedrag"
    tableCells[2].style.textAlign = "right";
    tableCells[2].innerHTML = "€ " + (totalCost / 12).toFixed(2);
    tableCells[3].style.textAlign = "right";
    tableCells[3].innerHTML = "Voorschot Bedrag"
    tableCells[4].style.textAlign = "right";
    tableCells[4].innerHTML = "€ " + (totalCost_1 / 12).toFixed(2);

    
    //--- hide canvas
    document.getElementById("dataChart").style.display  = "none";
    document.getElementById("gasChart").style.display   = "none";
    //--- show table
    if (document.getElementById('mCOST').checked)
    {
      document.getElementById("lastMonthsTableCosts").style.display = "block";
      document.getElementById("lastMonthsTable").style.display = "none";
    }
    else
    {
      document.getElementById("lastMonthsTable").style.display = "block";
      document.getElementById("lastMonthsTableCosts").style.display = "none";
    }
    document.getElementById("lastMonths").style.display = "block";

  } // showMonthsCosts()

  
  //============================================================================  
  function getDevSettings()
  {
    fetch(APIGW+"v1/dev/settings")
      .then(response => response.json())
      .then(json => {
        //console.log("parsed .., data is ["+ JSON.stringify(json)+"]");
        for( let i in json.settings ){
            if (json.settings[i].name == "ed_tariff1")
            {
              ed_tariff1 = json.settings[i].value;
            }
            else if (json.settings[i].name == "ed_tariff2")
            {
              ed_tariff2 = json.settings[i].value;
            }
            else if (json.settings[i].name == "er_tariff1")
            {
              er_tariff1 = json.settings[i].value;
            }
            else if (json.settings[i].name == "er_tariff2")
            {
              er_tariff2 = json.settings[i].value;
            }
            else if (json.settings[i].name == "gd_tariff")
            {
              gd_tariff = json.settings[i].value;
            }
            else if (json.settings[i].name == "electr_netw_costs")
            {
              electr_netw_costs = json.settings[i].value;
            }
            else if (json.settings[i].name == "gas_netw_costs")
            {
              gas_netw_costs = json.settings[i].value;
            }
            else if (json.settings[i].name == "mbus_nr_gas")
            {
              mbus_nr_gas = json.settings[i].value;
            }
            else if (json.settings[i].name == "pre_dsmr40")
            {
              pre_dsmr40 = json.settings[i].value;
            }
            else if (json.settings[i].name == "dailyreboot")
            {
              dailyreboot = json.settings[i].value;
            }
            else if (json.settings[i].name == "hostname")
            {
              hostName = json.settings[i].value;
            }
          }
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });     
  } // getDevSettings()
  
    
  //============================================================================  
  function setPresentationType(pType) {
    if (pType == "GRAPH") {
      console.log("Set presentationType to GRAPHICS mode!");
      presentationType = pType;
      document.getElementById('aGRAPH').checked = true;
      document.getElementById('aTAB').checked   = false;
      initActualGraph();
      document.getElementById('hGRAPH').checked = true;
      document.getElementById('hTAB').checked   = false;
      document.getElementById('dGRAPH').checked = true;
      document.getElementById('dTAB').checked   = false;
      document.getElementById('mGRAPH').checked = true;
      document.getElementById('mTAB').checked   = false;
      document.getElementById('mCOST').checked  = false;
      document.getElementById("lastMonthsTable").style.display      = "block";
      document.getElementById("lastMonthsTableCosts").style.display = "none";

    } else if (pType == "TAB") {
      console.log("Set presentationType to Tabular mode!");
      presentationType = pType;
      document.getElementById('aTAB').checked   = true;
      document.getElementById('aGRAPH').checked = false;
      document.getElementById('hTAB').checked   = true;
      document.getElementById('hGRAPH').checked = false;
      document.getElementById('dTAB').checked   = true;
      document.getElementById('dGRAPH').checked = false;
      document.getElementById('mTAB').checked   = true;
      document.getElementById('mGRAPH').checked = false;
      document.getElementById('mCOST').checked  = false;

    } else {
      console.log("setPresentationType to ["+pType+"] is quite shitty! Set to TAB");
      presentationType = "TAB";
    }

    document.getElementById("APIdocTab").style.display = "none";

    if (activeTab == "ActualTab")  refreshSmActual();
    if (activeTab == "HoursTab")   refreshHours();
    if (activeTab == "DaysTab")    refreshDays();
    if (activeTab == "MonthsTab")  refreshMonths();

  } // setPresenationType()
  
    
  //============================================================================  
  function setMonthTableType() {
    console.log("Set Month Table Type");
    if (presentationType == 'GRAPH') 
    {
      document.getElementById('mCOST').checked = false;
      return;
    }
    if (document.getElementById('mCOST').checked)
    {
      document.getElementById("lastMonthsTableCosts").style.display = "block";
      document.getElementById("lastMonthsTable").style.display      = "none";
    }
    else
    {
      document.getElementById("lastMonthsTable").style.display      = "block";
      document.getElementById("lastMonthsTableCosts").style.display = "none";
    }
    document.getElementById('lastMonthsTableCosts').getElementsByTagName('tbody').innerHTML = "";
    refreshMonths();
      
  } // setMonthTableType()

    
  //============================================================================  
  function showAPIdoc() {
    console.log("Show API doc ..@["+location.host+"]");
    document.getElementById("APIdocTab").style.display = "block";
    addAPIdoc("v1/dev/info",      "Device info in JSON format", true);
    addAPIdoc("v1/dev/time",      "Device time (epoch) in JSON format", true);
    addAPIdoc("v1/dev/settings",  "Device settings in JSON format", true);
    addAPIdoc("v1/dev/settings{jsonObj}", "[POST] update Device settings in JSON format\
        <br>test with:\
        <pre>curl -X POST -H \"Content-Type: application/json\" --data '{\"name\":\"mqtt_broker\",\"value\":\"hassio.local\"}' \
http://DSMR-API.local/api/v1/dev/settings</pre>", false);
    
    addAPIdoc("v1/sm/info",       "Smart Meter info in JSON format", true);
    addAPIdoc("v1/sm/actual",     "Smart Meter Actual data in JSON format", true);
    addAPIdoc("v0/sm/actual",     "Smart Meter Actual data in JSON format (backwards compatibility)", true);
    addAPIdoc("v1/sm/fields",     "Smart Meter all fields data in JSON format\
        <br>JSON format: {\"fields\":[{\"name\":\"&lt;fieldName&gt;\",\"value\":&lt;value&gt;,\"unit\":\"&lt;unit&gt;\"}]} ", true);
    addAPIdoc("v1/sm/fields/{fieldName}", "Smart Meter one field data in JSON format", false);

    addAPIdoc("v1/sm/telegram",   "raw telegram as send by the Smart Meter including all \"\\r\\n\" line endings", false);

    addAPIdoc("v1/hist/hours",    "History data per hour in JSON format", true);
    addAPIdoc("v1/hist/days",     "History data per day in JSON format", true);
    addAPIdoc("v1/hist/months",   "History data per month in JSON format", true);

  } // showAPIdoc()

    
  //============================================================================  
  function addAPIdoc(restAPI, description, linkURL) {
    if (document.getElementById(restAPI) == null)
    {
      var topDiv = document.getElementById("APIdocTab");
      var br = document.createElement("BR"); 
      br.setAttribute("id", restAPI, 0);
      br.setAttribute("style", "clear: left;");
      
      var div1 = document.createElement("DIV"); 
      div1.setAttribute("class", "div1", 0);
      var aTag = document.createElement('a');
      if (linkURL)
      {
        aTag.setAttribute('href',APIGW +restAPI);
        aTag.target = '_blank';
      }
      else
      {
        aTag.setAttribute('href',"#");
      }
      aTag.innerText = "/api/"+restAPI;
      aTag.style.fontWeight = 'bold';
      div1.appendChild(aTag);

      // <div class='div2'>Device time (epoch) in JSON format</div>
      var div2 = document.createElement("DIV"); 
      div2.setAttribute("class", "div2", 0);
      //var t2 = document.createTextNode(description);                   // Create a text node
      var t2 = document.createElement("p");
      t2.innerHTML = description;                   // Create a text node
      div2.appendChild(t2);     

      topDiv.appendChild(br);    // Append <br> to <div> with id="myDIV"
      topDiv.appendChild(div1);  // Append <div1> to <topDiv> with id="myDIV"
      topDiv.appendChild(div2);  // Append <div2> to <topDiviv> with id="myDIV"
    }
    
    
  } // addAPIdoc()
    
  //============================================================================  
  function refreshSettings()
  {
    console.log("refreshSettings() ..");
    data = {};
    fetch(APIGW+"v1/dev/settings")
      .then(response => response.json())
      .then(json => {
        console.log("then(json => ..)");
        data = json.settings;
        for( let i in data )
        {
          console.log("["+data[i].name+"]=>["+data[i].value+"]");
          var settings = document.getElementById('settings');
          if( ( document.getElementById("settingR_"+data[i].name)) == null )
          {
            var rowDiv = document.createElement("div");
            rowDiv.setAttribute("class", "settingDiv");
            rowDiv.setAttribute("id", "settingR_"+data[i].name);
            rowDiv.setAttribute("style", "text-align: right;");
            rowDiv.style.marginLeft = "10px";
            rowDiv.style.marginRight = "10px";
            rowDiv.style.width = "450px";
            rowDiv.style.border = "thick solid lightblue";
            rowDiv.style.background = "lightblue";
            //--- field Name ---
              var fldDiv = document.createElement("div");
                  fldDiv.setAttribute("style", "margin-right: 10px;");
                  fldDiv.style.width = "250px";
                  fldDiv.style.float = 'left';
                  fldDiv.textContent = translateToHuman(data[i].name);

                  rowDiv.appendChild(fldDiv);
            //--- input ---
              var inputDiv = document.createElement("div");
                  inputDiv.setAttribute("style", "text-align: left;");

                    var sInput = document.createElement("INPUT");
                    sInput.setAttribute("id", "setFld_"+data[i].name);

                    if (data[i].type == "s")
                    {
                      sInput.setAttribute("type", "text");
                      sInput.setAttribute("maxlength", data[i].maxlen);
                    }
                    else if (data[i].type == "f")
                    {
                      sInput.setAttribute("type", "number");
                      sInput.max = data[i].max;
                      sInput.min = data[i].min;
                      sInput.step = (data[i].min + data[i].max) / 1000;
                    }
                    else if (data[i].type == "i")
                    {
                      sInput.setAttribute("type", "number");
                      sInput.max = data[i].max;
                      sInput.min = data[i].min;
                      sInput.step = (data[i].min + data[i].max) / 1000;
                      sInput.step = 1;
                    }
                    sInput.setAttribute("value", data[i].value);
                    sInput.addEventListener('change',
                                function() { setBackGround("setFld_"+data[i].name, "lightgray"); },
                                            false
                                );
                  inputDiv.appendChild(sInput);
                  
            rowDiv.appendChild(inputDiv);
            settings.appendChild(rowDiv);
          }
          else
          {
            document.getElementById("setFld_"+data[i].name).style.background = "white";
            document.getElementById("setFld_"+data[i].name).value = data[i].value;
          }
        }
        //console.log("-->done..");
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });     

      document.getElementById('message').innerHTML = newVersionMsg;

  } // refreshSettings()
  
  
  //============================================================================  
  function getMonths()
  {
    console.log("fetch("+APIGW+"v1/hist/months/asc)");
    fetch(APIGW+"v1/hist/months/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
        //console.log(response);
        data = json.months;
        expandDataSettings(data);
        showMonths(data, monthType);
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });

      document.getElementById('message').innerHTML = newVersionMsg;
      
  } // getMonths()

  
  //============================================================================  
  function showMonths(data, type)
  { 
    console.log("showMonths("+type+")");
    //--- first remove all Children ----
    var allChildren = document.getElementById('editMonths');
    while (allChildren.firstChild) {
      allChildren.removeChild(allChildren.firstChild);
    }
    
    console.log("Now fill the DOM!");    
    for (let i=0; i<data.length; i++)
    {
      //console.log("["+i+"] >>>["+data[i].EEYY+"-"+data[i].MM+"]");
      var em = document.getElementById('editMonths');

      if( ( document.getElementById("em_R"+i)) == null )
      {
        var div1 = document.createElement("div");
            div1.setAttribute("class", "settingDiv");
            div1.setAttribute("id", "em_R"+i);
            div1.style.borderTop = "thick solid lightblue";
            if (i == (data.length -1))  // last row
            {
              div1.style.borderBottom = "thick solid lightblue";
            }
            div1.style.marginLeft = "150px";
            div1.style.marginRight = "400px";
            var span2 = document.createElement("span");
            span2.style.borderTop = "thick solid lightblue";
              //--- create input for EEYY
              var sInput = document.createElement("INPUT");
              sInput.setAttribute("id", "em_YY_"+i);
              sInput.setAttribute("type", "number");
              sInput.setAttribute("min", 2000);
              sInput.setAttribute("max", 2099);
              sInput.size              = 5;
              sInput.style.marginLeft  = '10px';
              sInput.style.marginRight = '20px';
              sInput.addEventListener('change',
                      function() { setNewValue(i, "EEYY", "em_YY_"+i); }, false);
              span2.appendChild(sInput);
              //--- create input for months
              var sInput = document.createElement("INPUT");
              sInput.setAttribute("id", "em_MM_"+i);
              sInput.setAttribute("type", "number");
              sInput.setAttribute("min", 1);
              sInput.setAttribute("max", 12);
              sInput.size              = 3;
              sInput.style.marginRight = '20px';
              sInput.addEventListener('change',
                      function() { setNewValue(i, "MM", "em_MM_"+i); }, false);
              span2.appendChild(sInput);
              //--- create input for data column 1
              sInput = document.createElement("INPUT");
              sInput.setAttribute("id", "em_in1_"+i);
              sInput.setAttribute("type", "number");
              sInput.setAttribute("step", 0.001);
              sInput.style.marginRight = '20px';
              
              if (type == "ED")
              {
                sInput.addEventListener('change',
                    function() { setNewValue(i, "edt1", "em_in1_"+i); }, false );
              }
              else if (type == "ER")
              {
                sInput.addEventListener('change',
                    function() { setNewValue(i, "ert1", "em_in1_"+i); }, false);
              }
              else if (type == "GD")
              {
                sInput.addEventListener('change',
                    function() { setNewValue(i, "gdt", "em_in1_"+i); }, false);
              }
              
              span2.appendChild(sInput);
              //--- if not GD create input for data column 2
              if (type == "ED")
              {
                //console.log("add input for edt2..");
                var sInput = document.createElement("INPUT");
                sInput.setAttribute("id", "em_in2_"+i);
                sInput.setAttribute("type", "number");
                sInput.setAttribute("step", 0.001);
                sInput.style.marginRight = '20px';
                sInput.addEventListener('change',
                      function() { setNewValue(i, "edt2", "em_in2_"+i); }, false);
                span2.appendChild(sInput);
              }
              else if (type == "ER")
              {
                //console.log("add input for ert2..");
                var sInput = document.createElement("INPUT");
                sInput.setAttribute("id", "em_in2_"+i);
                sInput.setAttribute("type", "number");
                sInput.setAttribute("step", 0.001);
                sInput.style.marginRight = '20px';
                sInput.addEventListener('change',
                      function() { setNewValue(i, "ert2", "em_in2_"+i); }, false);
                span2.appendChild(sInput);
              }
              div1.appendChild(span2);
              em.appendChild(div1);
      } // document.getElementById("em_R"+i)) == null 

      //--- year
      document.getElementById("em_YY_"+i).value = data[i].EEYY;
      document.getElementById("em_YY_"+i).style.background = "white";
      //--- month
      document.getElementById("em_MM_"+i).value = data[i].MM;
      document.getElementById("em_MM_"+i).style.background = "white";
      
      if (type == "ED")
      {
        document.getElementById("em_in1_"+i).style.background = "white";
        document.getElementById("em_in1_"+i).value = (data[i].edt1 *1).toFixed(3);
        document.getElementById("em_in2_"+i).style.background = "white";
        document.getElementById("em_in2_"+i).value = (data[i].edt2 *1).toFixed(3);
      }
      else if (type == "ER")
      {
        document.getElementById("em_in1_"+i).style.background = "white";
        document.getElementById("em_in1_"+i).value = (data[i].ert1 *1).toFixed(3);
        document.getElementById("em_in2_"+i).style.background = "white";
        document.getElementById("em_in2_"+i).value = (data[i].ert2 *1).toFixed(3);
      }
      else if (type == "GD")
      {
        document.getElementById("em_in1_"+i).style.background = "white";
        document.getElementById("em_in1_"+i).value = (data[i].gdt *1).toFixed(3);
      }
      
    } // for all elements in data
    
    console.log("Now sequence EEYY/MM values ..(data.length="+data.length+")");
    //--- sequence EEYY and MM data
    var changed = false;
    for (let i=0; i<(data.length -1); i++)
      {
      //--- month
      if (data[i+1].MM == 0)
      {
        data[i+1].MM    = data[i].MM -1;
        changed = true;
        if (data[i+1].MM < 1) {
          data[i+1].MM   = 12;
          if (data[i+1].EEYY == 2000) {
            data[i+1].EEYY = data[i].EEYY -1;
            document.getElementById("em_YY_"+(i+1)).value = data[i+1].EEYY;
            //document.getElementById("em_YY_"+(i+1)).style.background = "lightgray";
          }
        }
        document.getElementById("em_MM_"+(i+1)).value = data[i+1].MM;
        //document.getElementById("em_MM_"+(i+1)).style.background = "lightgray";
      }
      if (data[i+1].EEYY == 2000) {
        data[i+1].EEYY = data[i].EEYY;
        changed = true;
        document.getElementById("em_YY_"+(i+1)).value = data[i+1].EEYY;
        //document.getElementById("em_YY_"+(i+1)).style.background = "lightgray";
      }
      if (changed) sendPostReading((i+1), data);

    } // sequence EEYY and MM

  } // showMonths()

  
  //============================================================================  
  function expandDataSettings(data)
  { 
    for (let i=0; i<data.length; i++)
    {
      data[i].EEYY = {};
      data[i].MM   = {};
      data[i].EEYY = parseInt("20"+data[i].recid.substring(0,2));
      data[i].MM   = parseInt(data[i].recid.substring(2,4));
    }

  } // expandDataSettings()
  
      
  //============================================================================  
  function undoReload()
  {
    if (activeTab == "tabEditMonths") {
      console.log("getMonths");
      getMonths();
    } else if (activeTab == "tabEditSettings") {
      console.log("undoReload(): reload Settings..");
      data = {};
      refreshSettings();

    } else {
      console.log("undoReload(): I don't knwo what to do ..");
    }

  } // undoReload()
  
  
  //============================================================================  
  function saveData() 
  {
    document.getElementById('message').innerHTML = "Gegevens worden opgeslagen ..";

    if (activeTab == "tabEditSettings")
    {
      saveSettings();
    } 
    else if (activeTab == "tabEditMonths")
    {
      saveMeterReadings();
    }
    
  } // saveData()
  
  
  //============================================================================  
  function saveSettings() 
  {
    for(var i in data)
    {
      var fldId  = data[i].name;
      var newVal = document.getElementById("setFld_"+fldId).value;
      if (data[i].value != newVal)
      {
        console.log("save data ["+data[i].name+"] => from["+data[i].value+"] to["+newVal+"]");
        sendPostSetting(fldId, newVal);
      }
    }    
    // delay refresh as all fetch functions are asynchroon!!
    setTimeout(function() {
      refreshSettings();
    }, 1000);
    
  } // saveSettings()
  
  
  //============================================================================  
  function saveMeterReadings() 
  {
    console.log("Saving months-data ..");
    let changes = false;
    
    /** skip this for now **
    if (!validateReadings(monthType))
    {
      return;
    }
    **/
    
    //--- has anything changed?
    for (i in data)
    {
      //console.log("saveMeterReadings["+i+"] ..");
      changes = false;

      if (getBackGround("em_YY_"+i) == "lightgray")
      {
        setBackGround("em_YY_"+i, "white");
        changes = true;
      }
      if (getBackGround("em_MM_"+i) == "lightgray")
      {
        setBackGround("em_MM_"+i, "white");
        changes = true;
      }

      if (document.getElementById("em_in1_"+i).style.background == 'lightgray')
      {
        changes = true;
        document.getElementById("em_in1_"+i).style.background = 'white';
      }
      if (monthType != "GD")
      {
        if (document.getElementById("em_in2_"+i).style.background == 'lightgray')
        {
          changes = true;
          document.getElementById("em_in2_"+i).style.background = 'white';
        }
      }
      if (changes) {
        console.log("Changes where made in ["+i+"]["+data[i].EEYY+"-"+data[i].MM+"]");
        //processWithTimeout([(data.length -1), 0], 2, data, sendPostReading);
        sendPostReading(i, data);
      }
    } 

  } // saveMeterReadings()

    
  //============================================================================  
  function sendPostSetting(field, value) 
  {
    const jsonString = {"name" : field, "value" : value};
    //console.log("send JSON:["+jsonString+"]");
    const other_params = {
        headers : { "content-type" : "application/json; charset=UTF-8"},
        body : JSON.stringify(jsonString),
        method : "POST",
        mode : "cors"
    };

    fetch(APIGW+"v1/dev/settings", other_params)
      .then(function(response) {
            //console.log(response.status );    //=> number 100–599
            //console.log(response.statusText); //=> String
            //console.log(response.headers);    //=> Headers
            //console.log(response.url);        //=> String
            //console.log(response.text());
            //return response.text()
      }, function(error) {
        console.log("Error["+error.message+"]"); //=> String
      });
      
  } // sendPostSetting()

    
  //============================================================================  
  function validateReadings(type) 
  {
    let withErrors = false;
    let prevMM     = 0;
    let lastBG     = "white";
        
    console.log("validate("+type+")");
    
    for (let i=0; i<(data.length -1); i++)
    {
      //--- reset background for the years
      if (getBackGround("em_YY_"+i) == "red")
      {
        setBackGround("em_YY_"+i, "lightgray");
      }
      //--- zelfde jaar, dan prevMM := (MM -1)
      if ( data[i].EEYY == data[i+1].EEYY )
      {
        prevMM = data[i].MM -1;
        //console.log("["+i+"].EEYY == ["+(i+1)+"].EEYY => ["+data[i].EEYY+"] prevMM["+prevMM+"]");
      }
      //--- jaar == volgend jaar + 1
      else if ( data[i].EEYY == (data[i+1].EEYY +1) )
      {
        prevMM = 12;
        //console.log("["+i+"].EEYY == ["+(i+1)+"].EEYY +1 => ["+data[i].EEYY+"]/["+data[i+1].EEYY+"] ("+prevMM+")");
      }
      else
      {
        setBackGround("em_YY_"+(i+1), "red");
        withErrors = true;
        prevMM = data[i].MM -1;
        //console.log("["+i+"].EEYY == ["+(i+1)+"].EEYY +1 => ["+data[i].EEYY+"]/["+data[i+1].EEYY+"] (error)");
      }
      
      //--- reset background for the months
      if (getBackGround("em_MM_"+(i+1)) == "red")
      {
        setBackGround("em_MM_"+(i+1), "lightgray");
      }
      //--- if next month != prevMM and this MM != next MM
      if (data[i+1].MM != prevMM && data[i].MM != data[i+1].MM)
      {
        setBackGround("em_MM_"+(i+1), "red");
        withErrors = true;
        //console.log("(["+(i+1)+"].MM != ["+prevMM+"].prevMM) && => ["+data[i].MM+"]/["+data[i+1].MM+"] (error)");
      }
      else
      {
        //setBackGround("em_MM_"+i, "lightgreen");
      }
      if (type == "ED")
      {
        if (getBackGround("em_in1_"+(i+1)) == "red")
        {
          setBackGround("em_in1_"+(i+1), "lightgray");
        }
        if (data[i].edt1 < data[i+1].edt1)
        {
          setBackGround("em_in1_"+(i+1), "red");
          withErrors = true;
        }
        if (getBackGround("em_in2_"+(i+1)) == "red")
        {
          setBackGround("em_in2_"+(i+1), "lightgray");
        }
        if (data[i].edt2 < data[i+1].edt2)
        {
          setBackGround("em_in2_"+(i+1), "red");
          withErrors = true;
        }
      }
      else if (type == "ER")
      {
        if (getBackGround("em_in1_"+(i+1)) == "red")
        {
          setBackGround("em_in1_"+(i+1), "lightgray");
        }
        if (data[i].ert1 < data[i+1].ert1)
        {
          setBackGround("em_in1_"+(i+1), "red");
          withErrors = true;
        }
        if (getBackGround("em_in2_"+(i+1)) == "red")
        {
          setBackGround("em_in2_"+(i+1), "lightgray");
        }
        if (data[i].ert2 < data[i+1].ert2)
        {
          setBackGround("em_in2_"+(i+1), "red");
          withErrors = true;
        }
      }
      else if (type == "GD")
      {
        if (getBackGround("em_in1_"+(i+1)) == "red")
        {
          setBackGround("em_in1_"+(i+1), "lightgray");
        }
        if (data[i].gdt < data[i+1].gdt)
        {
          setBackGround("em_in1_"+(i+1), "red");
          withErrors = true;
        }
      }
      
    }
    if (withErrors)  return false;

    return true;
    
  } // validateReadings()
  
    
  //============================================================================  
  function sendPostReading(i, row) 
  {
    console.log("sendPostReadings["+i+"]..");
    let sYY = (row[i].EEYY - 2000).toString();
    let sMM = "00";
    if ((row[i].MM *1) < 1 || (row[i].MM *1) > 12)
    {
      console.log("send: ERROR MM["+row[i].MM+"]");
      return;
    }
    if (row[i].MM < 10)
          sMM = "0"+(row[i].MM).toString();
    else  sMM = ((row[i].MM * 1)).toString();
    let sDDHH = "0101";
    let recId = sYY + sMM + sDDHH;
    console.log("send["+i+"] => ["+recId+"]");
    
    const jsonString = {"recid": recId, "edt1": row[i].edt1, "edt2": row[i].edt2,
                         "ert1": row[i].ert1,  "ert2": row[i].ert2, "gdt":  row[i].gdt };

    const other_params = {
        headers : { "content-type" : "application/json; charset=UTF-8"},
        body : JSON.stringify(jsonString),
        method : "POST",
        mode : "cors"
    };
    
    fetch(APIGW+"v1/hist/months", other_params)
      .then(function(response) {
      }, function(error) {
        console.log("Error["+error.message+"]"); //=> String
      });

      
  } // sendPostReading()

  
  //============================================================================  
  function readGitHubVersion()
  {
    if (GitHubVersion != 0) return;
    
    fetch("https://cdn.jsdelivr.net/gh/mrWheel/DSMRloggerAPI@master/data/DSMRversion.dat", {cache: "no-store"})
      .then(response => {
        if (response.ok) {
          return response.text();
        } else {
          console.log('Something went wrong');
          return "";
        }
      })
      .then(text => {
        var tmpGHF     = text.replace(/(\r\n|\n|\r)/gm, "");
        console.log("parsed: tmpGHF is ["+tmpGHF+"]");
        GitHubVersion_dspl = tmpGHF;
        //console.log("parsed: GitHubVersion is ["+GitHubVersion_dspl+"]");
        tmpX = tmpGHF.substring(1, tmpGHF.indexOf(' '));
        //console.log("parsed: tmpX is ["+tmpX+"]");
        tmpN = tmpX.split(".");
        console.log("parsed: tmpN is ["+tmpN[0]+"|"+tmpN[1]+"|"+tmpN[2]+"]");
        GitHubVersion = tmpN[0]*10000 + tmpN[1]*1;
        
        console.log("firmwareVersion["+firmwareVersion+"] >= GitHubVersion["+GitHubVersion+"]");
        if (firmwareVersion == 0 || firmwareVersion >= GitHubVersion)
              newVersionMsg = "";
        else  newVersionMsg = firmwareVersion_dspl + " nieuwere versie ("+GitHubVersion_dspl+") beschikbaar";
        document.getElementById('message').innerHTML = newVersionMsg;
        console.log(newVersionMsg);

      })
      .catch(function(error) {
        console.log(error);
        GitHubVersion_dspl   = "";
        GitHubVersion        = 0;
      });     

  } // readGitHubVersion()

    
  //============================================================================  
  function setEditType(eType) {
    if (eType == "ED") {
      console.log("Edit Energy Delivered!");
      monthType = eType;
      getMonths()
      showMonths(data, monthType);
    } else if (eType == "ER") {
      console.log("Edit Energy Returned!");
      monthType = eType;
      getMonths()
      showMonths(data, monthType);
    } else if (eType == "GD") {
      console.log("Edit Gas Delivered!");
      monthType = eType;
      getMonths()
      showMonths(data, monthType);
    } else {
      console.log("setEditType to ["+eType+"] is quit shitty!");
      monthType = "";
    }

  } // setEditType()

   
  //============================================================================  
  function setNewValue(i, dField, field) {
    document.getElementById(field).style.background = "lightgray";
    //--- this is ugly!!!! but don't know how to do it better ---
    if (dField == "EEYY")       data[i].EEYY = document.getElementById(field).value;
    else if (dField == "MM")    data[i].MM   = document.getElementById(field).value;
    else if (dField == "edt1")  data[i].edt1 = document.getElementById(field).value;
    else if (dField == "edt2")  data[i].edt2 = document.getElementById(field).value;
    else if (dField == "ert1")  data[i].ert1 = document.getElementById(field).value;
    else if (dField == "ert2")  data[i].ert2 = document.getElementById(field).value;
    else if (dField == "gdt")   data[i].gdt  = document.getElementById(field).value;
    
  } // setNewValue()

   
  //============================================================================  
  function setBackGround(field, newColor) {
    document.getElementById(field).style.background = newColor;
    
  } // setBackGround()

   
  //============================================================================  
  function getBackGround(field) {
    return document.getElementById(field).style.background;
    
  } // getBackGround()

  
  //============================================================================  
  function validateNumber(field) {
    console.log("validateNumber(): ["+field+"]");
    if (field == "EDT1" || field == "EDT2" || field == "ERT1" || field == "ERT2" || field == "GAS") {
      var pattern = /^\d{1,1}(\.\d{1,5})?$/;
      var max = 1.99999;
    } else {
      var pattern = /^\d{1,2}(\.\d{1,2})?$/;
      var max = 99.99;
    }
    var newVal = document.getElementById(field).value;
    newVal = newVal.replace( /[^0-9.]/g, '' );
    if (!pattern.test(newVal)) {
      document.getElementById(field).style.color = 'orange';
      console.log("wrong format");
    } else {
      document.getElementById(field).style.color = settingFontColor;
      console.log("valid number!");
    }
    if (newVal > max) {
      console.log("Number to big!");
      document.getElementById(field).style.color = 'orange';
      newVal = max;
    }
    document.getElementById(field).value = newVal * 1;
    
  } // validateNumber()

  
  //============================================================================  
  function translateToHuman(longName) {
    //for(var index = 0; index < (translateFields.length -1); index++) 
    for(var index = 0; index < translateFields.length; index++) 
    {
        if (translateFields[index][0] == longName)
        {
          return translateFields[index][1];
        }
    };
    return longName;
    
  } // translateToHuman()

  
  //============================================================================  
  function formatDate(type, dateIn) 
  {
    let dateOut = "";
    if (type == "Hours")
    {
      //date = "20"+dateIn.substring(0,2)+"-"+dateIn.substring(2,4)+"-"+dateIn.substring(4,6)+" ["+dateIn.substring(6,8)+"]";
      dateOut = "("+dateIn.substring(4,6)+") ["+dateIn.substring(6,8)+":00 - "+dateIn.substring(6,8)+":59]";
    }
    else if (type == "Days")
      dateOut = recidToWeekday(dateIn)+" "+dateIn.substring(4,6)+"-"+dateIn.substring(2,4)+"-20"+dateIn.substring(0,2);
    else if (type == "Months")
      dateOut = "20"+dateIn.substring(0,2)+"-["+dateIn.substring(2,4)+"]-"+dateIn.substring(4,6)+":"+dateIn.substring(6,8);
    else
      dateOut = "20"+dateIn.substring(0,2)+"-"+dateIn.substring(2,4)+"-"+dateIn.substring(4,6)+":"+dateIn.substring(6,8);
    return dateOut;
  }

  
  //============================================================================  
  function recidToEpoch(dateIn) 
  {
    var YY = "20"+dateIn.substring(0,2);
    console.log("["+YY+"]["+(dateIn.substring(2,4)-1)+"]["+dateIn.substring(4,6)+"]");
    //-------------------YY-------------------(MM-1)----------------------DD---------------------HH--MM--SS
    var epoch = Date.UTC(YY, (dateIn.substring(2,4)-1), dateIn.substring(4,6), dateIn.substring(6,8), 1, 1);
    //console.log("epoch is ["+epoch+"]");

    return epoch;
    
  } // recidToEpoch()
  
  
  //============================================================================  
  function recidToWeekday(dateIn)
  {
    var YY = "20"+dateIn.substring(0,2);
    //-------------------YY-------------------(MM-1)----------------------DD---------------------HH--MM--SS
    var dt = new Date(Date.UTC(YY, (dateIn.substring(2,4)-1), dateIn.substring(4,6), 1, 1, 1));

    return dt.toLocaleDateString('nl-NL', {weekday: 'long'});
    
  } // epochToWeekday()
  
    
  //============================================================================  
  function round(value, precision) 
  {
    var multiplier = Math.pow(10, precision || 0);
    return Math.round(value * multiplier) / multiplier;
  }
    
  var translateFields = [
           [ "author",                    "Auteur" ]
          ,[ "identification",            "Slimme Meter ID" ]
          ,[ "p1_version",                "P1 Versie" ]
          ,[ "p1_version_be",             "P1 Versie (BE)" ]
          ,[ "energy_delivered_tariff1",  "Energie Gebruikt tarief 1" ]
          ,[ "energy_delivered_tariff2",  "Energie Gebruikt tarief 2" ]
          ,[ "energy_returned_tariff1",   "Energie Opgewekt tarief 1" ]
          ,[ "energy_returned_tariff2",   "Energie Opgewekt tarief 2" ]
          ,[ "electricity_tariff",        "Electriciteit tarief" ]
          ,[ "power_delivered",           "Vermogen Gebruikt" ]
          ,[ "power_returned",            "Vermogen Opgewekt" ]
          ,[ "electricity_threshold",     "Electricity Threshold" ]
          ,[ "electricity_switch_position","Electricity Switch Position" ]
          ,[ "electricity_failures",      "Electricity Failures" ]
          ,[ "electricity_long_failures", "Electricity Long Failures" ]
          ,[ "electricity_failure_log",   "Electricity Failure log" ]
          ,[ "electricity_sags_l1",       "Electricity Sags l1" ]
          ,[ "electricity_sags_l2",       "Electricity Sags l2" ]
          ,[ "electricity_sags_l3",       "Electricity Sags l3" ]
          ,[ "electricity_swells_l1",     "Electricity Swells l1" ]
          ,[ "electricity_swells_l2",     "Electricity Swells l2" ]
          ,[ "electricity_swells_l3",     "Electricity Swells l3" ]
          ,[ "message_short",             "Korte Boodschap" ]
          ,[ "message_long",              "Lange Boodschap" ]
          ,[ "voltage_l1",                "Voltage l1" ]
          ,[ "voltage_l2",                "Voltage l2" ]
          ,[ "voltage_l3",                "Voltage l3" ]
          ,[ "current_l1",                "Current l1" ]
          ,[ "current_l2",                "Current l2" ]
          ,[ "current_l3",                "Current l3" ]
          ,[ "power_delivered_l1",        "Vermogen Gebruikt l1" ]
          ,[ "power_delivered_l2",        "Vermogen Gebruikt l2" ]
          ,[ "power_delivered_l3",        "Vermogen Gebruikt l3" ]
          ,[ "power_returned_l1",         "Vermogen Opgewekt l1" ]
          ,[ "power_returned_l2",         "Vermogen Opgewekt l2" ]
          ,[ "power_returned_l3",         "Vermogen Opgewekt l3" ]
          ,[ "mbus1_type",                "MBus-1 Type meter (0=geen)" ]
          ,[ "mbus1_device_type",         "MBus-1 Type meter (0=geen)" ]
          ,[ "mbus1_equipment_id_tc",     "MBus-1 Equipm. ID (tc)" ]
          ,[ "mbus1_equipment_id_ntc",    "MBus-1 Equipm. ID (ntc)" ]
          ,[ "mbus1_delivered",           "MBus-1 Gebruikt" ]
          ,[ "mbus1_delivered_ntc",       "MBus-1 Gebruikt (ntc)" ]
          ,[ "mbus1_delivered_dbl",       "MBus-1 Gebruikt" ]
          ,[ "mbus1_valve_position",      "MBus-1 Klep Positie" ]
          ,[ "mbus2_type",                "MBus-2 Type meter (0=geen)" ]
          ,[ "mbus2_device_type",         "MBus-2 Type meter (0=geen)" ]
          ,[ "mbus2_equipment_id_tc",     "MBus-2 Equipm. ID (tc)" ]
          ,[ "mbus2_equipment_id_ntc",    "MBus-2 Equipm. ID (ntc)" ]
          ,[ "mbus2_delivered",           "MBus-2 Gebruikt" ]
          ,[ "mbus2_delivered_ntc",       "MBus-2 Gebruikt (ntc)" ]
          ,[ "mbus2_delivered_dbl",       "MBus-2 Gebruikt" ]
          ,[ "mbus2_valve_position",      "MBus-2 Klep Positie" ]
          ,[ "mbus3_type",                "MBus-3 Type meter (0=geen)" ]
          ,[ "mbus3_device_type",         "MBus-3 Type meter (0=geen)" ]
          ,[ "mbus3_equipment_id_tc",     "MBus-3 Equipm. ID (tc)" ]
          ,[ "mbus3_equipment_id_ntc",    "MBus-3 Equipm. ID (ntc)" ]
          ,[ "mbus3_delivered",           "MBus-3 Gebruikt" ]
          ,[ "mbus3_delivered_ntc",       "MBus-3 Gebruikt (ntc)" ]
          ,[ "mbus3_delivered_dbl",       "MBus-3 Gebruikt" ]
          ,[ "mbus3_valve_position",      "MBus-3 Klep Positie" ]
          ,[ "mbus4_type",                "MBus-4 Type meter (0=geen)" ]
          ,[ "mbus4_device_type",         "MBus-4 Type meter (0=geen)" ]
          ,[ "mbus4_equipment_id_tc",     "MBus-4 Equipm. ID (tc)" ]
          ,[ "mbus4_equipment_id_ntc",    "MBus-4 Equipm. ID (ntc)" ]
          ,[ "mbus4_delivered",           "MBus-4 Gebruikt" ]
          ,[ "mbus4_delivered_ntc",       "MBus-4 Gebruikt (ntc)" ]
          ,[ "mbus4_delivered_dbl",       "MBus-4 Gebruikt" ]
          ,[ "mbus4_valve_position",      "MBus-4 Klep Positie" ]
          ,[ "gas_delivered",             "Gas Gebruikt" ]
          ,[ "ed_tariff1",                "Energy Verbruik Tarief-1/kWh" ]
          ,[ "ed_tariff2",                "Energy Verbruik Tarief-2/kWh" ]
          ,[ "er_tariff1",                "Energy Opgewekt Tarief-1/kWh" ]
          ,[ "er_tariff2",                "Energy Opgewekt Tarief-2/kWh" ]
          ,[ "gd_tariff" ,                "Gas Verbruik Tarief/m3" ]
          ,[ "electr_netw_costs",         "Netwerkkosten Energie/maand" ]
          ,[ "gas_netw_costs",            "Netwerkkosten Gas/maand" ]
          
          ,[ "smhasfaseinfo",             "SM Has Fase Info (0=No, 1=Yes)" ]
          ,[ "sm_has_fase_info",          "SM Has Fase Info (0=No, 1=Yes)" ]
          ,[ "pre_dsmr40",                "Pré DSMR 40 (0=No, 1=Yes)" ]
          ,[ "dailyreboot",               "Dagelijkse Reboot (0=No, 1=Yes)" ]
          ,[ "oled_type",                 "OLED type (0=None, 1=SDD1306, 2=SH1106)" ]
          ,[ "oled_flip_screen",          "Flip OLED scherm (0=No, 1=Yes)" ]
          ,[ "tlgrm_interval",            "Telegram Lees Interval (Sec.)" ]
          ,[ "telegraminterval",          "Telegram Lees Interval (Sec.)" ]
          ,[ "index_page",                "Te Gebruiken index.html Pagina" ]
          ,[ "oled_screen_time",          "Oled Screen Time (Min., 0=infinite)" ]
          ,[ "mqttbroker",                "MQTT Broker IP/URL" ]
          ,[ "mqtt_broker",               "MQTT Broker IP/URL" ]
          ,[ "mqttbrokerport",            "MQTT Broker Poort" ]
          ,[ "mqtt_broker_port",          "MQTT Broker Poort" ]
          ,[ "mqttuser",                  "MQTT Gebruiker" ]
          ,[ "mqtt_user",                 "MQTT Gebruiker" ]
          ,[ "mqttpasswd",                "Password MQTT Gebruiker" ]
          ,[ "mqtt_passwd",               "Password MQTT Gebruiker" ]
          ,[ "mqtttoptopic",              "MQTT Top Topic" ]
          ,[ "mqtt_toptopic",             "MQTT Top Topic" ]
          ,[ "mqttinterval",              "Verzend MQTT Berichten (Sec.)" ]
          ,[ "mqtt_interval",             "Verzend MQTT Berichten (Sec.)" ]
          ,[ "mqttbroker_connected",      "MQTT broker connected" ]
          ,[ "mindergas_token",           "Mindergas Token" ]
          ,[ "mindergas_response",        "Mindergas Terugkoppeling" ]
          ,[ "mindergas_status",          "Mindergas Status (@dag | tijd)" ]

          ,[ "telegramcount",             "Telegrammen verwerkt" ]
          ,[ "telegramerrors",            "Telegrammen met fouten" ]          
          ,[ "fwversion",                 "Firmware Versie" ]
          ,[ "compiled",                  "Gecompileerd" ]
          ,[ "hostname",                  "HostName" ]
          ,[ "ipaddress",                 "IP adres" ]
          ,[ "macaddress",                "MAC adres" ]
          ,[ "indexfile",                 "Te Gebruiken index.html Pagina" ]
          ,[ "freeheap",                  "Free Heap Space" ]
          ,[ "maxfreeblock",              "Max. Free Heap Blok" ]
          ,[ "chipid",                    "Chip ID" ]
          ,[ "coreversion",               "ESP8266 Core Versie" ]
          ,[ "sdkversion",                "SDK versie" ]
          ,[ "cpufreq",                   "CPU Frequency" ]
          ,[ "sketchsize",                "Sketch Size" ]
          ,[ "freesketchspace",           "Free Sketch Space" ]
          ,[ "flashchipid",               "Flash Chip ID" ]
          ,[ "flashchipsize",             "Flash Chip Size" ]
          ,[ "flashchiprealsize",         "Flash Chip Real Size" ]
          ,[ "spiffssize",                "SPIFFS Size" ]
          ,[ "flashchipspeed",            "Flash Chip Speed" ]
          ,[ "flashchipmode",             "Flash Chip Mode" ]
          ,[ "boardtype",                 "Board Type" ]
          ,[ "compileoptions",            "Compiler Opties" ]
          ,[ "ssid",                      "WiFi SSID" ]
          ,[ "wifirssi",                  "WiFi RSSI" ]
          ,[ "uptime",                    "Up Time [dagen] - [hh:mm]" ]
          ,[ "uptime_secs",               "Up Time in Seconds" ]
          ,[ "reboots",                   "Aantal keer opnieuw opgestart" ]
          ,[ "lastreset",                 "Laatste Reset reden" ]
          
                        ];

/*
***************************************************************************
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
***************************************************************************
*/
