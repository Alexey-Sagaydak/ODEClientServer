#pragma once
#include "Solver.hpp"

class STEKSSolver : public Solver
{
public:
    STEKSSolver(
        std::function<std::vector<double>(
            double,
            std::vector<double> const &)> func,
        double                            initialStep
    ) : Solver(func, initialStep) {}

    void Step(
        double               t,
        std::vector<double> &y,
        double              &h,
        double               tolerance) override;

    void Solve(
        double                     t0,
        std::vector<double> const &y0,
        double                     tEnd,
        Storage                   &storage,
        double                     tolerance) override;

private:
    const double SAFETY    = 0.9;
    const double MIN_SCALE = 0.2;
    const double MAX_SCALE = 5.0;

    double computeError(
        std::vector<double> const &k1,
        std::vector<double> const &k2,
        std::vector<double> const &k3,
        std::vector<double> const &k4,
        std::vector<double> const &k5,
        double                     h,
        double                     tolerance
    );
};
