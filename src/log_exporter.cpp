#include "mili/log_exporter.hpp"

#include "mili/log_format.hpp"

#include <fstream>
#include <sstream>

namespace mili {

namespace {

void append_csv_from_logger(std::ostringstream& out, const IDecisionLogger& logger) {
    DecisionLogEntry entry{};
    for (std::size_t i = 0; i < logger.size(); ++i) {
        if (!logger.entry_at(i, entry)) {
            continue;
        }
        char line[mili::logfmt::kCsvLineMax];
        const auto len = mili::logfmt::format_csv_line(line, sizeof(line), entry);
        if (len > 0) {
            out.write(line, static_cast<std::streamsize>(len));
        }
    }
}

}  // namespace

std::string LogExporter::to_csv(const IDecisionLogger& logger) {
    std::ostringstream out;
    out << mili::logfmt::csv_header();
    append_csv_from_logger(out, logger);
    return out.str();
}

std::string LogExporter::to_json(const IDecisionLogger& logger) {
    std::ostringstream out;
    out << "{\"version\":1,\"entries\":[";
    DecisionLogEntry entry{};
    bool first = true;
    for (std::size_t i = 0; i < logger.size(); ++i) {
        if (!logger.entry_at(i, entry)) {
            continue;
        }
        char chunk[512];
        const auto len = mili::logfmt::format_json_entry(chunk, sizeof(chunk), entry, first);
        if (len > 0) {
            out.write(chunk, static_cast<std::streamsize>(len));
            first = false;
        }
    }
    out << "]}";
    return out.str();
}

bool LogExporter::write_csv(const IDecisionLogger& logger, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << to_csv(logger);
    return true;
}

bool LogExporter::write_json(const IDecisionLogger& logger, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << to_json(logger);
    return true;
}

std::size_t LogExporter::export_csv_lines(
    const IDecisionLogger& logger,
    char* buffer,
    std::size_t capacity,
    std::size_t start_index,
    std::size_t max_lines) {
    if (buffer == nullptr || capacity == 0) {
        return 0;
    }

    std::size_t offset = 0;
    std::size_t exported = 0;
    DecisionLogEntry entry{};

    for (std::size_t i = start_index; i < logger.size() && exported < max_lines; ++i) {
        if (!logger.entry_at(i, entry)) {
            continue;
        }
        if (offset >= capacity) {
            break;
        }
        const auto len = mili::logfmt::format_csv_line(buffer + offset, capacity - offset, entry);
        if (len == 0) {
            break;
        }
        offset += len;
        ++exported;
    }
    return exported;
}

}  // namespace mili
