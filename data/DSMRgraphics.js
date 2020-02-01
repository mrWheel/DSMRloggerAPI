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
  function renderEnergyChart(dataSet) {
    console.log("Now in renderEnergyChart() ..");
    
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
//        mode: 'index',
          mode: 'label',
//        displayColors: true,
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
    
  } // renderEnergyChart()
  
  
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
      if (data[i].p_edw > 0) chartData.datasets[0].data[y]  = data[i].p_edw;
      if (data[i].p_erw > 0) chartData.datasets[1].data[y]  = data[i].p_erw * -1.0;

    }
    //--- show canvas
    document.getElementById("dataChart").style.display = "block";

  } // copyDataToChart()
  

  
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
      dateOut = "20"+dateIn.substring(0,2)+"-["+dateIn.substring(2,4)+"]-"+dateIn.substring(4,6)+":"+dateIn.substring(6,8);
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
