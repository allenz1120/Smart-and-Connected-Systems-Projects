
## Code README

### udp_client.c (Actuators)
The code for the actuator ESP is overall much simpler than the sensor ESP. The actuator ESP is not sending any actual data to the UDP server on the Raspberry Pi. Instead, it repeatedly sends an “empty” packet to the UDP server (line 313) and waits for a response.

The response it receives varies depending on if the Raspberry Pi has detected motion based on the sensor data received from the other ESP. These responses are handled with conditionals beginning on line 343. If no motion is detected, no action is taken and the ESP waits for the next UDP response. If cat or dog motion is detected, its respective flag is tripped, and the servo tasks on line 608 and 633 are executed.

### udp_client.c (Sensors)
Lines 237 - 347 handle the UDP client on this ESP. It concatenates a payload using the sensor data and adds a "0" at the front of the payload to signify that the message is coming from the sensor ESP. It then sends the packet to the node server on the raspberry pi and listens for a response from the server. It does not have an action based on the response.

Lines 132 - 205 creates the timer isr function and timer task. The timer runs every second and reads data from sensors in a 3 second cycle. Everytime that the timer evt.flag is triggered, we perform this operation on a timerCounter variable: (timerCounter = (timerCounter + 1) % 3). This gives use values of 0, 1, and 2 which are mapped to thermistor, ultrasonic, and lidar respectively. Using this timerCounter variable, we are able to allocate a specific time interval for each sensor to read data without overlap.

Lines 506 - 572 handle reading the thermistor data. It reads the raw adc reading and converts it to celcius.

Lines 574 - 608 handle reading the ultrasonic data. It reads the raw adc reading and converts it to centimeters.

Lines 659 - 750 handle the read, write functions and reading the lidar data using those functions. It initalizes the lidar sensor by writing 0x04 to register 0x00 and reads register 0x01 until it is a low bit. Then, it takes the high and low readings from register 0x10 and 0x11 and adds them to make the 16 bit lidar distance in centimeters.

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


