# Code Readme

### Fob Code
The fobs are all running the same code with the exception of their respective device IDs, which are unique and can be found on lines 121 and 122. Each fob also makes use of a global ipTable variable, found on line 130, that maps device IDs to their respective IP addresses (The indices correspond with the device ID). We also define a NUM_FOBS constant on line 135 for looping through the device IPs. Due to limited hardware, we used 3 fobs for this demo. 

[EXPLAIN LEADER ELECTION HERE]
(Use Skill 28 code README)

The majority of the election handling is done inside the UDP client and server tasks. The "udp_server_task" on line 560 uses a series of conditionals to handle state changing of the fob after receiving leader election data over UDP. On line 655, a for loop deconstructs the received payload, determines if it contains leader election data or poll data, and stores its contents in related variables. 

Once a poll leader has been elected, the actual voting process can begin. The fobs toggle between voting red or blue inside button_task() on line 346. Pressing the button changes the colorID variable, which is then checked with a switch case inside led_task() on line 464. This LED task checks the state of the colorID variable, and will turn on the appropriate red or blue LED on the fob.

After choosing to vote red or blue, a second button is pressed to transmit the fob ID and it's vote ('R' or 'B') over NFC/IR to an adjacent fob. The second button trips a flag (sendFlag) inside of button_2_task() on line 373. Conceptually, we chose to only transmit voter data over NFC/IR if the fob is not in a leader state. We chose to do this because we felt that the poll leader should have the ability to transmit its vote directly to the node.js server over UDP, rather than having to transmit to an adjacent fob, receive its own vote back, and only then being able to transmit to the node.js server.

Once the vote has been sent over IR, the receiving fob will "blink" the registered vote to show that it has been received. This happens on lines 432 - 441. The correct led pins that correspond to the received vote are set to a value of 1 and then turns off on the next cycle of led_task.

So, if the fob is not in a leader state (Is not the poll leader), and the sendFlag is tripped, it will enter a conditional inside the recv_task() on line 397. The code inside this conditional will then transmit the fob ID and vote to an adjacent fob over NFC/IR. The adjacent fob will then have the RecvFlag tripped on line 442 upon receiving the IR data. If the adjacent fob is not the poll leader, a conditional will then execute inside the udp_client_task() on line 1045, which handles sending the original fob's ID and vote to the poll leader over UDP. If the adjacent fob happens to be the poll leader already, a conditional inside the udp_client_task() on line 961 will instead execute, sending the original fob's ID and vote directly to the node.js server over UDP.

If the poll leader is sending its own vote, the conditional on line 1120 will execute, which sends its own fob ID and vote directly to the node.js server over UDP.


### Votes.js Code
This is our nodejs program that handles receiving udp packets from the c program, parsing the information, and storing it to the database and sending it to the front end. Lines 76-82 handle our html and web client configuration with app.get and app.post /table.html. This program uses sockets to emit information on the front end. Once we establish a connection we created a 9 fob functions that will include time, FobID, and vote objects. Additionally, every fob will have it's own time, ID, and will be able to cast their own vote. 

Lines 185-192 receive the message from the c program and parse the data into payload which will have the fobID and the fob's respective vote. 

Lines 200 - 251 is our readData function. This takes in argument payload and depending on which fob we are looking at we push the time, ID, and vote into information{}. 

Lines 254-388 is our Database send function. Lines 254 - 305 are going to take the information we just created for each fob and store it in the created database using a db.put('fobID', information#). Whereas lines 306 - 388 use db.get to but all the information into each respective fob to be used in the front end. 

Finally, our last function is deleteVote() on line 390 which uses the rimraf module to synchronously delete our database directoy so we can store new user votes simulating a new election. This function is called in the front end on a button click in table.html.

### Front-End table.html
This is our front end program for the webpage "Voting Hype!" This program uses sockets and jquery to access information from node server. Lines 23 to 241 include the fob queries for up to 9 fobs from the node program to access time, fobID and vote. Once each socket is created we creat a table with the Date and Time, FobID, and Vote. This is the same for all 9 fobs and can first be seen on lines 27-30. From that we populate each table column with each fob object (fob.time, fob.FobID, fob.vote). First iteration of this can be seen on lines 35-37. 

Additionally, after each fob's objects are created and populated into the table, we check what the fob.vote is and increment either blue votes or red votes. This happens after each fob table is setup. The first iteration is on lines 43- 50.

Finally, towards the end of the program is where we have our buttons setup to count the votes and delete the ballots. These use the onclick button feature and are called on lines 248 and 249 respectively. 

### Database db.js code
This file sets up our leveldb database to store all of our voter data. This file is imported into votes.js so we can put information into the database. The directory name to be created in the pi is on line 10 and the value encoding is 'json' on line 13.
