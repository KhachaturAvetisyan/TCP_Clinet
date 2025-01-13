#pragma once

#include "AbstractProtocol.hpp"

class AS3_Protocol final : public AbstractProtocol
{
public:
    AS3_Protocol();
    ~AS3_Protocol() override = default;

public:
    void handler_loop(int _socket_fd) override;
};