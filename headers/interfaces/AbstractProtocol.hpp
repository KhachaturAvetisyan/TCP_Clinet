#pragma once

#include <iostream>

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#elif defined(__linux__)
#include <arpa/inet.h>
#endif

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
