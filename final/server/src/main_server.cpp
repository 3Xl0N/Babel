#include "../include/ClientSession.hpp"
#include "../include/Server.hpp"
#include <asio.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    try {
        asio::io_context ioContext;
        Server server(ioContext, std::stoi(argv[1]));
        ioContext.run();
    } catch(const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
}
