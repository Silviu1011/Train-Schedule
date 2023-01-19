# Train Schedule

Train Schedule is a client-server app developed in C++ using C functions for communication between clients. The server is multithreaded which allows it to accept multiple connections from different clients and provides information about the train schedule in the current day. The data about the trains is read from a xml file using rapidxml.

# Usage

For compilation and execution:

```bash
g++ client.cpp -o client
./client 127.0.0.1 2048

g++ server.cpp -o server -lpthread
./server
```

### The IP address may differ depanding on where the server process is located. For this example, the server is located on the same system as the client.