// Temperature display and JSON data parsing
var isCelsius = true;
var currentTempC = 0;
var highestTempC = 0;
var lowestTempC = 0;
var averageTempC = 0;
var graphTempVals = [];
var graphTimeVals = [];

document.getElementById('toggle-temp').addEventListener('click', function() {
  isCelsius = isCelsius ? false : true;
  updateTempDisplay();
});

loadTemp();

function loadTemp() {
  console.log('calling load temp');
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'data.json', true);

  xhr.onload = function() {
    if (this.status == 200) {
      var temp = JSON.parse(this.responseText);
      graphTempVals.push(temp.current);
      graphTimeVals.push(Date.now());

      currentTempC = temp.current;
      highestTempC = temp.highest;
      lowestTempC = temp.lowest;
      averageTempC = temp.average;

      updateTempDisplay();
    } else {
      document.getElementById('current-temp').innerHTML = 'No Data';
      document.getElementById('lowest-temp').innerHTML = 'No Data';
      document.getElementById('highest-temp').innerHTML = 'No Data';
      document.getElementById('average-temp').innerHTML = 'No Data';
    }
  };

  xhr.send()
  setTimeout(loadTemp, 5000);
}

function updateTempDisplay() {
  document.getElementById('current-temp').innerHTML = isCelsius ?
      currentTempC + "&deg;C" : currentTempC * 9 / 5 + 32 + "&deg;F";
  document.getElementById('lowest-temp').innerHTML = isCelsius ?
      lowestTempC + "&deg;C" : lowestTempC * 9 / 5 + 32 + "&deg;F";
  document.getElementById('highest-temp').innerHTML = isCelsius ?
      highestTempC + "&deg;C" : highestTempC * 9 / 5 + 32 + "&deg;F";
  document.getElementById('average-temp').innerHTML = isCelsius ?
      averageTempC + "&deg;C" : averageTempC * 9 / 5 + 32 + "&deg;F";
}

// Elapsed time display
var startTime = new Date();
var mSecs = 0;

function updateTime() {
  var hours = Math.floor(mSecs / 3.6e6);
  var mins = Math.floor((mSecs - hours * 3.6e6) / 60000);
  var secs = Math.floor((mSecs - hours * 3.6e6 - mins * 60000) / 1000);

  var hoursStr = Math.floor(hours / 10) < 1 ? '0' + hours : hours;
  var minsStr = Math.floor(mins / 10) < 1 ? '0' + mins : mins;
  var secsStr = Math.floor(secs / 10) < 1 ? '0' + secs : secs;

  document.getElementById('time-elapsed').innerHTML = [hoursStr, minsStr, secsStr].join(':');

  mSecs += 1000;

  setTimeout(updateTime, 1000);
}

updateTime();
