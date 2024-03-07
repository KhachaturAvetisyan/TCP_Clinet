#include "TCP_Client.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <fcntl.h>


TCP_Client::TCP_Client(const std::string& _ip, const uint16_t _port, std::shared_ptr<AbstractProtocol> _protocol) :
    ip(inet_addr(_ip.c_str())),
    port(_port),
    protocol(std::move(_protocol))
{}

TCP_Client::~TCP_Client()
{
    stop();
}


void TCP_Client::create_socket()
{
    // Initialize protocol
    const protoent* proto = getprotobyname("tcp");

    // Check protocol
    if (proto == nullptr)
    {
        throw std::runtime_error("Get protocol failed: " + std::string(strerror(errno)));
    }

    // Creating socket file descriptor
    if ((client_socket = socket(PF_INET, SOCK_STREAM, proto->p_proto)) < 0)
    {
        throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
    }

#ifdef NON_BLOCKING
    // Set the socket to non-blocking
    if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0)
    {
        close(client_socket);
        throw std::runtime_error("Set socket to non-blocking failed: " + std::string(strerror(errno)));
    }
#endif // NON_BLOCKING

    // init timeval struct
    constexpr timeval wrt_timeval = {CLIENT_SOCKET_SEND_TIMEOUT, 0};
    constexpr timeval rcv_timeval = {CLIENT_SOCKET_RCV_TIMEOUT, 0};

    // Setting timeout for receive
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeval, sizeof(rcv_timeval)) < 0)
    {
        close(client_socket);
        throw std::runtime_error("Set socket options failed: " + std::string(strerror(errno)));
    }

    // Setting timeout for send
    if (setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &wrt_timeval, sizeof(wrt_timeval)) < 0)
    {
        close(client_socket);
        throw std::runtime_error("Set socket options failed: " + std::string(strerror(errno)));
    }
}

void TCP_Client::run()
{
    // try create socket
    try
    {
        create_socket();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("TCP_Client::run : " + std::string(e.what()));
    }

    // init sockaddr_in struct
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ip;

    // Check is port available
    if (serv_addr.sin_port == 0)
    {
        close(client_socket);
        throw std::runtime_error("Port is 0");
    }

    // connect to server
    if (connect(client_socket, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) != 0
#ifdef NON_BLOCKING
    && errno != EINPROGRESS
#endif // NON_BLOCKING
        )
    {
        close(client_socket);
        throw std::runtime_error("Connect failed: " + std::string(strerror(errno)));
    }

    // initialize ip
    char ip_s[INET_ADDRSTRLEN];

    // check serv_addr
    if (inet_ntop(serv_addr.sin_family, &serv_addr.sin_addr, ip_s, sizeof(ip_s)) == nullptr)
    {
        close(client_socket);
        throw std::runtime_error("inet_ntop failed: " + std::string(strerror(errno)));
    }

    // print server ip and port
    std::cout << "Connected to server: " << ip_s << ":" << ntohs(serv_addr.sin_port) << std::endl;

    // run protocol
    protocol->handler_loop(client_socket);
}

void TCP_Client::stop() const
{
    close(client_socket);
}