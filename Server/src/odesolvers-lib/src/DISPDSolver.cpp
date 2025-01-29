#include "../include/DISPDSolver.hpp"
#include <vector>
#include <cmath>

using namespace std;

void DISPDSolver::Step(double t, vector<double> &y, double &h, double tolerance) {
    vector<double> y_temp = y;
    double g = 1.0/6.0; // Примерное значение, может потребовать настройки

    // Вычисление k1, k2, k3 согласно алгоритму
    vector<double> k1 = MultiplyScalarByVector(h, f(t, y_temp));
    vector<double> y2 = AddVectors(y_temp, MultiplyScalarByVector(2.0/3.0, k1));
    vector<double> k2 = MultiplyScalarByVector(h, f(t + 2.0*h/3.0, y2));

    vector<double> y3 = AddVectors(
        AddVectors(y_temp, MultiplyScalarByVector(1.0/3.0, k1)),
        MultiplyScalarByVector(1.0/3.0, k2)
    );
    vector<double> k3 = MultiplyScalarByVector(h, f(t + h, y3));

    // Вычисление оценок ошибок
    double Aprime = computeAprime(k1, k2, g);
    double Asecond = computeAsecond(k1, k3, h, g);
    double V = computeV(k3, k2, k1);

    // Вычисление параметров для адаптации шага
    double s = log(tolerance / Aprime) / (2 * log(q));
    double nu = log(tolerance / Asecond) / (2 * log(q));

    // Проверка условий переключения алгоритмов
    double h1 = h * pow(q, min({s, nu, log(18/V)/log(q)}));
    double h2 = h * pow(q, min({s, s, log(6/V)/log(q)}));

    // Выбор следующего алгоритма и шага
    if (h2 > h1) {
        useAlgorithmA = false;
        h = h2;
    } else {
        useAlgorithmA = true;
        h = h1;
    }

    // Обновление решения в зависимости от алгоритма
    vector<double> y_new;
    if (useAlgorithmA) {
        // Параметры для алгоритма A (5.2)
        double p1 = 0.25;
        double p2 = (3 - 18*g)/4;
        double p3 = 4.5*g;
        y_new = AddVectors(y_temp, AddVectors(
            MultiplyScalarByVector(p1, k1),
            AddVectors(
                MultiplyScalarByVector(p2, k2),
                MultiplyScalarByVector(p3, k3)
            )
        ));
    } else {
        // Параметры для алгоритма B (5.12)
        double p1 = 7.0/9.0;
        double p2 = 16.0/81.0;
        double p3 = 2.0/81.0;
        y_new = AddVectors(y_temp, AddVectors(
            MultiplyScalarByVector(p1, k1),
            AddVectors(
                MultiplyScalarByVector(p2, k2),
                MultiplyScalarByVector(p3, k3)
            )
        ));
    }

    // Проверка ошибки и коррекция шага
    double error = max(Aprime, Asecond);
    if (error > tolerance) {
        h *= computeStepScale(error, tolerance);
        throw runtime_error("Step rejected");
    }

    y = y_new;
    h *= computeStepScale(error, tolerance);
}

double DISPDSolver::computeAprime(const vector<double>& k1, const vector<double>& k2, double g) {
    vector<double> delta = SubtractVectors(k2, k1);
    return abs(1 - 6*g)/4 * Norm(delta);
}

double DISPDSolver::computeAsecond(const vector<double>& k1, const vector<double>& k_new, double h, double g) {
    vector<double> delta = SubtractVectors(k_new, k1);
    return abs(1 - 6*g)/6 * Norm(delta);
}

double DISPDSolver::computeV(const vector<double>& k3, const vector<double>& k2, const vector<double>& k1) {
    vector<double> numerator = SubtractVectors(k3, k2);
    vector<double> denominator = SubtractVectors(k2, k1);
    double max_ratio = 0.0;
    for (size_t i = 0; i < numerator.size(); ++i) {
        double denom = abs(denominator[i]) > 1e-12 ? denominator[i] : 1e-12;
        max_ratio = max(max_ratio, abs(numerator[i] / denom));
    }
    return 3 * max_ratio;
}

double DISPDSolver::computeStepScale(double error, double tolerance) {
    double scale = SAFETY * pow(tolerance / error, 1.0/3.0);
    return clamp(scale, MIN_SCALE, MAX_SCALE);
}

void DISPDSolver::Solve(
    double t0,
    vector<double> const &y0,
    double tEnd,
    Storage &storage,
    double tolerance) 
{
    double t = t0;
    vector<double> y = y0;
    double h = stepSize;
    storage.Add(t, y);

    while (t < tEnd) {
        double h_attempt = min(h, tEnd - t);
        if (h_attempt < 1e-12) break;

        try {
            vector<double> y_new = y;
            Step(t, y_new, h_attempt, tolerance);
            t += h_attempt;
            y = y_new;
            storage.Add(t, y);
            h = h_attempt;
        } catch (const runtime_error&) {
            h *= 0.5;
        }
    }
}
