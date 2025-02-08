#pragma once
#include "HttpService.h"
#include "ParseUtils.hpp"
#include "HttpContext.h"

#include "odesolvers-lib/include/ODESolvers.hpp"

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
