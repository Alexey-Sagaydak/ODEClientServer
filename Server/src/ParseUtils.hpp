#ifndef UTILS_HPP
#define UTILS_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <regex>
#include <stdexcept>

std::vector<double> ExtractInitialConditions(const std::unordered_map<std::string, double>& parameters);

#endif // UTILS_HPP
