#include "Solver.hpp"

void Storage::Add(const std::vector<double>& values) {
    data.push_back(values);
}

std::vector<double>& Storage::operator[](size_t index) {
    if (index >= data.size()) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

const std::vector<double>& Storage::operator[](size_t index) const {
    if (index >= data.size()) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

size_t Storage::Size() const {
    return data.size();
}
