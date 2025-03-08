#pragma once
#include <algorithm>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cmath>

// Флаги выбора схем
struct DispsEnabledFlags {
    bool Disps13 = false;
    bool Disps15 = false;
    bool Disps23 = false;
    bool Disps25 = false;
    bool Disps35 = false;
    bool Disps36 = false;
};

// Описание варианта схемы
struct DispsVariant {
    int stages;             // Число стадий (3, 5 или 6)
    int order;              // Порядок метода (1, 2 или 3)
    std::vector<double> p;  // Коэффициенты
    double gamma;           // Порог устойчивости для переключения
};

class DISPSSolver {
public:
    // Конструктор: f - функция ОДУ, initialStep - начальный шаг, flags - выбранные схемы
    DISPSSolver(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                double initialStep,
                const DispsEnabledFlags& flags);

    // Решение ОДУ: возвращает точки [t, y...]
    std::vector<std::vector<double>> Solve(double t0,
                                           const std::vector<double>& y0,
                                           double tEnd,
                                           double tolerance);

private:
    std::function<std::vector<double>(double, const std::vector<double>&)> f_;
    double stepSize_;
    std::vector<DispsVariant> variants_;
    int currentIndex_;

    // Выполнение одного шага без контроля
    std::vector<double> DoOneStep(double t,
                                  const std::vector<double>& y,
                                  double h,
                                  const DispsVariant& variant,
                                  std::vector<std::vector<double>>& kStages);

    // Шаги для различных схем
    static std::vector<double> Step3Stage(double t,
                                          const std::vector<double>& y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          const std::vector<double>& p,
                                          std::vector<std::vector<double>>& kStages);

    static std::vector<double> Step5Stage(double t,
                                          const std::vector<double>& y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          const std::vector<double>& p,
                                          std::vector<std::vector<double>>& kStages);

    static std::vector<double> Step6Stage(double t,
                                          const std::vector<double>& y,
                                          double h,
                                          std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                          const std::vector<double>& p,
                                          std::vector<std::vector<double>>& kStages);

    // Контроль точности и устойчивости
    bool Control1stOrder(const std::vector<std::vector<double>>& kStages,
                         const std::vector<double>& yOld,
                         const std::vector<double>& yNew,
                         double h,
                         double tolerance,
                         double& newH,
                         bool& needSwitch);

    bool Control2ndOrder(const std::vector<std::vector<double>>& kStages,
                         const std::vector<double>& yOld,
                         const std::vector<double>& yNew,
                         double h,
                         double tolerance,
                         double& newH,
                         bool& needSwitch);

    bool Control3rdOrder(const std::vector<std::vector<double>>& kStages,
                         const std::vector<double>& yOld,
                         const std::vector<double>& yNew,
                         double h,
                         double tolerance,
                         double& newH,
                         bool& needSwitch);

    // Выбор оптимальной схемы
    void SwitchScheme(int currentOrder);

    // Вспомогательные функции для работы с векторами
    static std::vector<double> Add(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> Mul(double alpha, const std::vector<double>& v);
    static std::vector<double> Subtract(const std::vector<double>& a, const std::vector<double>& b);
    static double Norm(const std::vector<double>& v);
};
