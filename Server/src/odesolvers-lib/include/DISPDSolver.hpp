#pragma once
#include "Solver.hpp"
#include <vector>
#include <stdexcept>
#include <cmath>
#include <algorithm>

class DISPDSolver : public Solver {
public:
    DISPDSolver(
        std::function<std::vector<double>(
            double,
            std::vector<double> const &)> func,
        double initialStep,
        double q = 1.2 // параметр q из описания метода
    ) : Solver(func, initialStep), q(q) {}

    void Step(
        double t,
        std::vector<double> &y,
        double &h,
        double tolerance) override;

    void Solve(
        double t0,
        std::vector<double> const &y0,
        double tEnd,
        Storage &storage,
        double tolerance) override;

private:
    const double q; // параметр для адаптации шага
    const double SAFETY = 0.9;
    const double MIN_SCALE = 0.2;
    const double MAX_SCALE = 5.0;

    bool useAlgorithmA = true; // Флаг для переключения между алгоритмами

    // Вспомогательные функции
    double computeAprime(const std::vector<double>& k1, const std::vector<double>& k2, double g);
    double computeAsecond(const std::vector<double>& k1, const std::vector<double>& k_new, double h, double g);
    double computeV(const std::vector<double>& k3, const std::vector<double>& k2, const std::vector<double>& k1);
    double computeStepScale(double error, double tolerance);
};
