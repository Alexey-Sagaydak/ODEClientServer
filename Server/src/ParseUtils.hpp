#pragma once
#include <unordered_map>
#include <string>
#include <regex>
#include <stdexcept>

std::vector<double> ExtractInitialConditions(
    std::unordered_map<std::string, double> const &parameters);
