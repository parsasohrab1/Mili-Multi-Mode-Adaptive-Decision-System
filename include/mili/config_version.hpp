#pragma once

#include <string>

namespace mili {

constexpr const char* kWeightConfigSchemaVersion = "1.0.0";

struct SemVer {
    int major = 0;
    int minor = 0;
    int patch = 0;
};

bool parse_semver(const std::string& text, SemVer& out);
bool is_compatible_weight_config(const std::string& version);

}  // namespace mili
