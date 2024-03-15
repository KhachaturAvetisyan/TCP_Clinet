#pragma once

#include <sstream>
#include <iomanip>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

#include "AbstractProtocol.hpp"


template <typename T>
void AbstractProtocol::log_buffer_hex(T buffer, const size_t size)
{
    std::ostringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; ++i)
    {
        hex_stream << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(buffer[i])) << " ";
    }

    // Log the hexadecimal representation
    std::cout << "Data: " << hex_stream.str() << std::endl;
}

template <typename T>
void AbstractProtocol::read_data_poll(T data, size_t size, const uint16_t timeout)
{
    std::cout << "Reading data, timeout " << timeout << std::endl;

    // clear buffer
    std::memset(data, 0, size);

    // init poll struct
    pollfd mypoll = { socket_fd, POLLIN|POLLPRI };
    ssize_t retval = 0;

    if (poll(&mypoll, 1, timeout * 1000) <= 0)
    {
        retval = read(socket_fd, data, size);

        if(retval < 0)
        {
            throw std::runtime_error("Error reading data: " + std::string(strerror(errno)));
        }
        else if(retval == 0)
        {
            throw std::runtime_error("Connection closed: " + std::string(strerror(errno)));
        }
        else
        {
            if (retval !=  size)
            {
                throw std::runtime_error("Reading data failed: " + std::string(strerror(errno)));
            }
        }

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
        log_buffer_hex(data, size);
#endif

    }
    else
    {
        throw std::runtime_error("Reading data timeout: " + std::string(strerror(errno)));
    }
}

template <typename T>
void AbstractProtocol::send_data_poll(T data, size_t size, const uint16_t timeout)
{
    std::cout << "Send data, timeout " << timeout << std::endl;

    // init poll struct
    pollfd mypoll = { socket_fd, POLLOUT|POLLPRI };
    ssize_t retval = 0;

    if (poll(&mypoll, 1, timeout * 1000))
    {
        retval = write(socket_fd, data, size);

        if(retval < 0)
        {
            throw std::runtime_error("Error sending data: " + std::string(strerror(errno)));
        }

        if (retval !=  size)
        {
            throw std::runtime_error("Sending data failed: " + std::string(strerror(errno)));
        }

#ifdef DEBUG
        log_buffer_hex(data, size);
#endif

    }
    else
    {
        throw std::runtime_error("Sending data timeout: " + std::string(strerror(errno)));
    }
}

template <typename T>
void AbstractProtocol::recv_data(T data, size_t size)
{
    std::cout << "Receiving data" << std::endl;

    // clear buffer
    std::memset(data, 0, size);

    // init variables
    const ssize_t retval = recv(socket_fd, data, size, RECV_FLAGS);

    // check return value
    if (retval == 0)
    {
        throw std::runtime_error("Connection closed: " + std::string(strerror(errno)));
    }
    else if (retval < 0)
    {
        throw std::runtime_error("Error receiving data: " + std::string(strerror(errno)));
    }
    if (retval != size)
    {
        throw std::runtime_error("Receiving data failed: " + std::string(strerror(errno)));
    }

#ifdef DEBUG
    log_buffer_hex(data, size);
#endif
}

template <typename T>
void AbstractProtocol::send_data(T data, size_t size)
{
    std::cout << "Sending data" << std::endl;

    // init variable
    const ssize_t retval = send(socket_fd, data, size, SEND_FLAGS);

    // check return value
    if (retval < 0)
    {
        throw std::runtime_error("Error sending data: " + std::string(strerror(errno)) + " <-> " + std::to_string(errno));
    }

    if (retval != size)
    {
        throw std::runtime_error("Sending data failed: " + std::string(strerror(errno)));
    }

#ifdef DEBUG
    log_buffer_hex(data, size);
#endif
}
