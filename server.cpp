/**
 * @file server.cpp
 * @brief Server program for handling burger orders from clients.
 * 
 * This program simulates a server that prepares and serves burgers to clients.
 * It uses multiple threads to handle chef tasks (preparing burgers) and client requests (ordering burgers).
 * 
 * @author Michael Barry
 */

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <atomic>
#include <queue>

using namespace std;

// Function declarations
void chefFunction(int id);
void clientHandler(int clientSocket);

// Global Variables
mutex mtx; // Mutex for synchronization
condition_variable cv_burger_ready; // Condition variable for burger availability
atomic<int> burgersPrepared(0); // Atomic counter for burgers prepared
atomic<int> burgersServed(0); // Atomic counter for burgers served
int maxBurgers = 25; // Maximum number of burgers to prepare
int numChefs = 2; // Number of chef threads
atomic<bool> serverRunning(true); // Atomic flag to indicate server status
int server_fd; // Server socket file descriptor

/**
 * @brief The main function for the burger shop server.
 * 
 * @param argc The number of command line arguments.
 * @param argv An array of command line arguments.
 * @return int Returns 0 on successful execution, 1 otherwise.
 */
int main(int argc, char* argv[]) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(54321);

    // Parse command line arguments
    if (argc > 1) {
        if (argc == 3) {
            maxBurgers = atoi(argv[1]);
            numChefs = atoi(argv[2]);
        } else {
            cout << "Usage: " << argv[0] << " <MaxBurgers> <NumChefs>" << endl;
            return 1;
        }
    }

    // Start the server
    cout << "Server listening on port 54321 with " << maxBurgers << " burgers and " << numChefs << " chefs." << endl;

    // Create chef threads
    vector<thread> chefs;
    for (int i = 0; i < numChefs; ++i) {
        chefs.emplace_back(chefFunction, i + 1);
    }

    // Bind and listen for client connections
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    // Accept and handle client connections
    vector<thread> clientThreads;
    while (serverRunning) {
        int clientSocket = accept(server_fd, nullptr, nullptr);
        if (clientSocket >= 0) {
            clientThreads.emplace_back(clientHandler, clientSocket);
        }
    }

    // Wait for all client threads to finish
    for (auto& clientThread : clientThreads) {
        if (clientThread.joinable()) {
            clientThread.join();
        }
    }

    // Ensure all chefs finish their work
    for (auto& chef : chefs) {
        if (chef.joinable()) {
            chef.join();
        }
    }

    cout << "Customer denied. No more burgers. Server shutting down." << endl;
    close(server_fd); // Close the server socket
    return 0;
}

/**
 * @brief Function executed by each chef thread.
 *
 * This function simulates a chef preparing burgers. It generates a random
 * preparation time for each burger and notifies the server when a burger is ready.
 *
 * @param id The ID of the chef thread.
 */
void chefFunction(int id) {
    while (true) {
        int preparationTime = (rand() % 2 == 0) ? 2 : 4; // Random preparation time of 2 or 4 seconds
        {
            unique_lock<mutex> lock(mtx);
            if (burgersPrepared >= maxBurgers) break;
            burgersPrepared++;
            cout << "Chef " << id << " prepared burger #" << burgersPrepared << " in " << preparationTime << " seconds. " << (maxBurgers - burgersPrepared) << " burgers left to prepare." << endl;
        }
        cv_burger_ready.notify_all(); // Notify all waiting on this condition
        this_thread::sleep_for(chrono::seconds(preparationTime)); // Simulate preparation time
    }
}

/**
 * @brief Function to handle client requests.
 *
 * This function is executed for each client connection. It receives orders from clients,
 * serves burgers if available, and handles client disconnections.
 *
 * @param clientSocket The client socket file descriptor.
 */
void clientHandler(int clientSocket) {
    char orderBuffer[1024];
    int ordersProcessed = 0;

    while (serverRunning && ordersProcessed < maxBurgers) {
        memset(orderBuffer, 0, sizeof(orderBuffer)); // Clear the buffer
        int bytesReceived = recv(clientSocket, orderBuffer, sizeof(orderBuffer), 0); // Wait for order

        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                cout << "Client disconnected. Order Done." << endl;
            } else {
                cout << "Error occurred in receiving. Stopping handler." << endl;
            }
            break; // Exit if error in receiving or client disconnected
        }

        unique_lock<mutex> lock(mtx);
        if (strcmp(orderBuffer, "Order") == 0 && burgersPrepared > burgersServed) {
            burgersServed++;
            send(clientSocket, "Burger Served", strlen("Burger Served"), 0);
            cout << "Served burger #" << burgersServed << " to client." << endl;
            ordersProcessed++;
            if (burgersServed >= maxBurgers) {
                serverRunning = false; // Stop the server once all burgers are served
                cv_burger_ready.notify_all(); // Wake up any waiting clients
                cout << "No more burgers to serve. Accepting no more customers (Press 'CTRL + C' to exit)" << endl;
                send(clientSocket, "No more burgers", strlen("No more burgers"), 0); // Notify the last client
                break; // Break out of the loop to end the client session
            }
        } else {
            cv_burger_ready.wait(lock); // Wait for a burger to be ready
        }
    }

    close(clientSocket); // Close the client socket
}