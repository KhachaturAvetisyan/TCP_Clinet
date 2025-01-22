#pragma once

#include "AbstractProtocol.hpp"

class LV_Protocol final : public AbstractProtocol
{
public:
    LV_Protocol() = default;
    ~LV_Protocol() override = default;

public:
    void handler_loop(int _socket_fd) override;
};