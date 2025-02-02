#ifndef SERVER_HPP
#define SERVER_HPP

#include <asio.hpp>
#include <memory>
#include <iostream>
#include <atomic>

using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, int id);
    void start();

private:
    void do_read();
    void do_write(std::size_t length);

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    int id_; // ID unique du client
};

class Server {
public:
    Server(asio::io_context& io_context, short port);

private:
    void do_accept();

    tcp::acceptor acceptor_;
    std::atomic<int> next_id_; // Compteur pour générer des ID uniques
};

#endif // SERVER_HPP
