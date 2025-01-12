#include "../include/TaskManager.hpp"
#include <stdexcept>
#include <cmath>

TaskManager::TaskManager() {
    tasks["VanDerPol"] = [this](double t, const std::vector<double>& y) {
        double mu = parameters["mu"];
        double p = parameters["p"];
        return std::vector<double>{
            y[1],
            (mu * (1 - y[0] * y[0]) * y[1] - y[0]) / p
        };
    };

    tasks["ForcedOscillator"] = [this](double t, const std::vector<double>& y) {
        double omega = parameters["omega"];
        double gamma = parameters["gamma"];
        double F = parameters["F"];
        double omega_drive = parameters["omega_k"];
        return std::vector<double>{
            y[1],
            -omega * omega * y[0] - gamma * y[1] + F * std::cos(omega_drive * t)
        };
    };

    tasks["RobertsonSystem"] = [this](double t, const std::vector<double>& y) {
        double k1 = parameters["k1"];
        double k2 = parameters["k2"];
        double k3 = parameters["k3"];
        return std::vector<double>{
            -k1 * y[0] + k2 * y[1] * y[2],
            k1 * y[0] - k2 * y[1] * y[2] - k3 * y[1] * y[1],
            k3 * y[1] * y[1]
        };
    };
}

void TaskManager::LoadParameters(const json& params) {
    for (auto& [key, value] : params.items()) {
        parameters[key] = value.get<double>();
    }
}

const ODEFunction& TaskManager::GetTask(const std::string& taskName) const {
    auto it = tasks.find(taskName);
    if (it == tasks.end()) {
        throw std::runtime_error("Task not found: " + taskName);
    }
    return it->second;
}
