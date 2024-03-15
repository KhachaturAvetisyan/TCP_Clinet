#include <iostream>

#include "TCP_Client.hpp"
#include "TestProtocol.hpp"

int main()
{
    // create a protocol
    const auto protocol = std::make_shared<TestProtocol>();

    TCP_Client client("127.0.0.1", 9846, protocol);
    client.run();

}