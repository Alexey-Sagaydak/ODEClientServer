#include "EulerSolver.hpp"

void EulerSolver::Step(double t, std::vector<double>& y, double h) {
    std::vector<double> dydt = f(t, y);
    for (size_t i = 0; i < y.size(); ++i) {
        y[i] += h * dydt[i];
    }
}

void EulerSolver::Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) {
    double t = t0;
    std::vector<double> y = y0;
    storage.Add(y);

    while (t < tEnd) {
        double h = stepSize;
        std::vector<double> yTemp = y;

        Step(t, yTemp, h); // Perform a single step

        std::vector<double> fNext = f(t + h, yTemp);
        double error = 0.0;
        for (size_t i = 0; i < y.size(); ++i) {
            error = std::max(error, std::abs(0.5 * h * (fNext[i] - f(t, y)[i])));
        }
        double q = std::sqrt(tolerance / error);

        if (q < 1.0) {
            h /= 2.0;
        }
        else {
            t += h;
            y = yTemp;
            storage.Add(y);
            if (q > 2.0) {
                h *= 2.0;
            }
        }

        stepSize = h;
    }
}
