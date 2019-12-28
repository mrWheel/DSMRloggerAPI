/*
***************************************************************************  
**  Program  : DSMRindex.js, part of DSMRfirmwareAPI
**  Version  : v0.0.8
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
  const APIGW='http://'+window.location.host+'/api/';

"use strict";

  let needReload  = true;
  let activeTab   = "none";
  let TimerTab;
 	
 	var data = [];
  
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
    
    document.getElementById('bActual').addEventListener('click',function()
                                                {openTab('Actual');});
    document.getElementById('bLastHours').addEventListener('click',function() 
                                                {openTab('LastHours');});
    document.getElementById('bLastDays').addEventListener('click',function() 
                                                {openTab('LastDays');});
    document.getElementById('bLastMonths').addEventListener('click',function() 
                                                {openTab('LastMonths');});
    document.getElementById('bTelegram').addEventListener('click',function() 
                                                {openTab('Telegram');});
    document.getElementById('bSysInfo').addEventListener('click',function() 
                                                {openTab('SysInfo');});
    document.getElementById('restAPI').addEventListener('click',function() 
                                                { console.log("newPage: goAPI");
                                                  location.href = "/api";
                                                });
    document.getElementById('FSexplorer').addEventListener('click',function() 
                                                { console.log("newTab: goFSexplorer");
                                                  location.href = "/FSexplorer";
                                                });
    needReload = false;
    refreshDevTime();
    refreshDevInfo();
    TimerTime = setInterval(refreshDevTime, 10 * 1000); // repeat every 10s

    openTab("Actual");
  
  } // bootsTrap()
  
  //============================================================================  
  function openTab(tabName) {
    
    activeTab = tabName;
    clearInterval(TimerTab);  
    
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
      refreshSmFields();
      TimerTab = setInterval(refreshSmFields, 60 * 1000); // repeat every 60s

    } else if (tabName == "LastHours") {
      console.log("newTab: LastHours");
      refreshHours();
      TimerTab = setInterval(refreshHours, 60 * 1000); // repeat every 60s

    } else if (tabName == "LastDays") {
      console.log("newTab: LastDays");
      refreshDays();
      TimerTab = setInterval(refreshDays, 60 * 1000); // repeat every 60s

    } else if (tabName == "LastMonths") {
      console.log("newTab: LastMonths");
      refreshMonths();
      TimerTab = setInterval(refreshMonths, 60 * 1000); // repeat every 60s
    
    } else if (tabName == "SysInfo") {
      console.log("newTab: SysInfo");
      refreshDevInfo();
      TimerTab = setInterval(refreshDevInfo, 60 * 1000); // repeat every 30s

    } else if (tabName == "Telegram") {
      console.log("newTab: Telegram");
      refreshSmTelegram();
      TimerTab = setInterval(refreshSmTelegram, 60 * 1000); // repeat every 60s
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
  
  
  //============================================================================  
  function refreshSmFields()
  {
    fetch(APIGW+"v1/sm/fields")
      .then(response => response.json())
      .then(json => {
      		//console.log("parsed .., fields is ["+ JSON.stringify(json)+"]");
      		data = json.fields;
      		for (var i in data) {
      			var tableRef = document.getElementById('actualTable').getElementsByTagName('tbody')[0];
      			if( ( document.getElementById("actualTable_"+data[i].name)) == null )
      			{
      				var newRow   = tableRef.insertRow();
							newRow.setAttribute("id", "actualTable_"+data[i].name, 0);
							// Insert a cell in the row at index 0
							var newCell  = newRow.insertCell(0);
						  var newText  = document.createTextNode('');
							newCell.appendChild(newText);
							newCell  = newRow.insertCell(1);
							newCell.appendChild(newText);
							newCell  = newRow.insertCell(2);
							newCell.appendChild(newText);
						}
						tableCells = document.getElementById("actualTable_"+data[i].name).cells;
						tableCells[0].innerHTML = data[i].name;
						tableCells[1].innerHTML = data[i].value;
	     			if (data[i].hasOwnProperty('unit'))
	     			{
		     			tableCells[1].style.textAlign = "right";
							tableCells[2].innerHTML = data[i].unit;
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
  };	// refreshSmFields()
  
  
  //============================================================================  
  function refreshHours()
  {
 		console.log("fetch("+APIGW+"v1/hist/hours/asc)");
    fetch(APIGW+"v1/hist/hours/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
      	console.log("Ok, now processJson() for Hours");
      	//console.log(json);
      	data = json.hours;
      	//console.log(data);
	      processJson(data, "Hours");
  		})
  		.catch(function(error) {
      	var p = document.createElement('p');
      	p.appendChild(
        	document.createTextNode('Error: ' + error.message)
      	);
      });	
  }	// resfreshHours()
  
	
  //============================================================================  
  function refreshDays()
  {
 		console.log("fetch("+APIGW+"v1/hist/days/asc)");
    fetch(APIGW+"v1/hist/days/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
      	console.log("Ok, now processJson() for Days");
      	data = json.days;
      	//console.log(data);
	      processJson(data, "Days");
  		})
  		.catch(function(error) {
      	var p = document.createElement('p');
      	p.appendChild(
        	document.createTextNode('Error: ' + error.message)
      	);
  		});
	}	// resfreshDays()
  
	
  //============================================================================  
  function refreshMonths()
  {
 		console.log("fetch("+APIGW+"v1/hist/months/asc)");
    fetch(APIGW+"v1/hist/months/asc", {"setTimeout": 2000})
      .then(response => response.json())
      .then(json => {
      	console.log("Ok, now processJson() for Months");
      	//console.log(response);
      	data = json.months;
	      processJson(data, "Months");
  		})
  		.catch(function(error) {
      	var p = document.createElement('p');
      	p.appendChild(
        	document.createTextNode('Error: ' + error.message)
      	);
  		});
	}	// resfreshMonths()

    
  //============================================================================  
  function refreshSmTelegram()
  {
    fetch(APIGW+"v1/sm/telegram")
      .then(response => response.text())
      .then(response => {
        console.log("parsed .., data is ["+ response+"]");
        console.log('-------------------');
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
  }	// refreshSmTelegram()
  		
	
  //============================================================================  
  function processJson(data, type)
  {
  	
     //for (var i in data) 
   	 for (let i=0; i<data.length; i++)
		 {
     	//console.log("processJson(): data["+i+"] => data["+i+"].recid["+data[i].recid+"]");
     	data[i].p_ed = {};
     	data[i].p_er = {};
     	data[i].p_gd = {};
     	if (i < (data.length -1))
     	{
     		data[i].p_ed = (data[i].edt1 +data[i].edt2)-(data[i+1].edt1 +data[i+1].edt2);
     		data[i].p_ed = (data[i].p_ed * 1000).toFixed(0);
     		data[i].p_er = (data[i].ert1 +data[i].ert2)-(data[i+1].ert1 +data[i+1].ert2);
     		data[i].p_er = (data[i].p_er * 1000).toFixed(0);
     	  data[i].p_gd = ( data[i].gdt  -data[i+1].gdt).toFixed(3);
     	}
     	else
     	{
     		data[i].p_ed = (data[i].edt1 +data[i].edt2).toFixed(3);
     		data[i].p_er = (data[i].ert1 +data[i].ert2).toFixed(3);
     	  data[i].p_gd = (data[i].gdt).toFixed(3);
     	}
    }	// for i ..

    showTable(type);
    	
  }	// processJson()

    
  //============================================================================  
  function showTable(type)
  {	
  	console.log("showTable("+type+")");
  	for (let i=0; i<data.length; i++)
  	{
 			//console.log("showTable("+type+"): data["+i+"] => data["+i+"]name["+data[i].recid+"]");
  		var tableRef = document.getElementById('last'+type+'Table').getElementsByTagName('tbody')[0];
    	//if( ( document.getElementById(type +"Table_"+data[i].recid)) == null )
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
				newCell  = newRow.insertCell(4);
				newCell.appendChild(newText);
				newCell  = newRow.insertCell(5);
				newCell.appendChild(newText);
				newCell  = newRow.insertCell(6);
				newCell.appendChild(newText);
			}
			//tableCells = document.getElementById(type+"Table_"+data[i].recid).cells;
			tableCells = document.getElementById(type+"Table_"+type+"_R"+i).cells;
	    tableCells[0].style.textAlign = "left";
			tableCells[0].innerHTML = data[i].recnr;
	    tableCells[1].style.textAlign = "left";
			tableCells[1].innerHTML = data[i].slot;
	    tableCells[2].style.textAlign = "center";
			//tableCells[2].innerHTML = data[i].recid;
//	    let date = "20"+data[i].recid.substring(0,2)+"-"+data[i].recid.substring(2,4)+"-"+data[i].recid.substring(4,6)+" ["+data[i].recid.substring(6,8)+"]";
			tableCells[2].innerHTML = formatDate(type, data[i].recid);
	   	tableCells[3].style.textAlign = "right";
			tableCells[3].innerHTML = data[i].p_ed;
	   	tableCells[4].style.textAlign = "right";
			tableCells[4].innerHTML = data[i].p_er;
	    tableCells[5].style.textAlign = "right";
			tableCells[5].innerHTML = data[i].p_gd;
    };
  }	// showTable()

  
  //============================================================================  
  function createNode(element) {
    return document.createElement(element); // Create the type of element you pass in the parameters
  }

  function append(parent, el) {
    return parent.appendChild(el); // Append the second parameter(element) to the first one
  }
  

  
  //============================================================================  
  function formatDate(type, dateIn) {
  	let dateOut = "";
    if (type == "Hours")
	    date = "20"+dateIn.substring(0,2)+"-"+dateIn.substring(2,4)+"-"+dateIn.substring(4,6)+" ["+dateIn.substring(6,8)+"]";
    else if (type == "Days")
	    date = "20"+dateIn.substring(0,2)+"-"+dateIn.substring(2,4)+"-["+dateIn.substring(4,6)+"]:"+dateIn.substring(6,8);
    else if (type == "Months")
	    date = "20"+dateIn.substring(0,2)+"-["+dateIn.substring(2,4)+"]-"+dateIn.substring(4,6)+":"+dateIn.substring(6,8);
		else
	    date = "20"+dateIn.substring(0,2)+"-"+dateIn.substring(2,4)+"-"+dateIn.substring(4,6)+":"+dateIn.substring(6,8);
    return date;
  }

  
  //============================================================================  
  function round(value, precision) {
    var multiplier = Math.pow(10, precision || 0);
    return Math.round(value * multiplier) / multiplier;
  }

  //============================================================================  
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

  //============================================================================  
  function cssrules() {
      var rules = {};
      for (var i=0; i<document.styleSheets.length; ++i) {
        var cssRules = document.styleSheets[i].cssRules;
        for (var j=0; j<cssRules.length; ++j)
          rules[cssRules[j].selectorText] = cssRules[j];
      }
      return rules;
  } // cssrules()

  //============================================================================  
  function css_getclass(name) {
    var rules = cssrules();
    if (!rules.hasOwnProperty(name))
        throw 'TODO: deal_with_notfound_case';
    return rules[name];
  } // css_getclass()
  
  //============================================================================  
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
