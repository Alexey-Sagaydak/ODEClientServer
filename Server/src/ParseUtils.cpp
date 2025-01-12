#include "ParseUtils.hpp"

std::vector<double> ExtractInitialConditions(const std::unordered_map<std::string, double>& parameters) {
    std::vector<std::pair<int, double>> indexedConditions;
    std::regex regex("^y(\\d+)_init$");

    for (const auto& [key, value] : parameters) {
        std::smatch match;
        if (std::regex_match(key, match, regex)) {
            int index = std::stoi(match[1].str());
            indexedConditions.emplace_back(index, value);
        }
    }

    std::sort(indexedConditions.begin(), indexedConditions.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    std::vector<double> initialConditions;
    for (const auto& [index, value] : indexedConditions) {
        initialConditions.push_back(value);
    }

    if (initialConditions.empty()) {
        throw std::runtime_error("No initial conditions found in parameters.");
    }

    return initialConditions;
}
