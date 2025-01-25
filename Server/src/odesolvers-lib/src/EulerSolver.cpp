#include "../include/EulerSolver.hpp"

void EulerSolver::Step(
        double               t,
        std::vector<double> &y,
        double              &h,
        double               tolerance)
{
    std::vector<double> y_temp = y;
    
    // Пробный шаг Эйлера
    std::vector<double> k = MultiplyScalarByVector(h, f(t, y_temp));
    y_temp = AddVectors(y_temp, k);
    
    // Оценка ошибки через разность производных
    std::vector<double> f_curr = f(t, y);
    std::vector<double> f_next = f(t + h, y_temp);
    
    // Локальная ошибка (порядок h^2)
    double error = 0.0;
    for (size_t i = 0; i < y.size(); ++i)
    {
        error = std::max(error, std::abs(0.5 * h * (f_next[i] - f_curr[i])));
    }
    
    // Адаптация шага
    if (error > tolerance)
    {
        h *= std::max(SAFETY_FACTOR * sqrt(tolerance / error), MIN_FACTOR);
        throw std::runtime_error("Step rejected"); // Шаг отклонен
    }
    else
    {
        // Принимаем шаг и корректируем размер
        double factor = std::min(SAFETY_FACTOR * sqrt(tolerance / (error + 1e-12)), MAX_FACTOR);
        h *= factor;
        y = y_temp; // Обновляем состояние только при успехе
    }
}

void EulerSolver::Solve(
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
        if (h_attempt <= 0) break;

        try
        {
            std::vector<double> y_new = y;
            Step(t, y_new, h_attempt, tolerance); // Проверка точности внутри Step
            t += h_attempt;
            y = y_new;
            storage.Add(t, y);
            h = h_attempt; // Обновляем базовый шаг
        } 
        catch (const std::runtime_error&)
        {
            // Шаг отклонен - повторяем с уменьшенным h
            h = h_attempt * 0.5;
        }
    }
}
