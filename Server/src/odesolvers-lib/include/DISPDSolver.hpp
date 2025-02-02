#pragma once
#include "Solver.hpp"

class DISPDSolver : public Solver
{
public:
    DISPDSolver(
        std::function<std::vector<double>(
            double,
            std::vector<double> const &)> func,
        double                            initialStep,
        double                            q = 1.5  // Параметр адаптации шага (q > 1)
    ) : Solver(func, initialStep), q(q) {}

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
    const double q;        // Параметр для формул (5.17-5.23)
    const double SAFETY    = 0.8;
    const double MIN_SCALE = 0.2;
    const double MAX_SCALE = 5.0;

    // Параметры для схем (5.2) и (5.12)
    struct SchemeParams {
        double p1, p2, p3;
        double g;          // Параметр g (зависит от схемы)
    };

    struct SchemeParams currentScheme;

    // Вычисление k1, k2, k3 для текущей схемы
    void computeKTerms(
        double                     t,
        std::vector<double> const &y,
        double                     h,
        std::vector<double>       &k1,
        std::vector<double>       &k2,
        std::vector<double>       &k3
    );

    // Оценки ошибок и параметров адаптации
    double computeAprime(
        std::vector<double> const &k1,
        std::vector<double> const &k2);
    
    double computeAddoublePrime(
        double                     h,
        std::vector<double> const &k1,
        std::vector<double> const &yNext);
    
    double computeV(
        std::vector<double> const &k1,
        std::vector<double> const &k2,
        std::vector<double> const &k3);

    // Переключение между схемами
    bool shouldSwitchToSchemeB(
        double h,
        double tolerance);
    
    void switchScheme(bool toSchemeB);
};
