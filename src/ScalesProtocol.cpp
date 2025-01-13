#include "ScalesProtocol.hpp"

#include <string>
#include <unistd.h>

void ScalesProtocol::handler_loop(int _socket_fd)
{
    // set socket fd
    socket_fd = _socket_fd;

    std::array<uint8_t, 128> buffer{0x3d, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2d};

    while (true)
    {
        send_data(buffer.data(), 8);
        std::cout << "ScalesProtocol::handler_loop()" << std::endl;

        sleep(30);
    }
}