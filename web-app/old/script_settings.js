document.addEventListener('DOMContentLoaded', function(){
    const isValidSSID = function(value){
        return value.length >= 2 && value.length <= 32;
    };

    const isValidWifiPW = function(value){
        return value.length >= 8 && value.length <= 70;
    };

    const sendHttpRequest = function(url, data, callback){
        var xhr = new XMLHttpRequest();
        xhr.open("POST", url, true);
        xhr.timeout = 2000; // time in milliseconds
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.ontimeout = (e) => {
            // XMLHttpRequest timed out. Do something here.
            console.log('timeout');
        };
        // xhr.onload = function(callback){
        //     callback(this.responseText);
        // };
        xhr.send(JSON.stringify(data));
    };

    document.getElementById('submitWifi').onclick = function(e){
        let ssid = document.getElementById('inputSSID').value;
        let wifipw = document.getElementById('inputPW').value;

        if(isValidSSID(ssid) && isValidWifiPW(wifipw)){
            console.log('submit');
            sendHttpRequest( '/settingsPost', ssid + ',.,' + wifipw, function() { });

            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/settingsPost', true);
            xhr.timeout = 2000; // time in milliseconds
            xhr.onload = () => {
              // Request finished. Do processing here.
            };
            
            xhr.ontimeout = (e) => {
              // XMLHttpRequest timed out. Do something here.
              console.log(timeout)
            };
            
            xhr.send(ssid + ',.,' + wifipw);
            
        }
    }
});


