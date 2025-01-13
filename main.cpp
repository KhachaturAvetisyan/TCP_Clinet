#include <iostream>

#include "TCP_Client.hpp"
#include "TestProtocol.hpp"
#include "IntercomAppProtocol.hpp"
#include "ScalesProtocol.hpp"
#include "AS3_Protocol.hpp"

#define SERVER_DOMAIN       "192.168.0.92"
//#define SERVER_DOMAIN       "91.103.28.124"
#define SERVER_PORT         5678

int main()
{
    // create a protocol
    const auto protocol = std::make_shared<AS3_Protocol>();

    TCP_Client client(SERVER_DOMAIN, SERVER_PORT, protocol);
    client.run();

}