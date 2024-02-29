# Burger Shop Simulation

This repository contains a simple client-server application that simulates a burger shop. The application allows multiple clients to connect, place orders for burgers, and receive updates when their burgers are served by chefs. This simulation aims to demonstrate basic client-server interactions and multi-threading in C++.

## How It Works

### Server
- Listens for client connections on port `54321`.
- Manages a set number of chefs who prepare burgers in random order and time (either 2 or 4 seconds).
- Accepts "Order" requests from clients.
- Once all burgers are served, the server gracefully shuts down.
- Handles client disconnections and errors gracefully.

### Client
- Connects to the server on port `54321`.
- Can order multiple burgers.
- Receives notifications when burgers are served.

## Running the Application

### Prerequisites
- A C++ compiler (e.g., GCC).
- Basic understanding of C++ programming.

### Compilation
Navigate to the directory containing the source code. Compile the server and client executables using a C++ compiler with the following commands:
```bash
g++ -o burger_shop_server server.cpp -lpthread
g++ -o burger_shop_client client.cpp
```

## Execution

### Server
To run the server, use the following command:
```bash
./burger_shop_server [MaxBurgers] [NumChefs]
```
- 'MaxBurgers': Maximum number of burgers the server can manage (default 25).
- 'NumChefs': Number of chefs available to prepare the burgers (default 2).

### Client
To connect as a client, use the following command:
```bash
./burger_shop_client [ServerIP] [Port] [MaxOrders]
```
- 'ServerIP': IP Address of server (default 127.0.0.1).
- 'Port': Port of server (default 54321).
- 'MaxOrders': Maximum number of orders the client will make (default 10).


## Termination
To gracefully shut down the server or client, press 'CTRL + C' in the terminal window.

## Additional Details
For more in-depth information about the program's functionality and specific implementation details, please refer to the source code files server.cpp and client.cpp.
