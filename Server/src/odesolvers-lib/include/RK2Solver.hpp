#pragma once

#include "Solver.hpp"

class RK2Solver : public Solver
{
public:
    RK2Solver(
        std::function<std::vector<double>(
            double,
            std::vector<double> const &)> func, 
        double                            initialStep);

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
    const double a2  = 2.0 / 3.0;
    const double b21 = 2.0 / 3.0;
    const double p1  = 1.0 / 4.0;
    const double p2  = 3.0 / 4.0;

    const double SAFETY_FACTOR = 0.9;
    const double MAX_FACTOR    = 5.0;
    const double MIN_FACTOR    = 0.2;
};
