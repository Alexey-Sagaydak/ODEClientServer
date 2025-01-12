#pragma once
#include "Storage.hpp"
#include <functional>
#include <algorithm>
#include <cmath>

class Solver {
public:
    Solver(std::function<std::vector<double>(double, const std::vector<double>&)> func, double initialStep)
        : f(func), stepSize(initialStep) {
    }
    virtual ~Solver() = default;

    virtual void Step(double t, std::vector<double>& y, double& h, double tolerance) = 0;
    virtual void Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) = 0;


protected:
    std::function<std::vector<double>(double, const std::vector<double>&)> f;
    double stepSize;
};
