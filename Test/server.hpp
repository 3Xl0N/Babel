#ifndef SERVER_HPP
#define SERVER_HPP

#include <asio.hpp>
#include <memory>
#include "client_session.hpp"

// The Server class accepts TCP connections and bridges audio data between two clients.
class Server {
public:
    Server(asio::io_context& io_context, unsigned short port);

private:
    void start_accept();

    asio::ip::tcp::acceptor acceptor_;
    // We allow two sessions maximum.
    std::shared_ptr<ClientSession> session1_;
    std::shared_ptr<ClientSession> session2_;
};

#endif // SERVER_HPP
