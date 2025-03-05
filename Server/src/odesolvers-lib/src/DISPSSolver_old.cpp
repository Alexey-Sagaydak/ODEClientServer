#include "../include/DISPSSolver_old.hpp"

static std::vector<double> Add(
    std::vector<double> const &a,
    std::vector<double> const &b)
{
    if (a.size() != b.size())
        throw std::runtime_error("Размеры векторов не совпадают (Add)");

    std::vector<double> r(a.size());
    for (size_t i = 0; i < a.size(); i++)
        r[i] = a[i] + b[i];
    return r;
}

static std::vector<double> Mul(
    double alpha,
    std::vector<double> const &v)
{
    std::vector<double> r(v.size());
    for (size_t i = 0; i < v.size(); i++)
        r[i] = alpha * v[i];
    return r;
}

DISPSSolver_old::DISPSSolver_old(
    std::function<std::vector<double>(double, std::vector<double> const &)> odeFunc,
    double initialStep,
    DispsEnabledFlags const &flags)
    : f(odeFunc),
      stepSize(initialStep),
      currentMethodIndex(0)
{
    if (flags.Disps13)
    {
        methods.push_back({
            3,
            { 0.76561395265924, 0.2066991671971, 0.027686880143657 }
        });
    }

    if (flags.Disps23)
    {
        methods.push_back({
            3,
            { 0.25, 0.461290700433682, 0.28870929566318 }
        });
    }

    if (flags.Disps15)
    {
        methods.push_back({
            5,
            { 0.49900087977418, 0.33439107198377, 0.15517028825608,
                0.011387362618405, 0.50397367568471e-4 }
        });
    }

    if (flags.Disps25)
    {
        methods.push_back({
            5,
            { 0.25, 0.39269911064722, 0.2811818347201,
              0.06530730655278, 0.010811748097894 }
        });
    }

    if (flags.Disps35) {
        methods.push_back({5,
            {1.0/6.0, -2.644193175547, 0.15929230363104,
                3.1515675385826, 1.0/6.0}
        });
    }
    
    if (flags.Disps36) {
        methods.push_back({6,
            {1.0/6.0, -6.0414736520435, 0.12355552068869, 
                0.18196838110013, 6.4026164169214, 1.0/6.0}
        });
    }

    if (methods.empty())
        throw std::runtime_error("Нет включённых вариантов DISPS");
}

std::vector<std::vector<double>> DISPSSolver_old::Solve(double                     t0,
                                                    std::vector<double> const &y0,
                                                    double                     tEnd,
                                                    double                     /*tolerance*/)
{
    std::vector<std::vector<double>> results;
    results.reserve(10000);

    double t = t0;
    std::vector<double> y = y0;

    // Записываем начальную точку
    {
        std::vector<double> row;
        row.push_back(t);
        row.insert(row.end(), y.begin(), y.end());
        results.push_back(row);
    }

    while (t < tEnd)
    {
        double hAttempt = std::min(stepSize, tEnd - t);
        if (hAttempt < 1e-14)
            break;

        // Всегда берём первый вариант (currentMethodIndex=0)
        DispsVariant_old const &variant = methods[currentMethodIndex];

        // Делаем шаг
        DoStep(t, y, hAttempt, variant);

        t += hAttempt;
        std::vector<double> row;
        row.push_back(t);
        row.insert(row.end(), y.begin(), y.end());
        results.push_back(row);
    }

    return results;
}

// Выполнение одного шага
void DISPSSolver_old::DoStep(
    double               t,
    std::vector<double> &y,
    double              &h,
    DispsVariant_old const  &variant)
{
    if (variant.stages == 3)
    {
        y = Step3Stage(t, y, h, f, variant.p);
    }
    else if (variant.stages == 5)
    {
        y = Step5Stage(t, y, h, f, variant.p);
    }
    else if (variant.stages == 6) {
        y = Step6Stage(t, y, h, f, variant.p);
    }
    else
    {
        throw std::runtime_error("Пока поддерживаются только 3- или 5-стадийные варианты.");
    }
}

// Шаг трёхстадийного метода
std::vector<double> DISPSSolver_old::Step3Stage(
    double t,
    std::vector<double> const &y,
    double h,
    std::function<std::vector<double>(double, std::vector<double> const &)> f,
    std::vector<double> const &p)
{
    // k1
    std::vector<double> k1 = Mul(h, f(t, y));

    // k2
    std::vector<double> y2 = Add(y, Mul(2.0/3.0, k1));
    std::vector<double> k2 = Mul(h, f(t + 2.0/3.0*h, y2));

    // k3
    std::vector<double> y3 = y;
    y3 = Add(y3, Mul(1.0/3.0, k1));
    y3 = Add(y3, Mul(1.0/3.0, k2));
    std::vector<double> k3 = Mul(h, f(t + 1.0*h, y3));

    // Итог
    std::vector<double> yNext = y;
    yNext = Add(yNext, Mul(p[0], k1));
    yNext = Add(yNext, Mul(p[1], k2));
    yNext = Add(yNext, Mul(p[2], k3));

    return yNext;
}

// Шаг пятистадийного метода
std::vector<double> DISPSSolver_old::Step5Stage(
    double t,
    std::vector<double> const &y,
    double h,
    std::function<std::vector<double>(double, std::vector<double> const &)> f,
    std::vector<double> const &p)
{
    // k1
    std::vector<double> k1 = Mul(h, f(t, y));

    // k2
    std::vector<double> y2 = Add(y, Mul(1.0/3.0, k1));
    std::vector<double> k2 = Mul(h, f(t + 1.0/3.0*h, y2));

    // k3
    std::vector<double> y3 = y;
    y3 = Add(y3, Mul(1.0/6.0, k1));
    y3 = Add(y3, Mul(1.0/6.0, k2));
    std::vector<double> k3 = Mul(h, f(t + 1.0/2.0*h, y3));

    // k4
    std::vector<double> y4 = y;
    y4 = Add(y4, Mul(1.0/8.0, k1));
    y4 = Add(y4, Mul(1.0/8.0, k3));
    std::vector<double> k4 = Mul(h, f(t + 3.0/4.0*h, y4));

    // k5
    std::vector<double> y5 = y;
    y5 = Add(y5, Mul(1.0/2.0, k1));
    y5 = Add(y5, Mul(-3.0/2.0, k3));
    y5 = Add(y5, Mul(2.0, k4));
    std::vector<double> k5 = Mul(h, f(t + 1.0*h, y5));

    // Итог
    std::vector<double> yNext = y;
    yNext = Add(yNext, Mul(p[0], k1));
    yNext = Add(yNext, Mul(p[1], k2));
    yNext = Add(yNext, Mul(p[2], k3));
    yNext = Add(yNext, Mul(p[3], k4));
    yNext = Add(yNext, Mul(p[4], k5));

    return yNext;
}

// Реализация Step6Stage для шестистадийного метода
std::vector<double> DISPSSolver_old::Step6Stage(
    double t,
    std::vector<double> const &y,
    double h,
    std::function<std::vector<double>(double, const std::vector<double>&)> f,
    std::vector<double> const &p) 
{
    // k1
    std::vector<double> k1 = Mul(h, f(t, y));

    // k2: y + 0.5*k1, t + 0.5*h
    std::vector<double> y2 = Add(y, Mul(0.5, k1));
    std::vector<double> k2 = Mul(h, f(t + 0.5 * h, y2));

    // k3: y + 0.5*k2
    std::vector<double> y3 = Add(y, Mul(0.5, k2));
    std::vector<double> k3 = Mul(h, f(t + 0.5 * h, y3));

    // k4: y + 0.5*k3
    std::vector<double> y4 = Add(y, Mul(0.5, k3));
    std::vector<double> k4 = Mul(h, f(t + 0.5 * h, y4));

    // k5: y + β51*k1 + β54*k4
    std::vector<double> y5 = Add(y, Mul(0.497828275994247056, k1));
    y5 = Add(y5, Mul(0.002171724005752944, k4));
    std::vector<double> k5 = Mul(h, f(t + 0.5 * h, y5));

    // k6: y + 1.0*k5
    std::vector<double> y6 = Add(y, k5);
    std::vector<double> k6 = Mul(h, f(t + h, y6));

    // Итог
    std::vector<double> yNext = y;
    yNext = Add(yNext, Mul(p[0], k1));
    yNext = Add(yNext, Mul(p[1], k2));
    yNext = Add(yNext, Mul(p[2], k3));
    yNext = Add(yNext, Mul(p[3], k4));
    yNext = Add(yNext, Mul(p[4], k5));
    yNext = Add(yNext, Mul(p[5], k6));

    return yNext;
}
