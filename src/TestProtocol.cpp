#include "TestProtocol.hpp"

#include <unistd.h>
#include <vector>

TestProtocol::TestProtocol()
{
    // TestProtocol arguments initialization
}

void TestProtocol::handler_loop(const int _socket_fd)
{
    socket_fd = _socket_fd;

    // print socket fd number
    std::cout << "Socket fd: " << socket_fd << std::endl;

    std::array<char, 1024> buffer{};

    for (;;)
    {
        std::cin.getline(buffer.data(), buffer.size());

        try
        {
            send_data(buffer.data(), buffer.size());
        }
        catch (const std::exception& e)
        {
            std::cerr << "send error: " << e.what() << std::endl;
            return;
        }

        sleep(5);
    }
}
