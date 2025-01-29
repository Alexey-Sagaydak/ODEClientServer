#ifndef UTILS_HPP
#define UTILS_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <regex>
#include <stdexcept>

std::vector<double> ExtractInitialConditions(
    std::unordered_map<std::string, double> const &parameters);

#endif // UTILS_HPP
