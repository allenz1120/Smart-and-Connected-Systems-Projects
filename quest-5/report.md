# Cruise Control
Authors: Allen Zou, Alex Prior, John Kircher

Date: 2020-11-30
-----

## Summary
For this quest, we were tasked with creating a cruise control and collision avoidance system for an RC car. We achieve this using an ultrasonic sensor, a LIDAR sensor, an optical encoder, and PID control. Using these tools, we are able to stay within +/- 25cm of a centerline, detect hazards in front of the car to stop collisions, and maintain a fixed speed. Finally, we implemented UDP socket communication allowing us to instantaneously start and stop the car from a server. 

## Self-Assessment

### Objective Criteria

| Objective Criterion | Rating | Max Value  | 
|---------------------------------------------|:-----------:|:---------:|
| Controls steering to maintain center of course +/- 25cm for entire length | 1 |  1     | 
| Uses PID for speed control holding a fixed speed setpoint after startup and before slowdown [0.1-0.4 m/s] | 1 |  1     | 
| Stops within 20 cm of end without collision | 1 |  1     | 
| Start and stop instructions issued wirelessly from phone, laptop or ESP) | 1 |  1     | 
| Measures wheel speed or distance | 1 |  1     | 
| Uses alpha display to show current distance or speed | 1 |  1     | 
| Successfully traverses A-B in one go, no hits or nudges | 1 |  1     | 


### Qualitative Criteria

| Qualitative Criterion | Rating | Max Value  | 
|---------------------------------------------|:-----------:|:---------:|
| Quality of solution |  |  5     | 
| Quality of report.md including use of graphics |  |  3     | 
| Quality of code reporting |  |  3     | 
| Quality of video presentation |  |  3     | 


## Solution Design
### PID Speed Control and Ultrasonic Sensing

### LIDAR and Steering Control

### UDP Communication

### Wheel Speed Detection Using Optical Encoder

### Alphanumeric Display
We used a second I2C bus for the alphanumeric display, choosing two GPIO pins (27 and 33) to accomplish this.

## Sketches and Photos
<center><img src="./images/ece444.png" width="25%" /></center>  
<center> </center>


## Supporting Artifacts
- [Link to video demo](). Not to exceed 120s


## Modules, Tools, Source Used Including Attribution

## References

-----

