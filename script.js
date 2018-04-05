// Code for elapsed time display
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
