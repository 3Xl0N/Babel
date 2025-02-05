#include "server.hpp"
#include <asio.hpp>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: server <port>\n";
        return 1;
    }
    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));

    try {
        asio::io_context io_context;
        Server server(io_context, port);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
