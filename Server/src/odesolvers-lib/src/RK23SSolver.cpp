#include "../include/RK23SSolver.hpp"
#include <algorithm>

void RK23SSolver::Step(
        double               t,
        std::vector<double> &y,
        double              &h,
        double               tolerance)
{
    std::vector<double> y_temp = y;
    
    // Вычисление стадий
    std::vector<double> k1 = MultiplyScalarByVector(h, f(t, y_temp));
    
    std::vector<double> y2 = AddVectors(y_temp, MultiplyScalarByVector(beta21, k1));
    std::vector<double> k2 = MultiplyScalarByVector(h, f(t + alpha2*h, y2));
    
    std::vector<double> y3 = AddVectors(
        y_temp,
        AddVectors(
            MultiplyScalarByVector(beta31, k1),
            MultiplyScalarByVector(beta32, k2)
        )
    );
    std::vector<double> k3 = MultiplyScalarByVector(h, f(t + alpha3*h, y3));
    
    // Оценка ошибки
    double error = computeError(k1, k2, k3, h, tolerance);
    
    // Адаптация шага
    if (error > tolerance)
    {
        double scale = std::max(SAFETY * pow(tolerance/error, 1.0/3.0), MIN_SCALE);
        h *= scale;
        throw std::runtime_error("Step rejected");
    }
    
    // Применение шага
    y_temp = AddVectors(
        y_temp,
        AddVectors(
            MultiplyScalarByVector(p1, k1),
            AddVectors(
                MultiplyScalarByVector(p2, k2),
                MultiplyScalarByVector(p3, k3)
            )
        )
    );
    
    // Корректировка шага
    double scale = std::min(SAFETY * pow(tolerance/error, 1.0/3.0), MAX_SCALE);
    h *= scale;
    y = y_temp;
}

double RK23SSolver::computeError(
    std::vector<double> const &k1,
    std::vector<double> const &k2,
    std::vector<double> const &k3,
    double                     h,
    double                     tolerance)
{
    // Условие точности (4.7)
    std::vector<double> diff_k2k1 = SubtractVectors(k2, k1);
    double err_precision = (6.0 * alpha2 * tolerance) / (1.0 - 6.0*g) * Norm(diff_k2k1);
    
    // Условие устойчивости (4.14)
    double stability = 0.0;
    for (size_t i = 0; i < k1.size(); ++i)
    {
        double denominator = k3[i] - k1[i];
        if (fabs(denominator) < 1e-12) denominator = 1e-12;
        stability = std::max(stability, 3.0 * fabs((k3[i] - k2[i]) / denominator));
    }
    
    return std::max(err_precision, stability);
}

void RK23SSolver::Solve(
        double                     t0,
        const std::vector<double> &y0,
        double                     tEnd,
        Storage                   &storage,
        double                     tolerance)
{
    double t = t0;
    std::vector<double> y = y0;
    double h = stepSize;
    storage.Add(t, y);

    while (t < tEnd)
    {
        double h_attempt = std::min(h, tEnd - t);
        if (h_attempt < 1e-12) break;

        try
        {
            std::vector<double> y_new = y;
            Step(t, y_new, h_attempt, tolerance);
            t += h_attempt;
            y = y_new;
            storage.Add(t, y);
            h = h_attempt;
        } 
        catch (const std::runtime_error&) {
            h = h_attempt * 0.5;
        }
    }
}
