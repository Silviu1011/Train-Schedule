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

## The IP address may differ depanding on where the server process is located. For this example, the server is located on the same system as the client.

The command protocol contains 8 different commands:

* ```get_trains_info``` - information about the schedule of the trains that are running on the current datel

* ```get_arrival_info [station]``` - information about the arrival schedule of the trains that are running in the current hour;

* ```get_departure_info [station]``` - information about the departure schudule of the trains that are running the current hour;

* ```add_arrival_delay <ID> <station> <delay>``` - modifies the arrival delay for a specific train and station (works only if a user is connected);

* ```add_departute_delay <ID> <station> <delay>``` - modifies the departure delay for a specific train and station (works only if a user is connected);

* ```login <username>``` - connects a user based on a username;

* ```logout``` - disconnects the connected user;

* ```close``` - closes the client;

# License

[MIT](https://choosealicense.com/licenses/mit/)