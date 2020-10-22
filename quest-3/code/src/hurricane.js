var fs = require("fs");
const path = require("path");
const express = require("express");
var dgram = require("dgram");
const hbs = require("hbs");

const app = express(); //doesn't take in args

//Define paths
const publicDirPath = path.join(__dirname, "../public");
//this is optional (allows you to call views directory by another name) Make sure you use app.set('views',viewsPath)
const viewsPath = path.join(__dirname, "../public");

//express needs the first key to be view engine and second to be hbs
//this sets up handlebars
app.set("view engine", "hbs");
app.set("views", viewsPath);
app.use(express.static(publicDirPath));

// Global variable to keep track of LED Button status
let ledButtonPress = 0;

// Port and IP
var PORT = 9001;
var HOST = "192.168.1.21";

// Create socket
var server = dgram.createSocket("udp4");

// Create server
server.on("listening", function () {
  var address = server.address();
  console.log(
    "UDP Server listening on " + address.address + ":" + address.port
  );
});

// On connection, print out received message
server.on("message", function (message, remote) {
  console.log(remote.address + ":" + remote.port + " - " + message);

  // Send Ok acknowledgement
  server.send("Ok!", remote.port, remote.address, function (error) {
    if (error) {
      console.log("MEH!");
    } else {
      console.log("Sent: Ok!");
    }
  });
});

// Bind server to port and IP
server.bind(PORT, HOST);

let data;
let thermistorData = 0;
let accelXData = 0;
let accelYData = 0;
let accelZData = 0;
let accelRollData = 0;
let accelPitchData = 0;

//read data from ESP port
// port.on("readable", function () {
//   let data = String(port.read()).split(",");
//   if (data[0] == "Temperature") {
//     thermistorData = Number(data[1].slice(0, -2));
//   } else if (data[0] == "X") {
//     accelXData = Number(data[1].slice(0, -2));
//   } else if (data[0] == "Y") {
//     accelYData = Number(data[1].slice(0, -2));
//   } else if (data[0] == "Z") {
//     accelZData = Number(data[1].slice(0, -2));
//   } else if (data[0] == "Roll") {
//     accelRollData = Number(data[1].slice(0, -2));
//   } else if (data[0] == "Pitch") {
//     accelPitchData = Number(data[1].slice(0, -2));
//   }
// });

//serves the html file on the client side
app.get("/", function (req, res) {
  res.render("index.hbs");
});

app.get("/button", (req, res) => {
  //change button press variable to true
  ledButtonPress = 1;
});

// //Sends the thermistorData array
// app.get("/data1", function (req, res) {
//   res.send(thermistorData); // Send array of data back to requestor
// });
// //Sends the irData array
// app.get("/data2", function (req, res) {
//   res.send(irData); // Send array of data back to requestor
// });
// //Sends the ultrasonicData array
// app.get("/data3", function (req, res) {
//   res.send(ultrasonicData); // Send array of data back to requestor
// });

//Serve on localhost:8081
app.listen(8081);
