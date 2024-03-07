#pragma once

#include <iostream>


#define RECV_FLAGS          0
#define SEND_FLAGS          0


class AbstractProtocol
{
protected:
    int socket_fd = 0;

    template <typename T>
    static void log_buffer_hex(T buffer, size_t size);

    template <typename T>
    void read_data_poll(T data, size_t size, uint16_t timeout);
    template <typename T>
    void send_data_poll(T data, size_t size, uint16_t timeout);

    template <typename T>
    void recv_data(T data, size_t size);
    template <typename T>
    void send_data(T data, size_t size);

public:
    AbstractProtocol() = default;
    virtual ~AbstractProtocol() = default;

public:
    virtual void handler_loop(int _socket_fd) = 0;
};

#include "AbstractProtocol.tpp" // include template implementation
