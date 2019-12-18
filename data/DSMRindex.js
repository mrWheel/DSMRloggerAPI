/*
***************************************************************************  
**  Program  : DSMRindex.js, part of DSMRfirmwareAPI
**  Version  : v0.0.1
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
  const APIGW='http://'+window.location.host+'/api/v1/';

"use strict";

  let needReload  = true;
  let validJson   = false;
  let activeTab   = "none";
  let jsonMessage;   
  let singlePair;
  let onePair;
  let daysMaxRows, hoursMaxRows, monthsMaxRows;
  let TimerTab;
  let chartType         = 'bar';
  let graphType         = 'W';
  let settingBgColor    = 'deepskyblue';
  let settingFontColor  = 'white'
  let settingBackEDC    = "red";
  let settingLineEDC    = "red";
  let settingBackERC    = "green";
  let settingLineERC    = "green";
  let settingBackGDC    = "blue";
  let settingLineGDC    = "blue";
  let settingBackED2C   = "tomato";
  let settingLineED2C   = "tomato";
  let settingBackER2C   = "lightgreen";
  let settingLineER2C   = "lightgreen";
  let settingBackGD2C   = "lightblue";
  let settingLineGD2C   = "lightblue";
  let settingBackPR123C = "green";
  let settingLinePR123C = "black";
  let settingBackPD1C   = "yellow";
  let settingLinePD1C   = "black";
  let settingBackPD2C   = "lightgreen";
  let settingLinePD2C   = "black";
  let settingBackPD3C   = "lime";
  let settingLinePD3C   = "black";

  var requestDevInfo = new XMLHttpRequest();
  //var requestSmFields = new XMLHttpRequest();
  
  window.onload=bootsTrap;
  window.onfocus = function() {
    if (needReload) {
      window.location.reload(true);
    }
  };
    
  
  function bootsTrap() {
    console.log("bootsTrap()");
    needReload = true;
    
    document.getElementById('bActual').addEventListener('click',function()
                                                {openTab('Actual');});
    document.getElementById('bLastHours').addEventListener('click',function() 
                                                {openTab('LastHours');});
    document.getElementById('bLastDays').addEventListener('click',function() 
                                                {openTab('LastDays');});
    document.getElementById('bLastMonths').addEventListener('click',function() 
                                                {openTab('LastMonths');});
    document.getElementById('bGraphics').addEventListener('click',function() 
                                                {openTab('Graphics');});
    document.getElementById('bSysInfo').addEventListener('click',function() 
                                                {openTab('SysInfo');});
    document.getElementById('FSexplorer').addEventListener('click',function() 
                                                { console.log("newTab: goFSexplorer");
                                                  location.href = "/FSexplorer";
                                                });
    let count = 0;
    /*
    while (document.getElementById('devVersion').value == "[version]") {
      count++;
      console.log("count ["+count+"] devVersion is ["+document.getElementById('devVersion').value+"]");
      if (count > 10) {
        alert("Count="+count+" => reload from server!");
        window.location.reload(true);
      }
      setTimeout("", 500);
    }
    */
    needReload = false;
    openTab("Actual");
  
  } // bootsTrap()
  
  function refreshDevInfo()
  {
    fetch(APIGW+"dev/info")
      .then(response => response.json())
      .then(response => {
        console.log("parsed .., data is ["+ JSON.stringify(response)+"]");
    	  for( let fld in response ){
    		  console.log( "Fld["+fld+"], value ["+response[fld]+"]" );
    	  }
        console.log('-------------------');
      })

      .catch(function(error) {
  		console.log(error);
    	});     
  }	// refreshDevInfo()
  
  function refreshSmFields()
  {
    fetch(APIGW+"sm/fields")
      .then(response => response.json())
      .then(json => {
      		//console.log("parsed .., fields is ["+ JSON.stringify(json)+"]");
      		
      		for (var i in json.fields) {
      		/*
      			if (json.fields[i].hasOwnProperty('unit'))
      						console.log(">>"+ json.fields[i].name + ": " + json.fields[i].value +", "+json.fields[i].unit);
						else	console.log(">>"+ json.fields[i].name + ": " + json.fields[i].value);
					*/	
      			var tableRef = document.getElementById('lastHoursTable').getElementsByTagName('tbody')[0];
      			if( ( document.getElementById("hoursTable_"+json.fields[i].name)) == null )
      			{
      				var newRow   = tableRef.insertRow();
							newRow.setAttribute("id", "hoursTable_"+json.fields[i].name, 0);
							// Insert a cell in the row at index 0
							var newCell  = newRow.insertCell(0);
						  var newText  = document.createTextNode('');
							newCell.appendChild(newText);
							newCell  = newRow.insertCell(1);
							newCell.appendChild(newText);
							newCell  = newRow.insertCell(2);
							newCell.appendChild(newText);
						}
						tableCells = document.getElementById("hoursTable_"+json.fields[i].name).cells;
						tableCells[0].innerHTML = json.fields[i].name;
						tableCells[1].innerHTML = json.fields[i].value;
	     			if (json.fields[i].hasOwnProperty('unit'))
	     			{
		     			tableCells[1].style.textAlign = "right";
							tableCells[2].innerHTML = json.fields[i].unit;
						}
      		}
      		//console.log("-->done..");
      })
      .catch(function(error) {
  		  console.log(error);
    	});     
  };	// refreshSmFields()
  
  function openTab(tabName) {
    
    activeTab = tabName;
    clearInterval(TimerTab);  
    
    let bID = "b" + tabName;
    let i;
    //---- update buttons in navigation bar ---
    let x = document.getElementsByClassName("tabButton");
    for (i = 0; i < x.length; i++) {
      x[i].style.background     = settingBgColor;
      x[i].style.border         = 'none';
      x[i].style.textDecoration = 'none';  
      x[i].style.outline        = 'none';  
      x[i].style.boxShadow      = 'none';
    }
    //--- hide all tab's -------
    x = document.getElementsByClassName("tabName");
    for (i = 0; i < x.length; i++) {
      x[i].style.display    = "none";  
    }
    //--- and set active tab to 'block'
    console.log("now set ["+bID+"] to block ..");
    document.getElementById(bID).style.background='lightgray';
    document.getElementById(tabName).style.display = "block";  
    if (tabName == "Actual") {
      console.log("newTab: Actual");

    } else if (tabName == "LastHours") {
      console.log("newTab: LastHours");
      refreshSmFields();
      var TimerTab = setInterval(refreshSmFields, 60 * 1000); // repeat every 60s

    } else if (tabName == "LastDays") {
      console.log("newTab: LastDays");
//      webSocketConn.send("tabLastDays");
//      TimerTab=setInterval(function(){
//                    webSocketConn.send("tabLastDays");
//                  },120000);

    } else if (tabName == "LastMonths") {
      console.log("newTab: LastMonths");
//      webSocketConn.send("tabLastMonths");
//      TimerTab=setInterval(function(){
//                    webSocketConn.send("tabLastMonths");
//                  },120000);
    
    } else if (tabName == "SysInfo") {
      console.log("newTab: SysInfo");
      refreshDevInfo();
      var TimerTab = setInterval(refreshDevInfo, 60 * 1000); // repeat every 30s

    } else if (tabName == "Graphics") {
      console.log("newTab: Graphics");
//      webSocketConn.send("tabGraphics");
//      //TimerChart = setInterval(plotChart, 1000, chart);
    }
  } // openTab()

  
  function createNode(element) {
    return document.createElement(element); // Create the type of element you pass in the parameters
  }

  function append(parent, el) {
    return parent.appendChild(el); // Append the second parameter(element) to the first one
  }
  

  
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
  
  function existingId(elementId) {
    if(document.getElementById(elementId)){
      return true;
    } 
    console.log("cannot find elementId [" + elementId + "]");
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
