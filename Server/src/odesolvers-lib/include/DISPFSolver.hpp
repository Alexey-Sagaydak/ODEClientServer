#pragma once
#include "Solver.hpp"
#include "CommonFunctions.hpp"
#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <functional>
#include <algorithm>

class DISPFSolver : public Solver
{
public:
    // Конструктор теперь принимает дополнительные параметры:
    // I - порядок метода (0 - переменный порядок, 1,2,5 - фиксированный порядок),
    // J - контроль устойчивости (0 - включён, 1 - выключен),
    // K - метод оценки матрицы Якоби (0 - специальной подпрограммой, 1 - степенной метод, 2 - осреднение),
    // если J == 1, то K игнорируется.
    DISPFSolver(
        std::function<std::vector<double>(double, const std::vector<double>&)> func,
        double initialStep,
        double gamma,
        int I, int J, int K
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

    // Константы метода (как в исходном коде)
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
    std::vector<std::vector<double>> k; // Матрица коэффициентов (n уравнений x 6)
    std::vector<double> F;              // Вектор для f(t,y)
    std::vector<double> Y0;             // Вспомогательный вектор

    // Переменная для выбора метода
    Method currentMethod;
    int vnBreakCount = 0;
    bool jumpToRadau5 = false;

    // Новые параметры управления порядком и стабильностью:
    bool variableOrder;   // true, если I==0 (переменный порядок)
    int fixedOrder;       // если не переменный порядок, то фиксированный порядок (1,2 или 5)
    bool stabilityControlEnabled; // true, если J==0 (контроль устойчивости включён)
    int jacobianMethod;   // метод оценки Якоби: 0,1 или 2 (используется только если контроль включён)

    // Вспомогательная функция для вычисления суммы стадий для уравнения eqNum и стадии stage
    double StadScalar(size_t eqNum, int stage);

    // Методы шага
    bool StepDISPFC(double& t, std::vector<double>& y, double& h, double tolerance);
    bool StepDISPFA(double& t, std::vector<double>& y, double& h, double tolerance);
    bool StepDISPFB(double& t, std::vector<double>& y, double& h, double tolerance);

    // Оценка ошибки (как в старом коде)
    double CalcVn(const std::vector<double>& /*dummy*/, const std::vector<std::vector<double>>& k);

    // Новая функция для оценки наибольшего собственного значения матрицы Якоби
    double EstimateJacobianEigenvalue(double t, const std::vector<double>& y);
};
