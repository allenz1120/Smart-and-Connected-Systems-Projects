const SerialPort = require("serialport");
var fs = require("fs");
const path = require("path");
var app = require('express')();
var http = require("http").Server(app);
const io = require("socket.io")(http);

const port = new SerialPort("COM5", {
  baudRate: 115200,
});

let data;
let thermistorData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let irData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let ultrasonicData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];

port.on("readable", function () {
  let data = String(port.read()).split(',');
  // console.log(data.split(','));
  if (data[0] == 'Temperature') {
    thermistorData.shift();
    thermistorData.push(Number(data[1].slice(0, -2)));
    // console.log('t', data[1].slice(0, -2))
  } else if (data[0] == 'irDistance') {
    irData.shift();
    irData.push(Number(data[1].slice(0, -2)));
    // console.log('i', data[1].slice(0, -2))
  } else if (data[0] == 'ultraDistance') {
    ultrasonicData.shift();
    ultrasonicData.push(Number(data[1].slice(0, -2)));
    // console.log('u', data[1].slice(0, -2))
  }
  // console.log('THERMISTOR', thermistorData);
  // console.log('IR', irData);
  // console.log('ULTRASONIC', ultrasonicData);

});


var chartSettings = {
  animationEnabled: true,
  title: {
    text: "Thermistor : Ultrasonic : IR Rangefinder"
  },
  axisX: {
    interval: 1,
    intervalType: "Sensor Data"
  },
  axisY: {
    title: "Temperature / Distance",
    prefix: ""
  },
  legend: {
    cursor: "pointer",
    fontSize: 16,
    itemclick: toggleDataSeries
  },
  toolTip: {
    shared: true
  },
  data: [{
    name: "Thermistor",
    type: "spline",
    yValueFormatString: "#C",
    showInLegend: true,
    dataPoints: thermistorData
  },
  {
    name: "Infared Sensor",
    type: "spline",
    yValueFormatString: "#cm",
    showInLegend: true,
    dataPoints: irData
  },
  {
    name: "Ultrasonic Senor",
    type: "spline",
    yValueFormatString: "#cm",
    showInLegend: true,
    dataPoints: ultrasonicData
  }]
};

function toggleDataSeries(e) {
  if (typeof (e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
    e.dataSeries.visible = false;
  }
  else {
    e.dataSeries.visible = true;
  }
  chart.render();
}

app.get('/', function (req, res) {
  res.sendFile(__dirname + '/canvasJS.html');
});

io.on("connect", (socket) => {
  console.log("CONNECTED");
  let counter = 0;
  io.emit("", chartSettings);

});

http.listen(8080, () => {
  console.log("go to http://localhost:8080");
});