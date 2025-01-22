#include <numeric>
#include <atomic>
#include <vector>
#include <unistd.h>
#include "AS3_Protocol.hpp"

#define OK_DATA                                     (0x01U)
#define ERROR_DATA                                  (0x00U)

#define PHONE_NUMBER_MAX_COUNT                      (5U)

#define HANDSHAKE_STARTBYTE                         (0xfeffU)
#define PING_STARTBYTE                              (0xa1U)
#define GET_DEVICE_CONFIGS_PACKET_STARTBYTE         ('$')
#define GET_DEVICE_CONFIGS_STARTBYTE                (0xb1U)
#define SET_DEVICE_CONFIGS_STARTBYTE                (0xb2U)
#define COMMAND_STARTBYTE                           (0xb3U)
#define HISTORY_STARTBYTE                           (0xa2U)
#define TIME_SYNC_STARTBYTE                         (0xb4U)


#define HANDSHAKE_PACKET_SIZE                       (15U)
#define PING_PACKET_SIZE                            (16U)
#define DEVICE_CONFIGS_PACKET_HEADER_SIZE           (3U)
#define COMMAND_PACKET_SIZE                         (11U)
#define HISTORY_PACKET_SIZE                         (11U)
#define TIME_SYNC_PACKET_SIZE                       (5U)

#define STRING_DELIMITER                            ('\t')
#define LISTENER_ADDRESS_MAX_SIZE                   (63U)
#define SIM_APN_MAX_SIZE                            (31U)
#define SIM_USERNAME_MAX_SIZE                       (31U)
#define SIM_PASSWORD_MAX_SIZE                       (31U)
#define DNS_SERVER_ADDRESS_MAX_SIZE                 (15U)
#define MAX_PHONE_NUMBER_COUNT                      (5U)
#define PHONE_NUMBER_STR_MAX_SIZE                   (15U)


#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#elif defined(__linux__)
#include <arpa/inet.h>
#endif


enum connection_type_t
{
    ETHERNET,
    GSM
};

enum sim_t
{
    SIM1,
    SIM2
};

enum command_t
{
    STOP,
    ALARM1,
    ALARM2,
    WAITING
};

enum command_src_t
{
    SERVER,
    DEVICE,
    SMS,
    CALL,
    IDLE
};

struct CommandObject
{
    command_t command;
    command_src_t command_src;
    std::uint16_t duration;
    std::time_t datetime;
};

struct PhoneNumber
{
    std::string number;
    bool sms;
    bool call;
};

struct DeviceConfig
{
    std::time_t update_time;
    bool qc_passed;
    float bvm_multiplier;
    std::uint16_t alarm1_working_time;
    std::uint16_t alarm2_working_time;
    std::uint16_t alarm1_on_time;
    std::uint16_t alarm2_on_time;
    std::uint16_t alarm1_off_time;
    std::uint16_t alarm2_off_time;
    std::string listener_address;
    std::uint16_t listener_port;
    std::string sim1_apn;
    std::string sim2_apn;
    std::string sim1_username;
    std::string sim2_username;
    std::string sim1_password;
    std::string sim2_password;
    std::string dns_server_address;
    std::string alternative_dns_server_address;
    bool call_sms_availability;
    std::vector<PhoneNumber> phone_numbers_arr;
}; // struct DeviceConfig

struct DeviceObject
{
    std::uint64_t imei;
    std::uint8_t firmware_major;
    std::uint8_t firmware_minor;
    std::uint8_t firmware_patch;

    std::time_t device_time;
    std::time_t configs_update_time;
    connection_type_t connection_type;
    bool phase_status;
    float battery_voltage;
    bool sim1_present;
    bool sim2_present;
    sim_t active_sim;
    uint8_t sim1_signal_quality;
    uint8_t sim2_signal_quality;
    uint8_t gsm_connection_type;
    command_t active_command;
    command_src_t active_command_src;
}; // struct DeviceObject

void get_string_from_buffer(const uint8_t *buff, std::uintptr_t &bufiter, std::string &str,
                            const char delimiter, const std::uint8_t str_max_size)
{
    if (str_max_size == 0)
    {
        throw std::runtime_error("Invalid string max size");
    }

    if (str.empty())
    {
        str.reserve(str_max_size);
    }
    else
    {
        str.clear();
    }

    for (std::uint8_t i = 0; i < str_max_size; ++i)
    {
        if (buff[bufiter] == delimiter)
        {
            ++bufiter;
            break;
        }
        str.push_back(static_cast<char>(buff[bufiter++]));
    }
} // get_string_from_buffer

void create_handshake_packet(std::uint8_t *buff, DeviceObject &device_object)
{
    std::uintptr_t bufiter = 0;

    // write start byte
    *reinterpret_cast<uint16_t*>(buff + bufiter) = htobe16(HANDSHAKE_STARTBYTE);
    bufiter += 2;

    // write imei
    *reinterpret_cast<uint64_t*>(buff + bufiter) = htobe64(device_object.imei);
    bufiter += sizeof(device_object.imei);

    // write firmware major
    buff[bufiter++] = device_object.firmware_major;

    // write firmware minor
    buff[bufiter++] = device_object.firmware_minor;

    // write a firmware patch
    buff[bufiter++] = device_object.firmware_patch;

    // count and write crc
    *reinterpret_cast<uint16_t*>(buff + bufiter) = htobe16(std::accumulate(buff, buff + HANDSHAKE_PACKET_SIZE - 2, 0));
} // create_handshake_packet

void create_ping_packet(std::uint8_t *buff)
{
    // init ping params
    std::uint32_t device_date_time = std::time(nullptr); // Fri Dec 30 2022 18:40:00 GMT+0000
    std::uint8_t connection_type = 0x01; // GSM
    std::uint8_t phase_status = 0x01; // true
    std::uint16_t battery_voltage = 13740; // 13.74V
    std::uint8_t sim_info = 0b00000011; // sim1 present, sim2 present, active sim is sim1
    std::uint8_t sim1_signal_quality = 26;
    std::uint8_t sim2_signal_quality = 31;
    std::uint8_t active_command = 0x01; // ALARM1
    std::uint8_t active_command_src = 0x03; // CALL

    std::uintptr_t bufiter = 0;

    // write start byte
    buff[bufiter++] = PING_STARTBYTE;

    // write device date time
    *reinterpret_cast<std::uint32_t*>(buff + bufiter) = htobe32(device_date_time);
    bufiter += sizeof(device_date_time);

    // write a connection type
    buff[bufiter++] = connection_type;

    // write phase status
    buff[bufiter++] = phase_status;

    // write battery voltage
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(battery_voltage);
    bufiter += sizeof(battery_voltage);

    // write sim info
    buff[bufiter++] = sim_info;

    // write sim1 signal quality
    buff[bufiter++] = sim1_signal_quality;

    // write sim2 signal quality
    buff[bufiter++] = sim2_signal_quality;

    // write active command
    buff[bufiter++] = active_command;

    // write active command src
    buff[bufiter++] = active_command_src;

    // count and write crc
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(std::accumulate(buff, buff + PING_PACKET_SIZE - 2, 0));
} // create_ping_packet

void create_history_packet(std::uint8_t *buff)
{
    // init command
    CommandObject command{};
    command.command = command_t::ALARM1;
    command.command_src = command_src_t::CALL;
    command.duration = 60;
    command.datetime = std::time(nullptr);

    std::uintptr_t bufiter = 0;

    // write start byte
    buff[bufiter++] = HISTORY_STARTBYTE;

    // write command
    buff[bufiter++] = static_cast<std::uint8_t>(command.command);

    // write a command source
    buff[bufiter++] = static_cast<std::uint8_t>(command.command_src);

    // write a command duration
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(command.duration);
    bufiter += sizeof(command.duration);

    // write a command datetime
    *reinterpret_cast<std::uint32_t*>(buff + bufiter) = htobe32(command.datetime);
    bufiter += sizeof(std::uint32_t);

    // count and write crc
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(std::accumulate(buff, buff + HISTORY_PACKET_SIZE - 2, 0));
}

void parse_command(const std::uint8_t *data, CommandObject &command)
{
    std::uintptr_t bufiter = 0;

    // read start byte
    if (*data != COMMAND_STARTBYTE)
    {
        throw std::runtime_error("Invalid start byte for command packet: " + std::to_string(*data));
    }
    ++bufiter;

    // read and check crc
    std::uint16_t crc = be16toh(*reinterpret_cast<const std::uint16_t *>(data + COMMAND_PACKET_SIZE - 2));

    // read command
    command.command = static_cast<command_t>(data[bufiter++]);

    // read a command source
    command.command_src = static_cast<command_src_t>(data[bufiter++]);

    // read a command duration
    command.duration = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(command.duration);

    // read a command datetime
    command.datetime = be32toh(*reinterpret_cast<const std::uint32_t *>(data + bufiter));
}

void parse_device_configs(const std::uint8_t *data, DeviceConfig &device_config)
{
    // check start byte
    if (*data != SET_DEVICE_CONFIGS_STARTBYTE)
    {
        throw std::runtime_error("Invalid start byte for device configs packet: " + std::to_string(*data));
    }

    // read packet size
    std::uint16_t packet_size = be16toh(*reinterpret_cast<const std::uint16_t *>(data + 1));

    // read and check crc
    std::uint16_t crc = be16toh(*reinterpret_cast<const std::uint16_t *>(data + packet_size + DEVICE_CONFIGS_PACKET_HEADER_SIZE - 2));
    std::uint16_t real_crc = std::accumulate(data, data + packet_size + DEVICE_CONFIGS_PACKET_HEADER_SIZE - 2, 0);
    if (crc != real_crc)
    {
        throw std::runtime_error("Invalid crc for device configs packet: " +
                                 std::to_string(crc) + " != " + std::to_string(real_crc));
    }

    std::uintptr_t bufiter = 3;

    // read update time
    device_config.update_time = be32toh(*reinterpret_cast<const std::uint32_t *>(data + bufiter));
    bufiter += sizeof(std::uint32_t);

    // read QC Passed
    device_config.qc_passed = static_cast<bool>(data[bufiter++]);

    // read BVM Multiplier
    std::uint32_t tmp = be32toh(*reinterpret_cast<const std::uint32_t *>(data + bufiter));
    std::memcpy(&device_config.bvm_multiplier, &tmp, sizeof(device_config.bvm_multiplier));
    bufiter += sizeof(device_config.bvm_multiplier);

    // read Alarm 1 Working Time
    device_config.alarm1_working_time = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.alarm1_working_time);

    // read Alarm 1 on time
    device_config.alarm1_on_time = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.alarm1_on_time);

    // read Alarm 1 off time
    device_config.alarm1_off_time = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.alarm1_off_time);

    // read Alarm 2 Working Time
    device_config.alarm2_working_time = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.alarm2_working_time);

    // read Alarm 2 on time
    device_config.alarm2_on_time = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.alarm2_on_time);

    // read Alarm 2 off time
    device_config.alarm2_off_time = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.alarm2_off_time);

    // read Listener Address
    get_string_from_buffer(data, bufiter, device_config.listener_address, STRING_DELIMITER, LISTENER_ADDRESS_MAX_SIZE);

    // read Listener Port
    device_config.listener_port = be16toh(*reinterpret_cast<const std::uint16_t *>(data + bufiter));
    bufiter += sizeof(device_config.listener_port);

    // read Sim1 APN
    get_string_from_buffer(data, bufiter, device_config.sim1_apn, STRING_DELIMITER, SIM_APN_MAX_SIZE);

    // read Sim2 APN
    get_string_from_buffer(data, bufiter, device_config.sim2_apn, STRING_DELIMITER, SIM_APN_MAX_SIZE);

    // read Sim1 Username
    get_string_from_buffer(data, bufiter, device_config.sim1_username, STRING_DELIMITER, SIM_USERNAME_MAX_SIZE);

    // read Sim2 Username
    get_string_from_buffer(data, bufiter, device_config.sim2_username, STRING_DELIMITER, SIM_USERNAME_MAX_SIZE);

    // read Sim1 Password
    get_string_from_buffer(data, bufiter, device_config.sim1_password, STRING_DELIMITER, SIM_PASSWORD_MAX_SIZE);

    // read Sim2 Password
    get_string_from_buffer(data, bufiter, device_config.sim2_password, STRING_DELIMITER, SIM_PASSWORD_MAX_SIZE);

    // read DNS Server Address
    get_string_from_buffer(data, bufiter, device_config.dns_server_address, STRING_DELIMITER, DNS_SERVER_ADDRESS_MAX_SIZE);

    // read Alternative DNS Server Address
    get_string_from_buffer(data, bufiter, device_config.alternative_dns_server_address, STRING_DELIMITER, DNS_SERVER_ADDRESS_MAX_SIZE);

    // read Call SMS Availability
    device_config.call_sms_availability = static_cast<bool>(data[bufiter++]);

    // read phone number count
    std::uint8_t phone_number_count = data[bufiter++];

    // check phone numbers count
    if (phone_number_count > PHONE_NUMBER_MAX_COUNT)
    {
        throw std::runtime_error("Invalid phone number count: " + std::to_string(phone_number_count));
    }

    // check if the phone numbers array is empty
    // check if the phone numbers array is empty
    if (!device_config.phone_numbers_arr.empty())
    {
        device_config.phone_numbers_arr.clear();
    }

    device_config.phone_numbers_arr.resize(phone_number_count);

    // read phone numbers
    for (std::uint8_t i = 0; i < phone_number_count; ++i)
    {
        // read phone number
        get_string_from_buffer(data, bufiter, device_config.phone_numbers_arr[i].number, STRING_DELIMITER, PHONE_NUMBER_STR_MAX_SIZE);

        // read SMS Availability
        device_config.phone_numbers_arr[i].call = static_cast<bool>(data[bufiter] & static_cast<uint8_t>(0x01));

        // read Call Availability
        device_config.phone_numbers_arr[i].sms = static_cast<bool>((data[bufiter++] >> 1) & static_cast<uint8_t>(0x01));
    }
}

std::uint16_t create_device_configs(std::uint8_t *buff)
{
    // init device config
    DeviceConfig device_config{};
    device_config.qc_passed = true;
    device_config.bvm_multiplier = 0.01065;
    device_config.alarm1_working_time = 60;
    device_config.alarm2_working_time = 120;
    device_config.alarm1_on_time = 10;
    device_config.alarm2_on_time = 20;
    device_config.alarm1_off_time = 5;
    device_config.alarm2_off_time = 10;
    device_config.listener_address = "alarm.mes.locator.am";
    device_config.listener_port = 8080;
    device_config.sim1_apn = "internet1";
    device_config.sim2_apn = "internet2";
    device_config.sim1_username = "user1";
    device_config.sim2_username = "user2";
    device_config.sim1_password = "password1";
    device_config.sim2_password = "password2";
    device_config.dns_server_address = "1.1.1.1";
    device_config.alternative_dns_server_address = "9.9.9.9";
    device_config.call_sms_availability = true;
    device_config.phone_numbers_arr = {
            PhoneNumber{"37411223344", true, false},
            PhoneNumber{"37455667788", true, true}
    };

    // create a device configs packet
    std::uintptr_t bufiter = 0;

    // write start byte
    buff[bufiter++] = GET_DEVICE_CONFIGS_PACKET_STARTBYTE;

    // skip packet size
    bufiter += sizeof(std::uint16_t);

    // write qc passed
    buff[bufiter++] = static_cast<std::uint8_t>(device_config.qc_passed);

    // write bvm multiplier
    std::uint32_t temp;
    std::memcpy(&temp, &device_config.bvm_multiplier, sizeof(device_config.bvm_multiplier));
    *reinterpret_cast<std::uint32_t*>(buff + bufiter) = htobe32(temp);
    bufiter += sizeof(device_config.bvm_multiplier);

    // write alarm1 working time
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.alarm1_working_time);
    bufiter += sizeof(device_config.alarm1_working_time);

    // write alarm1 on time
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.alarm1_on_time);
    bufiter += sizeof(device_config.alarm1_on_time);

    // write alarm1 off time
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.alarm1_off_time);
    bufiter += sizeof(device_config.alarm1_off_time);

    // write alarm2 working time
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.alarm2_working_time);
    bufiter += sizeof(device_config.alarm2_working_time);

    // write alarm2 on time
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.alarm2_on_time);
    bufiter += sizeof(device_config.alarm2_on_time);

    // write alarm2 off time
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.alarm2_off_time);
    bufiter += sizeof(device_config.alarm2_off_time);

    // write listener address
    std::copy(device_config.listener_address.begin(), device_config.listener_address.end(), buff + bufiter);
    bufiter += device_config.listener_address.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write listener port
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(device_config.listener_port);
    bufiter += sizeof(device_config.listener_port);

    // write sim1 apn
    std::copy(device_config.sim1_apn.begin(), device_config.sim1_apn.end(), buff + bufiter);
    bufiter += device_config.sim1_apn.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write sim2 apn
    std::copy(device_config.sim2_apn.begin(), device_config.sim2_apn.end(), buff + bufiter);
    bufiter += device_config.sim2_apn.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write sim1 username
    std::copy(device_config.sim1_username.begin(), device_config.sim1_username.end(), buff + bufiter);
    bufiter += device_config.sim1_username.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write sim2 username
    std::copy(device_config.sim2_username.begin(), device_config.sim2_username.end(), buff + bufiter);
    bufiter += device_config.sim2_username.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write sim1 password
    std::copy(device_config.sim1_password.begin(), device_config.sim1_password.end(), buff + bufiter);
    bufiter += device_config.sim1_password.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write sim2 password
    std::copy(device_config.sim2_password.begin(), device_config.sim2_password.end(), buff + bufiter);
    bufiter += device_config.sim2_password.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write dns server address
    std::copy(device_config.dns_server_address.begin(), device_config.dns_server_address.end(), buff + bufiter);
    bufiter += device_config.dns_server_address.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write alternative dns server address
    std::copy(device_config.alternative_dns_server_address.begin(), device_config.alternative_dns_server_address.end(), buff + bufiter);
    bufiter += device_config.alternative_dns_server_address.size();
    // write delimiter
    buff[bufiter++] = STRING_DELIMITER;

    // write call sms availability
    buff[bufiter++] = static_cast<std::uint8_t>(device_config.call_sms_availability);

    // write phone number count
    buff[bufiter++] = device_config.phone_numbers_arr.size();

    // write phone numbers
    for (auto & i : device_config.phone_numbers_arr)
    {
        // write phone number
        std::copy(i.number.begin(), i.number.end(), buff + bufiter);
        bufiter += i.number.size();
        // write delimiter
        buff[bufiter++] = STRING_DELIMITER;

        // write call availability
        buff[bufiter] = static_cast<std::uint8_t>(i.call);

        // write sms availability
        buff[bufiter++] |= static_cast<std::uint8_t>(i.sms) << 1;
    }

    // write packet size
    *reinterpret_cast<std::uint16_t*>(buff + 1) = htobe16(bufiter - DEVICE_CONFIGS_PACKET_HEADER_SIZE + 2);

    // count and write crc
    std::uint16_t crc = std::accumulate(buff, buff + bufiter, 0);
    *reinterpret_cast<std::uint16_t*>(buff + bufiter) = htobe16(crc);
    bufiter += sizeof(std::uint16_t);
    
    return bufiter;
}

AS3_Protocol::AS3_Protocol()
{
}

void AS3_Protocol::handler_loop(int _socket_fd)
{
    std::cout << "AS3_Protocol::handler_loop" << std::endl;

    // copy socket fd
    socket_fd = _socket_fd;

    // init device object
    DeviceObject device_object{};
    device_object.imei = 862686042898620;
    device_object.firmware_major = 1;
    device_object.firmware_minor = 2;
    device_object.firmware_patch = 10;

    // init buffer
    std::array<std::uint8_t, 1024> buffer{};

    // create a handshake packet
    create_handshake_packet(buffer.data(), device_object);

    // send handshake packet
    try
    {
        send_data(buffer.data(), HANDSHAKE_PACKET_SIZE);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error sending handshake packet: " << e.what() << std::endl;
        return;
    }

    // read handshake response
    try
    {
        recv_data(buffer.data(), 4);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error reading handshake response: " << e.what() << std::endl;
        return;
    }

    // check handshake response
    std::time_t server_time = be32toh(*reinterpret_cast<std::uint32_t*>(buffer.data()));
    std::cout << "Server time: " << server_time << std::endl;
    if (server_time == 0)
    {
        std::cerr << "Server time is 0" << std::endl;
        return;
    }

    for (;;)
    {
        // create a ping packet
        create_ping_packet(buffer.data());

        // send ping packet
        try
        {
            send_data(buffer.data(), PING_PACKET_SIZE);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error sending ping packet: " << e.what() << std::endl;
            return;
        }

        // read one byte
        try
        {
            recv_data(buffer.data(), 1);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error reading one byte: " << e.what() << std::endl;
            return;
        }

        // check response
        if (buffer[0] != OK_DATA)
        {
            std::cerr << "Ping response is not OK" << std::endl;
            return;
        }

        std::cout << "Input mode (1 - ping, 2 - send history, 3 - read command, 4 - rcv device configs, 5 - get device configs)" << std::endl;
        std::cin >> buffer[0];

        switch (buffer[0])
        {
            case '1':
            {
                continue;
            } // end case '1'

            case '2':
            {
                // create a history packet
                create_history_packet(buffer.data());

                // send a history packet
                try {
                    send_data(buffer.data(), HISTORY_PACKET_SIZE);
                }
                catch (const std::exception &e) {
                    std::cerr << "Error sending history packet: " << e.what() << std::endl;
                    return;
                }

                // read one byte
                try {
                    recv_data(buffer.data(), 1);
                }
                catch (const std::exception &e) {
                    std::cerr << "Error reading one byte: " << e.what() << std::endl;
                    return;
                }

                // check response
                if (buffer[0] != OK_DATA) {
                    std::cerr << "History response is not OK" << std::endl;
                    return;
                }

                continue;
            } // end case '2'

            case '3':
            {
                try
                {
                    recv_data(buffer.data(), COMMAND_PACKET_SIZE);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error reading command packet: " << e.what() << std::endl;
                    return;
                }

                CommandObject command{};
                parse_command(buffer.data(), command);

                // send response
                buffer[0] = OK_DATA;
                try
                {
                    send_data(buffer.data(), 1);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error sending response: " << e.what() << std::endl;
                    return;
                }

                std::cout << "Command: " << command.command << std::endl;
                std::cout << "Command source: " << command.command_src << std::endl;
                std::cout << "Command duration: " << command.duration << std::endl;
                std::cout << "Command datetime: " << command.datetime << std::endl;

                continue;
            } // end case '3'

            case '4':
            {
                // rcv device configs header
                try
                {
                    recv_data(buffer.data(), DEVICE_CONFIGS_PACKET_HEADER_SIZE);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error reading device configs header: " << e.what() << std::endl;
                    return;
                }

                // read packet size
                std::uint16_t packet_size = be16toh(*reinterpret_cast<const std::uint16_t*>(buffer.data() + 1));

                // read device configs
                try
                {
                    recv_data(buffer.data() + DEVICE_CONFIGS_PACKET_HEADER_SIZE, packet_size);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error reading device configs: " << e.what() << std::endl;
                    return;
                }

                DeviceConfig device_config{};
                parse_device_configs(buffer.data(), device_config);

                // send response
                buffer[0] = OK_DATA;
                try
                {
                    send_data(buffer.data(), 1);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error sending response: " << e.what() << std::endl;
                    return;
                }

                std::cout << "Device configs:" << std::endl;
                std::cout << "Update time: " << device_config.update_time << std::endl;
                std::cout << "QC Passed: " << device_config.qc_passed << std::endl;
                std::cout << "BVM Multiplier: " << device_config.bvm_multiplier << std::endl;
                std::cout << "Alarm 1 Working Time: " << device_config.alarm1_working_time << std::endl;
                std::cout << "Alarm 1 on time: " << device_config.alarm1_on_time << std::endl;
                std::cout << "Alarm 1 off time: " << device_config.alarm1_off_time << std::endl;
                std::cout << "Alarm 2 Working Time: " << device_config.alarm2_working_time << std::endl;
                std::cout << "Alarm 2 on time: " << device_config.alarm2_on_time << std::endl;
                std::cout << "Alarm 2 off time: " << device_config.alarm2_off_time << std::endl;
                std::cout << "Listener Address: " << device_config.listener_address << std::endl;
                std::cout << "Listener Port: " << device_config.listener_port << std::endl;
                std::cout << "Sim1 APN: " << device_config.sim1_apn << std::endl;
                std::cout << "Sim2 APN: " << device_config.sim2_apn << std::endl;
                std::cout << "Sim1 Username: " << device_config.sim1_username << std::endl;
                std::cout << "Sim2 Username: " << device_config.sim2_username << std::endl;
                std::cout << "Sim1 Password: " << device_config.sim1_password << std::endl;
                std::cout << "Sim2 Password: " << device_config.sim2_password << std::endl;
                std::cout << "DNS Server Address: " << device_config.dns_server_address << std::endl;
                std::cout << "Alternative DNS Server Address: " << device_config.alternative_dns_server_address << std::endl;
                std::cout << "Call SMS Availability: " << device_config.call_sms_availability << std::endl;
                std::cout << "Phone numbers count: " << device_config.phone_numbers_arr.size() << std::endl;
                for (const auto& phone_number : device_config.phone_numbers_arr)
                {
                    std::cout << "Phone number: " << phone_number.number << std::endl;
                    std::cout << "SMS Availability: " << phone_number.sms << std::endl;
                    std::cout << "Call Availability: " << phone_number.call << std::endl;
                }

                continue;
            } // end case '4'

            case '5':
            {
                // rcv get device configs startbyte
                try
                {
                    recv_data(buffer.data(), 1);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error reading get device configs startbyte: " << e.what() << std::endl;
                    return;
                }

                // check start byte
                if (buffer[0] != GET_DEVICE_CONFIGS_STARTBYTE)
                {
                    std::cerr << "Invalid start byte for get device configs packet: " << buffer[0] << std::endl;
                    return;
                }

                // create a device configs packet
                std::uint16_t packet_size = create_device_configs(buffer.data());

                // send a device configs packet
                try
                {
                    send_data(buffer.data(), packet_size);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error sending device configs packet: " << e.what() << std::endl;
                    return;
                }

                // read 4 bytes
                try
                {
                    recv_data(buffer.data(), 4);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error reading one byte: " << e.what() << std::endl;
                    return;
                }

                // check response
                std::cout << "Device configs response: " << be32toh(*reinterpret_cast<std::uint32_t*>(buffer.data())) << std::endl;

                continue;
            } // end case '5'
        } // end switch (buffer[0])

        sleep(30);

    } // for (;;)
}