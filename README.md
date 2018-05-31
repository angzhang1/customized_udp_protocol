# customized_udp_protocol
Implement a customized protocol on top of UDP. Simulate the functionalities for both server and client.

# Build Instruction
The project uses cmake. Please follow the steps to build the code. Tested on OSX.

```
mkdir build
cmake ..
make
```

# Test Instruction

## Project 1
### Server
At build folder, run ``./server_assgn1`` to start the server in a terminal. You may kill the server by Ctlr-C.

### Client
At build folder, run ``client_assgn1 ../good_data.txt`` to start the client in a terminal which sends good data to the server.

Run ``./client_assgn1 ../bad_data.txt`` to send some mixed good and bad data to the server which will be processed accordingly.

The input data file format is:

``segment length payload end_identifier``

The other fields of the data packet is fixed. Currently, the payload is a string, and there cannot be a space in the string.

## Project 2
### Server
At build folder, run ``./server_assgn2 ../Verification_Database.txt`` to start the server. ``Verification_Database.txt`` is the database file containing subsriber, technology and payment status.

The server will read in the database and save in its memory for easy table lookup.

### Client
At build folder, run ``./client_assgn2 ../subscriber_input.txt`` to start the client which sends access requests for the subscribers in the input file. The client will need to receive the response from server in 3 seconds. Otherwise, it will retry for 3 times. After all the retries are done, the client will exit.

Once receive the request, the client will print out the information received to indicate the status of the request: paid, unpaid or not exist.

