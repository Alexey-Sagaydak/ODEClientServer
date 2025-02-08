#pragma once
#include "Solver.hpp"
#include "CommonFunctions.hpp"
#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <functional>

class DispFSolver : public Solver
{
public:
    DispFSolver(
        std::function<std::vector<double>(double, const std::vector<double>&)> func,
        double initialStep,
        double gamma = 0.0
    );

    virtual void Step(
        double t,
        std::vector<double>& y,
        double& h,
        double tolerance
    ) override;

    virtual void Solve(
        double t0,
        const std::vector<double>& y0,
        double tEnd,
        Storage& storage,
        double tolerance
    ) override;

private:
    enum Method { DISPFA, DISPFB, DISPFC };

    // Константы метода (взяты из старого кода)
    const std::array<std::array<double, 5>, 5> B = {{
        {1.0/4.0, 0, 0, 0, 0},
        {3.0/32.0, 9.0/32.0, 0, 0, 0},
        {1932.0/2197.0, -7200.0/2197.0, 7296.0/2197.0, 0, 0},
        {439.0/216.0, -8.0, 3680.0/513.0, -845.0/4104.0, 0},
        {-8.0/27.0, 2.0, -3544.0/2565.0, 1859.0/4104.0, -11.0/40.0}
    }};

    // Коэффициенты для комбинирования стадий
    const std::array<double, 6> P36 = {0.13196429763096, 0.0, 0.44293129620849, 0.53013219758834, -0.20587627627628, 0.10084848484848};
    const std::array<double, 6> P72 = {0.41975960186956, 0.44944365216575, 0.1296419611922, 0.0012199235635231, -0.000066250690732054, 0.0000011118997045939};
    const std::array<double, 6> P28 = {-0.38402741318519, 0.28983442536296, 1.0619636598535, 0.036673230572343, -0.0046504806400000, 0.00020657863636364};
    const std::array<double, 6> P4  = {25.0/216.0, 0.0, 1408.0/2565.0, 2197.0/4104.0, -1.0/5.0, 0.0};
    const std::array<double, 5> ACoef = {1.0/4.0, 3.0/8.0, 12.0/13.0, 1.0, 0.5};

    // Другие параметры метода
    double q = 1.1;
    double Lnq = 1.0 / std::log(q);
    double GAMMA;
    double CA1A = 1.351851851851851851852;
    double CA1B = 0.314690666666666666667;
    double d_coef = 0.24;

    // Дополнительные константы
    static constexpr double MAX_P = 400;
    static constexpr double MAXDOUBLE = 100.0e+300;

    // Параметры и состояние
    std::vector<std::vector<double>> k; // Матрица коэффициентов (размер: число уравнений x 6)
    std::vector<double> F;              // Вектор для хранения f(t,y)
    std::vector<double> Y0;             // Вспомогательный вектор
    Method currentMethod;
    int vnBreakCount = 0;
    bool jumpToRadau5 = false;

    // Вспомогательная функция, вычисляющая сумму стадий для уравнения eqNum и стадии stage
    double StadScalar(size_t eqNum, int stage);

    // Реализация методов по типу DISPF
    bool StepDISPFC(double& t, std::vector<double>& y, double& h, double tolerance);
    bool StepDISPFA(double& t, std::vector<double>& y, double& h, double tolerance);
    bool StepDISPFB(double& t, std::vector<double>& y, double& h, double tolerance);

    // Функция оценки ошибки (перенесена без изменений из старого кода)
    double CalcVn(const std::vector<double>& /*dummy*/, const std::vector<std::vector<double>>& k);
};
