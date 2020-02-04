/*
***************************************************************************  
**  Program  : DSMRgraphics.js, part of DSMRloggerAPI
**  Version  : v0.3.0
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
var Label       = [];
var Delivered   = [];
var Returned    = []; 
var Gas         = []; 
var Delivered2  = [];
var Returned2   = []; 
var Gas2        = []; 
var DeliveredL1 = [];
var DeliveredL2 = [];
var DeliveredL3 = [];
let TimerActual;

var colors          = [   'Red', 'Blue', 'Green', 'Yellow', 'FireBrick', 'CornflowerBlue', 'Orange'
                        , 'DeepSkyBlue', 'Gray', 'Purple', 'Brown', 'MediumVioletRed', 'LightSalmon'
                        , 'BurlyWood', 'Gold'
                       ];

var chartData     = {};     //declare an object
chartData.labels   = [];     // add 'labels' element to object (X axis)
chartData.datasets = [];     // add 'datasets' array element to object

//----------------Energy Chart--------------------------------------------------
var myEnergyChart;

  //============================================================================  
  function renderHistElectrChart(dataSet) {
    console.log("Now in renderHistElectrChart() ..");
    
    if (myEnergyChart) {
      myEnergyChart.destroy();
    }

    var ctxEnergy = document.getElementById("dataChart").getContext("2d");
    myEnergyChart = new Chart(ctxEnergy, {
      type: 'bar',
      data: dataSet,
      options : {
        responsive: true,
        maintainAspectRatio: true,
        /*
        tooltips: {
          mode: 'index',
          mode: 'label',
          displayColors: true,
          callbacks: {
            label: function(tooltipItem, data) { 
               let temp = parseFloat(tooltipItem.yLabel);
               return data.datasets[tooltipItem.datasetIndex].label+": "+parseFloat(temp).toFixed(1)+"*C";
             }
          }
        },  
        */      
        scales: {
          yAxes: [{
            ticks : {
              beginAtZero : true
            },
            scaleLabel: {
              display: true,
              labelString: 'Watt/Uur',
            },
            //ticks: {
              //max: 35,
              //min: 15,
            //  stepSize: 5,
            //},
          }]
        } // scales
      } // options
    });
    
  } // renderHistElectrChart()

  //============================================================================  
  function renderMonthsElectrChart(dataSet) {
    console.log("Now in renderMonthsElectrChart() ..");
    
    if (myEnergyChart) {
      myEnergyChart.destroy();
    }

    var ctxEnergy = document.getElementById("dataChart").getContext("2d");
    myEnergyChart = new Chart(ctxEnergy, {
      type: 'bar',
      data: dataSet,
      options : {
        responsive: true,
        maintainAspectRatio: true,
        /*
        tooltips: {
          mode: 'index',
          mode: 'label',
          displayColors: true,
          callbacks: {
            label: function(tooltipItem, data) { 
               let temp = parseFloat(tooltipItem.yLabel);
               return data.datasets[tooltipItem.datasetIndex].label+": "+parseFloat(temp).toFixed(1)+"*C";
             }
          }
        },  
        */      
        scales: {
          yAxes: [{
            ticks : {
              beginAtZero : true
            },
            scaleLabel: {
              display: true,
              labelString: 'kWh',
            },
            //ticks: {
              //max: 35,
              //min: 15,
            //  stepSize: 5,
            //},
          }]
        } // scales
      } // options
    });
    
  } // renderMonthsElectrChart()
  
  
  //============================================================================  
  function showHistGraph(data, type)
  {
    console.log("Now in showHistGraph()..");
    copyDataToChart(data, type);
    renderHistElectrChart(chartData);
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
    renderMonthsElectrChart(chartData);
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
    chartData     = {};     // empty chartData
    chartData.labels   = [];     // empty .labels
    chartData.datasets = [];     // empty .datasets
    
    // idx 0 => ED
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[0].fill            = 'false';
    chartData.datasets[0].borderColor     = "red";
    chartData.datasets[0].backgroundColor = "red";
    chartData.datasets[0].data            = []; //contains the 'Y; axis data
    chartData.datasets[0].label           = "ED"; //"S"+s; //contains the 'Y; axis label
    // idx 0 => ER
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[1].fill            = 'false';
    chartData.datasets[1].borderColor     = "green";
    chartData.datasets[1].backgroundColor = "green";
    chartData.datasets[1].data            = []; //contains the 'Y; axis data
    chartData.datasets[1].label           = "ER"; //"S"+s; //contains the 'Y; axis label
    
    for(let i=(data.length -2); i>=0; i--)
    {
      let y = (data.length -2) - i;
      console.log("["+i+"] label["+data[i].recid+"] => y["+y+"]");
      chartData.labels.push(formatDateShort(type, data[i].recid)); // adds x axis labels (timestamp)
      if (data[i].p_edw >= 0) chartData.datasets[0].data[y]  = data[i].p_edw;
      if (data[i].p_erw >= 0) chartData.datasets[1].data[y]  = data[i].p_erw;

    }
    //--- show canvas
    //document.getElementById("dataChart").style.display = "block";

  } // copyDataToChart()
  
  
  //============================================================================  
  function copyMonthsToChart(data)
  {
    console.log("Now in copyMonthsToChart()..");
    chartData     = {};     // empty chartData
    chartData.labels   = [];     // empty .labels
    chartData.datasets = [];     // empty .datasets
    
    // idx 0 => ED
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[0].fill            = 'false';
    chartData.datasets[0].borderColor     = "red";
    chartData.datasets[0].backgroundColor = "red";
    chartData.datasets[0].data            = []; //contains the 'Y; axis data
    chartData.datasets[0].label           = "Verbruik deze periode"; //"S"+s; //contains the 'Y; axis label
    // idx 0 => ER
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[1].fill            = 'false';
    chartData.datasets[1].borderColor     = "green";
    chartData.datasets[1].backgroundColor = "green";
    chartData.datasets[1].data            = []; //contains the 'Y; axis data
    chartData.datasets[1].label           = "Terug deze Periode"; //"S"+s; //contains the 'Y; axis label
    // idx 0 => ED
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[2].fill            = 'false';
    chartData.datasets[2].borderColor     = "orange";
    chartData.datasets[2].backgroundColor = "orange";
    chartData.datasets[2].data            = []; //contains the 'Y; axis data
    chartData.datasets[2].label           = "Verbruik vorige periode"; //"S"+s; //contains the 'Y; axis label
    // idx 0 => ER
    chartData.datasets.push({}); //create a new dataset
    chartData.datasets[3].fill            = 'false';
    chartData.datasets[3].borderColor     = "lightgreen";
    chartData.datasets[3].backgroundColor = "lightgreen";
    chartData.datasets[3].data            = []; //contains the 'Y; axis data
    chartData.datasets[3].label           = "Terug vorige Periode"; //"S"+s; //contains the 'Y; axis label
    
    console.log("there are ["+data.length+"] rows");
    let p = 0;
    var showRows = 0;
    if (data.length > 24) showRows = 11;
    else                  showRows = data.length / 2;
    console.log("showRows is ["+showRows+"]");
    for (let i=showRows; i>=0; i--)
    {
      let y = i +12;
      console.log("i["+i+"] label["+data[i].recid+"] => y["+y+"] label["+data[y].recid+"]");
      chartData.labels.push(formatDateShort("Months", data[i].recid)); // adds x axis labels (timestamp)
      //chartData.labels.push(p); // adds x axis labels (timestamp)
      if (data[i].p_ed >= 0) chartData.datasets[0].data[p]  = (data[i].p_ed).toFixed(3);
      if (data[i].p_er >= 0) chartData.datasets[1].data[p]  = (data[i].p_er).toFixed(3);
      if (data[y].p_ed >= 0) chartData.datasets[2].data[p]  = (data[y].p_ed).toFixed(3);
      if (data[y].p_er >= 0) chartData.datasets[3].data[p]  = (data[y].p_er).toFixed(3);
      p++;

    }
    //--- show canvas
    document.getElementById("dataChart").style.display = "block";

  } // copyMonthsToChart()
  

  
  //============================================================================  
  function formatDateShort(type, dateIn) 
  {
    let dateOut = "";
    if (type == "Hours")
    {
      dateOut = "("+dateIn.substring(4,6)+") "+dateIn.substring(6,8);
    }
    else if (type == "Days")
      dateOut = dateIn.substring(4,6)+"-"+dateIn.substring(2,4);
    else if (type == "Months")
    {
      let MM = parseInt(dateIn.substring(2,4))
      dateOut = MM;
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
