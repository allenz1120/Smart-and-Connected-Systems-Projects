var fs = require("fs");
const path = require("path");
const express = require("express");
var dgram = require("dgram");
const hbs = require("hbs");

const app = express(); //doesn't take in args

var payload = "";
var data = []; // Array to store payload data from UDP datagram

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
let catButton = 0;
let dogButton = 0;
let Nk = 0;

// Global variables for sensor data
let thermistorData = 0;
let lidarData = 0;
let ultrasonicData = 0;
// let accelZData = 0;
// let accelRollData = 0;
// let accelPitchData = 0;

// Port and IP
var PORT = 9001;
var HOST = "192.168.1.164";

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
  payload = message.toString();
  console.log("Payload: ", payload);

  // Send Ok acknowledgement
  if (catButton == 1) {
    catButton = 0;
    server.send("Cat!", remote.port, remote.address, function (error) {
      if (error) {
        console.log("MEH!");
      } else {
        console.log("Sent: Cat!");
        data = payload.split(",");
        console.log(data);

        thermistorData = data[0];
        lidarData = data[1];
        ultrasonicData = data[2];

        console.log(thermistorData);
        console.log(lidarData);
        console.log(ultrasonicData);
      }
    });

  }
  else if (dogButton == 1) {
    dogButton = 0;
    server.send("Dog!", remote.port, remote.address, function (error) {
      if (error) {
        console.log("MEH!");
      } else {
        console.log("Sent: Dog!");
        data = payload.split(",");
        console.log(data);

        thermistorData = data[0];
        lidarData = data[1];
        ultrasonicData = data[2];

        console.log(thermistorData);
        console.log(lidarData);
        console.log(ultrasonicData);
      }
    });
  }
  else {
    //dogButton = 0;
    server.send("Nk!", remote.port, remote.address, function (error) {
      if (error) {
        console.log("MEH!");
      } else {
        console.log("Sent: Nk!");
        data = payload.split(",");
        console.log(data);

        thermistorData = data[0];
        lidarData = data[1];
        ultrasonicData = data[2];

        console.log(thermistorData);
        console.log(lidarData);
        console.log(ultrasonicData);
      }
    });
  }
  data = payload.split(",");
  console.log(data);

  thermistorData = data[0];
  lidarData = data[1];
  ultrasonicData = data[2];

  // console.log(thermistorData);
  // console.log(lidarData);
  // console.log(ultrasonicData);
});

// Bind server to port and IP
server.bind(PORT, HOST);

//serves the html file on the client side
app.get("/", function (req, res) {
  res.render("index.hbs");
});

app.get("/catButton", (req, res) => {
  //change button press variable to true
  console.log("CAT BUTTON PRESSED");
  catButton = 1;
  res.send({
    message: "YEETING SOME CAT FOOD"
  });
});

app.get("/dogButton", (req, res) => {
  //change button press variable to true
  console.log("DOG BUTTON PRESSED");
  dogButton = 1;
  res.send({
    message: "YEETING SOME DOG FOOD"
  });
});

// Data Routes

//Send thermistorData
app.get("/data1", function (req, res) {
  res.send({ data: thermistorData });
});

app.get("/data2", function (req, res) {
  res.send({ data: lidarData });
});

app.get("/data3", function (req, res) {
  res.send({ data: ultrasonicData });
});


//Serve on localhost:8080
app.listen(1130);
