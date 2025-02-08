#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include <asio.hpp>
#include <memory>
#include <vector>
#include <functional>

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    using Ptr = std::shared_ptr<ClientSession>;

    ClientSession(asio::ip::tcp::socket socket);

    asio::ip::tcp::socket& socket();

    void start(std::function<void(const std::vector<char>&, Ptr)> on_read);

    void set_on_disconnect(std::function<void(Ptr)> on_disconnect);

    void do_write(const std::vector<char>& data);

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
