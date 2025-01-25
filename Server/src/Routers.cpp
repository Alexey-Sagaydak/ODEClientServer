#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "Routers.hpp"
#include "odesolvers-lib/include/TaskManager.hpp"
#include "odesolvers-lib/include/RK2Solver.hpp"
#include "odesolvers-lib/include/EulerSolver.hpp"
#include "odesolvers-lib/include/RK23SSolver.hpp"
#include "odesolvers-lib/include/STEKSSolver.hpp"
#include "ParseUtils.hpp"

std::mutex usersMutex;

void route::RegisterResources(hv::HttpService &router, std::unordered_map<std::string, User> &users)
{
    router.GET("/", [](HttpRequest *req, HttpResponse *resp)
    {
        resp->SetBody("Server is working fine!");
        resp->content_type = TEXT_PLAIN;
        return 200;
    });

    router.POST("/solve", [](HttpRequest *req, HttpResponse *resp) {
        try {
            auto body = nlohmann::json::parse(req->Body());
            std::cout << body << std::endl;
            std::string taskName = body["Equation"].get<std::string>();
            std::string method = body["Method"].get<std::string>();
            nlohmann::json parameters = body["Parameters"];
            double t0 = parameters["t0"].get<double>();
            double tEnd = parameters["t1"].get<double>();
            double initialStep = 0.001;
            double tolerance = parameters["tolerance"].get<double>();

            Storage storage;

            TaskManager taskManager;
            taskManager.LoadParameters(parameters);
            std::vector<double> y0 = ExtractInitialConditions(taskManager.parameters);

            auto odeFunction = taskManager.GetTask(taskName);

            if (method == "ExplicitEuler") {
                EulerSolver solver = EulerSolver(odeFunction, initialStep);
                solver.Solve(t0, y0, tEnd, storage, tolerance);
            } else if (method == "RungeKutta2") {
                RK2Solver solver = RK2Solver(odeFunction, initialStep);
                solver.Solve(t0, y0, tEnd, storage, tolerance);
            } else if (method == "RK23S") {
                RK23SSolver solver = RK23SSolver(odeFunction, initialStep);
                solver.Solve(t0, y0, tEnd, storage, tolerance);
            } else if (method == "STEKS") {
                STEKSSolver solver = STEKSSolver(odeFunction, initialStep);
                solver.Solve(t0, y0, tEnd, storage, tolerance);
            } else if (method == "DISPD" || method == "DISPF") {
                resp->status_code = HTTP_STATUS_BAD_REQUEST;
                resp->SetBody("Error: Method '" + method + "' is not implemented.");
                resp->content_type = TEXT_PLAIN;
                return 400;
            } else {
                resp->status_code = HTTP_STATUS_BAD_REQUEST;
                resp->SetBody("Error: Unknown method '" + method + "'.");
                resp->content_type = TEXT_PLAIN;
                return 400;
            }

            nlohmann::json response;
            response["status"] = "success";
            response["results"] = nlohmann::json::array();
            for (size_t i = 0; i < storage.Size(); ++i) {
                nlohmann::json result;
                result["t"] = storage[i].first;
                result["values"] = storage[i].second;
                response["results"].push_back(result);
            }
            resp->SetBody(response.dump());
            resp->content_type = APPLICATION_JSON;
            return 200;

        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            resp->status_code = HTTP_STATUS_BAD_REQUEST;
            resp->SetBody(std::string("Error: ") + e.what());
            resp->content_type = TEXT_PLAIN;
            return 400;
        }
    });
}

void route::authenticate(const HttpRequest* req, HttpResponse* resp, std::unordered_map<std::string, User> &users, bool* isAuth, User* currentUser) {
    HashUtils hashUtils;
    std::string hashedPassword;
    auto authHeader = req->headers.find("Authorization");

    if (authHeader == req->headers.end()) {
        resp->status_code = HTTP_STATUS_UNAUTHORIZED;
        resp->SetHeader("WWW-Authenticate", "Basic realm=\"Authentication Required\"");
        resp->SetBody("Unauthorized access");
        resp->content_type = TEXT_PLAIN;
        *isAuth = false;
        return;
    }

    std::string authStr = authHeader->second;
    if (!boost::starts_with(authStr, "Basic ")) {
        resp->status_code = HTTP_STATUS_UNAUTHORIZED;
        resp->SetBody("Invalid Authorization header");
        resp->content_type = TEXT_PLAIN;
        *isAuth = false;
        return;
    }

    authStr = authStr.substr(6);
    std::string decoded = base64_decode(authStr);

    std::istringstream iss(decoded);
    std::string username, password;
    std::getline(iss, username, ':');
    std::getline(iss, password);

    auto userIt = std::find_if(users.begin(), users.end(), [&](const auto& pair) {
        return pair.second.username == username;
    });

    hashUtils.computeHash(password, hashedPassword);
    if (userIt == users.end() || userIt->second.password != hashedPassword) {
        resp->status_code = HTTP_STATUS_UNAUTHORIZED;
        resp->SetHeader("WWW-Authenticate", "Basic realm=\"Authentication Required\"");
        resp->SetBody("Unauthorized access");
        resp->content_type = TEXT_PLAIN;
        *isAuth = false;
        return;
    }

    *currentUser = userIt->second;
    *isAuth = true;
}

std::string route::base64_decode(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}
