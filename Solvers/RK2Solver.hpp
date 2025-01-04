#pragma once

#include "Solver.hpp"

class RK2Solver : public Solver {
public:
    RK2Solver(std::function<std::vector<double>(double, const std::vector<double>&)> func, double initialStep);

    void Step(double t, std::vector<double>& y, double& h, double tolerance) override;
    void Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) override;

private:
    const double g = 1.0 / 16.0;
    const double b21 = 1.0 / 3.0;
    const double b31 = 7.0 / 18.0;
    const double b32 = 7.0 / 18.0;
    const double p1 = 1.0 / 7.0;
    const double p2 = 3.0 / 8.0;
    const double p3 = 27.0 / 56.0;

    std::vector<double> ComputeK(double t, const std::vector<double>& y, double h, double coeff1,
        const std::vector<double>& k1, double coeff2 = 0.0,
        const std::vector<double>& k2 = {});
    double EstimateError(const std::vector<double>& k1, const std::vector<double>& k2);
    double AdjustStepSize(double error, double tolerance);
};
