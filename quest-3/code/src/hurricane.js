// const SerialPort = require("serialport");
var fs = require("fs");
const path = require("path");
var app = require("express")();
var dgram = require("dgram");

// Port and IP
var PORT = 3333;
var HOST = "192.168.1.105";

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

// const port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
//   baudRate: 115200,
// });

// let data;
// let thermistorData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
// let irData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
// let ultrasonicData = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];

// //read data from ESP port
// port.on("readable", function () {
//   let data = String(port.read()).split(",");
// });

//serves the html file on the client side
app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname, "../public/index.html"));
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

//Serve on localhost:8080
app.listen(8081);
