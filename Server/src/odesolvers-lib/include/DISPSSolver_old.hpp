#pragma once
#include <vector>
#include <functional>
#include <stdexcept>
#include <cmath>
#include "DISPSSolver.hpp"

// Структура, описывающая один вариант DISPS (число стадий + коэффициенты p)
struct DispsVariant_old
{
    int                 stages;
    std::vector<double> p;
};

class DISPSSolver_old
{
public:
    // Конструктор принимает:
    //   - odeFunc: правая часть ОДУ
    //   - initialStep: начальный шаг
    //   - flags: набор флажков, какие методы включать (Disps13, Disps23, и т.д.)
    DISPSSolver_old(std::function<std::vector<double>(
                    double,
                    std::vector<double> const &)> odeFunc,
                double                            initialStep,
                DispsEnabledFlags const          &flags);

    // Метод решения. Накапливает точки (t, y) и возвращает их
    std::vector<std::vector<double>> Solve(double                     t0,
                                           std::vector<double> const &y0,
                                           double                     tEnd,
                                           double                     tolerance);

private:
    // Ссылка на функцию правой части
    std::function<std::vector<double>(double, const std::vector<double>&)> f;
    // Начальный шаг
    double stepSize;

    // Вектор включённых вариантов (число стадий + коэффициенты p)
    std::vector<DispsVariant_old> methods;
    int currentMethodIndex; // Пока всегда 0

    // Функция, выполняющая один шаг, выбирая нужную схему по числу стадий
    void DoStep(
        double               t,
        std::vector<double> &y,
        double              &h,
        DispsVariant_old const  &variant);

    // Отдельные реализации шага для 3-, 5- и 6-стадийных методов
    static std::vector<double> Step3Stage(double t,
                                          std::vector<double> const &y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          std::vector<double> const &p);

    static std::vector<double> Step5Stage(double t,
                                          std::vector<double> const &y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          std::vector<double> const &p);

    static std::vector<double> Step6Stage(double t,
                                          std::vector<double> const &y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          std::vector<double> const &p);
};
