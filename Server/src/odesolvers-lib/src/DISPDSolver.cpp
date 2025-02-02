#include "../include/DISPDSolver.hpp"

void DISPDSolver::Step(
    double               t,
    std::vector<double> &y,
    double              &h,
    double               tolerance) 
{
    std::vector<double> yTemp = y;
    std::vector<double> k1, k2, k3;
    bool stepAccepted = false;
    int attempts = 0;

    while (!stepAccepted && attempts < 10) 
    {
        computeKTerms(t, yTemp, h, k1, k2, k3);
        double Aprime = computeAprime(k1, k2);
        double sn = log(tolerance / (pow(q, 2*attempts) * Aprime)) / (2 * log(q));

        if (sn < 0) 
        {
            h *= pow(q, sn);
            attempts++;
            continue;
        }

        // Вычисление y_{n+1} и A''
        std::vector<double> yNext = AddVectors(
            yTemp, 
            AddVectors(
                MultiplyScalarByVector(currentScheme.p1, k1),
                AddVectors(
                    MultiplyScalarByVector(currentScheme.p2, k2),
                    MultiplyScalarByVector(currentScheme.p3, k3)
                )
            )
        );
        std::vector<double> kNext = MultiplyScalarByVector(h, f(t + h, yNext));
        double AddPrime = computeAddoublePrime(h, k1, kNext);
        double vn = log(tolerance / (pow(q, 2*attempts) * AddPrime)) / (2 * log(q));

        if (vn < 0) 
        {
            h *= pow(q, vn);
            attempts++;
            continue;
        }

        // Проверка устойчивости через V_n
        double V = computeV(k1, k2, k3);
        double rn = log(18.0 / (pow(q, attempts) * V)) / log(q);

        // Адаптация шага
        double scale = SAFETY * std::min(
            std::min(pow(q, sn), pow(q, vn)),
            std::min(pow(q, rn), MAX_SCALE)
        );
        scale = std::clamp(scale, MIN_SCALE, MAX_SCALE);

        if (V > 18.0 / pow(q, attempts) || Aprime > tolerance / pow(q, 2*attempts)) 
        {
            h *= scale;
            attempts++;
        } 
        else 
        {
            y = yNext;
            h *= scale;
            stepAccepted = true;
        }
    }

    if (!stepAccepted) 
    {
        throw std::runtime_error("Step adaptation failed");
    }

    // Проверка переключения на схему Б
    if (shouldSwitchToSchemeB(h, tolerance)) 
    {
        switchScheme(true);
    }
}

void DISPDSolver::computeKTerms(
    double                     t,
    std::vector<double> const &y,
    double                     h,
    std::vector<double>       &k1,
    std::vector<double>       &k2,
    std::vector<double>       &k3) 
{
    k1 = MultiplyScalarByVector(h, f(t, y));
    
    std::vector<double> y2 = AddVectors(y, MultiplyScalarByVector(2.0/3.0, k1));
    k2 = MultiplyScalarByVector(h, f(t, y2));
    
    std::vector<double> y3 = AddVectors(
        AddVectors(y, MultiplyScalarByVector(1.0/3.0, k1)),
        MultiplyScalarByVector(1.0/3.0, k2)
    );
    k3 = MultiplyScalarByVector(h, f(t, y3));
}

double DISPDSolver::computeAprime(
    std::vector<double> const &k1,
    std::vector<double> const &k2) 
{
    std::vector<double> diff = SubtractVectors(k2, k1);
    return (std::abs(1 - 6 * currentScheme.g) / 4.0) * Norm(diff);
}

double DISPDSolver::computeAddoublePrime(
    double                     h,
    std::vector<double> const &k1,
    std::vector<double> const &yNext) 
{
    std::vector<double> diff = SubtractVectors(MultiplyScalarByVector(h, yNext), k1);
    return (std::abs(1 - 6 * currentScheme.g) / 6.0) * Norm(diff);
}

double DISPDSolver::computeV(
    std::vector<double> const &k1,
    std::vector<double> const &k2,
    std::vector<double> const &k3) 
{
    double maxRatio = 0.0;
    for (size_t i = 0; i < k1.size(); ++i) 
    {
        double denominator = k2[i] - k1[i];
        denominator = (std::abs(denominator) < 1e-12) ? 1e-12 : denominator;
        double ratio = std::abs((k3[i] - k2[i]) / denominator);
        maxRatio = std::max(maxRatio, ratio);
    }

    return 3.0 * maxRatio;
}

bool DISPDSolver::shouldSwitchToSchemeB(
    double h,
    double tolerance) 
{
    // Условие переключения: прогнозируемый шаг для схемы Б больше
    return (h * MAX_SCALE > currentScheme.g * tolerance);
}

void DISPDSolver::switchScheme(bool toSchemeB) 
{
    if (toSchemeB) 
    {
        currentScheme = {7.0/9.0, 16.0/81.0, 2.0/81.0, 0.0};  // Параметры схемы Б
    } 
    else 
    {
        currentScheme = {1.0/4.0, (3 - 18*currentScheme.g)/4, 9.0/2.0 * currentScheme.g, 0.15}; 
    }
}

void DISPDSolver::Solve(
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
    switchScheme(false);  // Начинаем с алгоритма А

    while (t < tEnd) 
    {
        double hAttempt = std::min(h, tEnd - t);
        if (hAttempt < 1e-12) break;

        try 
        {
            Step(t, y, hAttempt, tolerance);
            t += hAttempt;
            storage.Add(t, y);
            h = hAttempt;
        } 
        catch (const std::runtime_error&) 
        {
            h *= 0.5;
        }
    }
}
