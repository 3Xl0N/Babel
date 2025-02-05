#include "client.hpp"
#include <asio.hpp>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: client <server_ip> <server_port>\n";
        return 1;
    }
    std::string server_ip = argv[1];
    unsigned short server_port = static_cast<unsigned short>(std::atoi(argv[2]));

    try {
        asio::io_context io_context;
        Client client(io_context, server_ip, server_port);
        client.start();
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
