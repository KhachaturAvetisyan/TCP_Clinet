#pragma once

#include <sstream>
#include <iomanip>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <climits>


constexpr int RECV_FLAGS = 0; // Replace it with actual flags if needed
constexpr int SEND_FLAGS = 0; // Replace it with actual flags if needed

constexpr size_t CHUNK_SIZE = 256U; // Replace it with actual value


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
ssize_t AbstractProtocol::recv_data(T data, size_t size)
{
    std::cout << "Reading " << size << " bytes data ..." << std::endl;

    ssize_t bytes_received = 0;
    ssize_t result;
    size_t block_size = CHUNK_SIZE;

    // check size
    if (size == 0)
    {
        throw std::invalid_argument("Invalid size");
    }
    if (size > static_cast<size_t>(INT_MAX))
    {
        throw std::out_of_range("Size too big");
    }

    // Check buffer (only if T is a pointer)
    if constexpr (std::is_pointer_v<T>)
    {
        if (data == nullptr)
        {
            throw std::invalid_argument("Buffer must not be nullptr");
        }
    }
    else
    {
        throw std::invalid_argument("Buffer must be a pointer");
    }


    while (bytes_received < static_cast<ssize_t>(size))
    {
        // Calculate bytes to read
        block_size = std::min(block_size, size - bytes_received);

        // check block size
        if (block_size == 0)
        {
            throw std::logic_error("Invalid block size");
        }

        // rcv data
        result = recv(socket_fd, reinterpret_cast<char*>(data) + bytes_received, block_size, RECV_FLAGS);

        // check return value
        if (result == 0)
        {
            break;
        }
        else if (result < 0)
        {
            switch (errno)
            {
                case EINTR:
                    std::cout << "Interrupted system call, retrying..." << std::endl;
                    continue;

                case EAGAIN:
                    throw std::runtime_error("Resource temporarily unavailable: timeout !!");

                default:
                    throw std::runtime_error("Error receiving data: " + std::string(strerror(errno)));
            }
        }
        else
        {
            bytes_received += result;
        }
    } // while

    // check bytes received
    if (bytes_received <= 0 || bytes_received > INT_MAX)
    {
        throw std::out_of_range("Invalid bytes received: " + std::to_string(bytes_received));
    }
    if (bytes_received != static_cast<ssize_t>(size))
    {
        throw std::runtime_error("Reading data failed, received: " + std::to_string(bytes_received));
    }

    // log data
    log_buffer_hex(data, bytes_received);

    return bytes_received;
}

template <typename T>
ssize_t AbstractProtocol::send_data(T data, size_t size)
{
    std::cout << "Sending " << size << " bytes data ..." << std::endl;

    ssize_t bytes_sent = 0;
    ssize_t result;
    size_t block_size = CHUNK_SIZE;

    // check size
    if (size == 0)
    {
        throw std::invalid_argument("Invalid size");
    }
    if (size > static_cast<size_t>(INT_MAX))
    {
        throw std::out_of_range("Size too big");
    }

    // Check buffer (only if T is a pointer)
    if constexpr (std::is_pointer_v<T>)
    {
        if (data == nullptr)
        {
            throw std::invalid_argument("Buffer must not be nullptr");
        }
    }
    else
    {
        throw std::invalid_argument("Buffer must be a pointer");
    }


    while (bytes_sent < static_cast<ssize_t>(size))
    {
        // Calculate bytes to read
        block_size = std::min(block_size, size - bytes_sent);

        // check block size
        if (block_size == 0)
        {
            throw std::logic_error("Invalid block size");
        }

        // send data
        result = send(socket_fd, reinterpret_cast<const char*>(data) + bytes_sent, block_size, SEND_FLAGS);

        // check return value
        if (result == 0)
        {
            break;
        }
        else if (result < 0)
        {
            switch (errno)
            {
                case EINTR:
                    std::cout << "Interrupted system call, retrying..." << std::endl;
                    continue;

                case EAGAIN:
                    throw std::runtime_error("Resource temporarily unavailable: timeout !!");

                default:
                    throw std::runtime_error("Error sending data: " + std::string(strerror(errno)));
            }
        }
        else
        {
            bytes_sent += result;
        }
    } // while

    // check bytes sent
    if (bytes_sent <= 0 || bytes_sent > INT_MAX)
    {
        throw std::out_of_range("Invalid bytes sent: " + std::to_string(bytes_sent));
    }
    if (bytes_sent != static_cast<ssize_t>(size))
    {
        throw std::runtime_error("Sending data failed, sent: " + std::to_string(bytes_sent));
    }

    // log data
    log_buffer_hex(data, bytes_sent);

    return bytes_sent;
}
