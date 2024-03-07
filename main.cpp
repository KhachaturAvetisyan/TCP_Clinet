#include <iostream>

#include "TCP_Client.hpp"
#include "TestProtocol.hpp"

int main()
{
    // create a protocol
    const auto protocol = std::make_shared<TestProtocol>();

    TCP_Client client("127.0.0.1", 59687, protocol);
    try
    {
        client.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }


}