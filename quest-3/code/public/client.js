console.log("clientside js running");

const button = document.querySelector("button");
const temperature = document.querySelector('#temp');
const accelerometerX = document.querySelector('#accelx');
const accelerometerY = document.querySelector('#accely');
const accelerometerZ = document.querySelector('#accelz');
const accelerometerRoll = document.querySelector('#accelr');
const accelerometerPitch = document.querySelector('#accelp');


button.addEventListener("click", (e) => {
  e.preventDefault();
  console.log("BUTTON PRESSED");
  temperature.textContent=69;

  fetch("http://localhost:8081/button").then((response) => {
    response.json().then((data) => {
      console.log(data.message);
      
    });
  });
});
