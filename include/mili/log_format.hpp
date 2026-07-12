#pragma once

#include "mili/decision_logger_interface.hpp"
#include "mili/types.hpp"

#include <cstddef>

namespace mili::logfmt {

constexpr std::size_t kCsvHeaderLen = 128;
constexpr std::size_t kCsvLineMax = 256;

const char* csv_header();
std::size_t format_csv_line(char* buffer, std::size_t capacity, const DecisionLogEntry& entry);
std::size_t format_json_entry(char* buffer, std::size_t capacity, const DecisionLogEntry& entry, bool first);

}  // namespace mili::logfmt
