#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include <asio.hpp>
#include <memory>
#include <vector>
#include <functional>

// ClientSession encapsulates one client connection.
class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    using Ptr = std::shared_ptr<ClientSession>;

    // Construct from an accepted socket.
    ClientSession(asio::ip::tcp::socket socket);

    asio::ip::tcp::socket& socket();

    // Start asynchronous reading from the socket.
    // The on_read callback is invoked with any data read.
    void start(std::function<void(const std::vector<char>&, Ptr)> on_read);

    // Set a callback to be notified on disconnection.
    void set_on_disconnect(std::function<void(Ptr)> on_disconnect);

    // Write data asynchronously to the client.
    void do_write(const std::vector<char>& data);

    // Close the connection.
    void close();

private:
    void do_read();

    asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];

    std::function<void(const std::vector<char>&, Ptr)> on_read_;
    std::function<void(Ptr)> on_disconnect_;
};

#endif // CLIENT_SESSION_HPP
