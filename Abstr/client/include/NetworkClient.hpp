#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <asio.hpp>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <functional>

class NetworkClient {
public:
    using ReceiveHandler = std::function<void(const std::vector<char>&)>;

    NetworkClient(asio::io_context& ioContext, const std::string& host, unsigned short port);
    ~NetworkClient();

    void start(ReceiveHandler handler);
    void stop();
    void send(const std::vector<char>& data);

private:
    void doConnect();
    void doRead();
    void doWrite();

    asio::io_context& ioContext;
    asio::ip::tcp::socket socket;
    std::string host;
    unsigned short port;
    
    std::queue<std::vector<char>> sendQueue;
    std::mutex queueMutex;
    std::atomic<bool> running{false};
    ReceiveHandler receiveHandler;
    std::thread networkThread;
};

#endif
