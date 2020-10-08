const SerialPort = require("serialport");
var fs = require("fs");
const path = require("path");
var app = require("express")();

const port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
  baudRate: 115200,
});

let data;
let thermistorData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let irData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
let ultrasonicData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];

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
