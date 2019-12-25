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
  let activeTab   = "none";
  let TimerTab;

  //var requestDevInfo = new XMLHttpRequest();
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
    document.getElementById('bTelegram').addEventListener('click',function() 
                                                {openTab('Telegram');});
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
      			var tableRef = document.getElementById('actualTable').getElementsByTagName('tbody')[0];
      			if( ( document.getElementById("actualTable_"+json.fields[i].name)) == null )
      			{
      				var newRow   = tableRef.insertRow();
							newRow.setAttribute("id", "actualTable_"+json.fields[i].name, 0);
							// Insert a cell in the row at index 0
							var newCell  = newRow.insertCell(0);
						  var newText  = document.createTextNode('');
							newCell.appendChild(newText);
							newCell  = newRow.insertCell(1);
							newCell.appendChild(newText);
							newCell  = newRow.insertCell(2);
							newCell.appendChild(newText);
						}
						tableCells = document.getElementById("actualTable_"+json.fields[i].name).cells;
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
  

	
	let maxI = 0;
 	let minI = 999;
	let id=[];
	let slot=[];
 	let date=[];
 	let edt1=[];
 	let edt2=[];
 	let ert1=[];
 	let ert2=[];
 	let gdt=[];
 /** 
  async function exampleFetch(url, type) {
    const response = await fetch(url)
      .then(response => response.json())
      .then(json => {
       console.log(json);
       processJson(json, type);
       });
	}
 **/
  
  function refreshHours()
  {
		maxI = 0;
 		minI = 999;
  	id.length   = 0;
  	slot.length = 0;
 		date.length = 0;
 		edt1.length = 0;
 		edt2.length = 0;
 		ert1.length = 0;
 		ert2.length = 0;
 		gdt.length  = 0;

 		console.log("fetch("+APIGW+"hist/hours/1)");
    fetch(APIGW+"hist/hours/1", {"setTimeout": 2000})
      .then(response => response.json())
      .then(response => {
      	console.log("Ok, now processJson()");
      	console.log(response);
	      processJson(response, "Hours");
  		//})
  		//.catch(function(error) {
      //	var p = document.createElement('p');
      //	p.appendChild(
      //  	document.createTextNode('Error: ' + error.message)
      //	);
      });	
  }	// resfreshHours()
  
	
  function refreshDays()
  {
		maxI = 0;
 		minI = 999;
  	id.length   = 0;
  	slot.length = 0;
 		date.length = 0;
 		edt1.length = 0;
 		edt2.length = 0;
 		ert1.length = 0;
 		ert2.length = 0;
 		gdt.length  = 0;

  	//for(let p=1; p<=3;p++)
  	//{
  		console.log("fetch("+APIGW+"hist/days/1)");
  		json = exampleFetch(APIGW+"hist/days/1", "Days");
  		console.log("fetch("+APIGW+"hist/days/2)");
  		json = exampleFetch(APIGW+"hist/days/2", "Days");
  		console.log("fetch("+APIGW+"hist/days/3)");
  		json = exampleFetch(APIGW+"hist/days/3", "Days");
  		
	}	// resfreshDays()
  		
	
  function processJson(json, type)
  {
      		for (var i in json.hist) 
      		{
      		  if (json.hist[i].rec > maxI)
      		  {
      		  	maxI = json.hist[i].rec;
      		  	//console.log("rec["+json.hist[i].rec+"] -> maxI["+maxI+"]");
      		  }
      		  if (json.hist[i].rec < minI)
      		  {
      		  	minI = json.hist[i].rec;
      		  	console.log("rec["+json.hist[i].rec+"] -> maxI["+minI+"]");
      		  }
      			id[json.hist[i].rec]   = json.hist[i].rec;
      			slot[json.hist[i].rec]   = json.hist[i].slot;
      			date[json.hist[i].rec] = json.hist[i].date;
      			edt1[json.hist[i].rec] = json.hist[i].edt1;
      			edt2[json.hist[i].rec] = json.hist[i].edt2;
      			ert1[json.hist[i].rec] = json.hist[i].ert1;
      			ert2[json.hist[i].rec] = json.hist[i].ert2;
      			gdt[json.hist[i].rec]  = json.hist[i].gdt;
      			//console.log("rec["+json.hist[i].rec+"] -> id["+id[json.hist[i].rec]+"] Date["+date[json.hist[i].rec]+"]");
      		  //console.log("["+json.hist[i].rec+"]");
      		}	// for i ..
	    		console.log(type+", minI["+minI+"], maxI["+maxI+"]");
	    		if (type == 'Hours')
	    		{
		    		if (minI == 0 && maxI > 70)
		    		{
				  		showTable(type, minI, maxI);
						}
					}
					else if (type == 'Days')
					{
		    		if (minI == 0 && maxI > 14)
		    		{
				  		showTable(type, minI, maxI);
						}
					}
    	
  }	// processJson()
  
  function showTable(type, minI, maxI)
  {	
  	console.log("showTable("+type+", "+minI+", "+maxI+")");
  	for (let i=minI; i<=maxI; i++)
  	{
  	  if (date[i] == null) continue;
  		var tableRef = document.getElementById('last'+type+'Table').getElementsByTagName('tbody')[0];
    	if( ( document.getElementById(type +"Table_"+id[i])) == null )
     	{
     		var newRow   = tableRef.insertRow();
				newRow.setAttribute("id", type+"Table_"+id[i], 0);
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
			tableCells = document.getElementById(type+"Table_"+id[i]).cells;
			tableCells[0].innerHTML = i;
	    tableCells[1].style.textAlign = "right";
			tableCells[1].innerHTML = slot[i];
			tableCells[2].innerHTML = date[i];
	   	tableCells[3].style.textAlign = "right";
			tableCells[3].innerHTML = edt1[i];
	   	tableCells[4].style.textAlign = "right";
			tableCells[4].innerHTML = ert1[i];
	    tableCells[5].style.textAlign = "right";
			tableCells[5].innerHTML = gdt[i];
    };
  }	// showTable()
  

    
  function refreshSmTelegram()
  {
    fetch(APIGW+"sm/telegram")
      .then(response => response.text())
      .then(response => {
        //console.log("parsed .., data is ["+ response+"]");
        //console.log('-------------------');
   			var divT = document.getElementById('rawTelegram');
   			if ( document.getElementById("TelData") == null )
      	{
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
  		console.log(error);
    	});     
  }	// refreshDevInfo()

    
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
//      webSocketConn.send("tabLastMonths");
//      TimerTab=setInterval(function(){
//                    webSocketConn.send("tabLastMonths");
//                  },120000);
    
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
