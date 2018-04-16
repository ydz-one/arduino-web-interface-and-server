// Load temperature from Arduino
loadTemp();

function loadTemp() {
  console.log('calling load temp');
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'data.txt', true);

  xhr.onload = function() {
    if (this.status == 200) {
      document.getElementById('current-temp').innerHTML = this.responseText;
    } else {
      document.getElementById('current-temp').innerHTML = 'No Data';
    }
  };

  xhr.send()
  setTimeout(loadTemp, 5000);
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
