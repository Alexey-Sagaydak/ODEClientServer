#pragma once
#include "HttpService.h"

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <cmath>

using json = nlohmann::json;

// Тип функции для систем ОДУ
using ODEFunction = std::function<std::vector<double>(double, std::vector<double> const &)>;

class TaskManager
{
private:
    std::unordered_map<std::string, ODEFunction> tasks;

public:
    TaskManager();

    void LoadParameters(json const &params);
    const ODEFunction& GetTask(std::string const &taskName) const;
    std::unordered_map<std::string, double> parameters;
};
