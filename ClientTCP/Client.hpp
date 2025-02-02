#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <asio.hpp>
#include <iostream>
#include <string>

using asio::ip::tcp;

class Client {
public:
    Client(asio::io_context& io_context, const std::string& host, const std::string& port);
    void send(const std::string& message);
    std::string receive();

private:
    tcp::socket socket_;
};

#endif // CLIENT_HPP
