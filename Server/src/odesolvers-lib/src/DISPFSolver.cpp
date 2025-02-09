#include "DISPFSolver.hpp"
#include <stdexcept>
#include <cmath>
#include <algorithm>

// --- Конструктор ---
DISPFSolver::DISPFSolver(
    std::function<std::vector<double>(double, const std::vector<double>&)> func,
    double initialStep,
    double gamma,
    int I, int J, int K
) : Solver(func, initialStep), GAMMA(gamma)
{
    // Параметр I: порядок метода
    if (I == 0) {
        variableOrder = true;
    } else {
        variableOrder = false;
        fixedOrder = I;
    }
    // Если порядок фиксированный, устанавливаем currentMethod согласно значению fixedOrder:
    if (!variableOrder)
    {
        if (fixedOrder == 1)
            currentMethod = DISPFA;
        else if (fixedOrder == 2)
            currentMethod = DISPFB;
        else if (fixedOrder == 5)
            currentMethod = DISPFC;
        else
            currentMethod = DISPFA; // по умолчанию
    }
    else {
        // Если переменный порядок, по умолчанию начинаем с DISPFA
        currentMethod = DISPFA;
    }

    // Параметр J: контроль устойчивости
    // Если J==0, то контроль включён; если J==1 – контроль отключается.
    stabilityControlEnabled = (J == 0);
    if (stabilityControlEnabled)
    {
        jacobianMethod = K; // K принимает значения 0,1,2
    }
    // Изначально vnBreakCount и jumpToRadau5 уже обнулены.
}

// --- Основной метод шага ---
void DISPFSolver::Step(double t, std::vector<double>& y, double& h, double tolerance) {
    switch(currentMethod) {
        case DISPFA: StepDISPFA(t, y, h, tolerance); break;
        case DISPFB: StepDISPFB(t, y, h, tolerance); break;
        case DISPFC: StepDISPFC(t, y, h, tolerance); break;
    }
}

// --- Метод Solve ---
void DISPFSolver::Solve(
    double t0,
    const std::vector<double>& y0,
    double tEnd,
    Storage& storage,
    double tolerance
) {
    double t = t0;
    double h = stepSize;
    std::vector<double> y = y0;
    // Если порядок переменный, currentMethod уже установлен, иначе остаётся фиксированным.
    vnBreakCount = 0;
    jumpToRadau5 = false;

    // Инициализация векторов (число уравнений = y.size())
    k.resize(y.size(), std::vector<double>(6, 0.0));
    F.resize(y.size());
    Y0.resize(y.size());

    storage.Add(t, y);

    while (t < tEnd) {
        if (t + h > tEnd)
            h = tEnd - t;

        Step(t, y, h, tolerance);

        if (jumpToRadau5 && GAMMA != 0.0 && (tEnd - t) > h)
            break;

        t += h;
        storage.Add(t, y);
    }
}

// --- Функция оценки ошибки Vn ---
double DISPFSolver::CalcVn(const std::vector<double>& /*dummy*/, const std::vector<std::vector<double>>& k) {
    double Vn = 0.0;
    for (size_t i = 0; i < k.size(); ++i) {
        double n = k[i][1] - k[i][0];
        if (n == 0.0) n = 1e-15;
        double vn = std::fabs((32.0 * k[i][2] - 48.0 * k[i][1] + 16.0 * k[i][0]) / n);
        Vn = std::max(Vn, vn);
    }
    return Vn / 9.0;
}

// --- Вспомогательная функция StadScalar ---
double DISPFSolver::StadScalar(size_t eqNum, int stage) {
    double sum = 0.0;
    for (int j = 0; j < stage; ++j) {
        sum += B[stage-1][j] * k[eqNum][j];
    }
    return sum;
}

// --- Функция оценки Якобиана ---
// Вычисляем приближённую матрицу Якоби методом конечных разностей,
// затем в зависимости от параметра jacobianMethod оцениваем спектральный радиус.
double DISPFSolver::EstimateJacobianEigenvalue(double t, const std::vector<double>& y) {
    size_t n = y.size();
    std::vector<std::vector<double>> J(n, std::vector<double>(n, 0.0));
    double eps = 1e-8;
    std::vector<double> f0 = f(t, y);
    for (size_t j = 0; j < n; j++) {
        std::vector<double> y_pert = y;
        y_pert[j] += eps;
        std::vector<double> f_pert = f(t, y_pert);
        for (size_t i = 0; i < n; i++) {
            J[i][j] = (f_pert[i] - f0[i]) / eps;
        }
    }
    if (jacobianMethod == 2) {
        // Осреднение по формуле: для каждой строки берём max(|J[i][j]|), затем усредняем.
        double sum = 0.0;
        for (size_t i = 0; i < n; i++) {
            double row_max = 0.0;
            for (size_t j = 0; j < n; j++) {
                row_max = std::max(row_max, std::fabs(J[i][j]));
            }
            sum += row_max;
        }
        return sum / n;
    } else {
        // Для вариантов 0 и 1 используем степенной метод.
        std::vector<double> x(n, 1.0);
        double lambda = 0.0;
        for (int iter = 0; iter < 100; iter++) {
            std::vector<double> y_vec(n, 0.0);
            for (size_t i = 0; i < n; i++) {
                for (size_t j = 0; j < n; j++) {
                    y_vec[i] += J[i][j] * x[j];
                }
            }
            double norm = 0.0;
            for (double val : y_vec)
                norm = std::max(norm, std::fabs(val));
            if (norm == 0.0)
                break;
            for (size_t i = 0; i < n; i++)
                x[i] = y_vec[i] / norm;
            lambda = norm;
        }
        return lambda;
    }
}

// -------------------- Реализация метода DISPFC --------------------
bool DISPFSolver::StepDISPFC(double& t, std::vector<double>& y, double& h, double tolerance) {
    size_t n = y.size();
    // Вычисляем f(t,y) и заполняем k[][0]
    std::vector<double> ff = f(t, y);
    for (size_t i = 0; i < n; ++i)
        k[i][0] = ff[i] * h;

    while (true) { // аналог метки DISPFC_A1
        // Стадии 2..6 (m = 1..5)
        for (int m = 1; m < 6; ++m) {
            for (size_t i = 0; i < n; ++i)
                Y0[i] = y[i] + StadScalar(i, m);
            std::vector<double> fm = f(t + ACoef[m-1] * h, Y0);
            for (size_t i = 0; i < n; ++i)
                k[i][m] = fm[i] * h;
        }

        // 2a. Вычисляем Cn1
        double Cn1 = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double cn1 = 0.0;
            for (int j = 0; j < 6; ++j)
                cn1 += (P36[j] - P4[j]) * k[i][j];
            cn1 = std::fabs(cn1);
            Cn1 = std::max(Cn1, cn1);
        }
        Cn1 *= 17.0/24.0;

        // 3a. Оценка Sn
        double Sn = (Cn1 == 0.0) ? MAXDOUBLE : (Lnq * std::log(tolerance / Cn1)) / 5.0;

        // 4a. Если ошибка слишком велика, уменьшаем шаг и повторяем
        if (Cn1 > tolerance) {
            int Sn_int = static_cast<int>(Sn);
            double factor = std::pow(q, Sn_int);
            if (factor == 1.0)
                factor = 0.9;
            h *= factor;
            for (size_t i = 0; i < n; ++i)
                k[i][0] *= factor;
            continue;
        }

        Sn = static_cast<int>(Sn);

        // 5a. Вычисляем новое приближение решения по P36
        for (size_t i = 0; i < n; ++i) {
            double update = 0.0;
            for (int j = 0; j < 6; ++j)
                update += P36[j] * k[i][j];
            Y0[i] = y[i] + update;
            y[i] = Y0[i];
        }
        t += h;

        // Вычисляем An1
        ff = f(t, y);
        double An1 = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double diff = std::fabs(ff[i] * h - k[i][0]);
            An1 = std::max(An1, diff);
        }
        An1 *= CA1B * d_coef;

        double Vn = CalcVn(F, k);

        // Если включён контроль устойчивости, оцениваем по Якобиану:
        if (stabilityControlEnabled) {
            double lambda_est = EstimateJacobianEigenvalue(t, y);
            if (lambda_est != 0.0 && (h * lambda_est) / 72.0 < GAMMA) {
                vnBreakCount++;
                if (vnBreakCount >= 35)
                    jumpToRadau5 = true;
            } else {
                vnBreakCount = 0;
            }
        }

        double rn = (Vn != 0.0) ? Lnq * std::log(3.6 / Vn) : MAXDOUBLE;
        double min_v = std::min(Sn, rn);
        // Корректировка шага – умножаем на коэффициент (вместо max, чтобы действительно масштабировать)
        if (min_v < MAX_P && Vn > 0)
            h *= std::pow(q, min_v);
        else
            h *= 1.1;

        // Если порядок переменный, можно переключаться; иначе оставляем метод фиксированным.
        if (variableOrder && Vn > 3.6 && An1 <= tolerance)
            currentMethod = DISPFB;

        return true;
    }
    return true;
}

// -------------------- Реализация метода DISPFA --------------------
bool DISPFSolver::StepDISPFA(double& t, std::vector<double>& y, double& h, double tolerance) {
    size_t n = y.size();
    double An, An1, Sn, Vn;
    while (true) {  // аналог метки DISPFA_A1
        // 1a. Вычисляем k2
        for (size_t i = 0; i < n; ++i)
            Y0[i] = y[i] + StadScalar(i, 1);
        std::vector<double> f_stage = f(t + 0.25 * h, Y0);
        for (size_t i = 0; i < n; ++i)
            k[i][1] = f_stage[i] * h;
        // 2a. Вычисляем An = max|k[i][1]-k[i][0]| и умножаем на CA1A
        An = 0.0;
        for (size_t i = 0; i < n; ++i)
            An = std::max(An, std::fabs(k[i][1] - k[i][0]));
        An *= CA1A;
        // 3a. Оценка Sn
        Sn = (An != 0.0) ? 0.5 * Lnq * std::log(tolerance / An) : MAXDOUBLE;
        // 4a. Если An слишком велико, уменьшаем шаг и повторяем
        if (An > tolerance) {
            double factor = std::pow(q, 2.0 * Sn);
            h *= factor;
            for (size_t i = 0; i < n; ++i)
                k[i][0] *= factor;
            continue;
        }
        break;
    }

    // 5a. Стадии k3..k6
    for (int m = 2; m < 6; ++m) {
        for (size_t i = 0; i < n; ++i)
            Y0[i] = y[i] + StadScalar(i, m);
        std::vector<double> f_stage = f(t + ACoef[m-1] * h, Y0);
        for (size_t i = 0; i < n; ++i)
            k[i][m] = f_stage[i] * h;
    }

    // 6a. Составляем приближение по P72
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < 6; ++j)
            sum += P72[j] * k[i][j];
        Y0[i] = y[i] + sum;
    }
    // 8a. Вычисляем An1
    std::vector<double> f_end = f(t + h, Y0);
    An1 = 0.0;
    for (size_t i = 0; i < n; ++i)
        An1 = std::max(An1, std::fabs(f_end[i] * h - k[i][0]));
    An1 *= CA1A;

    // 9a. Оценка Vn
    Vn = (An1 != 0.0) ? 0.5 * Lnq * std::log(tolerance / An1) : MAXDOUBLE;
    // 10a. Если ошибка An1 слишком велика, уменьшаем шаг и повторяем
    if (An1 > tolerance) {
        double factor = std::pow(q, 2.0 * Vn);
        h *= factor;
        for (size_t i = 0; i < n; ++i)
            k[i][0] *= factor;
        return StepDISPFA(t, y, h, tolerance);
    }
    // 7a. Принимаем шаг: обновляем время и состояние
    t += h;
    y = Y0;
    double Vn1 = CalcVn(F, k);
    double rn = (Vn1 != 0.0) ? Lnq * std::log(72.0 / Vn1) : MAXDOUBLE;
    double min_v = std::min(Vn, rn);
    if (min_v < MAX_P)
        h *= std::pow(q, min_v);
    else
        h *= 2.0;
    for (size_t i = 0; i < n; ++i)
        k[i][0] = f_end[i] * h;

    // Если контроль устойчивости включён, используем оценку Якобиана
    if (stabilityControlEnabled) {
        double lambda_est = EstimateJacobianEigenvalue(t, y);
        if (lambda_est != 0.0 && (h * lambda_est) / 72.0 < GAMMA) {
            vnBreakCount++;
            if (vnBreakCount > 40)
                jumpToRadau5 = true;
        } else {
            vnBreakCount = 0;
        }
    }

    if (variableOrder && std::pow(q, std::min(Sn, Vn)) * Vn1 < 28.5)
        currentMethod = DISPFB;
    return true;
}

// -------------------- Реализация метода DISPFB --------------------
bool DISPFSolver::StepDISPFB(double& t, std::vector<double>& y, double& h, double tolerance) {
    size_t n = y.size();
    double An, An1, Sn, Vn, Cn1;
    while (true) {
        // 1a. Вычисляем k2
        for (size_t i = 0; i < n; ++i)
            Y0[i] = y[i] + (1.0/4.0) * k[i][0];
        std::vector<double> f_stage = f(t + 0.25 * h, Y0);
        for (size_t i = 0; i < n; ++i)
            k[i][1] = f_stage[i] * h;
        An = 0.0;
        for (size_t i = 0; i < n; ++i)
            An = std::max(An, std::fabs(k[i][1] - k[i][0]));
        An *= CA1B * d_coef;
        Sn = (An == 0.0) ? MAXDOUBLE : 0.5 * Lnq * std::log(tolerance / An);
        if (An > tolerance) {
            double factor = std::pow(q, 2.0 * Sn);
            h *= factor;
            for (size_t i = 0; i < n; ++i)
                k[i][0] *= factor;
            continue;
        }
        break;
    }
    // 5a. Стадии k3..k6
    for (int m = 2; m < 6; ++m) {
        for (size_t i = 0; i < n; ++i)
            Y0[i] = y[i] + StadScalar(i, m);
        std::vector<double> f_stage = f(t + ACoef[m-1] * h, Y0);
        for (size_t i = 0; i < n; ++i)
            k[i][m] = f_stage[i] * h;
    }
    // 6a. Составляем приближение по P28
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < 6; ++j)
            sum += P28[j] * k[i][j];
        Y0[i] = y[i] + sum;
    }
    std::vector<double> f_end = f(t + h, Y0);
    An1 = 0.0;
    for (size_t i = 0; i < n; ++i)
        An1 = std::max(An1, std::fabs(f_end[i] * h - k[i][0]));
    An1 *= CA1B * d_coef;
    Vn = (An1 == 0.0) ? MAXDOUBLE : 0.5 * Lnq * std::log(tolerance / An1);
    if (An1 > tolerance) {
        double factor = std::pow(q, 2.0 * Vn);
        h *= factor;
        for (size_t i = 0; i < n; ++i)
            k[i][0] *= factor;
        return StepDISPFB(t, y, h, tolerance);
    }
    // 7a. Принимаем шаг
    t += h;
    y = Y0;
    Cn1 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double cn1 = 0.0;
        for (int j = 0; j < 6; ++j)
            cn1 += (P36[j] - P4[j]) * k[i][j];
        Cn1 = std::max(Cn1, std::fabs(cn1));
    }
    Cn1 *= 17.0/24.0;
    double Vn1 = CalcVn(F, k);
    double rn = (Vn1 != 0.0) ? Lnq * std::log(28.5 / Vn1) : MAXDOUBLE;
    double min_v = std::min(Vn, rn);
    if (min_v < MAX_P)
        h *= std::pow(q, min_v);
    else
        h *= 2.0;
    for (size_t i = 0; i < n; ++i)
        k[i][0] = f_end[i] * h;
    if (stabilityControlEnabled) {
        double lambda_est = EstimateJacobianEigenvalue(t, y);
        if (lambda_est != 0.0 && (h * lambda_est) / 28.5 < GAMMA) {
            vnBreakCount++;
            if (vnBreakCount > 40)
                jumpToRadau5 = true;
        } else {
            vnBreakCount = 0;
        }
    }
    if (variableOrder) {
        double qn1 = std::pow(q, Vn);
        double qn2 = std::pow(q, std::min(Vn, rn));
        if (qn1 * Vn1 > 28.5 && std::pow(qn2, 2.0) * An1 <= tolerance)
            currentMethod = DISPFA;
        else if (qn1 <= 3.6 && std::pow(qn2, 6.0) * Cn1 <= tolerance)
            currentMethod = DISPFC;
    }
    if (stabilityControlEnabled) {
        double lambda_est = EstimateJacobianEigenvalue(t, y);
        if (lambda_est != 0.0 && (h * lambda_est) / 28.5 < GAMMA) {
            vnBreakCount++;
            if (vnBreakCount > 40)
                jumpToRadau5 = true;
        } else {
            vnBreakCount = 0;
        }
    }
    return true;
}
