#pragma once

#include "AbstractProtocol.hpp"

struct PingPacket
{
    uint8_t working_mode{};
    uint16_t firmware_version{};
    uint8_t sim_info{};
    uint8_t sim1_conn_quality{};
    uint8_t sim2_conn_quality{};
    uint16_t battery_voltage{};
    uint32_t nfc_update_time{};
    uint32_t pin_update_time{};
    uint16_t temporary_pin_list_size{};
};


class IntercomAppProtocol final : public AbstractProtocol
{
public:
    IntercomAppProtocol() = default;
    ~IntercomAppProtocol() override = default;

public:
    void handler_loop(int _socket_fd) override;
};