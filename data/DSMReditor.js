/*
***************************************************************************  
**  Program  : DSMReditor.js, part of DSMRloggerAPI
**  Version  : v0.1.0
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
  window.onfocus = function() {
    if (needReload) {
      window.location.reload(true);
    }
  };
    
  
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

    openTab("tabSettings");
    
    //---- update buttons in navigation bar ---
    let x = document.getElementsByClassName("editButton");
    for (var i = 0; i < x.length; i++) {
      x[i].style.background     = settingBgColor;
      x[i].style.border         = 'none';
      x[i].style.textDecoration = 'none';  
      x[i].style.outline        = 'none';  
      x[i].style.boxShadow      = 'none';
    }
/*
    document.getElementById("DT1").addEventListener('focus',      function() {hoverOn('DT1');});
    document.getElementById("DT1").addEventListener('blus',       function() {hoverOn('DT1');});
    document.getElementById("DT1").addEventListener('mouseover',  function() {hoverOn('DT1');});
    document.getElementById("DT1").addEventListener('mouseout',   function() {hoverOff('DT1');});
    
    document.getElementById("DT2").addEventListener('focus',      function() {hoverOn('DT2');});
    document.getElementById("DT2").addEventListener('blur',       function() {hoverOff('DT2');});
    document.getElementById("DT2").addEventListener('mouseover',  function() {hoverOn('DT2');});
    document.getElementById("DT2").addEventListener('mouseout',   function() {hoverOff('DT2');});

    document.getElementById("RT1").addEventListener('focus',      function() {hoverOn('RT1');});
    document.getElementById("RT1").addEventListener('blur',       function() {hoverOff('RT1');});
    document.getElementById("RT1").addEventListener('mouseover',  function() {hoverOn('RT1');});
    document.getElementById("RT1").addEventListener('mouseout',   function() {hoverOff('RT1');});

    document.getElementById("RT2").addEventListener('focus',      function() {hoverOn('RT2');});
    document.getElementById("RT2").addEventListener('blur',       function() {hoverOff('RT2');});
    document.getElementById("RT2").addEventListener('mouseover',  function() {hoverOn('RT2');});
    document.getElementById("RT2").addEventListener('mouseout',   function() {hoverOff('RT2');});
    
    document.getElementById("ENBK").addEventListener('focus',     function() {hoverOn('ENBK');});
    document.getElementById("ENBK").addEventListener('blur',      function() {hoverOff('ENBK');});
    document.getElementById("ENBK").addEventListener('mouseover', function() {hoverOn('ENBK');});
    document.getElementById("ENBK").addEventListener('mouseout',  function() {hoverOff('ENBK');});
    
    document.getElementById("GAST").addEventListener('focus',     function() {hoverOn('GAST');});
    document.getElementById("GAST").addEventListener('blur',      function() {hoverOff('GAST');});
    document.getElementById("GAST").addEventListener('mouseover', function() {hoverOn('GAST');});
    document.getElementById("GAST").addEventListener('mouseout',  function() {hoverOff('GAST');});
    
    document.getElementById("GNBK").addEventListener('focus',     function() {hoverOn('GNBK');});
    document.getElementById("GNBK").addEventListener('blur',      function() {hoverOff('GNBK');});
    document.getElementById("GNBK").addEventListener('mouseover', function() {hoverOn('GNBK');});
    document.getElementById("GNBK").addEventListener('mouseout',  function() {hoverOff('GNBK');});
*/

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
    console.log("openTab: now set all fields in ["+tabName+"] to block ..");
    document.getElementById(tabName).style.background='white';
    document.getElementById(tabName).style.display = "block";  
    if (tabName == "tabMonths") {
      console.log("newTab: tabMonths");

    } else if (tabName == "tabSettings") {
      console.log("newTab: tabSettings");
      
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
    	  for( let i in data ){
    	  /***
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
			 ***/
						if (data[i].name == "FwVersion")
						{
							document.getElementById('devVersion').innerHTML = json.devinfo[i].value;
						} else if (data[i].name == 'Hostname')
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
  }	// refreshDevInfo()

  
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
  }	// refreshDevTime()

  
  function editSettings(payload) {
    singlePair   = payload.split(",");
    for ( var i = 1; i < singlePair.length; i++ ) {
      onePair = singlePair[i].split("=");
      console.log("setting ["+onePair[0]+"] value ["+onePair[1]+"]");
      if (   onePair[0].trim() == "DT1" || onePair[0].trim() == "DT2"
          || onePair[0].trim() == "RT1" || onePair[0].trim() == "RT2"       
          || onePair[0].trim() == "GAST"      
          || onePair[0].trim() == "ENBK" || onePair[0].trim() == "GNBK" 
          || onePair[0].trim() == "Interval" || onePair[0].trim() == "SleepTime")
            document.getElementById(onePair[0].trim()).value = onePair[1].trim() * 1;
      else  document.getElementById(onePair[0].trim()).value = onePair[1].trim();
      document.getElementById(onePair[0].trim()).style.color = settingFontColor;

    }

  } // editSettings
  
  
  function undoReload() {
    if (activeTab == "tabMonths") {
      console.log("sendMonths");

    } else if (activeTab == "tabSettings") {
      console.log("undoReload(): reload Settings..");
      console.log("sendSettings");

    } else {
      console.log("undoReload(): I don't knwo what to do ..");
    }

  } // undoRelaod()
  
  
  function setDebugMode() {
    if (document.getElementById('debug').checked)  {
      console.log("DebugMode checked!");
      document.getElementById('logWindow').style.display = "block";
    } else {
      console.log("DebugMode unchecked");
      document.getElementById('logWindow').style.display = "none";
    }
  } // setDebugMode()
   
  
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
  
  function setEditType(eType) {
    if (eType == "D") {
      console.log("Edit Energy Delivered!");
      monthType = eType;
    } else if (eType == "R") {
      console.log("Edit Energy Returned!");
      monthType = eType;
    } else if (eType == "G") {
      console.log("Edit Gas Delivered!");
      monthType = eType;
    } else {
      console.log("setEditType to ["+eType+"] is quit shitty!");
      monthType = "";
    }
    console.log("sendMonths");

  } // setEditType()

  function round(value, precision) {
    var multiplier = Math.pow(10, precision || 0);
    return Math.round(value * multiplier) / multiplier;
  }

  function setStyle(cell, width, Align, Border, Val) {
    cell.style.width = width + 'px';
    cell.style.textAlign = Align;
    cell.style.border = "none";
    cell.style.borderLeft = Border + " solid black";
    cell.style.color = settingFontColor;
    cell.style.fontWeight = "normal";
    cell.innerHTML = Val;
    return cell;
    
  };  // setStyle()
  
  function cssrules() {
      var rules = {};
      for (var i=0; i<document.styleSheets.length; ++i) {
        var cssRules = document.styleSheets[i].cssRules;
        for (var j=0; j<cssRules.length; ++j)
          rules[cssRules[j].selectorText] = cssRules[j];
      }
      return rules;
  } // cssrules()

  function css_getclass(name) {
    var rules = cssrules();
    if (!rules.hasOwnProperty(name))
        throw 'TODO: deal_with_notfound_case';
    return rules[name];
  } // css_getclass()

  function hoverOn(field) {
    document.getElementById(field).style.background = "lightgray";
  } // hoverOn()
  function hoverOff(field) {
    document.getElementById(field).style.background = settingBgColor;
  } // hoverOff()

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
