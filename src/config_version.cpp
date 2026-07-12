#include "mili/config_version.hpp"

#include <cctype>

namespace mili {

namespace {

bool parse_component(const std::string& text, std::size_t& pos, int& value) {
    if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos]))) {
        return false;
    }

    value = 0;
    while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
        value = value * 10 + (text[pos] - '0');
        ++pos;
    }
    return true;
}

}  // namespace

bool parse_semver(const std::string& text, SemVer& out) {
    std::size_t pos = 0;
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
        ++pos;
    }

    if (!parse_component(text, pos, out.major)) {
        return false;
    }
    if (pos >= text.size() || text[pos] != '.') {
        return false;
    }
    ++pos;

    if (!parse_component(text, pos, out.minor)) {
        return false;
    }
    if (pos >= text.size() || text[pos] != '.') {
        return false;
    }
    ++pos;

    if (!parse_component(text, pos, out.patch)) {
        return false;
    }

    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
        ++pos;
    }
    return pos == text.size();
}

bool is_compatible_weight_config(const std::string& version) {
    SemVer file_version{};
    SemVer supported{};
    if (!parse_semver(version, file_version) || !parse_semver(kWeightConfigSchemaVersion, supported)) {
        return false;
    }

    // Same major version is compatible (1.x.x reads with 1.0.0 runtime).
    return file_version.major == supported.major;
}

}  // namespace mili
