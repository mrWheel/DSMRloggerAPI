/*
***************************************************************************  
**  Program  : DSMReditor.js, part of DSMRloggerAPI
**  Version  : v0.2.4
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

  const APIGW='http://'+window.location.host+'/api/';

"use strict";

  let needReload  = true;
  let activeTab   = "none";
  let monthType   = "ED";
  let settingBgColor   = 'deepskyblue';
  let settingFontColor = 'white'
  var data = [];
  
  window.onload=bootsTrap;
  /*
  window.onfocus = function() {
    if (needReload) {
      window.location.reload(true);
    }
  };
  */
  
  function bootsTrap() {
    console.log("bootsTrap()");
    needReload = true;
    
    document.getElementById('bTerug').addEventListener('click',function()
                                                {location.href = "/";});
    document.getElementById('bEditMonths').addEventListener('click',function()
                                                {openTab('tabMonths');});
    document.getElementById('bEditSettings').addEventListener('click',function()
                                                {openTab('tabSettings');});
    document.getElementById('bUndo').addEventListener('click',function() 
                                                {undoReload();});
    document.getElementById('bSave').addEventListener('click',function() 
                                                {saveData();});
    refreshDevTime();
    refreshDevInfo();
    TimerTime = setInterval(refreshDevTime, 10 * 1000); // repeat every 10s

    //openTab("tabSettings");
    
    //---- update buttons in navigation bar ---
    let x = document.getElementsByClassName("editButton");
    for (var i = 0; i < x.length; i++) {
      x[i].style.background     = settingBgColor;
      x[i].style.border         = 'none';
      x[i].style.textDecoration = 'none';  
      x[i].style.outline        = 'none';  
      x[i].style.boxShadow      = 'none';
    }

  } // bootsTrap()
  
   
  
  function openTab(tabName) {
    
    activeTab = tabName;
    
    let bID = "b" + tabName;
    let i;
    //---- update buttons in navigation bar ---
    console.log("openTab: First set all fields in [tabName] to none ..");
    let x = document.getElementsByClassName("tabName"); // set all fields to "none"
    for (i = 0; i < x.length; i++) {
      console.log("Field["+i+"] set to none");
      x[i].style.display        = "none";  
      x[i].style.background     = 'white'; /*'deepskyblue';*/
      x[i].style.border         = 'none';
      x[i].style.textDecoration = 'none';  
      x[i].style.outline        = 'none';  
      x[i].style.boxShadow      = 'none';
    }
    //--- hide all tab's -------
//    x = document.getElementsByClassName("tabName");
//    for (i = 0; i < x.length; i++) {
//      x[i].style.display    = "none";  
//    }
    //--- and set active tab to 'block'
    console.log("openTab: now set all fields in ["+bID+"] to block ..");
    document.getElementById(tabName).style.background='white';
    document.getElementById(tabName).style.display = "block";  
    if (tabName == "tabMonths") {
      console.log("newTab: tabMonths");
      document.getElementById('tabMaanden').style.display = 'block';
      getMonths();

    } else if (tabName == "tabSettings") {
      console.log("newTab: tabSettings");
      refreshSettings();

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
            if (data[i].name == "fwversion")
            {
              document.getElementById('devVersion').innerHTML = data[i].value;
            } 
            else if (data[i].name == 'hostname')
            {
              document.getElementById('devName').innerHTML = data[i].value;
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
          if( ( document.getElementById("setting_"+data[i].name)) == null )
          {
              var div1 = document.createElement("div");
                  div1.setAttribute("class", "settingDiv");
                  div1.style.marginLeft = "200px";
                  div1.style.marginRight = "200px";
              var div2 = document.createElement("div");
                  div2.style.width = "200px";
                  div2.style.float = 'left';
                  div2.textContent = data[i].name;
              div1.appendChild(div2);
                  div2 = document.createElement("span");
                  var sInput = document.createElement("INPUT");
                    sInput.setAttribute("id", "setting_"+data[i].name);

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
                                            function() { setChanged("setting_"+data[i].name); },
                                            false
                                          );
                  div2.appendChild(sInput);
              div1.appendChild(div2);
              settings.appendChild(div1);
          }
          else
          {
            document.getElementById("setting_"+data[i].name).style.background = "white";
            document.getElementById("setting_"+data[i].name).value = data[i].value;
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
        showMonths(data, monthType);
      })
      .catch(function(error) {
        var p = document.createElement('p');
        p.appendChild(
          document.createTextNode('Error: ' + error.message)
        );
      });
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
    
    for (let i=0; i<data.length; i++)
    {
      console.log("["+i+"] >>>["+data[i].recid+"]");
      var em = document.getElementById('editMonths');
    //if( ( document.getElementById("em_R"+i)) == null )
      if( ( document.getElementById("em_R"+i)) == null )
      {
        var div1 = document.createElement("div");
            div1.setAttribute("class", "settingDiv");
            div1.setAttribute("id", "em_R"+i);
            div1.style.marginLeft = "150px";
            div1.style.marginRight = "300px";
            //div1.style.width = "100px";
        var div2 = document.createElement("div");
            div2.style.width = "100px";
            div2.style.float = 'left';
            div2.style.textAlign = 'center';
            
            div2.textContent = "20"+data[i].recid.substring(0,2)+"-"+data[i].recid.substring(2,4);
            div1.appendChild(div2);
              div2 = document.createElement("span");
              var sInput = document.createElement("INPUT");
                sInput.setAttribute("id", "em_in1_"+i);
                sInput.setAttribute("type", "number");
                sInput.addEventListener('change',
                           function() { setChanged("em_in1_"+i); },
                              false
                           );
                div2.appendChild(sInput);
              if (type != "GD")
              {
                var sInput = document.createElement("INPUT");
                sInput.setAttribute("id", "em_in2_"+i);
                sInput.setAttribute("type", "number");
                sInput.addEventListener('change',
                           function() { setChanged("em_in2_"+i); },
                              false
                           );
                div2.appendChild(sInput);
              }
              div1.appendChild(div2);
              em.appendChild(div1);
      }
      
      if (type == "ED")
      {
        document.getElementById("em_in1_"+i).style.background = "white";
        document.getElementById("em_in1_"+i).value = data[i].edt1.toFixed(3);
        document.getElementById("em_in2_"+i).style.background = "white";
        document.getElementById("em_in2_"+i).value = data[i].edt2.toFixed(3);
      }
      else if (type == "ER")
      {
        document.getElementById("em_in1_"+i).style.background = "white";
        document.getElementById("em_in1_"+i).value = data[i].ert1.toFixed(3);
        document.getElementById("em_in2_"+i).style.background = "white";
        document.getElementById("em_in2_"+i).value = data[i].ert2.toFixed(3);
      }
      else if (type == "GD")
      {
        document.getElementById("em_in1_"+i).style.background = "white";
        document.getElementById("em_in1_"+i).value = data[i].gdt.toFixed(3);
      }
    } // for all elements in data

  } // showMonths()

      
  //============================================================================  
  function undoReload()
  {
    if (activeTab == "tabMonths") {
      console.log("sendMonths");

    } else if (activeTab == "tabSettings") {
      console.log("undoReload(): reload Settings..");
      data = {};
      refreshSettings();

    } else {
      console.log("undoReload(): I don't knwo what to do ..");
    }

  } // undoReload()
  
  
  //============================================================================  
  function saveData() {
    if (activeTab == "tabSettings")
    {
      saveSettings();
    } 
    else if (activeTab == "tabMonths")
    {
      saveMeterReadings();
    }
    
  } // saveData()
  
  
  //============================================================================  
  function saveSettings() {
    for(var i in data)
    {
      var fldId  = data[i].name;
      var newVal = document.getElementById("setting_"+fldId).value;
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
  function saveMeterReadings() {
    console.log("Saving months-data ..");
    let changes = false;
    //--- has enything changed?
    for (i in data)
    {
      if (document.getElementById("em_in1_"+i).style.background != 'white')
      {
        changes = true;
        document.getElementById("em_in1_"+i).style.background = 'white';
      }
      if (document.getElementById("em_in2_"+i).style.background != 'white')
      {
        changes = true;
        document.getElementById("em_in2_"+i).style.background = 'white';
      }
    } 
    if (changes)
          console.log("Changes where made!!");
    else  console.log("No changes found.");
    
    processWithTimeout([(data.length -1), 0], 2, data, sendPostReading);
    
  } // saveMeterReadings()

    
  //============================================================================  
  function sendPostSetting(field, value) {
    //document.getElementById('message').innerHTML = "sending data ..";

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
  function sendPostReading(i, row) {
    //document.getElementById('message').innerHTML = "sending data ..";
    console.log("["+i+"] => ["+row[i].recid+"]");
    
    const jsonString = {"recid": row[i].recid, "edt1": row[i].edt1, "edt2": row[i].edt2,
                         "ert1": row[i].ert1,  "ert2": row[i].ert2, "gdt":  row[i].gdt };
    //console.log("send JSON:["+jsonString+"]");
    const other_params = {
        headers : { "content-type" : "application/json; charset=UTF-8"},
        body : JSON.stringify(jsonString),
        method : "POST",
        mode : "cors"
    };
    const postRequest = async () => {
      const response = await fetch(APIGW+"v1/hist/months", other_params, processWithTimeout = 7000);
      const json = await response.json();
      console.log(json);
    }

    postRequest();  
      
  } // sendPostReading()
  
  
  //============================================================================  
  function processWithTimeout(range, time, row, callback)
  {
    var i = range[0];                
    callback(i, row);
    Loop();
    function Loop()
    {
      setTimeout(function() {
          i--;
          if (i>=range[1]){
              callback(i, row);
              Loop();
          }
      }, time*100)
    } 
  } // processWithTimeout()
  
  //============================================================================  
  //This function prints the loop number every second
  //processWithTimeout([0, 5], 1, function(i) {
  //  console.log(i);
  //});

    
  //============================================================================  
  function setEditType(eType) {
    if (eType == "ED") {
      console.log("Edit Energy Delivered!");
      monthType = eType;
      showMonths(data, monthType);
    } else if (eType == "ER") {
      console.log("Edit Energy Returned!");
      monthType = eType;
      showMonths(data, monthType);
    } else if (eType == "GD") {
      console.log("Edit Gas Delivered!");
      monthType = eType;
      showMonths(data, monthType);
    } else {
      console.log("setEditType to ["+eType+"] is quit shitty!");
      monthType = "";
    }

  } // setEditType()

   
  //============================================================================  
  function setChanged(field) {
    document.getElementById(field).style.background = "lightgray";
  } // setChanged()

  
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
  function round(value, precision) {
    var multiplier = Math.pow(10, precision || 0);
    return Math.round(value * multiplier) / multiplier;
  }

  //============================================================================  
  function hoverOn(field) {
    document.getElementById(field).style.background = "lightgray";
  } // hoverOn()
  
  //============================================================================  
  function hoverOff(field) {
    document.getElementById(field).style.background = "red";
  } // hoverOff()

  //============================================================================  
  function existingId(elementId) {
    if(document.getElementById(elementId)){
      return true;
    } 
  //console.log("cannot find elementId [" + elementId + "]");
    return false;
    
  } // existingId()
  
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
