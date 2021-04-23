const APIGW=window.location.protocol+'//'+window.location.host+'/api/';
const AMPS = 25
const PHASES = 3

var AmpG = [4];
var PhaseAmps = [4];
var MaxAmps = [4];
var TotalAmps = 0.0, 
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
        //tickAmount: 0,
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
        min: -25,
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

function getFieldByName(json, prefix, factor = 1) {
    for (const field of json.fields) {
        if (field.name.startsWith(prefix)) {
             return {
                 name: field.name,
                 value: factor * Number(field.value)
             };
        }
    }
    throw new Error(`Did not find field '${prefix}' in the JSON`);
}

function update()
{
    var phase;
    for( let phase = 1 ; phase <= PHASES ; phase++ )
    {
        fetch(APIGW+"v1/sm/fields/power_delivered_l"+phase)
        .then(response => response.json())
        .then(json => {
            const field = getFieldByName(json, "power_delivered_l"+phase);
            let cvKW=document.getElementById(field.name).innerHTML;
            let nvKW=Number(field.value);
            (nvKW == 0 // check if power is generated
                ? fetch(APIGW+"v1/sm/fields/power_returned_l"+phase)
                    .then(response => response.json())
                    .then(json2 => getFieldByName(json2,"power_returned_l"+phase, -1))
                : Promise.resolve(field)
            ).then(({name, value: nvKW}) => {
                //console.log("nvKW = "+ nvKW.toString()) // here nvKW is 0
                let nvA=nvKW*1000.0/230.0  // estimated amps using fixed voltage

                // console.log("about to get elements");
                // update view
                document.getElementById(field.name).innerHTML = nvKW.toFixed(1);

                if (nvKW < minKW[phase]) {
                    minKW[phase] = nvKW                      
                    //console.log(`power_delivered_${phase}min`);
                    document.getElementById(`power_delivered_${phase}min`).innerHTML = nvKW.toFixed(2);
                }
                if (nvKW > maxKW[phase]) {
                    maxKW[phase] = nvKW;
                    //console.log(`power_delivered_${phase}max`);
                    document.getElementById(`power_delivered_${phase}max`).innerHTML = nvKW.toFixed(2);
                }

                // update gauge with actual values
                var chart,
                    point,
                    newValue;
                chart = AmpG[phase];
                point = chart.series[0].points[0];   
                newValue = Math.round(nvA*1000.0 ) / 1000.0;
                point.update(newValue);
                
                if (nvA > MaxAmps[phase]) { //new record
                    MaxAmps[phase] = nvA;
                    point = AmpG[phase].series[1].points[0];
                    point.update(Math.round(MaxAmps[phase]));
                } 
                
                // trend coloring
                if(abs(cvKW - nvKW) < 0.15) {// don't highlight small changes < 150W
                    //console.log("value remains the same")
                    document.getElementById(field.name+"h").style.background="#314b77";
                } else if( nvKW > cvKW ) {
                    //console.log(json.fields[j].name+"=increased")
                    document.getElementById(field.name+"h").style.background="Red";
                } else {
                    //console.log(json.fields[j].name+"=decreased")
                    document.getElementById(field.name+"h").style.background="Green";
                }
                PhaseAmps[phase] = nvA;
            });
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
    for (let t=1; t<= PHASES ; t++) {
        TotalAmps += PhaseAmps[t];
    }
    if (TotalAmps != NaN && TotalAmps != 0.0) {
        let TotalKW = TotalAmps * 230.0 / 1000.0;

        if (TotalKW < minKW[4]) {
            minKW[4] = TotalKW;
            document.getElementById("power_delivered_tmin").innerHTML = TotalKW.toFixed(2);
        }
        if (TotalKW > maxKW[4]) {
            maxKW[4] = TotalKW;
            document.getElementById("power_delivered_tmax").innerHTML = TotalKW.toFixed(2);
        }
        
        document.getElementById("power_delivered_t").innerHTML = TotalKW.toFixed(1);
        point = AmpG[4].series[0].points[0];
        point.update(Math.round(TotalAmps*1000.0 ) /1000.0);

        if (TotalAmps >  MaxAmps[4]){
            MaxAmps[4] = TotalAmps;
            point = AmpG[4].series[1].points[0];
            point.update(Math.round(TotalAmps*1000.0 ) /1000.0);
        }
    }
}

AmpG[1] = Highcharts.chart('container-1', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[2] = Highcharts.chart('container-2', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[3] = Highcharts.chart('container-3', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[4] = Highcharts.chart('container-t', Highcharts.merge(gaugeOptions, {
                yAxis: {
                    min: -3*AMPS,
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
    MaxAmps[i] = -1 * AMPS;
    PhaseAmps[i] = 0.0;
    minKW [i] = 99.99;
    maxKW [i] = -99.99;
}
MaxAmps[4] = -1 * PHASES * AMPS;
