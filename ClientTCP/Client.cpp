#include "Client.hpp"

Client::Client(asio::io_context& io_context, const std::string& host, const std::string& port)
    : socket_(io_context) {
    tcp::resolver resolver(io_context);
    asio::connect(socket_, resolver.resolve(host, port));
}

void Client::send(const std::string& message) {
    std::cout << "Sending message: " << message << std::endl;
    asio::write(socket_, asio::buffer(message));
}

std::string Client::receive() {
    char reply[1024];
    std::size_t length = asio::read(socket_, asio::buffer(reply, 1024));
    return std::string(reply, length);
}
