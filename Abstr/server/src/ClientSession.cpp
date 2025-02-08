#include "../include/ClientSession.hpp"
#include <iostream>
#include <cstring>

ClientSession::ClientSession(asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
{
}

asio::ip::tcp::socket& ClientSession::socket() {
    return socket_;
}

void ClientSession::start(std::function<void(const std::vector<char>&, Ptr)> on_read) {
    on_read_ = on_read;
    do_read();
}

void ClientSession::set_on_disconnect(std::function<void(Ptr)> on_disconnect) {
    on_disconnect_ = on_disconnect;
}

void ClientSession::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(data_, max_length),
        [this, self](std::error_code ec, std::size_t length) {
            if (!ec) {
                std::vector<char> msg(data_, data_ + length);
                if (on_read_) {
                    on_read_(msg, self);
                }
                do_read();
            } else {
                std::cerr << "Client disconnected: " << ec.message() << "\n";
                if (on_disconnect_) {
                    on_disconnect_(self);
                }
                close();
            }
        });
}

void ClientSession::do_write(const std::vector<char>& data) {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(data.data(), data.size()),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << "\n";
                if (on_disconnect_) {
                    on_disconnect_(self);
                }
                close();
            }
        });
}

void ClientSession::close() {
    std::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}
