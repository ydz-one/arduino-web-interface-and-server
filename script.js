// Global variables for temperature display
var isCelsius = true;
var currentTempC = 0;
var highestTempC = 0;
var lowestTempC = 200;
var averageTempC = 0;
var xhr;
var changeColor = false;
var standbyMode = false;

// Global variables for graph
var graphData = [];
var timeNow;
var padding = 60;
var w = 700;
var h = 400;

// EventListeners for buttons
document.getElementById('toggle-temp').addEventListener('click', function() {
  // Change browser temperature display if not in standbyMode
  if (!standbyMode) {
    isCelsius = isCelsius ? false : true;
    updateTempDisplay();
  }

  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?toggleTemp', true);
  xhr.send();
});

document.getElementById('toggle-standby').addEventListener('click', function() {
  standbyMode = standbyMode ? false : true;

  if (standbyMode) {
    document.getElementById('current-temp').innerHTML = 'Standby';
    document.getElementById('lowest-temp').innerHTML = 'Standby';
    document.getElementById('highest-temp').innerHTML = 'Standby';
    document.getElementById('average-temp').innerHTML = 'Standby';
  } else {
    loadTemp();
  }

  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?toggleStandby', true);
  xhr.send();
});

document.getElementById('toggle-light').addEventListener('click', function() {
  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?toggleLight', false);
  xhr.send();
});

document.getElementById('change-light-color').addEventListener('click', function() {
  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?changeLightColor', true);
  xhr.send();
});


// Temperature update
loadTemp();

function loadTemp() {
  // Do nothing if in standby mode
  if (standbyMode) {
    return;
  }

  xhr = new XMLHttpRequest();
  xhr.open('GET', 'data.json', true);

  xhr.onload = function() {
    if (this.status == 200) {
      var temp = JSON.parse(this.responseText);

      if (temp.status == 0) {
        alert("Error occurred while attempting to read from Arduino.");
        return;
      }

      timeNow = Date.now();
      graphData.push([temp.current, timeNow]);

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
  plotGraph();
  xhr.send();
  setTimeout(loadTemp, 5000);
}

function updateTempDisplay() {
  document.getElementById('current-temp').innerHTML = isCelsius ?
      cleanNumber(currentTempC) + "&deg;C" : cleanNumber(currentTempC * 9 / 5 + 32) + "&deg;F";
  document.getElementById('lowest-temp').innerHTML = isCelsius ?
      cleanNumber(lowestTempC) + "&deg;C" : cleanNumber(lowestTempC * 9 / 5 + 32) + "&deg;F";
  document.getElementById('highest-temp').innerHTML = isCelsius ?
      cleanNumber(highestTempC) + "&deg;C" : cleanNumber(highestTempC * 9 / 5 + 32) + "&deg;F";
  document.getElementById('average-temp').innerHTML = isCelsius ?
      cleanNumber(averageTempC) + "&deg;C" : cleanNumber(averageTempC * 9 / 5 + 32) + "&deg;F";
}

// Round numbers to 1 decimal place
function cleanNumber(num) {
  // Round number to 1 decimal place
  var out = Math.round(num * 10) / 10;

  // If whole number, append '.0' to it
  if (out % 1 === 0) {
    out = out + '.0';
  }

  return out;
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

// D3 graph
function plotGraph() {
  document.getElementById('svg-plot').innerHTML = "";
  var xScale = d3.scaleTime()
                   .domain([timeNow - 3.6e6, timeNow])
                   .range([padding, w - padding]);

  var yScaleC = d3.scaleLinear()
                   .domain([d3.min(graphData, (d) => d[0]) - 1, d3.max(graphData, (d) => d[0]) + 1])
                   .range([h - padding, padding]);

  var yScaleF = d3.scaleLinear()
                   .domain([(d3.min(graphData, (d) => d[0]) - 1) * 9/5 + 32, (d3.max(graphData, (d) => d[0]) + 1) * 9/5 + 32])
                   .range([h - padding, padding]);

  var svg = d3.select('#svg-plot');
  svg.selectAll("circle")
     .data(graphData)
     .enter()
     .append("circle")
     .attr("cx", (d) => xScale(d[1]))
     .attr("cy",(d) => yScaleC(d[0]))
     .attr("r", (d) => 2);

  var xAxis = d3.axisBottom(xScale);

  svg.append("g")
     .attr("transform", "translate(0, " + (h - padding) + ")")
     .attr("class", "axis")
     .call(xAxis);

  var yAxisC = d3.axisLeft(yScaleC);

  svg.append("g")
     .attr("transform", "translate(" + padding + ", 0)")
     .attr("class", "axis")
     .call(yAxisC);

  var yAxisF = d3.axisRight(yScaleF);

  svg.append("g")
      .attr("transform", "translate(" + (w - padding) + ", 0)")
      .attr("class", "axis")
      .call(yAxisF);

  svg.append("text")
      .attr("x", w / 2)
      .attr("y", h - padding / 4)
      .attr("class", "axis-label")
      .text("Time");

  svg.append("text")
      .attr("x", padding / 4)
      .attr("y", padding)
      .attr("class", "axis-label")
      .text("°C");

  svg.append("text")
      .attr("x", w - padding / 2)
      .attr("y", padding)
      .attr("class", "axis-label")
      .text("°F");
}
