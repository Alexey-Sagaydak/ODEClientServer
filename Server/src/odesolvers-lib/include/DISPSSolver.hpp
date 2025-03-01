#pragma once
#include "Solver.hpp"           // Базовый класс Solver с определением f(t,y) и stepSize

struct DispsEnabledFlags {
    bool Disps13;
    bool Disps15;

    bool Disps23;
    bool Disps25;

    bool Disps35;
    bool Disps36;
};

// Структура, описывающая один вариант DISPS (число стадий + коэффициенты p)
struct DispsVariant {
    int stages;
    std::vector<double> p;
};

// Класс решателя DISPS
class DISPSSolver
{
public:
    // Конструктор принимает:
    //   - odeFunc: правая часть ОДУ
    //   - initialStep: начальный шаг
    //   - flags: набор флажков, какие методы включать (Disps13, Disps23, и т.д.)
    DISPSSolver(std::function<std::vector<double>(double, const std::vector<double>&)> odeFunc,
                double initialStep,
                const DispsEnabledFlags& flags);

    // Метод решения. Накапливает точки (t, y) и возвращает их
    std::vector<std::vector<double>> Solve(double t0,
                                           const std::vector<double>& y0,
                                           double tEnd,
                                           double tolerance);

private:
    // Ссылка на функцию правой части
    std::function<std::vector<double>(double, const std::vector<double>&)> f;
    // Начальный шаг
    double stepSize;

    // Вектор включённых вариантов (число стадий + коэффициенты p)
    std::vector<DispsVariant> methods;
    int currentMethodIndex; // Пока всегда 0

    // Функция, выполняющая один шаг, выбирая нужную схему по числу стадий
    void DoStep(double t, std::vector<double>& y, double& h, const DispsVariant& variant);

    // Отдельные реализации шага для 3- и 5-стадийных методов
    static std::vector<double> Step3Stage(double t,
                                          const std::vector<double>& y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          const std::vector<double>& p);

    static std::vector<double> Step5Stage(double t,
                                          const std::vector<double>& y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          const std::vector<double>& p);
    static std::vector<double> Step6Stage(double t,
                                          const std::vector<double>& y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          const std::vector<double>& p);
};
