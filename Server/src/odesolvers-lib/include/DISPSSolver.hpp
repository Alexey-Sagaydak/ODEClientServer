#pragma once

#include <vector>
#include <functional>
#include <stdexcept>
#include <cmath>
#include <string>

/*
    Флаги, указывающие, какие схемы включены (3ст,5ст,6ст и 1,2,3 порядок).
*/
struct DispsEnabledFlags {
    bool Disps13 = false; // 3 стадии, 1 порядок
    bool Disps15 = false; // 5 стадий, 1 порядок
    bool Disps23 = false; // 3 стадии, 2 порядок
    bool Disps25 = false; // 5 стадий, 2 порядок
    bool Disps35 = false; // 5 стадий, 3 порядок
    bool Disps36 = false; // 6 стадий, 3 порядок
};

/*
    Описание одного варианта DISPS:
      - stages: 3,5 или 6
      - order: 1,2 или 3
      - p: вектор коэффициентов p_i
*/
struct DispsVariant {
    int stages;
    int order;
    std::vector<double> p;
    double gamma;
};

/*
    Класс DISPSSolver:
      - хранит несколько вариантов схем (DispsVariant)
      - умеет делать шаги (Step3Stage, Step5Stage, Step6Stage)
      - умеет контролировать точность и устойчивость (для 1,2,3 порядка)
      - умеет переключать метод (SwitchScheme)
*/
class DISPSSolver
{
public:
    // Конструктор:
    //   - f: правая часть ОДУ
    //   - initialStep: начальный шаг
    //   - flags: какие схемы включены
    DISPSSolver(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                double initialStep,
                const DispsEnabledFlags& flags);

    // Основной метод решения:
    //   - t0, y0: начальные условия
    //   - tEnd: конечное время
    //   - tolerance: допуск
    // Возвращает массив вида [[t, y[0], y[1], ...], [...], ...]
    std::vector<std::vector<double>> Solve(double t0,
                                           const std::vector<double>& y0,
                                           double tEnd,
                                           double tolerance);

private:
    // Ссылка на функцию правой части
    std::function<std::vector<double>(double, const std::vector<double>&)> f_;
    // Текущий базовый шаг
    double stepSize_;

    // Набор включённых схем
    std::vector<DispsVariant> variants_;
    // Индекс текущей схемы
    int currentIndex_;

    // Выполняет один шаг (без контроля), возвращает yNext и заполняет kStages
    std::vector<double> DoOneStep(double t,
                                  const std::vector<double>& y,
                                  double h,
                                  const DispsVariant& variant,
                                  std::vector<std::vector<double>>& kStages);

    // Функции для 3,5,6 стадий
    // Они заполняют kStages[i] = k_i, а возвращают yNext
    static std::vector<double> Step3Stage(
        double t,
        const std::vector<double>& y,
        double h,
        std::function<std::vector<double>(double,const std::vector<double>&)> f,
        const std::vector<double>& p,
        std::vector<std::vector<double>>& kStages);

    static std::vector<double> Step5Stage(
        double t,
        const std::vector<double>& y,
        double h,
        std::function<std::vector<double>(double,const std::vector<double>&)> f,
        const std::vector<double>& p,
        std::vector<std::vector<double>>& kStages);

    static std::vector<double> Step6Stage(
        double t,
        const std::vector<double>& y,
        double h,
        std::function<std::vector<double>(double,const std::vector<double>&)> f,
        const std::vector<double>& p,
        std::vector<std::vector<double>>& kStages);

    // Контроль точности/устойчивости (1,2,3 порядок)
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

    // Переключение схемы (пример)
    void SwitchScheme(int currentOrder);
};
