#pragma once
#include <vector>
#include <stdexcept>

class Storage {
public:
    void Add(const std::vector<double>& values);

    std::vector<double>& operator[](size_t index);

    const std::vector<double>& operator[](size_t index) const;

    size_t Size() const;

private:
    // Internal storage of results
    std::vector<std::vector<double>> data;
};
