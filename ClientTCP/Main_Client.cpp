#include "Client.hpp"
#include <asio.hpp>

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>\n";
            return 1;
        }

        asio::io_context io_context;

        Client client(io_context, argv[1], argv[2]);

        std::string message = "helloooooo";
        std::cout << "Sending: " << message << std::endl;
        client.send(message);

        std::string reply = client.receive();
        std::cout << "Reply: " << reply << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
