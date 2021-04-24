const APIGW=window.location.protocol+'//'+window.location.host+'/api/';
const AMPS = 25
const PHASES = 3

var AmpG = [4];
var PhaseAmps = [4];
var MaxAmps = [4];
var MeterPhases = 0;
var updateCount = 0;
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
        minorTickInterval: 5,
        minorTicks: true,
        // minorTicksWidth: 3px,
        //tickAmount: 0,
        title: {
            y: -70
        },
        labels: {
            y: 2
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
        min: -1 * AMPS,
        max: AMPS,
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
                '<span style="font-weight:lighter;font-size:20px;font-family:Dosis">{point.y:.1f} A</span><br/>' +
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
    //prefix _l4 = total; strip it for matching API name
    prefix = prefix.replace(/_l4+$/, "");

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

function checkError(response) {
    if (response.status >= 200 && response.status <= 299) {
        return response.json();
    } else {
        throw Error(response.statusText);
    }
}

function update()
{
    var phase;
    if (MeterPhases == 0) { // Try to detect single or three phase meter (to reduce number of API calls)
        updateCount++;
        if (PhaseAmps[1] != 0 || PhaseAmps[2] != 0 || PhaseAmps[3] != 0) {
            MeterPhases = 3;
        } else if (updateCount > 6) { // 6 * 10sec. Change me to match update interval
            MeterPhases = 1;
        }
    }

    for( let phase = ( MeterPhases == 1 ? PHASES + 1 : 1) ; phase <= PHASES + 1 ; phase++ )
    {
        fetch(APIGW + "v1/sm/fields/power_delivered" + ((phase < 4) ? "_l" + phase : ""))
        .then(checkError)
        //.then(response => response.json())
        .then(json => {
            const field = getFieldByName(json, "power_delivered" + ((phase < 4) ? "_l" + phase : ""));
            //console.log("field.name = "+field.name);
            let cvKW=document.getElementById((phase < 4) ? field.name : "power_delivered_t").innerHTML;
            let nvKW=Number(field.value);
            (nvKW == 0 // check if power is generated
                ? fetch(APIGW + "v1/sm/fields/power_returned" + ((phase < 4) ? "_l" + phase : ""))
                    .then(checkError)
                    .then(json2 => getFieldByName(json2, "power_returned" + ((phase < 4) ? "_l" + phase : ""), -1))
                : Promise.resolve(field)
            ).then(({name, value: nvKW}) => {
                //console.log("nvKW = "+ nvKW.toString()) // here nvKW is 0
                let nvA=nvKW*1000.0/230.0  // estimated amps using fixed voltage
                // update view
                document.getElementById((phase < 4) ? field.name : "power_delivered_t").innerHTML = nvKW.toFixed(2);

                if (phase ==  1 || phase == 2 || phase == 3){
                    docId = "power_delivered_" + phase;
                }
                else if (phase == 4){
                    docId = "power_delivered_t";
                }

                if (nvKW < minKW[phase]) {
                    minKW[phase] = nvKW                      
                    //console.log(`power_delivered_${phase}min`);
                    document.getElementById(`${docId}min`).innerHTML = nvKW.toFixed(2);
                }
                if (nvKW > maxKW[phase]) {
                    maxKW[phase] = nvKW;
                    //console.log(`power_delivered_${phase}max`);
                    document.getElementById(`${docId}max`).innerHTML = nvKW.toFixed(2);
                }

                // update gauge with actual values
                var chart,
                    point,
                    newValue;
                chart = AmpG[phase];
                point = chart.series[0].points[0];   
                newValue = Math.round(nvA*10.0) / 10.0;
                point.update(newValue);
                
                if (nvA > MaxAmps[phase]) { //new record
                    MaxAmps[phase] = nvA;
                    point = AmpG[phase].series[1].points[0];
                    point.update(Math.round(MaxAmps[phase]));
                } 
                
                if (MeterPhases != 0) {
                    // trend coloring
                     if (phase ==  1 || phase == 2 || phase == 3){
                        headerId = field.name;
                    }
                    else if (phase == 4){
                        headerId = "power_delivered_t";
                    }

                    if(abs(cvKW - nvKW) < 0.005) {// don't highlight small changes < 5W
                        //console.log("value remains the same")
                        document.getElementById(headerId+"h").style.background="#314b77";
                    } else if( nvKW > cvKW ) {
                        //console.log(json.fields[j].name+"=increased")
                        document.getElementById(headerId+"h").style.background="Red";
                    } else {
                        //console.log(json.fields[j].name+"=decreased")
                        document.getElementById(headerId+"h").style.background="Green";
                    }
                }
                PhaseAmps[phase] = nvA;
            });
        })
        .catch(error => void 0); //fail silent on network errors
    }
}

AmpG[1] = Highcharts.chart('container-1', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[2] = Highcharts.chart('container-2', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[3] = Highcharts.chart('container-3', Highcharts.merge(gaugeOptions, AmpOptions));
AmpG[4] = Highcharts.chart('container-t', Highcharts.merge(gaugeOptions, {
                yAxis: {
                    min: -3 * AMPS,
                    max: 3 * AMPS,
                    minorTickInterval: 5,
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
                            '<span style="font-weight:lighter;font-size:20px;font-family:Dosis">{point.y:.1f} A</span><br/>' +
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
