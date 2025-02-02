#include "../include/Storage.hpp"

void Storage::Add(
    double time,
    std::vector<double> const &values)
{
    data.emplace_back(time, values);
}

std::pair<double, std::vector<double>>& Storage::operator[](size_t index) {
    if (index >= data.size()) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

const std::pair<double, std::vector<double>>& Storage::operator[](size_t index) const {
    if (index >= data.size()) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

size_t Storage::Size() const {
    return data.size();
}
