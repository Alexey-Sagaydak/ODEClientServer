#pragma once
#include "HttpService.h"
#include "ParseUtils.hpp"
#include "HttpContext.h"

#include "odesolvers-lib/include/TaskManager.hpp"
#include "odesolvers-lib/include/RK2Solver.hpp"
#include "odesolvers-lib/include/EulerSolver.hpp"
#include "odesolvers-lib/include/RK23SSolver.hpp"
#include "odesolvers-lib/include/STEKSSolver.hpp"
#include "odesolvers-lib/include/DISPDSolver.hpp"

#include <sstream>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>

namespace route
{
    void RegisterResources(hv::HttpService &router);
}
