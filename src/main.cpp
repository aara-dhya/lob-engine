#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/limit_order_book.h"

#define PORT 8080

int main() {
    // 1. Initialize the matching engine
    LimitOrderBook lob;
    std::cout << "[INFO] Limit Order Book Engine Initialized.\n";

    // 2. Create the socket file descriptor (IPv4, TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "[ERROR] Socket creation failed.\n";
        return -1;
    }

    // Optional: Forcefully attach socket to the port to avoid "Address already in use" errors during rapid testing restarts
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // 3. Configure the server network address structure
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 4. Bind the socket to the specific port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[ERROR] Bind failed.\n";
        return -1;
    }

    // 5. Listen for incoming client connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "[ERROR] Listen failed.\n";
        return -1;
    }
    std::cout << "[INFO] TCP Server listening on port " << PORT << "...\n";

    // 6. Accept a client connection (This blocks the thread until a client connects)
    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        std::cerr << "[ERROR] Accept failed.\n";
        return -1;
    }
    std::cout << "[INFO] Client connected! Ready to receive orders.\n";

    // 7. Event Loop: Read incoming data streams
    // 7. Event Loop: Read and Parse incoming data streams
    char buffer[1024] = {0};
    while (true) {
        int valread = read(new_socket, buffer, 1024);
        
        if (valread <= 0) {
            std::cout << "[INFO] Client disconnected.\n";
            break;
        }
        
        // --- THE PARSER ---
        char action[10];
        uint64_t order_id = 0;
        uint64_t price = 0;
        uint32_t quantity = 0;

        // Extract variables directly from the raw char buffer
        int parsed = sscanf(buffer, "%s %lu %lu %u", action, &order_id, &price, &quantity);

        if (parsed >= 2) {
            if (strcmp(action, "BUY") == 0 && parsed == 4) {
                lob.add_order(order_id, price, quantity, Side::BUY);
                std::cout << "[ENGINE] Executed BUY: ID=" << order_id << " Price=" << price << " Qty=" << quantity << "\n";
            } 
            else if (strcmp(action, "SELL") == 0 && parsed == 4) {
                lob.add_order(order_id, price, quantity, Side::SELL);
                std::cout << "[ENGINE] Executed SELL: ID=" << order_id << " Price=" << price << " Qty=" << quantity << "\n";
            } 
            else if (strcmp(action, "CANCEL") == 0 && parsed == 2) {
                lob.cancel_order(order_id);
                std::cout << "[ENGINE] Executed CANCEL: ID=" << order_id << "\n";
            } 
            else {
                std::cout << "[ERROR] Invalid format. Use: BUY/SELL <ID> <PRICE> <QTY> or CANCEL <ID>\n";
            }
        }

        // Clear the buffer for the next incoming message
        memset(buffer, 0, sizeof(buffer)); 
    }
    

    // Clean up file descriptors
    close(new_socket);
    close(server_fd);
    return 0;
}