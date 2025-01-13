#pragma once

#include "AbstractProtocol.hpp"

class ScalesProtocol : public AbstractProtocol
{
public:
    ScalesProtocol() = default;
    ~ScalesProtocol() override = default;

public:
    void handler_loop(int _socket_fd) override;
};