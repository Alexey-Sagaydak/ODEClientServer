#ifndef TASK_MANAGER_HPP
#define TASK_MANAGER_HPP

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include "HttpService.h"

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

#endif // TASK_MANAGER_HPP
