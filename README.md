# Team5-Biondo-Prior-Zou
Quests repository for Team 5

# Overview:
Using the ESP32 board’s built in timer, we created a countdown clock that descended from 120 seconds and rotated a servo 3 times upon hitting 0 seconds. This then loops back and continuously counts down the servo action from 2 minutes.

# Timer:
In order to utilize the ESP32’s timer, we first looked back to Skill 11 to understand how to initialize, start, pause, and modify its functionality. Then, we thought about how we wanted to use the timer in the context of the fish feeder. Since we wanted to feed our fat fish every 2 minutes, we initialized an integer counter variable at 120 seconds, decrementing it with every timer event, which we timed to occur every 1 second. Once the counter reaches 0, we use the timer_pause() function to stop the countdown and execute the servo functionality. Once the servo is done tipping food, we use the timer_start() function to begin counting again, reinitializing the counter integer to 120.

# Display:
The LED display uses the ESP32’s i2c ports to provide a visual representation of the countdown time for users. We accomplished this by taking advantage of the modulo operator and integer division to split the timer into its individual time increments. These are then displayed on the LED screen every second as the timer changes.

# Servo:
In order to coordinate the servo action with the end of the timer we used a large portion of the code from the servo example for skill 9. We added a conditional statement that once the countdown reached 0 the timer is paused and the servo is activated with specific angles. In addition at this step we reset the counter to begin at 2 minutes so the timer would be re initiated. 
