#pragma once

#include "mili/product_data.hpp"

#include <cstdint>

namespace mili {

class IProduct1Client {
public:
    virtual ~IProduct1Client() = default;
    virtual bool poll(Product1Data& out, std::uint64_t now_ms) = 0;
    virtual bool is_connected() const = 0;
};

class IProduct2Client {
public:
    virtual ~IProduct2Client() = default;
    virtual bool poll(Product2Data& out, std::uint64_t now_ms) = 0;
    virtual bool is_connected() const = 0;
};

class IControlPublisher {
public:
    virtual ~IControlPublisher() = default;
    virtual bool publish(const DecisionResult& result) = 0;
};

}  // namespace mili
