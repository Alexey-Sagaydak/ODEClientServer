#include "../include/RK2Solver.hpp"

RK2Solver::RK2Solver(std::function<std::vector<double>(double, const std::vector<double>&)> func, double initialStep)
    : Solver(func, initialStep) {
}

void RK2Solver::Step(double t, std::vector<double>& y, double& h, double tolerance) {
    while (true) {
        std::vector<double> k1 = ComputeK(t, y, h, 0.0, {});
        std::vector<double> k2 = ComputeK(t, y, h, b21, k1);
        std::vector<double> k3 = ComputeK(t, y, h, b31, k1, b32, k2);

        double e1 = EstimateError(k1, k2);
        double q1 = AdjustStepSize(e1, tolerance);

        if (q1 < 1.0) {
            h *= q1 / 1.1;
            continue;
        }

        for (size_t i = 0; i < y.size(); ++i) {
            y[i] += p1 * k1[i] + p2 * k2[i] + p3 * k3[i];
        }

        std::vector<double> f_new = f(t, y);
        double e2 = EstimateError(y, f_new);
        double q2 = AdjustStepSize(e2, tolerance);

        h = std::max(h, std::min(std::min(q1, q2), 1.0) * h);
        break;
    }
}

void RK2Solver::Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) {
    double t = t0;
    std::vector<double> y = y0;
    storage.Add(t, y);

    while (t < tEnd) {
        double h = std::min(stepSize, tEnd - t);
        Step(t, y, h, tolerance);
        t += h;
        storage.Add(t, y);
    }
}

std::vector<double> RK2Solver::ComputeK(double t, const std::vector<double>& y, double h, double coeff1,
    const std::vector<double>& k1, double coeff2,
    const std::vector<double>& k2) {
    std::vector<double> y_temp = y;
    for (size_t i = 0; i < y.size(); ++i) {
        y_temp[i] += coeff1 * (i < k1.size() ? k1[i] : 0.0) + coeff2 * (i < k2.size() ? k2[i] : 0.0);
    }
    std::vector<double> k = f(t, y_temp);
    for (double& val : k) {
        val *= h;
    }
    return k;
}

double RK2Solver::EstimateError(const std::vector<double>& k1, const std::vector<double>& k2) {
    double e = 0.0;
    for (size_t i = 0; i < k1.size(); ++i) {
        e = std::max(e, std::abs(k2[i] - k1[i]));
    }
    return e;
}

double RK2Solver::AdjustStepSize(double error, double tolerance) {
    return std::pow((6.0 * std::abs(b21) * tolerance / std::abs(1.0 - 6.0 * g)) / error, 1.0 / 3.0);
}
