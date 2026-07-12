#include "mili/operator_command_handler.hpp"

namespace mili {

void OperatorCommandHandler::update(const OperatorCommand& command, std::uint64_t now_ms) {
    if (!command.valid) {
        return;
    }

    latest_ = command;
    latest_.timestamp_ms = now_ms;
}

MissionPriority OperatorCommandHandler::resolve_priority(MissionPriority default_priority) const {
    if (latest_.valid && latest_.has_priority_override) {
        return latest_.priority_override;
    }
    return default_priority;
}

bool OperatorCommandHandler::has_active_override() const {
    return latest_.valid && latest_.has_priority_override;
}

const OperatorCommand& OperatorCommandHandler::latest() const {
    return latest_;
}

}  // namespace mili
