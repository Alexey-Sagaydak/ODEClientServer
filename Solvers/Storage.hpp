#pragma once
#include <vector>
#include <stdexcept>

class Storage {
public:
    void Add(double time, const std::vector<double>& values);

    std::pair<double, std::vector<double>>& operator[](size_t index);
    const std::pair<double, std::vector<double>>& operator[](size_t index) const;

    size_t Size() const;

private:
    std::vector<std::pair<double, std::vector<double>>> data;
};