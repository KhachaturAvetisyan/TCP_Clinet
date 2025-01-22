#pragma once

#include "AbstractProtocol.hpp"

class BA5_Protocol final : public AbstractProtocol
{
public:
    BA5_Protocol() = default;
    ~BA5_Protocol() override = default;

public:
    void handler_loop(int _socket_fd) override;
};
