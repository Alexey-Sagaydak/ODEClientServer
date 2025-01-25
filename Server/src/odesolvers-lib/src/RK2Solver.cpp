#include "../include/RK2Solver.hpp"

RK2Solver::RK2Solver(std::function<std::vector<double>(double, const std::vector<double>&)> func, double initialStep)
    : Solver(func, initialStep) {
}

void RK2Solver::Step(
        double               t,
        std::vector<double> &y,
        double              &h,
        double               tolerance)
{
    while (true)
    {
        std::vector<double> k1 = MultiplyScalarByVector(h, f(t, y));
        std::vector<double> k2 = MultiplyScalarByVector(h, f(t, AddVectors(y, MultiplyScalarByVector(b21, k1))));
        
        // Вычисляем следующее приближение
        std::vector<double> y_next = AddVectors(
            y,
            AddVectors(MultiplyScalarByVector(p1, k1), MultiplyScalarByVector(p2, k2))
        );

        // Оценка ошибки по двум критериям
        std::vector<double> hf_next = MultiplyScalarByVector(h, f(t + h, y_next));
        double error1 = Norm(SubtractVectors(k2, k1)) / 4.0;        // Условие (3.50)
        double error2 = Norm(SubtractVectors(hf_next, k1)) / 6.0;   // Условие (3.51)
        double error = std::max(error1, error2);

        // Приемлемая ошибка - принимаем шаг
        if (error <= tolerance)
        {
            // Адаптируем шаг на основе ошибки (порядок метода 2, оценка ошибки ~ h^3)
            double factor = SAFETY_FACTOR * pow(tolerance / error, 1.0/3.0);
            factor = std::clamp(factor, MIN_FACTOR, MAX_FACTOR);
            h *= factor;
            
            y = y_next;
            t += h;

            return;
        }
        
        // Уменьшаем шаг при большой ошибке
        double factor = SAFETY_FACTOR * pow(tolerance / error, 1.0/3.0);
        factor = std::max(factor, MIN_FACTOR);
        h *= factor;
    }
}

void RK2Solver::Solve(
        double                     t0,
        const std::vector<double> &y0,
        double                     tEnd,
        Storage                   &storage,
        double                     tolerance)
{
    double t = t0;
    double h = stepSize;
    std::vector<double> y = y0;
    storage.Add(t, y);
    
    while (t < tEnd)
    {
        double h_current = std::min(h, tEnd - t);
        Step(t, y, h_current, tolerance);
        t += h_current;
        storage.Add(t, y);
        
        // Обновляем базовый шаг для следующей итерации
        h = h_current;
    }
}
