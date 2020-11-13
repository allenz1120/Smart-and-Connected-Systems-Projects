# Code Readme

### Fob Code
The fobs are all running the same code with the exception of their respective device IDs, which are unique and can be found on lines 120 and 121. Each fob also makes use of a global ipTable variable, found on line 129, that maps device IDs to their respective IP addresses (The indices correspond with the device ID). We also define a NUM_FOBS constant on line 134 for looping through the device IPs. Due to limited hardware, we used 3 fobs for this demo. 

[EXPLAIN LEADER ELECTION HERE]
(Use Skill 28 code README)

The majority of the election handling is done inside the UDP client and server tasks. The "udp_server_task" on line 553 uses a series of conditionals to handle state changing of the fob after receiving leader election data over UDP. On line 649, a for loop deconstructs the received payload, determines if it contains leader election data or poll data, and stores its contents in related variables. 

In the "udp_client_task" on line 848 
