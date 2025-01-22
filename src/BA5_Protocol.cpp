#include "BA5_Protocol.hpp"

#define BUFFER_SIZE                (1024U)

#define BA5_HANDSHAKE_MAGIC        (0xFEFFU)


void BA5_Protocol::handler_loop(int _socket_fd)
{
    // set socket fd
    socket_fd = _socket_fd;

    // create a buffer for data exchange
    std::array<uint8_t, BUFFER_SIZE> buffer{};

    /* Send Handshake */
    *reinterpret_cast<uint16_t*>(buffer.data()) = htobe16(BA5_HANDSHAKE_MAGIC);

    try
    {
        send_data(buffer.data(), sizeof(uint16_t));
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::format("Error sending packet: {}", e.what()));
    }

    for (;;)
    {

    }
}
