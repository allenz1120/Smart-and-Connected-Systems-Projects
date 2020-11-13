# Code Readme

### Fob Code
The fobs are all running the same code with the exception of their respective device IDs, which are unique and can be found on lines 120 and 121. Each fob also makes use of a global ipTable variable, found on line 129, that maps device IDs to their respective IP addresses (The indices correspond with the device ID). We also define a NUM_FOBS constant on line 134 for looping through the device IPs. Due to limited hardware, we used 3 fobs for this demo. 

[EXPLAIN LEADER ELECTION HERE]
(Use Skill 28 code README)

The majority of the election handling is done inside the UDP client and server tasks. The "udp_server_task" on line 553 uses a series of conditionals to handle state changing of the fob after receiving leader election data over UDP. On line 649, a for loop deconstructs the received payload, determines if it contains leader election data or poll data, and stores its contents in related variables. 

In the "udp_client_task" on line 848 


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
