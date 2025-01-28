#pragma once
#include "Solver.hpp"
#include <vector>
#include <stdexcept>
#include <cmath>

class RK23SSolver : public Solver
{
public:
    RK23SSolver(
        std::function<std::vector<double>(
            double,
            std::vector<double> const &)> func,
        double                            initialStep)
        : Solver(func, initialStep) {}

    void Step(
        double               t,
        std::vector<double> &y,
        double              &h,
        double               tolerance) override;
    
    void Solve(
        double                     t0,
        const std::vector<double> &y0,
        double                     tEnd,
        Storage                   &storage,
        double                     tolerance) override;

private:
    const double alpha2 = 2.0/3.0;
    const double alpha3 = 2.0/3.0;
    const double beta21 = 2.0/3.0;
    const double beta31 = 1.0/3.0;
    const double beta32 = 1.0/3.0;
    const double p1     = 1.0/4.0;
    const double p2     = 15.0/32.0;
    const double p3     = 9.0/32.0;
    double g            = 1.0/16.0;

    const double SAFETY    = 0.9;
    const double MIN_SCALE = 0.2;
    const double MAX_SCALE = 5.0;

    double computeError(
        std::vector<double> const &k1,
        std::vector<double> const &k2,
        const std::vector<double> &k3,
        double                     h,
        double                     tolerance);
};
