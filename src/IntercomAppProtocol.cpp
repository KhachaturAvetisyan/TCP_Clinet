#include "IntercomAppProtocol.hpp"

#include <arpa/inet.h>
#include <numeric>
#include <unistd.h>

#define HAND_SHAKE_STARTBYTE                    0XFE
#define PING_DATA_STARTBYTE                     0XA1
#define HISTORY_DATA_STARTBYTE                  0XA2
#define REQUEST_FOR_UPDATE_STARTBYTE            0xB1
#define DATA_BODY_STARTBYTE                     0xB2
#define UPDATE_TEMPORARY_PIN_STARTBYTE          0xB3
#define FORCE_OPEN_STARTBYTE                    0xB4
#define RESET_TEMPORARY_PIN_LIST_STARTBYTE      0xB5
#define CHANGE_OPEN_TIME_STARTBYTE              0xB6

#define HAND_SHAKE_PACKET_SIZE                  16U
#define PING_PACKET_SIZE                        20U
#define HISTORY_PACKET_SIZE                     15U
#define REQUEST_FOR_UPDATE_PACKET_SIZE          10U
#define UPDATE_TEMPORARY_PIN_PACKET_SIZE        16U
#define FORCE_OPEN_PACKET_SIZE                  15U
#define RESET_TEMPORARY_PIN_LIST_PACKET_SIZE    27U
#define CHANGE_OPEN_TIME_PACKET_SIZE            20U


void IntercomAppProtocol::handler_loop(int _socket_fd)
{
    // set socket fd
    socket_fd = _socket_fd;

    // create buffer
    std::array<uint8_t, 256> buffer{};

    // create handshake packet
    // write start byte
    buffer[0] = HAND_SHAKE_STARTBYTE;

    // write imei
    std::string imei = "12345678909";

    std::copy(imei.begin(), imei.end(), buffer.begin() + 1);

    // send handshake packet
    try
    {
        send_data(buffer.data(), HAND_SHAKE_PACKET_SIZE);
    }
    catch (const std::exception& e)
    {
        std::cerr << "send error: " << e.what() << std::endl;
        return;
    }

    // read handshake response
    try
    {
        recv_data(buffer.data(), 1);
    }
    catch (const std::exception& e)
    {
        std::cerr << "read error: " << e.what() << std::endl;
        return;
    }

    std::cout << "Handshake response: " << std::hex << static_cast<int>(buffer[0]) << std::endl;

    // create ping struct
    PingPacket ping_packet
    {
            .working_mode = 0x05,
            .firmware_version = 0x0102,
            .sim_info = 0x15,
            .sim1_conn_quality = 0x0C,
            .sim2_conn_quality = 0x0A,
            .battery_voltage = 0xABCD,
            .nfc_update_time = 0,
            .pin_update_time = 0,
            .temporary_pin_list_size = 0x0002
    };

    while(1)
    {
        // create ping packet
        auto bufiter = buffer.begin();

        // write start byte
        *bufiter++ = PING_DATA_STARTBYTE;

        // write working mode
        *bufiter++ = ping_packet.working_mode;

        // write firmware version
        *reinterpret_cast<uint16_t*>(bufiter) = htons(ping_packet.firmware_version);
        bufiter += sizeof(ping_packet.firmware_version);

        // write sim info
        *bufiter++ = ping_packet.sim_info;

        // write sim1 conn quality
        *bufiter++ = ping_packet.sim1_conn_quality;

        // write sim2 conn quality
        *bufiter++ = ping_packet.sim2_conn_quality;

        // write battery voltage
        *reinterpret_cast<uint16_t*>(bufiter) = htons(ping_packet.battery_voltage);
        bufiter += sizeof(ping_packet.battery_voltage);

        // write nfc update time
        *reinterpret_cast<uint32_t*>(bufiter) = htonl(ping_packet.nfc_update_time);
        bufiter += sizeof(ping_packet.nfc_update_time);

        // write pin update time
        *reinterpret_cast<uint32_t*>(bufiter) = htonl(ping_packet.pin_update_time);
        bufiter += sizeof(ping_packet.pin_update_time);

        // write temporary pin list size
        *reinterpret_cast<uint16_t*>(bufiter) = htons(ping_packet.temporary_pin_list_size);
        bufiter += sizeof(ping_packet.temporary_pin_list_size);

        // calculate checksum
        const uint16_t checksum = htons(std::accumulate(buffer.begin(), bufiter, 0));

        // write checksum
        *reinterpret_cast<uint16_t*>(bufiter) = checksum;

        // send ping packet
        try
        {
            send_data(buffer.data(), PING_PACKET_SIZE + 1);
        }
        catch (const std::exception& e)
        {
            std::cerr << "send error: " << e.what() << std::endl;
            return;
        }

        // read ping response
        try
        {
            recv_data(buffer.data(), 1);
        }
        catch (const std::exception& e)
        {
            std::cerr << "read error: " << e.what() << std::endl;
            return;
        }

        std::cout << "Ping response: " << std::hex << static_cast<int>(buffer[0]) << std::endl;

        sleep(30);
    }
}
