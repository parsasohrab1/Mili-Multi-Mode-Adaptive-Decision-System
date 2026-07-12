#pragma once

#include "mili/decision_logger_interface.hpp"

#include <cstddef>
#include <string>

namespace mili {

class LogExporter {
public:
    static std::string to_csv(const IDecisionLogger& logger);
    static std::string to_json(const IDecisionLogger& logger);
    static bool write_csv(const IDecisionLogger& logger, const std::string& path);
    static bool write_json(const IDecisionLogger& logger, const std::string& path);

    static std::size_t export_csv_lines(
        const IDecisionLogger& logger,
        char* buffer,
        std::size_t capacity,
        std::size_t start_index,
        std::size_t max_lines);
};

}  // namespace mili
