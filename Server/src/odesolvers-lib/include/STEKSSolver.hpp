#pragma once
#include "Solver.hpp"
#include <vector>
#include <stdexcept>
#include <cmath>

class STEKSSolver : public Solver
{
public:
    STEKSSolver(std::function<std::vector<double>(double, const std::vector<double>&)> func, double initialStep)
        : Solver(func, initialStep) {}

    void Step(double t, std::vector<double>& y, double& h, double tolerance) override;
    void Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) override;

private:
    const double SAFETY = 0.9;
    const double MIN_SCALE = 0.1;
    const double MAX_SCALE = 5.0;
    const double STABILITY_LIMIT = 3.5 / 6.0; // 3.5 / 6 ≈ 0.5833

    double computeError(
        const std::vector<double>& k1,
        const std::vector<double>& k2,
        const std::vector<double>& k3,
        const std::vector<double>& k4,
        const std::vector<double>& k5,
        double tolerance
    );
};
