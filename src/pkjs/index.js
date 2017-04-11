
var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function getWeather() {
    // Construct URL
    var url = 'http://forecast.weather.gov/MapClick.php?lat=42.358429&lon=-71.059769&FcstType=json';

    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET', function(responseText) {
        // responseText contains a JSON object with weather info
        var json = JSON.parse(responseText);
        // Send to Pebble
        var dictionary = {
            "TEMPERATURE": json.currentobservation.Temp,
            "CONDITIONS": json.currentobservation.Weather
        };
        Pebble.sendAppMessage(dictionary, function(e) {
            // console.log("Weather info sent to Pebble successfully!");
        }, function(e) {
            // console.log("Error sending weather info to Pebble!");
        });
    });
}

Pebble.addEventListener('ready', function(e) {
    getWeather();
});

Pebble.addEventListener('appmessage', function(e) {
    getWeather();
});
