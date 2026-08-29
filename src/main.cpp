#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include "../include/limit_order_book.h"

#define PORT 8080

int main() {
    // 1. Initialize the matching engine
    LimitOrderBook lob;
    std::cout << "[INFO] Limit Order Book Engine Initialized.\n";

    // 2. Create the socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "[ERROR] Socket creation failed.\n";
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // 3. Configure address
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 4. Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[ERROR] Bind failed.\n";
        return -1;
    }

    // 5. Listen
    if (listen(server_fd, 3) < 0) {
        std::cerr << "[ERROR] Listen failed.\n";
        return -1;
    }
    std::cout << "[INFO] TCP Server listening on port " << PORT << "...\n";

    // 6. Accept
    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        std::cerr << "[ERROR] Accept failed.\n";
        return -1;
    }
    std::cout << "[INFO] Client connected! Ready to receive orders.\n";

    // 7. Event Loop
    char buffer[1024] = {0};
    while (true) {
        int valread = read(new_socket, buffer, 1024);
        
        if (valread <= 0) {
            std::cout << "[INFO] Client disconnected.\n";
            break;
        }
        
        char action[10];
        uint64_t order_id = 0;
        uint64_t price = 0;
        uint32_t quantity = 0;

        int parsed = sscanf(buffer, "%s %lu %lu %u", action, &order_id, &price, &quantity);

        if (parsed >= 2) {
            auto start_time = std::chrono::high_resolution_clock::now();

            if (strcmp(action, "BUY") == 0 && parsed == 4) {
                lob.add_order(order_id, price, quantity, Side::BUY);
            } 
            else if (strcmp(action, "SELL") == 0 && parsed == 4) {
                lob.add_order(order_id, price, quantity, Side::SELL);
            } 
            else if (strcmp(action, "CANCEL") == 0 && parsed == 2) {
                lob.cancel_order(order_id);
            } 
            else {
                std::cout << "[ERROR] Invalid format.\n";
                memset(buffer, 0, sizeof(buffer));
                continue;
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

            std::cout << "[ENGINE] Executed " << action << " | ID=" << order_id << " | Latency: " << latency << " ns\n";
        }

        memset(buffer, 0, sizeof(buffer)); 
    }

    close(new_socket);
    close(server_fd);
    return 0;
}