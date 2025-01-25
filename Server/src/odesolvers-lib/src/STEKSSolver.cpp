#include "../include/STEKSSolver.hpp"
#include <algorithm>
#include <stdexcept>

void STEKSSolver::Step(double t, std::vector<double>& y, double& h, double tolerance) {
    std::vector<double> y_temp = y;
    
    // Вычисление стадий с корректными временными метками
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
            AddVectors(y_temp, MultiplyScalarByVector(1.0/2.0, k1)),
            MultiplyScalarByVector(-3.0/2.0, k3)
        ),
        MultiplyScalarByVector(2.0, k4)
    );
    std::vector<double> k5 = MultiplyScalarByVector(h, f(t + h, y5));
    
    // Оценка ошибки
    double error = computeError(k1, k2, k3, k4, k5, tolerance);
    
    // Адаптация шага для метода 5-го порядка
    const double order = 5.0;
    if (error > 1.0) {
        double scale = std::max(SAFETY * pow(1.0 / error, 1.0/order), MIN_SCALE);
        h *= scale;
        throw std::runtime_error("Step rejected");
    }
    
    // Применение шага
    y_temp = AddVectors(
        y_temp,
        AddVectors(
            MultiplyScalarByVector(1.0/6.0, k1),
            AddVectors(
                MultiplyScalarByVector(2.0/3.0, k4),
                MultiplyScalarByVector(1.0/6.0, k5)
            )
        )
    );
    
    // Корректировка шага
    double scale = std::min(SAFETY * pow(1.0 / error, 1.0/order), MAX_SCALE);
    h *= scale;
    y = y_temp;
}

double STEKSSolver::computeError(
    const std::vector<double>& k1,
    const std::vector<double>& k2,
    const std::vector<double>& k3,
    const std::vector<double>& k4,
    const std::vector<double>& k5,
    double tolerance
) {
    // Условие точности (4.22): (1/30)||2k₁ -9k₃ +8k₄ -k₅|| ≤ 5 * tolerance
    std::vector<double> err_vec = AddVectors(
        AddVectors(
            MultiplyScalarByVector(2.0, k1),
            MultiplyScalarByVector(-9.0, k3)
        ),
        AddVectors(
            MultiplyScalarByVector(8.0, k4),
            MultiplyScalarByVector(-1.0, k5)
        )
    );
    double err_precision = Norm(err_vec) / (30.0 * 5.0 * tolerance);
    
    // Условие устойчивости (4.23): 6 * max|(k₃ -k₂)/(k₂ -k₁)| ≤ 3.5
    double stability = 0.0;
    for (size_t i = 0; i < k1.size(); ++i) {
        double delta_k2k1 = k2[i] - k1[i];
        if (fabs(delta_k2k1) < 1e-12) delta_k2k1 = 1e-12;
        double ratio = (k3[i] - k2[i]) / delta_k2k1;
        stability = std::max(stability, fabs(ratio));
    }
    double stability_error = (6.0 * stability > 3.5) ? 1.0 : 0.0;
    
    return std::max(err_precision, stability_error);
}

void STEKSSolver::Solve(double t0, const std::vector<double>& y0, double tEnd, Storage& storage, double tolerance) {
    double t = t0;
    std::vector<double> y = y0;
    double h = stepSize;
    storage.Add(t, y);

    while (t < tEnd) {
        double h_attempt = std::min(h, tEnd - t);
        if (h_attempt < 1e-12) {
            throw std::runtime_error("Minimum step size reached");
        }

        try {
            std::vector<double> y_new = y;
            Step(t, y_new, h_attempt, tolerance);
            t += h_attempt;
            y = y_new;
            storage.Add(t, y);
            h = h_attempt;
        } 
        catch (const std::runtime_error&) {
            h = h_attempt * 0.5;
            if (h < 1e-12) {
                throw std::runtime_error("Step size underflow");
            }
        }
    }
}
