#include "server.hpp"
#include "client_session.hpp"
#include <iostream>

Server::Server(asio::io_context& io_context, unsigned short port)
    : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    start_accept();
}

void Server::start_accept() {
    acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec) {
            auto session = std::make_shared<ClientSession>(std::move(socket));

            // Set up a disconnect callback so that if one client disconnects, we end the call.
            session->set_on_disconnect([this](ClientSession::Ptr s) {
                std::cout << "A client disconnected. Terminating the call.\n";
                if (session1_ && session2_) {
                    session1_->close();
                    session2_->close();
                    session1_.reset();
                    session2_.reset();
                }
            });

            // Fill the first available session slot.
            if (!session1_) {
                session1_ = session;
                std::cout << "Client 1 connected.\n";
                session1_->start([this](const std::vector<char>& data, ClientSession::Ptr /*sender*/) {
                    // Relay data to client 2 if connected.
                    if (session2_) {
                        session2_->do_write(data);
                    }
                });
            } else if (!session2_) {
                session2_ = session;
                std::cout << "Client 2 connected.\n";
                session2_->start([this](const std::vector<char>& data, ClientSession::Ptr /*sender*/) {
                    // Relay data to client 1 if connected.
                    if (session1_) {
                        session1_->do_write(data);
                    }
                });
            } else {
                // Reject any additional connection beyond two.
                std::cerr << "Rejecting connection: already two clients connected.\n";
                session->close();
            }
        } else {
            std::cerr << "Accept error: " << ec.message() << "\n";
        }
        // Continue accepting if there is room for another session.
        if (!session1_ || !session2_) {
            start_accept();
        }
    });
}
