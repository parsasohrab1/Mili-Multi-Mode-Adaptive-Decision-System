#pragma once

#include "mili/product_data.hpp"
#include "mili/types.hpp"

namespace mili {

class OperatorCommandHandler {
public:
    void update(const OperatorCommand& command, std::uint64_t now_ms);

    MissionPriority resolve_priority(MissionPriority default_priority) const;
    bool has_active_override() const;
    const OperatorCommand& latest() const;

private:
    OperatorCommand latest_{};
};

}  // namespace mili
