const APIGW='http://'+window.location.host+'/api/';
function abs(x)
{
    return (x < 0 ? -x : x);
}

function update()
{
    for( var i=1 ; i < 4 ; i++ )
    {
        fetch(APIGW+"v1/sm/fields/power_delivered_l"+i)
        .then(response => response.json())
        .then(json => {
            for( let j in json.fields ){
                // console.log(json.fields[j].name+" -- "+ "power_delivered_l"+i.toString())
                if (json.fields[j].name.startsWith("power_delivered_l"))
                {
                    let cv=document.getElementById(json.fields[j].name).innerHTML
                    let nv=(json.fields[j].value).toFixed(3)
                    let nva=(json.fields[j].value*1000.0/220.0).toFixed(3)   // estimated amps using fixed voltage

                    // update view

                    document.getElementById(json.fields[j].name+"p").innerHTML = cv.toString();
                    document.getElementById(json.fields[j].name).innerHTML = nv.toString();
                    document.getElementById(json.fields[j].name+"a").innerHTML = nva.toString();

                    // trend coloring

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
                }
            }
        })
        .catch(function(error) {
            var p = document.createElement('p');
            p.appendChild(
            document.createTextNode('Error: ' + error.message)
            );
        });
        
    }
}