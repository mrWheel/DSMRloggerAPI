const APIGW='http://'+window.location.host+'/api/';
const AMPS=25
const PHASES=3

var AmpG = [4];

var gaugeOptions = {

    chart: {
        type: 'solidgauge'
    },

    title: null,

    pane: {
        center: ['50%', '85%'],
        size: '100%',
        startAngle: -90,
        endAngle: 90,
        background: {
            backgroundColor:
                Highcharts.defaultOptions.legend.backgroundColor || '#EEE',
            innerRadius: '60%',
            outerRadius: '80%',
            shape: 'arc'
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
        minorTickInterval: null,
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
        data: [22],
        dataLabels: {
            format:
                '<div style="text-align:center">' +
                '<span style="font-weight:lighter;font-size:16px;font-family: Dosis">{y} A</span><br/>' +
                '</div>'
        }
    }]

};

function abs(x)
{
    return (x < 0 ? -x : x);
}

var PhaseAmps = [4];
var TotalAmps;

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

                    let cv=document.getElementById(json.fields[j].name).innerHTML
                    let nv=(json.fields[j].value).toFixed(3)
                    let nva=(json.fields[j].value*1000.0/220.0).toFixed(3)   // estimated amps using fixed voltage

                    console.log("about to get elements");
                    // update view

                    document.getElementById(json.fields[j].name+"p").innerHTML = cv.toString();
                    document.getElementById(json.fields[j].name).innerHTML = nv.toString();
                    //document.getElementById(json.fields[j].name+"a").innerHTML = nva.toString();

		            // update gauge with actual values

                    var chart,
                        point,
                        newValue,
                        myPhase;
                    
                    myPhase = json.fields[j].name.replace('power_delivered_l','');
                    
                    chart = AmpG[Number(myPhase)];
                    point = chart.series[0].points[0];
		               
                    newValue = Math.round(nva*1000.0 ) /1000.0;
                    
        	        point.update(newValue);
                    
                    // trend coloring

                    console.log("About to color headings");

                    if(abs(cv - nv) < 0.005)
                    {
                        //console.log("value remained the same")
                        document.getElementById(json.fields[j].name+"h").style.background="#314b77"
                    } else if( nv > cv )
                    {
                        //console.log(json.fields[j].name+"=verhoogd")
                        document.getElementById(json.fields[j].name+"h").style.background="Red"
                    } else {
                        //console.log(json.fields[j].name+"=verlaagd")
                        document.getElementById(json.fields[j].name+"h").style.background="Green"
                    }

                    PhaseAmps[myPhase] = newValue;
                    TotalAmps = 0;
                    for(t=1; t<= PHASES ; t++)
                        TotalAmps += PhaseAmps[t];

                    // document.getElementById("power_delivered_ta").innerHTML = TotalAmps.toFixed(3);
                    document.getElementById("power_delivered_t").innerHTML = (TotalAmps*220.0/1000.0).toFixed(3);

                    chart = AmpG[4];
                    point = chart.series[0].points[0];
		               
                    newValue = Math.round(TotalAmps*1000.0 ) /1000.0;
                    
        	        point.update(newValue);
                    
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
                    data: [22],
                    dataLabels: {
                        format:
                            '<div style="text-align:center">' +
                            '<span style="font-weight:lighter;font-size:16px;font-family: Dosis">{y} A</span><br/>' +
                            '</div>'
                    }
                }]

            }));

