#include <iostream>

#include "TCP_Client.hpp"
#include "TestProtocol.hpp"
#include "IntercomAppProtocol.hpp"
#include "ScalesProtocol.hpp"
#include "AS3_Protocol.hpp"
#include "LV_Protocol.hpp"
#include "BA5_Protocol.hpp"

//#define SERVER_DOMAIN       "192.168.0.92"
#define SERVER_DOMAIN       "91.103.28.124"
// #define SERVER_DOMAIN       "127.0.0.1"
#define SERVER_PORT         5681

int main()
{
    // create a protocol
    const auto protocol = std::make_shared<LV_Protocol>();

    TCP_Client client(SERVER_DOMAIN, SERVER_PORT, protocol);
    client.run();

}