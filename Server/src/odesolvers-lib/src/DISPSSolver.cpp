#include "../include/DISPSSolver.hpp"

DISPSSolver::DISPSSolver(function<vector<double>(double, const vector<double>&)> func,
                         double initialStep,
                         double q,
                         const json& parameters)
    : Solver(func, initialStep), q(q)
{
    // Регистрируем включённые варианты по ключам JSON
    if (parameters.contains("Disps13") && parameters["Disps13"].get<int>() == 1)
        methods.push_back(DISPS_13);
    if (parameters.contains("Disps15") && parameters["Disps15"].get<int>() == 1)
        methods.push_back(DISPS_15);
    if (parameters.contains("Disps16") && parameters["Disps16"].get<int>() == 1)
        methods.push_back(DISPS_16);
    if (parameters.contains("Disps23") && parameters["Disps23"].get<int>() == 1)
        methods.push_back(DISPS_23);
    if (parameters.contains("Disps25") && parameters["Disps25"].get<int>() == 1)
        methods.push_back(DISPS_25);
    if (parameters.contains("Disps26") && parameters["Disps26"].get<int>() == 1)
        methods.push_back(DISPS_26);
    if (parameters.contains("Disps33") && parameters["Disps33"].get<int>() == 1)
        methods.push_back(DISPS_33);
    if (parameters.contains("Disps35") && parameters["Disps35"].get<int>() == 1)
        methods.push_back(DISPS_35);
    if (parameters.contains("Disps36") && parameters["Disps36"].get<int>() == 1)
        methods.push_back(DISPS_36);
    if (methods.empty())
        throw std::runtime_error("Нет включённых вариантов метода DISPS");
    currentMethodIndex = 0;
    currentTable = methods[currentMethodIndex];
}

void DISPSSolver::Step(double t, vector<double>& y, double& h, double tolerance) {
    genericStep(t, y, h, tolerance, currentTable);
}

void DISPSSolver::Solve(double t0, const vector<double>& y0, double tEnd, Storage& storage, double tolerance) {
    double t = t0;
    vector<double> y = y0;
    double h_local = stepSize;
    storage.Add(t, y);
    while (t < tEnd) {
        double hAttempt = std::min(h_local, tEnd - t);
        if (hAttempt < 1e-12)
            break;
        try {
            Step(t, y, hAttempt, tolerance);
            t += hAttempt;
            storage.Add(t, y);
            h_local = hAttempt;
        }
        catch (const std::runtime_error&) {
            h_local *= 0.5;
        }
    }
}

void DISPSSolver::genericStep(double t, vector<double>& y, double& h, double tolerance, const ButcherTable& table) {
    int stages = table.c.size();
    vector<vector<double>> k(stages, vector<double>(y.size(), 0.0));

    // Вычисляем все стадии:
    for (int i = 0; i < stages; i++) {
        vector<double> y_stage = y;
        for (int j = 0; j < i; j++) {
            y_stage = AddVectors(y_stage, MultiplyScalarByVector(table.a[i][j], k[j]));
        }
        double t_stage = t + table.c[i] * h;
        k[i] = MultiplyScalarByVector(h, f(t_stage, y_stage));
    }

    // Комбинируем стадии: yNext = y + сумма(p_i * k_i)
    vector<double> yNext = y;
    for (int i = 0; i < stages; i++) {
        yNext = AddVectors(yNext, MultiplyScalarByVector(table.p[i], k[i]));
    }

    double Aprime = computeAprime(k[0], (stages > 1 ? k[1] : k[0]));
    double AddPrime = computeAddoublePrime(h, k[0], k[stages - 1]);
    double V = computeV(k[0], (stages > 1 ? k[1] : k[0]), k[stages - 1]);

    bool stepAccepted = false;
    int attempts = 0;
    while (!stepAccepted && attempts < 10) {
        double sn = log(tolerance / (pow(q, 2 * attempts) * Aprime)) / (2 * log(q));
        if (sn < 0) {
            h *= pow(q, sn);
            attempts++;
            continue;
        }
        double vn = log(tolerance / (pow(q, 2 * attempts) * AddPrime)) / (2 * log(q));
        if (vn < 0) {
            h *= pow(q, vn);
            attempts++;
            continue;
        }
        double rn = log(18.0 / (pow(q, attempts) * V)) / log(q);
        double scale = SAFETY * std::min({ pow(q, sn), pow(q, vn), pow(q, rn), MAX_SCALE });
        scale = clamp(scale, MIN_SCALE, MAX_SCALE);
        if (V > 18.0 / pow(q, attempts) || Aprime > tolerance / pow(q, 2 * attempts)) {
            h *= scale;
            attempts++;
        }
        else {
            y = yNext;
            h *= scale;
            stepAccepted = true;
        }
    }
    if (!stepAccepted)
        throw std::runtime_error("Проблема адаптации шага в методе DISPS");

    // Проверка условия переключения схем – здесь можно обновить currentTable при необходимости
    if (shouldSwitchToSchemeB(h, tolerance, table))
        switchScheme(true);
}

double DISPSSolver::computeAprime(const vector<double>& k1, const vector<double>& k2) {
    vector<double> diff = SubtractVectors(k2, k1);
    double g = currentTable.g;
    return (std::abs(1 - 6 * g) / 4.0) * Norm(diff);
}

double DISPSSolver::computeAddoublePrime(double h, const vector<double>& k1, const vector<double>& k_last) {
    vector<double> diff = SubtractVectors(MultiplyScalarByVector(h, k_last), k1);
    double g = currentTable.g;
    return (std::abs(1 - 6 * g) / 6.0) * Norm(diff);
}

double DISPSSolver::computeV(const vector<double>& k1, const vector<double>& k2, const vector<double>& k_last) {
    double maxRatio = 0.0;
    for (size_t i = 0; i < k1.size(); i++) {
        double denominator = k2[i] - k1[i];
        if (std::abs(denominator) < 1e-12)
            denominator = 1e-12;
        double ratio = std::abs((k_last[i] - k2[i]) / denominator);
        maxRatio = std::max(maxRatio, ratio);
    }
    return 3.0 * maxRatio;
}

bool DISPSSolver::shouldSwitchToSchemeB(double h, double tolerance, const ButcherTable& table) {
    double g = table.g;
    return (h * MAX_SCALE > g * tolerance);
}

double DISPSSolver::clamp(double val, double low, double high) {
    return std::max(low, std::min(val, high));
}

void DISPSSolver::switchScheme(bool toSchemeB) {
    // Заглушка для переключения схем – здесь можно обновить currentTable
    // Например, если toSchemeB==true, установить альтернативные коэффициенты
    // Для демонстрации оставляем схему без изменений.
}
