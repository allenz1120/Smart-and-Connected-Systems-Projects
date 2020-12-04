console.log("clientside js running");

const button = document.querySelector("#onButton");

button.addEventListener("click", (e) => {
  e.preventDefault();
  console.log("BUTTON PRESSED");

  fetch("button").then((response)=>{
    // response.json().then((data) => {
    // console.log(data.message);
    // })
    console.log("made it here \n");
  });
});
