#include "../include/STEKSSolver.hpp"

void STEKSSolver::Step(
    double               t,
    std::vector<double> &y,
    double              &h,
    double               tolerance)
{
    std::vector<double> y_temp = y;

    std::vector<double> k1 = MultiplyScalarByVector(h, f(t, y_temp));

    std::vector<double> y2 = AddVectors(y_temp, MultiplyScalarByVector(1.0/3.0, k1));
    std::vector<double> k2 = MultiplyScalarByVector(h, f(t + h/3.0, y2));

    std::vector<double> y3 = AddVectors(
        AddVectors(y_temp, MultiplyScalarByVector(1.0/6.0, k1)),
        MultiplyScalarByVector(1.0/6.0, k2)
    );
    std::vector<double> k3 = MultiplyScalarByVector(h, f(t + h/3.0, y3));

    std::vector<double> y4 = AddVectors(
        AddVectors(y_temp, MultiplyScalarByVector(1.0/8.0, k1)),
        MultiplyScalarByVector(3.0/8.0, k3)
    );
    std::vector<double> k4 = MultiplyScalarByVector(h, f(t + h/2.0, y4));

    std::vector<double> y5 = AddVectors(
        AddVectors(
            AddVectors(y_temp, MultiplyScalarByVector(0.5, k1)),
            MultiplyScalarByVector(-1.5, k3)
        ),
        MultiplyScalarByVector(2.0, k4)
    );
    std::vector<double> k5 = MultiplyScalarByVector(h, f(t + h, y5));

    // Оценка ошибки
    double error = computeError(k1, k2, k3, k4, k5, h, tolerance);

    // Адаптация шага
    if (error > tolerance)
    {
        double scale = SAFETY * std::pow(tolerance / error, 1.0/4.0);

        scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);
        h *= scale;

        throw std::runtime_error("Step rejected");
    }

    // Обновление решения
    y_temp = AddVectors(
        AddVectors(
            y_temp,
            MultiplyScalarByVector(1.0/6.0, k1)
        ),
        AddVectors(
            MultiplyScalarByVector(2.0/3.0, k4),
            MultiplyScalarByVector(1.0/6.0, k5)
        )
    );

    // Корректировка шага
    double scale = SAFETY * std::pow(tolerance / error, 1.0/4.0);
    scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);
    h *= scale;
    y = y_temp;
}

double STEKSSolver::computeError(
    std::vector<double> const &k1,
    std::vector<double> const &k2,
    std::vector<double> const &k3,
    std::vector<double> const &k4,
    std::vector<double> const &k5,
    double                     h,
    double                     tolerance)
{
    // Условие точности: (1/30)||2k1 -9k3 +8k4 -k5|| <= 5e^(5/4)
    std::vector<double> precision_term = SubtractVectors(
        AddVectors(
            MultiplyScalarByVector(2.0, k1),
            MultiplyScalarByVector(-9.0, k3)
        ),
        AddVectors(
            MultiplyScalarByVector(8.0, k4),
            k5
        )
    );
    double precision_error = (Norm(precision_term) / 30.0) / (5.0 * std::exp(5.0/4.0));

    // Условие устойчивости: 6*max|(k3 -k2)/(k2 -k1)| <= 3.5
    double stability_error = 0.0;
    for (size_t i = 0; i < k1.size(); ++i)
    {
        double denominator = k2[i] - k1[i];

        if (std::abs(denominator) < 1e-12) denominator = 1e-12;

        double ratio = std::abs((k3[i] - k2[i]) / denominator);
        stability_error = std::max(stability_error, ratio);
    }
    stability_error = (6.0 / 3.5) * stability_error;

    return std::max(precision_error, stability_error);
}

void STEKSSolver::Solve(
    double                     t0,
    std::vector<double> const &y0,
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

        try {
            std::vector<double> y_new = y;
            Step(t, y_new, h_attempt, tolerance);
            t += h_attempt;
            y = y_new;
            storage.Add(t, y);
            h = h_attempt;
        }
        catch (const std::runtime_error&)
        {
            h *= 0.5;
        }
    }
}
