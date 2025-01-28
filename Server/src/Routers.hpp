#ifndef _ROUTERS_HPP_
#define _ROUTERS_HPP_

#include "HttpService.h"
#include "HashUtils.hpp"

#include <mutex>
#include <sstream>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

namespace route
{
    void RegisterResources(hv::HttpService &router);
}

#endif