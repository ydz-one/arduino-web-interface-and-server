// Global variables for temperature display
var isCelsius = true;
var currentTempC = 0;
var highestTempC = 0;
var lowestTempC = 0;
var averageTempC = 0;
var xhr;

// Global variables for graph
var graphData = [];
var timeNow;
var padding = 60;
var w = 700;
var h = 400;

// EventListeners for buttons
document.getElementById('toggle-temp').addEventListener('click', function() {
  // Change browser temperature display
  isCelsius = isCelsius ? false : true;
  updateTempDisplay();

  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?toggleTemp', true);
});

document.getElementById('toggle-standby').addEventListener('click', function() {
  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?toggleStandby', true);
});

document.getElementById('toggle-light').addEventListener('click', function() {
  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?toggleLight', true);
});

document.getElementById('change-light-color').addEventListener('click', function() {
  // Send GET request to server
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/action?changeLightColor', true);
});


// Temperature update
loadTemp();

function loadTemp() {
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'data.json', true);

  xhr.onload = function() {
    if (this.status == 200) {
      var temp = JSON.parse(this.responseText);
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
     .call(xAxis);

  var yAxisC = d3.axisLeft(yScaleC);

  svg.append("g")
     .attr("transform", "translate(" + padding + ", 0)")
     .call(yAxisC);

  var yAxisF = d3.axisLeft(yScaleF);

  svg.append("g")
      .attr("transform", "translate(" + (w - padding) + ", 0)")
      .call(yAxisF);
}
