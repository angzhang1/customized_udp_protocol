# customized_udp_protocol
Implement a customized protocol on top of UDP. Simulate the functionalities for both server and client.

# Build Instruction
The project uses cmake. Please follow the steps to build the code. Tested with ``clang-902.0.39.1`` on Mac OSX.

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

Run ``client_assgn1 ../bad_data.txt`` to send some mixed good and bad data to the server which will be processed accordingly.

You may kill the client by Ctlr-C (not immediately, though).

