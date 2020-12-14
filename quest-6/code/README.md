
## Code README

### udp_client.c (Actuators)

### udp_client.c (Sensors)

### hurricane.js (Node.js Server)

This is our Node.js program that handles recieving udp packets from both communicating ESP32's and sending a message back to control the actuators. 

Lines 62-68 handles parsing our payload that is recieved from both c programs on the ESP32's. Our payload for this program contains temperature data, lidar distance, and ultrasonic distance. Additionally, the first index of the payload includes either a 1 or a 0 so we can distinguish between the sensor esp and the actuator esp.

Lines 71-104 handle our movement detection alert. First we check to see that we are recieving a message from the sensor esp. If we are then we check if the lidar and ultraosnic distance are less than a preset value of cat and dog feed distance. This distance is used to check if either a cat or dog has been waiting in front of the sensor. If they are, then every packet recieved will increment the dog or cat counter. If these counters get to 5 (5 seconds the pet has been waiting), then it will send a message back to the actuator esp to trip the correct servo. 

Lines 156-172 get the status of the buttons being pressed on our front end. When these buttons are pressed we change the respective cat or dog button status to 1 to be used in lines 115-146. 

Lines 115-146: as mentioned above, these lines check the status of the dog and cat button to see if it has been pressed. If it has, we also send a message back to the actuator esp so it knows to turn the correct servo. 

Lines 177-195 are all of our data routes for collecting temperature, lidar & ultrasonic distance. These are dynamically updated on the front end using ajax calls. 

### client.js (Client)

This is the client side js program that sets up our button event listeners to be implemented in the front end. 

Lines 6-28 setup each button handler for the cat and dog buttons. These are to be refernced in the front end. button being the cat button and button2 being the dog button. 

### index.hbs (Front-End)

This is our front end website that is dynaimcally updated with all sensor data, includes two feeding buttons, and a live video feed from the raspberry pi. 

Lines 12-15 setup our raspberry pi address that uses the pi camera to capture a live feed of our system. 

Lines 43-99 include all of our ajax calls that dynamically update all of our data as well as the button event handlers. These ajax calls are very important since we wanted to again, dynamically update all of our data with each udp message recieved. Setting the async value to false in each of these allowed us to do so. 

Lastly, Lines 104,106, and 109 include the buttons to be displayed on the page using button ID's and including the button handlers from the client.js program. 


