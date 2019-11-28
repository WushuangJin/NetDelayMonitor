Name : Wushuang Jin
USCID : 7575973730

What did I do in this project:
1. client.cpp

1.1 Implement client.cpp as a tcp client socket which could send query to aws server in the form like ./client MAP_ID START_INDEX FILE_SIZE.
1.2 Receive results from aws.cpp and display all computation information

2. aws.cpp 

2.1 Implement the aws.cpp as the tcp server socket corresponding to the client tcp socket
send the MAP_ID and START_INDEX to the serverA using udp socket.
2.2 Send the computation results of serverA to serverB and receive results from serverB,
send all information to client.cpp

3. serverA.cpp

Parse the given map.txt file, build map infomation of each city.
Receive query from aws.cpp and compute the shortest path from given start_index to all
other nodes.

4. serverB.cpp

Receive the distiance to all other node and compute transmission delay, propogation delay, end to end delay.

note : Transmission delay, end to end delay is in ms
