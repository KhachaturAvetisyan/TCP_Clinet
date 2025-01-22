#include "LV_Protocol.hpp"

#define BUFFER_SIZE             (1024U)

#define HANDSHAKE_PACKET_SIZE   (2U)
#define COMMAND_SIZE            (3U)

#define HANDSHAKE_MAGIC         (0xDEADU)
#define COMMAND_GET_LIST        "LST"
#define COMMAND_SEND_DATA       "SND"


void LV_Protocol::handler_loop(const int _socket_fd)
{
    // set socket fd
    socket_fd = _socket_fd;

    // create a buffer for data exchange
    std::array<uint8_t, BUFFER_SIZE> buffer{};

    // send handshake
    *reinterpret_cast<uint16_t*>(buffer.data()) = htobe16(HANDSHAKE_MAGIC);
    try
    {
        send_data(buffer.data(), HANDSHAKE_PACKET_SIZE);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::format("Error sending packet: {}", e.what()));
    }

    for (;;)
    {
        // choose a command
        std::cout << "Choose a command:\n"
                     "(1): Get list\n"
                     "(2): Send data\n"
                     "(3): Send wrong command\n"
                     "(4): Exit\n";

        std::cin >> buffer[0];

        switch (buffer[0] - '0')
        {
            /* Get Devices List */
            case 1:
            {
                // Send command
                std::copy_n(COMMAND_GET_LIST, COMMAND_SIZE, buffer.begin());
                try
                {
                    send_data(buffer.data(), COMMAND_SIZE);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error sending packet: {}", e.what()));
                }

                // Read list size
                try
                {
                    recv_data(buffer.data(), 2);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error reading packet: {}", e.what()));
                }

                // calculate packet size
                const uint16_t list_size = be16toh(*reinterpret_cast<uint16_t*>(buffer.data()));
                const size_t packet_size = list_size * sizeof(uint64_t);

                std::cout << "List size: " << list_size << std::endl;

                // Read list
                if (list_size > 0)
                {
                    try
                    {
                        recv_data(buffer.data(), packet_size);
                    }
                    catch (const std::exception& e)
                    {
                        throw std::runtime_error(std::format("Error reading packet: {}", e.what()));
                    }

                    // print list
                    for (size_t i = 0; i < list_size; ++i)
                    {
                        std::cout << "Device: " << be64toh(*reinterpret_cast<uint64_t*>(buffer.data() + i * sizeof(uint64_t))) << std::endl;
                    }
                }

                break;
            }
            case 2:
            {
                // Send command
                std::copy_n(COMMAND_SEND_DATA, COMMAND_SIZE, buffer.begin());
                try
                {
                    send_data(buffer.data(), COMMAND_SIZE);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error sending packet: {}", e.what()));
                }

                // Write IMEI
                std::uintptr_t bufiter = 0;
                constexpr uint64_t imei = 1234567890;
                *reinterpret_cast<uint64_t*>(buffer.data()) = htobe64(imei);
                bufiter += sizeof(uint64_t);

                // Get msg
                std::cout << "Enter message: ";
                std::cin >> std::ws;
                std::cin.getline(reinterpret_cast<char*>(buffer.data() + bufiter + 2), BUFFER_SIZE - bufiter - 2);

                // Write msg size
                const uint16_t data_size = strlen(reinterpret_cast<char*>(buffer.data() + bufiter + 2));
                *reinterpret_cast<uint16_t*>(buffer.data() + bufiter) = htobe16(data_size);
                bufiter += sizeof(uint16_t);

                // Send data
                try
                {
                    send_data(buffer.data(), bufiter + data_size);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error sending packet: {}", e.what()));
                }

                // reading response
                try
                {
                    recv_data(buffer.data(), 2);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error reading packet: {}", e.what()));
                }

                // get data size
                const std::uint16_t msg_size = be16toh(*reinterpret_cast<std::uint16_t*>(buffer.data()));

                try
                {
                    recv_data(buffer.data(), msg_size);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error reading packet: {}", e.what()));
                }

                std::cout << "Mag size: " << msg_size << '\n' << "Message: " << std::string(buffer.begin(), buffer.begin() + msg_size) << std::endl;

                break;
            }

            /* Send wrong command */
            case 3:
                // Send command
                std::copy_n("WRG", COMMAND_SIZE, buffer.begin());
                try
                {
                    send_data(buffer.data(), COMMAND_SIZE);
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(std::format("Error sending packet: {}", e.what()));
                }
                break;

            /* Unhandled msg */
            default:
                std::cerr << "Invalid command: " << buffer[0] << std::endl;
            return;
        }
    }
}
