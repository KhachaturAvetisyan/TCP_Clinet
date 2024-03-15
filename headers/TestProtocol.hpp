#pragma once

#include "AbstractProtocol.hpp"

class TestProtocol final : public AbstractProtocol
{
public:
    TestProtocol();
    ~TestProtocol() override = default;

public:
    void handler_loop(int _socket_fd) override;
};