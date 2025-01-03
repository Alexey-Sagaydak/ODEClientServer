#pragma once
#include "Solver.hpp"

class EulerSolver : public Solver {
public:
    EulerSolver(std::function<std::vector<double>(double, const std::vector<double>&)> func, double initialStep)
        : Solver(func, initialStep) {
    }

    void Step(double t, std::vector<double>& y, double h) override;
    void Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) override;
};
