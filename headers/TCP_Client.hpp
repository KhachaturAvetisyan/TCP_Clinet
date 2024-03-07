#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <netinet/in.h>

#include "AbstractProtocol.hpp"


#define CLIENT_SOCKET_SEND_TIMEOUT          30U
#define CLIENT_SOCKET_RCV_TIMEOUT           30U


class TCP_Client
{
private:
    std::shared_ptr<AbstractProtocol> protocol;

    in_addr_t ip;
    uint16_t port;
    int client_socket = 0;

private:
    void create_socket();

public:
    void run();
    void stop() const;

public:
    TCP_Client(const std::string& _ip, uint16_t _port, std::shared_ptr<AbstractProtocol> _protocol);
    ~TCP_Client();
};