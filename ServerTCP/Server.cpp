#include "Server.hpp"

Session::Session(tcp::socket socket, int id)
    : socket_(std::move(socket)), id_(id) {}

void Session::start() {
    std::cout << "Client " << id_ << " connected.\n";
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(data_, max_length),
        [this, self](std::error_code ec, std::size_t length) {
            if (!ec) {
                std::string received_data(data_, length);
                std::cout << "Received from client " << id_ << ": " << received_data << "\n";
                do_write(length);
            } else {
                std::cerr << "Error reading from client " << id_ << ": " << ec.message() << "\n";
            }
        });
}

void Session::do_write(std::size_t length) {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(data_, length),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::cout << "Echoed back to client " << id_ << "\n";
                do_read();
            } else {
                std::cerr << "Error writing to client " << id_ << ": " << ec.message() << "\n";
            }
        });
}

Server::Server(asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), next_id_(0) {
    std::cout << "Server is running on port " << port << ".\n";
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket) {
            if (!ec) {
                int client_id = next_id_++;
                std::make_shared<Session>(std::move(socket), client_id)->start();
            } else {
                std::cerr << "Error accepting connection: " << ec.message() << "\n";
            }
            do_accept();
        });
}
