#pragma once
#include "Solver.hpp"

class EulerSolver : public Solver
{
public:
    EulerSolver(
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
    const double SAFETY_FACTOR = 0.9;
    const double MAX_FACTOR    = 2.0;
    const double MIN_FACTOR    = 0.2;
};
