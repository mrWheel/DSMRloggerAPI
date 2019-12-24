/*
***************************************************************************  
**  Program  : DSMReditor.js, part of DSMRloggerAPI
**  Version  : v0.1.2
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
  let monthType   = "D";
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
      x[i].style.display    = "none";  
      x[i].style.background     = settingBgColor;
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
    fetch(APIGW+"v1/dev/settings")
      .then(response => response.json())
      .then(json => {
        data = json.settings;
        //data[i].changed = {};
        for( let i in data )
        {
          //console.log("Field["+i+"] => ["+data[i].name+"]");
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
    for(var i in data)
    {
      var fldId  = data[i].name;
      var newVal = document.getElementById("setting_"+fldId).value;
      if (data[i].value != newVal)
      {
        console.log("save data ["+data[i].name+"] => from["+data[i].value+"] to["+newVal+"]");
        sendPutCall(fldId, newVal);
      }
    }    
    data = {};
    refreshSettings();
    
  } // saveData()
  
  //============================================================================  
  function sendPutCall(field, value) {
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
        //document.getElementById('message').innerHTML = "Ok";
    }

   
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
