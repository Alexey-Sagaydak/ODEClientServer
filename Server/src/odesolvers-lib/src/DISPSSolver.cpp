#include "DISPSSolver.hpp"

// Вспомогательные функции
std::vector<double> DISPSSolver::Add(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size())
        throw std::runtime_error("Add: размеры векторов не совпадают");

    std::vector<double> r(a.size());
    for (size_t i = 0; i < a.size(); i++)
        r[i] = a[i] + b[i];

    return r;
}

std::vector<double> DISPSSolver::Mul(double alpha, const std::vector<double>& v) {
    std::vector<double> r(v.size());
    for (size_t i = 0; i < v.size(); i++)
        r[i] = alpha * v[i];

    return r;
}

std::vector<double> DISPSSolver::Subtract(const std::vector<double>& a, const std::vector<double>& b) {
    if(a.size() != b.size())
        throw std::runtime_error("Subtract: размеры векторов не совпадают");

    std::vector<double> res(a.size());
    for (size_t i = 0; i < a.size(); i++)
        res[i] = a[i] - b[i];

    return res;
}

double DISPSSolver::Norm(const std::vector<double>& v) {
    double sum = 0.0;
    for (double val : v)
        sum += val * val;
    return std::sqrt(sum);
}

// Конструктор
DISPSSolver::DISPSSolver(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                         double initialStep,
                         const DispsEnabledFlags& flags)
    : f_(f), stepSize_(initialStep), currentIndex_(0)
{
    if (flags.Disps13)
        variants_.push_back({3, 1, {0.76561395265924, 0.2066991671971, 0.027686880143657}, 17.0});

    if (flags.Disps15)
        variants_.push_back({5, 1, {0.49900087977418, 0.33439107198377, 0.15517028825608,
                                      0.011387362618405, 0.50397367568471e-4}, 46.8});

    if (flags.Disps23)
        variants_.push_back({3, 2, {0.25, 0.461290700433682, 0.28870929566318}, 6.0});

    if (flags.Disps25)
        variants_.push_back({5, 2, {0.25, 0.39269911064722, 0.2811818347201,
                                      0.06530730655278, 0.010811748097894}, 18.8});

    if (flags.Disps35)
        variants_.push_back({5, 3, {1.0/6.0, -2.644193175547, 0.15929230363104,
                                      3.1515675385826, 1.0/6.0}, 10.3});

    if (flags.Disps36)
        variants_.push_back({6, 3, {1.0/6.0, -6.0414736520435, 0.12355552068869,
                                      0.18196838110013, 6.4026164169214, 1.0/6.0}, 15.68});

    if (variants_.empty())
        throw std::runtime_error("Нет включённых вариантов DISPS!");
}

// Основной метод решения
std::vector<std::vector<double>> DISPSSolver::Solve(double t0,
                                                    const std::vector<double>& y0,
                                                    double tEnd,
                                                    double tolerance)
{
    std::vector<std::vector<double>> results;
    results.reserve(10000);
    
    double t = t0;
    std::vector<double> y = y0;
    
    {
        std::vector<double> row;
        row.push_back(t);
        row.insert(row.end(), y.begin(), y.end());
        results.push_back(row);
    }

    while (t < tEnd) {
        double hAttempt = std::min(stepSize_, tEnd - t);
        if (hAttempt < 1e-14)
            break;

        DispsVariant& variant = variants_[currentIndex_];
        bool stepAccepted = false;

        for (int tries = 0; tries < 30 && !stepAccepted; tries++) {
            std::vector<std::vector<double>> kStages;
            std::vector<double> yNext = DoOneStep(t, y, hAttempt, variant, kStages);

            bool needSwitch = false;
            double newH = hAttempt;
            bool ok = false;

            if (variant.order == 1)
                ok = Control1stOrder(kStages, y, yNext, hAttempt, tolerance, newH, needSwitch);
            else if (variant.order == 2)
                ok = Control2ndOrder(kStages, y, yNext, hAttempt, tolerance, newH, needSwitch);
            else
                ok = Control3rdOrder(kStages, y, yNext, hAttempt, tolerance, newH, needSwitch);

            if (ok) {
                stepAccepted = true;
                t += hAttempt;
                y = yNext;
                stepSize_ = newH;

                std::vector<double> row{ t };
                row.insert(row.end(), y.begin(), y.end());
                results.push_back(row);

                if (needSwitch) {
                    SwitchScheme(variant.order);
                }
            } else {
                if (variant.stages == 5 && variant.order == 2 && newH < hAttempt) {
                    SwitchScheme(variant.order);
                    break;
                }
                hAttempt = newH;
            }
        }

        if (!stepAccepted) {
            hAttempt = std::max(hAttempt * 0.5, 1e-10);
            stepSize_ = hAttempt;
        }
    }
    
    return results;
}

// Выполнение одного шага
std::vector<double> DISPSSolver::DoOneStep(double t,
                                           const std::vector<double>& y,
                                           double h,
                                           const DispsVariant& variant,
                                           std::vector<std::vector<double>>& kStages)
{
    if (variant.stages == 3)
        return Step3Stage(t, y, h, f_, variant.p, kStages);
    else if (variant.stages == 5)
        return Step5Stage(t, y, h, f_, variant.p, kStages);
    else if (variant.stages == 6)
        return Step6Stage(t, y, h, f_, variant.p, kStages);
    else
        throw std::runtime_error("Неизвестное число стадий");
}

// Реализация шага для 3 стадий
std::vector<double> DISPSSolver::Step3Stage(double t,
                                            const std::vector<double>& y,
                                            double h,
                                            std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                            const std::vector<double>& p,
                                            std::vector<std::vector<double>>& kStages)
{
    kStages.clear();
    kStages.resize(3);

    std::vector<double> k1 = Mul(h, f(t, y));

    kStages[0] = k1;

    std::vector<double> y2 = Add(y, Mul(2.0/3.0, k1));
    std::vector<double> k2 = Mul(h, f(t + 2.0/3.0 * h, y2));
    kStages[1] = k2;

    std::vector<double> y3 = Add(y, Mul(1.0/3.0, k1));
    y3 = Add(y3, Mul(1.0/3.0, k2));
    std::vector<double> k3 = Mul(h, f(t + h, y3));
    kStages[2] = k3;

    std::vector<double> yNext = Add(y, Mul(p[0], k1));
    yNext = Add(yNext, Mul(p[1], k2));
    yNext = Add(yNext, Mul(p[2], k3));

    return yNext;
}

// Реализация шага для 5 стадий
std::vector<double> DISPSSolver::Step5Stage(double t,
                                            const std::vector<double>& y,
                                            double h,
                                            std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                            const std::vector<double>& p,
                                            std::vector<std::vector<double>>& kStages)
{
    kStages.clear();
    kStages.resize(5);

    std::vector<double> k1 = Mul(h, f(t, y));
    kStages[0] = k1;

    std::vector<double> y2 = Add(y, Mul(1.0/3.0, k1));
    std::vector<double> k2 = Mul(h, f(t + 1.0/3.0 * h, y2));
    kStages[1] = k2;

    std::vector<double> y3 = Add(y, Mul(1.0/6.0, k1));
    y3 = Add(y3, Mul(1.0/6.0, k2));
    std::vector<double> k3 = Mul(h, f(t + 1.0/2.0 * h, y3));
    kStages[2] = k3;

    std::vector<double> y4 = Add(y, Mul(1.0/8.0, k1));
    y4 = Add(y4, Mul(1.0/8.0, k3));
    std::vector<double> k4 = Mul(h, f(t + 3.0/4.0 * h, y4));
    kStages[3] = k4;

    std::vector<double> y5 = Add(y, Mul(1.0/2.0, k1));
    y5 = Add(y5, Mul(-3.0/2.0, k3));
    y5 = Add(y5, Mul(2.0, k4));
    std::vector<double> k5 = Mul(h, f(t + h, y5));
    kStages[4] = k5;

    std::vector<double> yNext = Add(y, Mul(p[0], k1));
    yNext = Add(yNext, Mul(p[1], k2));
    yNext = Add(yNext, Mul(p[2], k3));
    yNext = Add(yNext, Mul(p[3], k4));
    yNext = Add(yNext, Mul(p[4], k5));

    return yNext;
}

// Реализация шага для 6 стадий
std::vector<double> DISPSSolver::Step6Stage(double t,
                                            const std::vector<double>& y,
                                            double h,
                                            std::function<std::vector<double>(double, const std::vector<double>&)> f,
                                            const std::vector<double>& p,
                                            std::vector<std::vector<double>>& kStages)
{
    kStages.clear();
    kStages.resize(6);

    std::vector<double> k1 = Mul(h, f(t, y));
    kStages[0] = k1;

    std::vector<double> y2 = Add(y, Mul(0.5, k1));
    std::vector<double> k2 = Mul(h, f(t + 0.5 * h, y2));
    kStages[1] = k2;

    std::vector<double> y3 = Add(y, Mul(0.5, k2));
    std::vector<double> k3 = Mul(h, f(t + 0.5 * h, y3));
    kStages[2] = k3;

    std::vector<double> y4 = Add(y, Mul(0.5, k3));
    std::vector<double> k4 = Mul(h, f(t + 0.5 * h, y4));
    kStages[3] = k4;

    std::vector<double> y5 = Add(y, Mul(0.497828275994247056, k1));
    y5 = Add(y5, Mul(0.002171724005752944, k4));
    std::vector<double> k5 = Mul(h, f(t + 0.5 * h, y5));
    kStages[4] = k5;

    std::vector<double> y6 = Add(y, k5);
    std::vector<double> k6 = Mul(h, f(t + h, y6));
    kStages[5] = k6;

    std::vector<double> yNext = Add(y, Mul(p[0], k1));
    yNext = Add(yNext, Mul(p[1], k2));
    yNext = Add(yNext, Mul(p[2], k3));
    yNext = Add(yNext, Mul(p[3], k4));
    yNext = Add(yNext, Mul(p[4], k5));
    yNext = Add(yNext, Mul(p[5], k6));

    return yNext;
}

// Контроль 1-го порядка
bool DISPSSolver::Control1stOrder(const std::vector<std::vector<double>>& kStages,
                                  const std::vector<double>&,
                                  const std::vector<double>&,
                                  double h,
                                  double tolerance,
                                  double& newH,
                                  bool& needSwitch)
{
    const double eps = 1e-15;
    const auto& variant = variants_[currentIndex_];

    double k1_norm = Norm(kStages[0]);
    double k2_norm = Norm(kStages[1]);
    double delta11 = (2.0 * std::fabs(1.0 - 2.0 * k1_norm)) / (k2_norm + eps);

    std::vector<double> diff = Subtract(kStages[1], kStages[0]);
    double A_prime = delta11 * Norm(diff);

    double Vn = (Norm(diff) * Norm(diff)) / ((k2_norm + eps) * (Norm(diff) + eps));
    needSwitch = (Vn > variant.gamma);

    double q = 0.8;
    if (A_prime > tolerance) {
        newH = h * q * std::pow(tolerance / (A_prime + eps), 0.5);
        return false;
    }

    newH = h * 1.2;
    return true;
}

// Контроль 2-го порядка
bool DISPSSolver::Control2ndOrder(const std::vector<std::vector<double>>& kStages,
                                  const std::vector<double>&,
                                  const std::vector<double>&,
                                  double h,
                                  double tolerance,
                                  double& newH,
                                  bool& needSwitch)
{
    const double eps = 1e-15;
    const auto& variant = variants_[currentIndex_];

    double k1_norm = Norm(kStages[0]);
    double delta11 = 1.0 - 4.0 * k1_norm;

    std::vector<double> diff = Subtract(kStages[2], kStages[1]);
    double B_prime = delta11 * Norm(diff);

    double k3_norm = Norm(kStages[2]);
    double Vn = (Norm(diff) * Norm(diff)) / ((k3_norm + eps) * (Norm(diff) + eps));
    needSwitch = (Vn > variant.gamma);

    double q = (variant.stages == 5 && variant.order == 2) ? 0.4 : 0.8;
    if (B_prime > tolerance) {
        newH = h * q * std::pow(tolerance / (B_prime + eps), 1.0/3.0);
        if (variant.stages == 5 && variant.order == 2)
            needSwitch = true;
        return false;
    }

    newH = h * 1.1;
    return true;
}

// Контроль 3-го порядка
bool DISPSSolver::Control3rdOrder(const std::vector<std::vector<double>>& kStages,
                                  const std::vector<double>&,
                                  const std::vector<double>&,
                                  double h,
                                  double tolerance,
                                  double& newH,
                                  bool& needSwitch)
{
    const double eps = 1e-15;
    const auto& variant = variants_[currentIndex_];

    double k1_norm = Norm(kStages[0]);
    double k2_norm = Norm(kStages[1]);
    double g_n1 = (1.0 - 2.0 * k1_norm) / (2.0 * k2_norm + eps);

    std::vector<double> diff = Subtract(kStages[3], kStages[2]);
    double C_prime = g_n1 * Norm(diff);

    double k4_norm = Norm(kStages[3]);
    double Vn = (Norm(diff) * Norm(diff)) / ((k4_norm + eps) * (Norm(diff) + eps));
    needSwitch = (Vn > variant.gamma);

    double q = 0.8;
    if (C_prime > tolerance) {
        newH = h * q * std::pow(tolerance / (C_prime + eps), 0.25);
        return false;
    }

    newH = h * 1.05;
    return true;
}

void DISPSSolver::SwitchScheme(int currentOrder) {
    int bestIndex = currentIndex_;
    int bestOrder = variants_[currentIndex_].order;
    double bestGamma = variants_[currentIndex_].gamma;

    for (size_t i = 0; i < variants_.size(); i++) {
        if (static_cast<int>(i) == currentIndex_)
            continue;

        int candidateOrder = variants_[i].order;
        double candidateGamma = variants_[i].gamma;

        if (candidateOrder > bestOrder) {
            bestOrder = candidateOrder;
            bestGamma = candidateGamma;
            bestIndex = static_cast<int>(i);
        } else if (candidateOrder == bestOrder && candidateGamma < bestGamma) {
            bestGamma = candidateGamma;
            bestIndex = static_cast<int>(i);
        }
    }

    if (bestIndex != currentIndex_) {
        currentIndex_ = bestIndex;
    }
}
