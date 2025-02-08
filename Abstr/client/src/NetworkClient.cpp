#include "../include/NetworkClient.hpp"
#include <iostream>

NetworkClient::NetworkClient(asio::io_context& ioContext, const std::string& host, unsigned short port)
    : ioContext(ioContext), socket(ioContext), host(host), port(port) {}

NetworkClient::~NetworkClient() { stop(); }

void NetworkClient::start(ReceiveHandler handler) {
    receiveHandler = handler;
    running = true;
    networkThread = std::thread([this]() {
        doConnect();
        while(running) {
            ioContext.restart();
            ioContext.run();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

void NetworkClient::stop() {
    running = false;
    if(networkThread.joinable()) networkThread.join();
    if(socket.is_open()) {
        asio::error_code ec;
        socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket.close(ec);
    }
}

void NetworkClient::send(const std::vector<char>& data) {
    std::lock_guard<std::mutex> lock(queueMutex);
    sendQueue.push(data);
}

void NetworkClient::doConnect() {
    asio::ip::tcp::resolver resolver(ioContext);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
    
    asio::async_connect(socket, endpoints, [this](std::error_code ec, const asio::ip::tcp::endpoint&) {
        if(!ec) {
            std::cout << "Connected to server\n";
            doRead();
            doWrite();
        } else {
            std::cerr << "Connection error: " << ec.message() << "\n";
        }
    });
}

void NetworkClient::doRead() {
    auto buffer = std::make_shared<std::array<char, 4096>>();
    socket.async_read_some(asio::buffer(*buffer), [this, buffer](std::error_code ec, size_t length) {
        if(!ec) {
            std::vector<char> data(buffer->begin(), buffer->begin() + length);
            if(receiveHandler) receiveHandler(data);
            doRead();
        } else {
            if(ec != asio::error::operation_aborted)
                std::cerr << "Read error: " << ec.message() << "\n";
            stop();
        }
    });
}

void NetworkClient::doWrite() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if(sendQueue.empty()) {
        asio::post(ioContext, [this]() { doWrite(); });
        return;
    }
    
    auto data = std::make_shared<std::vector<char>>(std::move(sendQueue.front()));
    sendQueue.pop();
    
    asio::async_write(socket, asio::buffer(*data), [this, data](std::error_code ec, size_t) {
        if(ec) {
            std::cerr << "Write error: " << ec.message() << "\n";
            stop();
        } else {
            doWrite();
        }
    });
}
