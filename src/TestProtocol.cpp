#include "TestProtocol.hpp"

#include <unistd.h>

TestProtocol::TestProtocol()
{
    // TestProtocol arguments initialization
}

void TestProtocol::handler_loop(const int _socket_fd)
{
    socket_fd = _socket_fd;

    // print socket fd number
    std::cout << "Socket fd: " << socket_fd << std::endl;

    // TestProtocol handler loop
    std::array<uint8_t, 256> buffer{"hello world"};

    while (true)
    {
        sleep(2);

        try
        {
            send_data(buffer.data(), buffer.size());
        }
        catch (const std::exception& e)
        {
            std::cerr << "send error: " << e.what() << std::endl;
            return;
        }

        try
        {
            recv_data(buffer.data(), buffer.size());
        }
        catch (const std::exception& e)
        {
            std::cerr << "recv error: " << e.what() << std::endl;
            return;
        }

        sleep(5);
    }
}
