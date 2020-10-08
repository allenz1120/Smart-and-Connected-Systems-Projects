const SerialPort = require("serialport");
var fs = require("fs");
const path = require("path");
var app = require("express")();
// var http = require("http").Server(app);
// const io = require("socket.io")(http);

const port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
  baudRate: 115200,
});

let data;
let thermistorData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let irData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let ultrasonicData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];

let test1 = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1];

port.on("readable", function () {
  let data = String(port.read()).split(",");
  // console.log(data);
  if (data[0] == "Temperature") {
    thermistorData.shift();
    thermistorData.push(Number(data[1].slice(0, -2)));
    // console.log("t", data[1].slice(0, -2));
  } else if (data[0] == "irDistance") {
    irData.shift();
    irData.push(Number(data[1].slice(0, -2)));
    // console.log("i", data[1].slice(0, -2));
  } else if (data[0] == "ultraDistance") {
    ultrasonicData.shift();
    ultrasonicData.push(Number(data[1].slice(0, -2)));
    // console.log("u", data[1].slice(0, -2));
  }
  // console.log("THERMISTOR", thermistorData);
  // console.log("IR", irData);
  // console.log("ULTRASONIC", ultrasonicData);
});

// var chartSettings = {
//   animationEnabled: true,
//   title: {
//     text: "Thermistor : Ultrasonic : IR Rangefinder",
//   },
//   axisX: {
//     interval: 1,
//     intervalType: "Sensor Data",
//   },
//   axisY: {
//     title: "Temperature / Distance",
//     prefix: "",
//   },
//   legend: {
//     cursor: "pointer",
//     fontSize: 16,
//     itemclick: toggleDataSeries,
//   },
//   toolTip: {
//     shared: true,
//   },
//   data: [
//     {
//       name: "Thermistor",
//       type: "spline",
//       yValueFormatString: "#C",
//       showInLegend: true,
//       dataPoints: test1,
//     },
// {
//   name: "Infared Sensor",
//   type: "spline",
//   yValueFormatString: "#cm",
//   showInLegend: true,
//   dataPoints: irData,
// },
// {
//   name: "Ultrasonic Senor",
//   type: "spline",
//   yValueFormatString: "#cm",
//   showInLegend: true,
//   dataPoints: ultrasonicData,
// },
//   ],
// };

var chartSettings = {
  animationEnabled: true,
  title: {
    text: "Daily High Temperature at Different Beaches",
  },
  axisX: {
    valueFormatString: "DD MMM,YY",
  },
  axisY: {
    title: "Temperature (in °C)",
    suffix: " °C",
  },
  legend: {
    cursor: "pointer",
    fontSize: 16,
    itemclick: toggleDataSeries,
  },
  toolTip: {
    shared: true,
  },
  data: [
    {
      name: "Myrtle Beach",
      type: "spline",
      yValueFormatString: "#0.## °C",
      showInLegend: true,
      dataPoints: [
        { x: new Date(2017, 6, 24), y: 31 },
        { x: new Date(2017, 6, 25), y: 31 },
        { x: new Date(2017, 6, 26), y: 29 },
        { x: new Date(2017, 6, 27), y: 29 },
        { x: new Date(2017, 6, 28), y: 31 },
        { x: new Date(2017, 6, 29), y: 30 },
        { x: new Date(2017, 6, 30), y: 29 },
      ],
    },
    {
      name: "Martha Vineyard",
      type: "spline",
      yValueFormatString: "#0.## °C",
      showInLegend: true,
      dataPoints: [
        { x: new Date(2017, 6, 24), y: 20 },
        { x: new Date(2017, 6, 25), y: 20 },
        { x: new Date(2017, 6, 26), y: 25 },
        { x: new Date(2017, 6, 27), y: 25 },
        { x: new Date(2017, 6, 28), y: 25 },
        { x: new Date(2017, 6, 29), y: 25 },
        { x: new Date(2017, 6, 30), y: 25 },
      ],
    },
    {
      name: "Nantucket",
      type: "spline",
      yValueFormatString: "#0.## °C",
      showInLegend: true,
      dataPoints: [
        { x: new Date(2017, 6, 24), y: 22 },
        { x: new Date(2017, 6, 25), y: 19 },
        { x: new Date(2017, 6, 26), y: 23 },
        { x: new Date(2017, 6, 27), y: 24 },
        { x: new Date(2017, 6, 28), y: 24 },
        { x: new Date(2017, 6, 29), y: 23 },
        { x: new Date(2017, 6, 30), y: 23 },
      ],
    },
  ],
};

function toggleDataSeries(e) {
  if (typeof e.dataSeries.visible === "undefined" || e.dataSeries.visible) {
    e.dataSeries.visible = false;
  } else {
    e.dataSeries.visible = true;
  }
  // chart.render();
}

app.get("/", function (req, res) {
  res.sendFile(__dirname + "/canvasJS.html");
});

app.get("/data1", function (req, res) {
  res.send(thermistorData); // Send array of data back to requestor
  // console.log(chartSettings.data);
});

app.get("/data2", function (req, res) {
  res.send(irData); // Send array of data back to requestor
  // console.log(chartSettings.data);
});

app.get("/data3", function (req, res) {
  res.send(ultrasonicData); // Send array of data back to requestor
  // console.log(chartSettings.data);
});

// io.on("connect", (socket) => {
//   console.log("CONNECTED");
//   let counter = 0;
//   io.emit("", chartSettings);
// });

// http.listen(8080, () => {
//   console.log("go to http://localhost:8080");
// });

app.listen(8080);
