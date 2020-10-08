const SerialPort = require("serialport");
var http = require("http");
var fs = require("fs");
const path = require("path");

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
  console.log('THERMISTOR', thermistorData);
  console.log('IR', irData);
  console.log('ULTRASONIC', ultrasonicData);

});
