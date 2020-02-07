/*
***************************************************************************  
**  Program  : DSMRindex.js, part of DSMRfirmwareAPI
**  Version  : v0.3.3
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
  const APIGW='http://'+window.location.host+'/api/';

  "use strict";

  let needReload          = true;
  let activeTab           = "none";
  let presentationType    = "TAB";
  let tabTimer            = 0;
  let actualTimer         = 0;
  let timeTimer           = 0;
  
  var tlgrmInterval       = 10;
  var ed_tariff1          = 0;
  var ed_tariff2          = 0;
  var er_tariff1          = 0;
  var er_tariff2          = 0;
  var gd_tariff           = 0;
  var electr_netw_costs   = 0;
  var gas_netw_costs      = 0;
  
  var data       = [];
  
  var longFields = [ "identification","p1_version","timestamp","equipment_id"
                    ,"energy_delivered_tariff1","energy_delivered_tariff2"
                    ,"energy_returned_tariff1","energy_returned_tariff2","electricity_tariff"
                    ,"power_delivered","power_returned"
                    ,"electricity_threshold","electricity_switch_position"
                    ,"electricity_failures","electricity_long_failures","electricity_failure_log"
                    ,"electricity_sags_l1","electricity_sags_l2","electricity_sags_l3"
                    ,"electricity_swells_l1","electricity_swells_l2","electricity_swells_l3"
                    ,"message_short","message_long"
                    ,"voltage_l1","voltage_l2","voltage_l3"
                    ,"current_l1","current_l2","current_l3"
                    ,"power_delivered_l1","power_delivered_l2","power_delivered_l3"
                    ,"power_returned_l1","power_returned_l2","power_returned_l3"
                    ,"gas_device_type","gas_equipment_id","gas_valve_position","gas_delivered"
                    ,"thermal_device_type","thermal_equipment_id"
                    ,"thermal_valve_position","thermal_delivered"
                    ,"water_device_type","water_equipment_id"
                    ,"water_valve_position","water_delivered"
                    ,"slave_device_type","slave_equipment_id"
                    ,"slave_valve_position","slave_delivered"
                    ,"\0"
                  ];
                    
  var humanFields = [ "Slimme Meter ID","P1 Versie","timestamp","Equipment ID"
                    ,"Energie Gebruikt tarief 1","Energy Gebruikt tarief 2"
                    ,"Energie Opgewekt tarief 1","Energie Opgewekt tarief 2","Electriciteit tarief"
                    ,"Vermogen Gebruikt","Vermogen Opgewekt"
                    ,"electricity_threshold","electricity_switch_position"
                    ,"electricity_failures","electricity_long_failures","electricity_failure_log"
                    ,"electricity_sags_l1","electricity_sags_l2","electricity_sags_l3"
                    ,"electricity_swells_l1","electricity_swells_l2","electricity_swells_l3"
                    ,"message_short","message_long"
                    ,"Voltage l1","Voltage l2","Voltage l3"
                    ,"Current l1","Current l2","Current l3"
                    ,"Vermogen Gebruikt l1","Vermogen Gebruikt l2","Vermogen Gebruikt l3"
                    ,"Vermogen Opgewekt l1","Vermogen Opgewekt l2","Vermogen Opgewekt l3"
                    ,"Gas Device Type","Gas Equipment ID","Gas Klep Positie","Gas Gebruikt"
                    ,"thermal_device_type","thermal_equipment_id"
                    ,"thermal_valve_position","thermal_delivered"
                    ,"water_device_type","water_equipment_id"
                    ,"water_valve_position","water_delivered"
                    ,"slave_device_type","slave_equipment_id"
                    ,"slave_valve_position","slave_delivered"
                    ,"\0"
                    ];
                    
  var monthNames = [ "indxNul","Januari","Februari","Maart","April","Mei","Juni"
                    ,"Juli","Augustus","September","Oktober","November","december"
                    ,"\0"
                   ];
  
  window.onload=bootsTrap;
  window.onfocus = function() {
    if (needReload) {
      window.location.reload(true);
    }
  };
    
  //============================================================================  
  function bootsTrap() {
    console.log("bootsTrap()");
    needReload = true;
    
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
    document.getElementById('restAPITab').addEventListener('click',function() 
                                                { console.log("newPage: goAPI");
                                                  location.href = "/api";
                                                });
    document.getElementById('FSexplorer').addEventListener('click',function() 
                                                { console.log("newTab: goFSexplorer");
                                                  location.href = "/FSexplorer";
                                                });
    needReload = false;
    refreshDevTime();
    getDevSettings();
    refreshDevInfo();
    
    timeTimer = setInterval(refreshDevTime, 10 * 1000); // repeat every 10s

    openTab("ActualTab");
    initActualGraph();
    setPresentationType('TAB');
      
  } // bootsTrap()
  
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
      actualTimer = setInterval(refreshSmActual, 60 * 1000);                  // repeat every 60s
    }
    
    if (tabName == "ActualTab") {
      console.log("newTab: ActualTab");
      refreshSmActual();
      if (tlgrmInterval < 10)
            actualTimer = setInterval(refreshSmActual, 10 * 1000);            // repeat every 10s
      else  actualTimer = setInterval(refreshSmActual, tlgrmInterval * 1000); // repeat every tlgrmInterval seconds

    } else if (tabName == "HoursTab") {
      console.log("newTab: HoursTab");
      refreshHours();
      tabTimer = setInterval(refreshHours, 58 * 1000); // repeat every 58s

    } else if (tabName == "DaysTab") {
      console.log("newTab: DaysTab");
      refreshDays();
      tabTimer = setInterval(refreshDays, 58 * 1000); // repeat every 58s
      /***
      console.log("ed_tariff1["+ed_tariff1+"]");
      console.log("ed_tariff2["+ed_tariff2+"]");
      console.log("er_tariff1["+er_tariff1+"]");
      console.log("er_tariff2["+er_tariff2+"]");
      console.log(" gd_tariff["+gd_tariff+"]");
      console.log("electr_netw_costs["+electr_netw_costs+"]");
      console.log("   gas_netw_costs["+gas_netw_costs+"]");    
      ***/

    } else if (tabName == "MonthsTab") {
      console.log("newTab: MonthsTab");
      refreshMonths();
      tabTimer = setInterval(refreshMonths, 118 * 1000); // repeat every 118s
    
    } else if (tabName == "SysInfoTab") {
      console.log("newTab: SysInfoTab");
      refreshDevInfo();
      tabTimer = setInterval(refreshDevInfo, 58 * 1000); // repeat every 58s

    } else if (tabName == "FieldsTab") {
      console.log("newTab: FieldsTab");
      refreshSmFields();
      tabTimer = setInterval(refreshSmFields, 58 * 1000); // repeat every 58s

    } else if (tabName == "TelegramTab") {
      console.log("newTab: TelegramTab");
      refreshSmTelegram();
      //tabTimer = setInterval(refreshSmTelegram, 60 * 1000); // do not repeat!
    }

  } // openTab()

  
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
            tableCells[0].innerHTML = data[i].name;
            tableCells[1].innerHTML = data[i].value;
            if (data[i].hasOwnProperty('unit'))
            {
              tableCells[1].style.textAlign = "right";
              tableCells[2].innerHTML = data[i].unit;
            }
            
            if (data[i].name == "fwversion")
            {
              document.getElementById('devVersion').innerHTML = json.devinfo[i].value;
            } else if (data[i].name == 'hostname')
            {
              document.getElementById('devName').innerHTML = data[i].value;
            } else if (data[i].name == 'tlgrm_interval')
            {
              tlgrmInterval = data[i].value;
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
  } // refreshDevTime()
  
  
  //============================================================================  
  function refreshSmActual()
  {
    fetch(APIGW+"v1/sm/actual")
      .then(response => response.json())
      .then(json => {
          //console.log("parsed .., fields is ["+ JSON.stringify(json)+"]");
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
            data[i].shortName = smToHuman(data[i].name);
            var tableRef = document.getElementById('fieldsTable').getElementsByTagName('tbody')[0];
            if( ( document.getElementById("fieldsTable_"+data[i].name)) == null )
            {
              var newRow   = tableRef.insertRow();
              newRow.setAttribute("id", "fieldsTable_"+data[i].name, 0);
              // Insert a cell in the row at index 0
              var newCell  = newRow.insertCell(0);                  // name
              var newText  = document.createTextNode('');
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(1);                      // shortName
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(2);                      // value
              newCell.appendChild(newText);
              newCell  = newRow.insertCell(3);                      // unit
              newCell.appendChild(newText);
            }
            tableCells = document.getElementById("fieldsTable_"+data[i].name).cells;
            tableCells[0].innerHTML = data[i].name;
            tableCells[1].innerHTML = data[i].shortName;
            tableCells[2].innerHTML = data[i].value;
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
              showMonthsHist(data);
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
     //console.log("now in expandData() ..");
     for (let i=0; i<data.length; i++)
     {
      data[i].p_ed  = {};
      data[i].p_edw = {};
      data[i].p_er  = {};
      data[i].p_erw = {};
      data[i].p_gd  = {};
      data[i].costs = {};

      if (i < (data.length -1))
      {
        data[i].p_ed  = ((data[i].edt1 +data[i].edt2)-(data[i+1].edt1 +data[i+1].edt2)).toFixed(3);
        data[i].p_edw = (data[i].p_ed * 1000).toFixed(0);
        data[i].p_er  = ((data[i].ert1 +data[i].ert2)-(data[i+1].ert1 +data[i+1].ert2)).toFixed(3);
        data[i].p_erw = (data[i].p_er * 1000).toFixed(0);
        data[i].p_gd  = (data[i].gdt  -data[i+1].gdt).toFixed(3);
        //var  day = data[i].recid.substring(4,6) * 1;
        //-- calculate Energy Delivered costs
        var     costs = ( (data[i].edt1 - data[i+1].edt1) * ed_tariff1 );
        costs = costs + ( (data[i].edt2 - data[i+1].edt2) * ed_tariff2 );
        //-- add Energy Returned costs
        costs = costs - ( (data[i].ert1 - data[i+1].ert1) * er_tariff1 );
        costs = costs - ( (data[i].ert2 - data[i+1].ert2) * er_tariff2 );
        //-- add Gas Delivered costs
        costs = costs + ( (data[i].gdt  - data[i+1].gdt)  * gd_tariff );
        //-- add network costs
        costs = costs + ( electr_netw_costs / 30 );
        costs = costs + ( gas_netw_costs    / 30 );
        data[i].costs = costs.toFixed(2)
      }
      else
      {
        data[i].p_ed  = (data[i].edt1 +data[i].edt2).toFixed(3);
        data[i].p_edw = (data[i].p_ed * 1000).toFixed(0);
        data[i].p_er  = (data[i].ert1 +data[i].ert2).toFixed(3);
        data[i].p_erw = (data[i].p_er * 1000).toFixed(0);
        data[i].p_gd  = (data[i].gdt).toFixed(3);
        data[i].costs = 0.0;
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
      data[i].shortName = smToHuman(data[i].name);
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
      tableCells[0].innerHTML = data[i].shortName;
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
        tableCells[4].innerHTML = (data[i].costs * 1.0).toFixed(2);
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
      document.getElementById('hGRAPH').checked = true;
      document.getElementById('hTAB').checked   = false;
      document.getElementById('dGRAPH').checked = true;
      document.getElementById('dTAB').checked   = false;
      document.getElementById('mGRAPH').checked = true;
      document.getElementById('mTAB').checked   = false;
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
    } else {
      console.log("setPresentationType to ["+pType+"] is quit shitty! Set to TAB");
      presentationType = "TAB";
    }

    if (activeTab == "ActualTab")  refreshSmActual();
    if (activeTab == "HoursTab")   refreshHours();
    if (activeTab == "DaysTab")    refreshDays();
    if (activeTab == "MonthsTab")  refreshMonths();

  } // setPresenationType()

  
  //============================================================================  
  function smToHuman(longName) {
    //console.log("smToHuman("+longName+") for ["+longFields.length+"] elements");
    for(var index = 0; index < (longFields.length -1); index++) 
    {
        if (longFields[index] == longName)
        {
          return humanFields[index];
        }
    };
    return longName;
    
  } // smToHuman()

  
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
