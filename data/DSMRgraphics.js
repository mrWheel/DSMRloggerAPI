/*
***************************************************************************  
**  Program  : DSMRgraphics.js, part of DSMRloggerAPI
**  Version  : v0.3.1
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

let TimerActual;
let actPoint       = 0;
var actLabel       = "-";

var chartData      = {};     //declare an object
chartData.labels   = [];     // add 'labels' element to object (X axis)
chartData.datasets = [];     // add 'datasets' array element to object


var actualOptions = {
        responsive: true,
        maintainAspectRatio: true,
        scales: {
          yAxes: [{
            ticks : {
              beginAtZero : true
            },
            scaleLabel: {
              display: true,
              labelString: 'kilo Watt',
            },
          }]
        } // scales
      }; // options

var hourOptions = {
        responsive: true,
        maintainAspectRatio: true,
        scales: {
          yAxes: [{
            ticks : {
              beginAtZero : true
            },
            scaleLabel: {
              display: true,
              labelString: 'Watt/Uur',
            },
          }]
        } // scales
      }; // options

var dayOptions = {
        responsive: true,
        maintainAspectRatio: true,
        scales: {
          yAxes: [{
            ticks : {
              beginAtZero : true
            },
            scaleLabel: {
              display: true,
              labelString: 'kWh',
            },
          }]
        } // scales
      }; // options

var monthOptions = {
        responsive: true,
        maintainAspectRatio: true,
        scales: {
          yAxes: [{
            ticks : {
              beginAtZero : true
            },
            scaleLabel: {
              display: true,
              labelString: 'kWh',
            },
          }]
        } // scales
      }; // options


//----------------Energy Chart--------------------------------------------------
var myEnergyChart;

  //============================================================================  
  function renderHistElectrChart(dataSet, options) {
    console.log("Now in renderHistElectrChart() ..");
    
    if (myEnergyChart) {
      myEnergyChart.destroy();
    }

    var ctxEnergy = document.getElementById("dataChart").getContext("2d");
    myEnergyChart = new Chart(ctxEnergy, {
      type: 'bar',
      data: dataSet,
      options: options,
    });
    
  } // renderHistElectrChart()

  
  //============================================================================  
  function renderMonthsElectrChart(dataSet, options) {
    console.log("Now in renderMonthsElectrChart() ..");
    
    if (myEnergyChart) {
      myEnergyChart.destroy();
    }

    var ctxEnergy = document.getElementById("dataChart").getContext("2d");
    myEnergyChart = new Chart(ctxEnergy, {
      type: 'bar',
      data: dataSet,
      options: options,
    });
    
  } // renderMonthsElectrChart()

  
  //============================================================================  
  function renderActualChart(dataSet, options) {
    console.log("Now in renderActualChart() ..");
    
    if (myEnergyChart)
    {
      myEnergyChart.destroy();
    }

    var ctxEnergy = document.getElementById("dataChart").getContext("2d");
    myEnergyChart = new Chart(ctxEnergy, {
      type: 'bar',
      data: dataSet,
      options: options,
    });
    
  } // renderActualChart()
  
  
  //============================================================================  
  function showHistGraph(data, type)
  {
    console.log("Now in showHistGraph()..");
    copyDataToChart(data, type);
    if (type == "Hours")
          renderHistElectrChart(chartData, hourOptions);
    else  renderHistElectrChart(chartData, dayOptions);
    myEnergyChart.update();
  
    //--- hide table
    document.getElementById("lastHours").style.display  = "none";
    document.getElementById("lastDays").style.display   = "none";
    document.getElementById("lastMonths").style.display = "none";
    //--- show canvas
    document.getElementById("dataChart").style.display  = "block";

  } // showHistGraph()
  
  
  //============================================================================  
  function showMonthsGraph(data, type)
  {
    console.log("Now in showMonthsGraph()..");
    copyMonthsToChart(data);
    renderMonthsElectrChart(chartData, monthOptions);
    myEnergyChart.update();
  
    //--- hide table
    document.getElementById("lastHours").style.display  = "none";
    document.getElementById("lastDays").style.display   = "none";
    document.getElementById("lastMonths").style.display = "none";
    //--- show canvas
    document.getElementById("dataChart").style.display  = "block";

  } // showMonthsGraph()
  
  
  //============================================================================  
  function copyDataToChart(data, type)
  {
    console.log("Now in copyDataToChart()..");
    actPoint      = 0;

    chartData     = {};     // empty chartData
    chartData.labels   = [];     // empty .labels
    chartData.stack    = [];     // empty .stack
    chartData.datasets = [];     // empty .datasets
    
    // idx 0 => ED
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[0].fill            = 'false';
    chartData.datasets[0].borderColor     = "red";
    chartData.datasets[0].backgroundColor = "red";
    chartData.datasets[0].data            = []; //contains the 'Y; axis data
    chartData.datasets[0].label           = "Verbruikt"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[0].stack           = "STACK"
    // idx 0 => ER
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[1].fill            = 'false';
    chartData.datasets[1].borderColor     = "green";
    chartData.datasets[1].backgroundColor = "green";
    chartData.datasets[1].data            = []; //contains the 'Y; axis data
    chartData.datasets[1].label           = "Teruggeleverd"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[1].stack           = "STACK"
    
    for(let i=(data.length -2); i>=0; i--)
    {
      let y = (data.length -2) - i;
      //console.log("["+i+"] label["+data[i].recid+"] => y["+y+"]");
      chartData.labels.push(formatGraphDate(type, data[i].recid)); // adds x axis labels (timestamp)
      if (type == "Hours")
      {
        if (data[i].p_edw >= 0) chartData.datasets[0].data[y]  = data[i].p_edw;
        if (data[i].p_erw >= 0) chartData.datasets[1].data[y]  = (data[i].p_erw * -1.0);
      }
      else
      {
        if (data[i].p_ed >= 0) chartData.datasets[0].data[y]  = (data[i].p_ed).toFixed(3);
        if (data[i].p_er >= 0) chartData.datasets[1].data[y]  = (data[i].p_er * -1.0).toFixed(3);
      }

    } // for i ..

  } // copyDataToChart()
  
  
  //============================================================================  
  function copyMonthsToChart(data)
  {
    console.log("Now in copyMonthsToChart()..");
    actPoint      = 0;
    
    chartData     = {};     // empty chartData
    chartData.labels   = [];     // empty .labels
    chartData.stack    = [];     // empty .stack
    chartData.datasets = [];     // empty .datasets
    
    // idx 0 => ED
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[0].fill            = 'false';
    chartData.datasets[0].borderColor     = "red";
    chartData.datasets[0].backgroundColor = "red";
    chartData.datasets[0].data            = []; //contains the 'Y; axis data
    chartData.datasets[0].label           = "Verbruik deze periode"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[0].stack           = "DP"
    // idx 0 => ER
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[1].fill            = 'false';
    chartData.datasets[1].borderColor     = "green";
    chartData.datasets[1].backgroundColor = "green";
    chartData.datasets[1].data            = []; //contains the 'Y; axis data
    chartData.datasets[1].label           = "Terug deze Periode"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[1].stack           = "DP"
    // idx 0 => ED
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[2].fill            = 'false';
    chartData.datasets[2].borderColor     = "orange";
    chartData.datasets[2].backgroundColor = "orange";
    chartData.datasets[2].data            = []; //contains the 'Y; axis data
    chartData.datasets[2].label           = "Verbruik vorige periode"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[2].stack           = "VP"
    // idx 0 => ER
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[3].fill            = 'false';
    chartData.datasets[3].borderColor     = "lightgreen";
    chartData.datasets[3].backgroundColor = "lightgreen";
    chartData.datasets[3].data            = []; //contains the 'Y; axis data
    chartData.datasets[3].label           = "Terug vorige Periode"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[3].stack           = "VP"
    
    console.log("there are ["+data.length+"] rows");
    var showRows = 0;
    var p        = 0;
    if (data.length > 24) showRows = 11;
    else                  showRows = data.length / 2;
    console.log("showRows is ["+showRows+"]");
    for (let i=showRows; i>=0; i--)
    {
      let y = i +12;
      //console.log("i["+i+"] label["+data[i].recid+"] => y["+y+"] label["+data[y].recid+"]");
      chartData.labels.push(formatGraphDate("Months", data[i].recid)); // adds x axis labels (timestamp)
      //chartData.labels.push(p); // adds x axis labels (timestamp)
      if (data[i].p_ed >= 0) chartData.datasets[0].data[p]  = (data[i].p_ed).toFixed(3);
      if (data[i].p_er >= 0) chartData.datasets[1].data[p]  = (data[i].p_er * -1.0).toFixed(3);
      if (data[y].p_ed >= 0) chartData.datasets[2].data[p]  = (data[y].p_ed).toFixed(3);
      if (data[y].p_er >= 0) chartData.datasets[3].data[p]  = (data[y].p_er * -1.0).toFixed(3);
      p++;
    }
    //--- show canvas
    document.getElementById("dataChart").style.display = "block";

  } // copyMonthsToChart()
  
  
  //============================================================================  
  function initActualGraph()
  {
    console.log("Now in initActualGraph()..");
    if (actPoint > 0) return;

    chartData     = {};     // empty chartData
    chartData.labels   = [];     // empty .labels
    chartData.stack    = [];     // empty .stack
    chartData.datasets = [];     // empty .datasets
    
    // idx 0 => EDL1
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[0].fill            = 'false';
    chartData.datasets[0].borderColor     = "red";
    chartData.datasets[0].backgroundColor = "red";
    chartData.datasets[0].data            = []; //contains the 'Y; axis data
    chartData.datasets[0].label           = "Gebruik L1"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[0].stack           = "A"
    
    // idx 1 => EDL2
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[1].fill            = 'false';
    chartData.datasets[1].borderColor     = "tomato";
    chartData.datasets[1].backgroundColor = "tomato";
    chartData.datasets[1].data            = []; //contains the 'Y; axis data
    chartData.datasets[1].label           = "Gebruik L2"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[1].stack           = "A"
    
    // idx 2 => EDL3
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[2].fill            = 'false';
    chartData.datasets[2].borderColor     = "salmon";
    chartData.datasets[2].backgroundColor = "salmon";
    chartData.datasets[2].data            = []; //contains the 'Y; axis data
    chartData.datasets[2].label           = "Gebruik L3"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[2].stack           = "A"
    
    // idx 3 => ERL1
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[3].fill            = 'false';
    chartData.datasets[3].borderColor     = "yellowgreen";
    chartData.datasets[3].backgroundColor = "yellowgreen";
    chartData.datasets[3].data            = []; //contains the 'Y; axis data
    chartData.datasets[3].label           = "Opgewekt L1"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[3].stack           = "A"
    
    // idx 4 => ERL2
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[4].fill            = 'false';
    chartData.datasets[4].borderColor     = "springgreen";
    chartData.datasets[4].backgroundColor = "springgreen";
    chartData.datasets[4].data            = []; //contains the 'Y; axis data
    chartData.datasets[4].label           = "Opgewekt L2"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[4].stack           = "A"
    
    // idx 5 => ERL3
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[5].fill            = 'false';
    chartData.datasets[5].borderColor     = "green";
    chartData.datasets[5].backgroundColor = "green";
    chartData.datasets[5].data            = []; //contains the 'Y; axis data
    chartData.datasets[5].label           = "Opgewekt L3"; //"S"+s; //contains the 'Y; axis label
    chartData.datasets[5].stack           = "A"

    actPoint = 0;
  
  } // initActualGraph()
  
  
  //============================================================================  
  function showActualGraph(data)
  {
    console.log("Now in showActualGraph()..");
    if (actPoint == 0)  initActualGraph();
    
    for (i in data)
    {
      //console.log("["+i+"] name["+data[i].name+"]");
      if (data[i].name == "timestamp")  
      {
        //console.log("i["+i+"] label["+data[i].value+"]");
        if (data[i].value == actLabel)
        {
          console.log("actLabel["+actLabel+"] == value["+data[i].value+"] =>break!");
          actPoint--; // 
          break; // skip all the rest
        }
        chartData.labels.push(formatGraphDate("Actual", data[i].value)); // adds x axis labels (timestamp)
        actLabel = data[i].value;
      }
      
      if (data[i].name == "power_delivered_l1") 
        chartData.datasets[0].data[actPoint]  = (data[i].value).toFixed(3);
      if (data[i].name == "power_delivered_l2") 
        chartData.datasets[1].data[actPoint]  = (data[i].value).toFixed(3);
      if (data[i].name == "power_delivered_l3") 
        chartData.datasets[2].data[actPoint]  = (data[i].value).toFixed(3);
      if (data[i].name == "power_returned_l1")  
        chartData.datasets[3].data[actPoint]  = (data[i].value * -1.0).toFixed(3);
      if (data[i].name == "power_returned_l2")  
        chartData.datasets[4].data[actPoint]  = (data[i].value * -1.0).toFixed(3);
      if (data[i].name == "power_returned_l3")  
        chartData.datasets[5].data[actPoint]  = (data[i].value * -1.0).toFixed(3);
    } // for i in data ..
    actPoint++;    
    
    if (actPoint > 3) 
    {
      chartData.labels.shift();
      chartData.datasets[0].data.shift();
      chartData.datasets[1].data.shift();
      chartData.datasets[2].data.shift();
      chartData.datasets[3].data.shift();
      chartData.datasets[4].data.shift();
      chartData.datasets[5].data.shift();
      actPoint--;
    }
    
    renderActualChart(chartData, actualOptions);
    myEnergyChart.update();

    //--- hide Table
    document.getElementById("actual").style.display    = "none";
    //--- show canvas
    document.getElementById("dataChart").style.display = "block";

  } // showActualGraph()
  
  
  //============================================================================  
  function formatGraphDate(type, dateIn) 
  {
    let dateOut = "";
    if (type == "Hours")
    {
      dateOut = "("+dateIn.substring(4,6)+") "+dateIn.substring(6,8);
    }
    else if (type == "Days")
      dateOut = [recidToWeekday(dateIn), dateIn.substring(4,6)+"-"+dateIn.substring(2,4)];
    else if (type == "Months")
    {
      let MM = parseInt(dateIn.substring(2,4))
      dateOut = monthNames[MM];
    }
    else if (type == "Actual")
    {
      dateOut = dateIn.substring(6,8)+":"+dateIn.substring(8,10)+":"+dateIn.substring(10,12);
    }
    else
      dateOut = "20"+dateIn.substring(0,2)+"-"+dateIn.substring(2,4)+"-"+dateIn.substring(4,6)+":"+dateIn.substring(6,8);
    
    return dateOut;
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
