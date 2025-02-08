#include "DispFSolver.hpp"
#include <algorithm>
#include <stdexcept>
#include <cmath>

// Конструктор
DispFSolver::DispFSolver(
    std::function<std::vector<double>(double, const std::vector<double>&)> func,
    double initialStep,
    double gamma
) : Solver(func, initialStep), GAMMA(gamma)
{
    // Здесь можно инициализировать дополнительные параметры, если нужно.
}

// Основной метод шага – переключается по выбранному алгоритму
void DispFSolver::Step(double t, std::vector<double>& y, double& h, double tolerance) {
    switch(currentMethod) {
        case DISPFA: StepDISPFA(t, y, h, tolerance); break;
        case DISPFB: StepDISPFB(t, y, h, tolerance); break;
        case DISPFC: StepDISPFC(t, y, h, tolerance); break;
    }
}

// Метод Solve аналогичен оригинальному – он вызывает Step и сохраняет результаты.
void DispFSolver::Solve(
    double t0,
    const std::vector<double>& y0,
    double tEnd,
    Storage& storage,
    double tolerance
) {
    double t = t0;
    double h = stepSize;
    std::vector<double> y = y0;
    currentMethod = DISPFA;
    vnBreakCount = 0;
    jumpToRadau5 = false;

    // Инициализация внутренних векторов (число уравнений = y.size())
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

// Функция оценки ошибки Vn – переносим из старого кода (первый параметр не используется)
double DispFSolver::CalcVn(const std::vector<double>& /*dummy*/, const std::vector<std::vector<double>>& k) {
    double Vn = 0.0;
    for (size_t i = 0; i < k.size(); ++i) {
        double n = k[i][1] - k[i][0];
        if (n == 0.0) n = 1e-15;
        double vn = std::fabs((32.0 * k[i][2] - 48.0 * k[i][1] + 16.0 * k[i][0]) / n);
        Vn = std::max(Vn, vn);
    }
    return Vn / 9.0;
}

// Вспомогательная функция для вычисления суммы стадий для одного уравнения
double DispFSolver::StadScalar(size_t eqNum, int stage) {
    double sum = 0.0;
    for (int j = 0; j < stage; ++j) {
        sum += B[stage-1][j] * k[eqNum][j];
    }
    return sum;
}

// -------------------- Реализация метода DISPFC --------------------
bool DispFSolver::StepDISPFC(double& t, std::vector<double>& y, double& h, double tolerance) {
    size_t n = y.size();
    // Вычисляем f(t,y) и заполняем k[][0]
    std::vector<double> ff = f(t, y);
    for (size_t i = 0; i < n; ++i) {
        k[i][0] = ff[i] * h;
    }

    while (true) { // аналог goto DISPFC_A1
        // Стадии 2..6 (m = 1..5)
        for (int m = 1; m < 6; ++m) {
            for (size_t i = 0; i < n; ++i) {
                Y0[i] = y[i] + StadScalar(i, m);
            }
            std::vector<double> fm = f(t + ACoef[m-1] * h, Y0);
            for (size_t i = 0; i < n; ++i) {
                k[i][m] = fm[i] * h;
            }
        }

        // 2a. Вычисляем Cn1
        double Cn1 = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double cn1 = 0.0;
            for (int j = 0; j < 6; ++j) {
                cn1 += (P36[j] - P4[j]) * k[i][j];
            }
            cn1 = std::fabs(cn1);
            if (cn1 > Cn1)
                Cn1 = cn1;
        }
        Cn1 *= 17.0/24.0;

        // 3a. Оценка величины Sn
        double Sn;
        if (Cn1 == 0.0)
            Sn = MAXDOUBLE;
        else
            Sn = (Lnq * std::log(tolerance / Cn1)) / 5.0;

        // 4a. Если ошибка слишком велика – уменьшаем шаг
        if (Cn1 > tolerance) {
            int Sn_int = static_cast<int>(Sn);
            double factor = std::pow(q, Sn_int);
            if (factor == 1.0)
                factor = 0.9;
            h *= factor;
            for (size_t i = 0; i < n; ++i) {
                k[i][0] *= factor;
            }
            // Пересчитываем с новым шагом
            continue;
        }

        // Округляем Sn
        Sn = static_cast<int>(Sn);

        // 5a. Вычисляем новое приближение решения с помощью коэффициентов P36
        for (size_t i = 0; i < n; ++i) {
            double update = 0.0;
            for (int j = 0; j < 6; ++j) {
                update += P36[j] * k[i][j];
            }
            Y0[i] = y[i] + update;
            y[i] = Y0[i];
        }
        // 6a. Обновляем время
        t += h;

        // Вычисляем An1
        ff = f(t, y);
        double An1 = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double diff = std::fabs(ff[i]*h - k[i][0]);
            if (diff > An1)
                An1 = diff;
        }
        An1 *= CA1B * d_coef;

        // Оцениваем Vn
        double Vn = CalcVn(F, k);

        // 11a. Вычисляем rn и сравниваем с гамма
        double rn = MAXDOUBLE;
        if (Vn != 0.0)
            rn = Lnq * std::log(3.6 / Vn);

        if (Vn != 0.0 && (Vn * h)/3.6 < GAMMA) {
            vnBreakCount++;
            if (vnBreakCount >= 35)
                jumpToRadau5 = true;
        } else {
            vnBreakCount = 0;
        }

        double min_v = (Sn < rn) ? Sn : rn;
        if (min_v < MAX_P && Vn > 0)
            h = std::max(h, std::pow(q, min_v) * h);
        else
            h *= 1.1;

        // При выполнении условия ошибки можем переключиться на метод DISPFB
        if (Vn > 3.6 && An1 <= tolerance)
            currentMethod = DISPFB;

        return true;
    }
    return true;
}

// -------------------- Реализация метода DISPFA --------------------
bool DispFSolver::StepDISPFA(double& t, std::vector<double>& y, double& h, double tolerance) {
    size_t n = y.size();
    double An, An1, Sn, Vn;
    // Переопределяем цикл (аналог метки DISPFA_A1)
    while (true) {
        // 1a. Стадия k2: вычисляем y + StadScalar(i,1)
        for (size_t i = 0; i < n; ++i) {
            Y0[i] = y[i] + StadScalar(i, 1);
        }
        std::vector<double> f_stage = f(t + 0.25 * h, Y0);
        for (size_t i = 0; i < n; ++i) {
            k[i][1] = f_stage[i] * h;
        }

        // 2a. Вычисляем An = max|k[i][1]-k[i][0]| и умножаем на CA1A
        An = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double diff = std::fabs(k[i][1] - k[i][0]);
            if (diff > An)
                An = diff;
        }
        An *= CA1A;

        // 3a. Оценка Sn
        if (An != 0.0)
            Sn = 0.5 * Lnq * std::log(tolerance / An);
        else
            Sn = MAXDOUBLE;

        // 4a. Если An слишком велико – уменьшаем шаг и повторяем
        if (An > tolerance) {
            double factor = std::pow(q, 2.0 * Sn);
            h *= factor;
            for (size_t i = 0; i < n; ++i)
                k[i][0] *= factor;
            continue;
        }
        break;
    }

    // 5a. Стадии k3..k6 (m = 2..5)
    for (int m = 2; m < 6; ++m) {
        for (size_t i = 0; i < n; ++i) {
            Y0[i] = y[i] + StadScalar(i, m);
        }
        std::vector<double> f_stage = f(t + ACoef[m-1] * h, Y0);
        for (size_t i = 0; i < n; ++i) {
            k[i][m] = f_stage[i] * h;
        }
    }

    // 6a. Составляем приближение решения с помощью коэффициентов P72
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < 6; ++j) {
            sum += P72[j] * k[i][j];
        }
        Y0[i] = y[i] + sum;
    }

    // 8a. Вычисляем An1 через разность f(t+h, Y0) и k[][0]
    std::vector<double> f_end = f(t + h, Y0);
    An1 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double diff = std::fabs(f_end[i] * h - k[i][0]);
        if (diff > An1)
            An1 = diff;
    }
    An1 *= CA1A;

    // 9a. Оценка Vn
    if (An1 != 0.0)
        Vn = 0.5 * Lnq * std::log(tolerance / An1);
    else
        Vn = MAXDOUBLE;

    // 10a. Если ошибка An1 превышает допуск – уменьшаем шаг и повторяем шаг
    if (An1 > tolerance) {
        double factor = std::pow(q, 2.0 * Vn);
        h *= factor;
        for (size_t i = 0; i < n; ++i)
            k[i][0] *= factor;
        return StepDISPFA(t, y, h, tolerance);
    }

    // 7a. Принимаем шаг: обновляем время и состояние
    t += h;
    y = Y0; // новое значение

    double Vn1 = CalcVn(F, k);
    double rn = MAXDOUBLE;
    if (Vn1 != 0.0)
        rn = Lnq * std::log(72.0 / Vn1);
    double min_v = std::min(Vn, rn);
    if (min_v < MAX_P)
        h = std::max(h, std::pow(q, min_v) * h);
    else
        h *= 2.0;

    // Обновляем k[][0] для следующего шага
    for (size_t i = 0; i < n; ++i) {
        k[i][0] = f_end[i] * h;
    }

    if (Sn != MAXDOUBLE && Vn != MAXDOUBLE) {
        if (std::pow(q, std::min(Sn, Vn)) * Vn1 < 28.5)
            currentMethod = DISPFB;
    }

    if (Vn1 != 0.0 && (h * Vn1) / 72.0 < GAMMA) {
        vnBreakCount++;
        if (vnBreakCount > 40)
            jumpToRadau5 = true;
    } else {
        vnBreakCount = 0;
    }

    return true;
}

// -------------------- Реализация метода DISPFB --------------------
bool DispFSolver::StepDISPFB(double& t, std::vector<double>& y, double& h, double tolerance) {
    size_t n = y.size();
    double An, An1, Sn, Vn, Cn1;
    while (true) {
        // 1a. Вычисляем k2 по формуле y + (1/4)*k[0]
        for (size_t i = 0; i < n; ++i) {
            Y0[i] = y[i] + (1.0/4.0) * k[i][0];
        }
        std::vector<double> f_stage = f(t + 0.25 * h, Y0);
        for (size_t i = 0; i < n; ++i) {
            k[i][1] = f_stage[i] * h;
        }
        An = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double diff = std::fabs(k[i][1] - k[i][0]);
            if (diff > An)
                An = diff;
        }
        An *= CA1B * d_coef;
        if (An == 0.0)
            Sn = MAXDOUBLE;
        else
            Sn = 0.5 * Lnq * std::log(tolerance / An);
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
        for (size_t i = 0; i < n; ++i) {
            Y0[i] = y[i] + StadScalar(i, m);
        }
        std::vector<double> f_stage = f(t + ACoef[m-1] * h, Y0);
        for (size_t i = 0; i < n; ++i) {
            k[i][m] = f_stage[i] * h;
        }
    }

    // 6a. Составляем приближение решения с помощью коэффициентов P28
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < 6; ++j) {
            sum += P28[j] * k[i][j];
        }
        Y0[i] = y[i] + sum;
    }
    std::vector<double> f_end = f(t + h, Y0);

    // 8a. Вычисляем An1
    An1 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double diff = std::fabs(f_end[i] * h - k[i][0]);
        if (diff > An1)
            An1 = diff;
    }
    An1 *= CA1B * d_coef;

    if (An1 == 0.0)
        Vn = MAXDOUBLE;
    else
        Vn = 0.5 * Lnq * std::log(tolerance / An1);

    if (An1 > tolerance) {
        double factor = std::pow(q, 2.0 * Vn);
        h *= factor;
        for (size_t i = 0; i < n; ++i)
            k[i][0] *= factor;
        return StepDISPFB(t, y, h, tolerance);
    }

    // 7a. Принимаем шаг: обновляем время и состояние
    t += h;
    y = Y0;
    Cn1 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double cn1 = 0.0;
        for (int j = 0; j < 6; ++j) {
            cn1 += (P36[j] - P4[j]) * k[i][j];
        }
        cn1 = std::fabs(cn1);
        if (cn1 > Cn1)
            Cn1 = cn1;
    }
    Cn1 *= 17.0/24.0;
    double Vn1 = CalcVn(F, k);
    double rn = MAXDOUBLE;
    if (Vn1 != 0.0)
        rn = Lnq * std::log(28.5 / Vn1);
    double min_v = std::min(Vn, rn);
    if (min_v < MAX_P)
        h = std::max(h, std::pow(q, min_v) * h);
    else
        h *= 2.0;
    for (size_t i = 0; i < n; ++i) {
        k[i][0] = f_end[i] * h;
    }
    if (Vn != MAXDOUBLE && rn != MAXDOUBLE) {
        double qn1 = std::pow(q, Vn);
        double qn2 = std::pow(q, std::min(Vn, rn));
        if (qn1 * Vn1 > 28.5 && std::pow(qn2, 2.0) * An1 <= tolerance)
            currentMethod = DISPFA;
        else if (qn1 <= 3.6 && std::pow(qn2, 6.0) * Cn1 <= tolerance)
            currentMethod = DISPFC;
    }
    if (Vn1 != 0.0 && (h * Vn1)/28.5 < GAMMA) {
        vnBreakCount++;
        if (vnBreakCount > 40)
            jumpToRadau5 = true;
    } else {
        vnBreakCount = 0;
    }
    return true;
}
