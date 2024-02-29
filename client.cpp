/**
 * @file client.cpp
 * @brief Client program for ordering and consuming burgers from a server.
 * 
 * This program connects to a server using TCP/IP and sends orders for burgers.
 * It waits for the server to respond with the status of the order and simulates
 * eating the burgers that are served.
 * 
 * @author Michael Barry
 */

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <cstdlib>

/**
 * @brief Connects to a server and orders burgers.
 *
 * This function connects to a server using TCP/IP and sends orders for burgers.
 * It waits for the server to respond with the status of the order and simulates
 * eating the burgers that are served.
 *
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @return 0 if the program executed successfully, 1 otherwise.
 */
int main(int argc, char* argv[]) {
    // Default server IP, port, and maximum orders
    const char* serverIP = "127.0.0.1";
    int port = 54321;
    int maxOrders = 10;

    // Parse command line arguments if provided
    if (argc == 4) {
        serverIP = argv[1];
        port = std::stoi(argv[2]);
        maxOrders = std::stoi(argv[3]);
    } else if (argc != 1) {
        // Print usage if incorrect number of arguments provided
        std::cout << "Usage: " << argv[0] << " <ServerIP> <Port> <MaxOrders>" << std::endl;
        return 1;
    }

    // Print connection details
    std::cout << "Connecting to server " << serverIP << " on port " << port << " with a maximum of " << maxOrders << " orders." << std::endl;

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Initialize server address
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return 1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        return 1;
    }

    // Seed for random number generation
    srand(time(nullptr));

    // Send orders to server and receive responses
    for (int i = 0; i < maxOrders; ++i) {
        const char* orderMessage = "Order";
        if (send(sock, orderMessage, strlen(orderMessage), 0) < 0) {
            std::cerr << "Failed to send order. Exiting." << std::endl;
            break;
        }
        std::cout << "Ordered burger #" << i + 1 << std::endl;

        char buffer[1024] = {0};
        buffer[0] = '\0'; // Clear buffer
        int bytesReceived = read(sock, buffer, 1024); // Wait for burger to be served
        if (bytesReceived > 0) {
            std::cout << "Server: " << buffer << std::endl;
            if (strncmp(buffer, "Burger Served", 13) == 0) {
                // Simulate consuming the burger
                int waitTimes[3] = {1, 3, 5};
                int waitTime = waitTimes[rand() % 3];
                std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                std::cout << "Finished eating burger #" << i + 1 << " in " << waitTime << " seconds." << std::endl;
                if (i + 1 < maxOrders) {
                    std::cout << maxOrders - (i + 1) << " burgers left in the order." << std::endl;
                }
            } else if (strncmp(buffer, "No more burgers", 15) == 0) {
                std::cout << "No more burgers available. Exiting." << std::endl;
                break; // Exit if no more burgers can be served
            }
        } else {
            std::cout << "No response from server or error occurred. Exiting." << std::endl;
            break; // Exit if there's an issue receiving server response
        }
    }
    close(sock);
    return 0;
}