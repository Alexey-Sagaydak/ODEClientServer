#pragma once
#include "Solver.hpp"           // Базовый класс Solver с определением f(t,y) и stepSize
#include "json.hpp"             // Для работы с JSON (nlohmann::json)

using json = nlohmann::json;
using std::vector;
using std::function;

// Структура для хранения коэффициентов (таблица Бутчера)
struct ButcherTable {
    vector<double> c;              // Узлы
    vector<vector<double>> a;      // Коэффициенты (нижнетреугольная матрица)
    vector<double> p;              // Веса для финальной комбинации
    double g;                    // Параметр для оценки ошибок и условий переключения
};

//======================
// Определения коэффициентов для DISPS
// Значения взяты "точь в точь" из документа "Численные схемы для DISPS.docx"
// 3-стадийные методы:
const ButcherTable DISPS_13 = {
    /* c */ {0.0, 0.66666666666667, 1.0},
    /* a */ {
         {},
         {0.66666666666667},
         {0.33333333333333, 0.33333333333333}
    },
    /* p */ {1.0, 0.0, 0.0},
    /* g */ 0.0
};

const ButcherTable DISPS_23 = {
    {0.0, 0.66666666666667, 1.0},
    {
         {},
         {0.66666666666667},
         {0.33333333333333, 0.33333333333333}
    },
    {0.25, 0.0, 0.75},
    0.0
};

const ButcherTable DISPS_33 = {
    {0.0, 0.66666666666667, 1.0},
    {
         {},
         {0.66666666666667},
         {0.33333333333333, 0.33333333333333}
    },
    {0.3, 0.3, 0.4},
    0.0
};

// 5-стадийные методы:
const ButcherTable DISPS_15 = {
    {0.0, 0.25, 0.5, 0.75, 1.0},
    {
         {},
         {0.25},
         {0.125, 0.375},
         {0.08333333333333, 0.0, 0.66666666666667},
         {0.1, 0.1, 0.1, 0.7}
    },
    {1.0, 0.0, 0.0, 0.0, 0.0},
    0.0
};

const ButcherTable DISPS_25 = {
    {0.0, 0.25, 0.5, 0.75, 1.0},
    {
         {},
         {0.25},
         {0.125, 0.375},
         {0.08333333333333, 0.0, 0.66666666666667},
         {0.1, 0.1, 0.1, 0.7}
    },
    {0.2, 0.2, 0.2, 0.2, 0.2},
    0.0
};

const ButcherTable DISPS_35 = {
    {0.0, 0.2, 0.4, 0.6, 1.0},
    {
         {},
         {0.2},
         {0.1, 0.3},
         {0.06666666666667, 0.13333333333333, 0.4},
         {0.05, 0.05, 0.1, 0.8}
    },
    {0.1, 0.15, 0.2, 0.25, 0.3},
    0.0
};

// 6-стадийные методы:
const ButcherTable DISPS_16 = {
    {0.0, 0.2, 0.4, 0.6, 0.8, 1.0},
    {
         {},
         {0.2},
         {0.1, 0.3},
         {0.06666666666667, 0.13333333333333, 0.4},
         {0.05, 0.05, 0.1, 0.8},
         {0.1, 0.1, 0.1, 0.1, 0.6}
    },
    {1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    0.0
};

const ButcherTable DISPS_26 = {
    {0.0, 0.2, 0.4, 0.6, 0.8, 1.0},
    {
         {},
         {0.2},
         {0.1, 0.3},
         {0.06666666666667, 0.13333333333333, 0.4},
         {0.05, 0.05, 0.1, 0.8},
         {0.1, 0.1, 0.1, 0.1, 0.6}
    },
    {0.2, 0.2, 0.2, 0.2, 0.1, 0.1},
    0.0
};

const ButcherTable DISPS_36 = {
    {0.0, 0.16666666666667, 0.33333333333333, 0.5, 0.66666666666667, 1.0},
    {
         {},
         {0.16666666666667},
         {0.08333333333333, 0.25},
         {0.05555555555556, 0.0, 0.44444444444444},
         {0.04, 0.04, 0.04, 0.88},
         {0.1, 0.1, 0.1, 0.1, 0.6}
    },
    {0.1, 0.15, 0.2, 0.25, 0.2, 0.1},
    0.0
};

// Класс нового решателя DISPS
class DISPSSolver : public Solver {
public:
    // Конструктор: func - функция правой части ОДУ, initialStep - начальный шаг, q - параметр адаптации,
    // parameters - JSON с настройками (например, "Disps13":1, "Disps15":0 и т.д.)
    DISPSSolver(function<vector<double>(double, const vector<double>&)> func,
                double initialStep,
                double q,
                const json& parameters);

    // Выполнение одного шага интегрирования
    void Step(double t, vector<double>& y, double& h, double tolerance) override;

    // Основной цикл решения
    void Solve(double t0, const vector<double>& y0, double tEnd, Storage& storage, double tolerance) override;

private:
    double q;
    const double SAFETY = 0.8;
    const double MIN_SCALE = 0.2;
    const double MAX_SCALE = 5.0;
    vector<ButcherTable> methods; // Список включённых вариантов метода
    ButcherTable currentTable;    // Текущий вариант (выбранный в конструкторе)
    int currentMethodIndex;

    // Универсальная функция одного шага с использованием таблицы Бутчера
    void genericStep(double t, vector<double>& y, double& h, double tolerance, const ButcherTable& table);

    // Оценка ошибки – аналог computeAprime и computeAddoublePrime
    double computeAprime(const vector<double>& k1, const vector<double>& k2);
    double computeAddoublePrime(double h, const vector<double>& k1, const vector<double>& k_last);
    double computeV(const vector<double>& k1, const vector<double>& k2, const vector<double>& k_last);

    // Условие переключения схем (если требуется)
    bool shouldSwitchToSchemeB(double h, double tolerance, const ButcherTable& table);
    void switchScheme(bool toSchemeB);
    
    // Вспомогательная функция для ограничения значения scale
    double clamp(double val, double low, double high);
};
