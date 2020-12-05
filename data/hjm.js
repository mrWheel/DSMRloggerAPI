const APIGW=window.location.protocol+'//'+window.location.host+'/api/';
const AMPS=25
const PHASES=3

var AmpG = [4];

var PhaseAmps = [4];
var MaxAmps = [4];

var TotalAmps=0.0, 
    minKW = [4], 
    maxKW = [4];


var gaugeOptions = {

    chart: {
        type: 'solidgauge'
    },

    title: null,

    pane: {
        center: ['50%', '75%'],
        size: '100%',
        startAngle: -90,
        endAngle: 90,
        background: {
            backgroundColor:
                Highcharts.defaultOptions.legend.backgroundColor || '#EEE',
            innerRadius: '80%',
            outerRadius: '105%',
            shape: 'arc'
            //opacity: '30%'
        }
    },

    tooltip: {
        enabled: false
    },

    // the value axis
    yAxis: {
        stops: [
            [0.1, '#55BF3B'], // green
            [0.5, '#DDDF0D'], // yellow
            [0.9, '#DF5353'] // red
        ],
        lineWidth: 1,
        //minorTickInterval: 'auto',
        minorTicks: true,
        // minorTicksWidth: 3px,
        tickAmount: 6,
        title: {
            y: -70
        },
        labels: {
            y: 16
        }
    },

    plotOptions: {
        solidgauge: {
            innerRadius: '85%',
            opacity: '60%',
            dataLabels: {
                y: 5,
                borderWidth: 0,
                useHTML: true
            }
        }
    }
};

var AmpOptions = {
    yAxis: {
        min: 00,
        max: 25,
    },

    credits: {
        enabled: false
    },

    series: [{
        name: 'A',
        data: [0],
        dataLabels: {
            format:
                '<div style="text-align:center">' +
                '<span style="font-weight:lighter;font-size:16px;font-family: Dosis">{y} A</span><br/>' +
                '</div>'
        }
    },{

        name: 'Max',
        data: [1],
        innerRadius:'100%',
        radius: '105%',
        dataLabels: {
            enabled: false
       }
    }]

};

function abs(x)
{
    return (x < 0 ? -x : x);
}



function update()
{
    var phase;
    for( phase=1 ; phase <= PHASES ; phase++ )
    {
        fetch(APIGW+"v1/sm/fields/power_delivered_l"+phase)
        .then(response => response.json())
        .then(json => 
          {
            for( let j in json.fields ){
                // console.log(json.fields[j].name+" -- "+ "power_delivered_l"+i.toString())
                if (json.fields[j].name.startsWith("power_delivered_l"))
                {

                    myPhase = Number(json.fields[j].name.replace('power_delivered_l',''));

                    let cvKW=document.getElementById(json.fields[j].name).innerHTML
                    let nvKW=Number(json.fields[j].value)
                    let nvA=nvKW*1000.0/220.0  // estimated amps using fixed voltage

                    // console.log("about to get elements");
                    // update view

                    //document.getElementById(json.fields[j].name+"p").innerHTML = cv.toString();
                    document.getElementById(json.fields[j].name).innerHTML = nvKW.toFixed(3);
                    //document.getElementById(json.fields[j].name+"a").innerHTML = nva.toString();

                    if (minKW[myPhase] == 0.0 || nvKW < minKW[myPhase])
                    {
                        minKW[myPhase] = nvKW                      
                        console.log(`power_delivered_${myPhase}min`);

                        document.getElementById(`power_delivered_${myPhase}min`).innerHTML = nvKW.toFixed(3);
                    }
                        

                    if (nvKW> maxKW[myPhase])
                    {
                        maxKW[myPhase] = nvKW;
                        console.log(`power_delivered_${myPhase}max`);
                        document.getElementById(`power_delivered_${myPhase}max`).innerHTML = nvKW.toFixed(3);
                    }

		            // update gauge with actual values

                    var chart,
                        point,
                        newValue,
                        myPhase;
                    
                    
                    chart = AmpG[myPhase];
                    point = chart.series[0].points[0];
		               
                    newValue = Math.round(nvA*1000.0 ) /1000.0;
                    
                    point.update(newValue);
                    
                    if (nvA > MaxAmps[myPhase])
                    {
                        // console.log('new record');
                        MaxAmps[myPhase] = nvA;
                        point = AmpG[myPhase].series[1].points[0];
                        point.update(Math.round(MaxAmps[myPhase]));

                    } 
                    
                    // trend coloring

                    //console.log("About to color headings");

                    if(abs(cvKW - nvKW) < 0.005) // don't highlight small changes < 5W
                    {
                        //console.log("value remained the same")
                        document.getElementById(json.fields[j].name+"h").style.background="#314b77"
                    } else if( nvKW > cvKW )
                    {
                        //console.log(json.fields[j].name+"=verhoogd")
                        document.getElementById(json.fields[j].name+"h").style.background="Red"
                    } else {
                        //console.log(json.fields[j].name+"=verlaagd")
                        document.getElementById(json.fields[j].name+"h").style.background="Green"
                    }

                    PhaseAmps[myPhase] = nvA;
                }
            }
          });
        //.catch(function(error) {
        //    var p = document.createElement('p');
        //    p.appendChild(
        //    document.createTextNode('Error: ' + error.message)
        //    );
        //}
        //);
        
    }

    TotalAmps = 0.0;
    for(t=1; t<= PHASES ; t++)
        TotalAmps += PhaseAmps[t];

    if(TotalAmps == NaN)
        TotalAmps = 0.0

    let TotalKW = TotalAmps * 220.0 / 1000.0

    // console.log(minTotal, maxTotal,TotalAmps)
    if (minKW[4] == 0.0 || TotalKW < minKW[4])
    {
        minKW[4] = TotalKW;
        document.getElementById("power_delivered_tmin").innerHTML = TotalKW.toFixed(3);
    }
        

    if (TotalKW > maxKW[4])
    {
        maxKW[4] = TotalKW;
        document.getElementById("power_delivered_tmax").innerHTML = TotalKW.toFixed(3);
    }
    
    document.getElementById("power_delivered_t").innerHTML = TotalKW.toFixed(3);

    point = AmpG[4].series[0].points[0];
    point.update(Math.round(TotalAmps*1000.0 ) /1000.0);

    if( TotalAmps >  MaxAmps[4])
    {
        MaxAmps[4] = TotalAmps;
        point = AmpG[4].series[1].points[0];
        point.update(Math.round(TotalAmps*1000.0 ) /1000.0);

    }
}

AmpG[1] = Highcharts.chart('container-1', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[2] = Highcharts.chart('container-2', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[3] = Highcharts.chart('container-3', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[4] = Highcharts.chart('container-t', Highcharts.merge(gaugeOptions, {
                yAxis: {
                    min: 00,
                    max: 3*AMPS,
                    
                },

                credits: {
                    enabled: false
                },

                series: [{
                    name: 'A',
                    data: [0],
                    dataLabels: {
                        format:
                            '<div style="text-align:center">' +
                            '<span style="font-weight:lighter;font-size:16px;font-family: Dosis">{y} A</span><br/>' +
                            '</div>'
                    },
                    dial: {            
                        rearLength: '5%'
                      
                    }
                },{
            
                    name: 'Max',
                    data: [1],
                    innerRadius:'100%',
                    radius: '105%',
                    dataLabels: {
                        enabled: false
                   }
                }]

            }));

for(i=1 ; i <= PHASES+1 ; i++)
{
    MaxAmps[i] = 0.0;
    PhaseAmps[i] = 0.0;
    minKW [i] = 0.0;
    maxKW [i] = 0.0;
}



