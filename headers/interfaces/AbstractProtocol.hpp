#pragma once

#include <iostream>


class AbstractProtocol
{
protected:
    int socket_fd = 0;

    template <typename T>
    static void log_buffer_hex(T buffer, size_t size);

    template <typename T>
    ssize_t recv_data(T data, size_t size);
    template <typename T>
    ssize_t send_data(T data, size_t size);

public:
    AbstractProtocol() = default;
    virtual ~AbstractProtocol() = default;

public:
    virtual void handler_loop(int _socket_fd) = 0;
};

#include "AbstractProtocol.tpp" // include template implementation
