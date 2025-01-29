#include "Routers.hpp"
#include "ParseUtils.hpp"

#include "odesolvers-lib/include/TaskManager.hpp"
#include "odesolvers-lib/include/RK2Solver.hpp"
#include "odesolvers-lib/include/EulerSolver.hpp"
#include "odesolvers-lib/include/RK23SSolver.hpp"
#include "odesolvers-lib/include/STEKSSolver.hpp"
#include "odesolvers-lib/include/DISPDSolver.hpp"

void route::RegisterResources(hv::HttpService &router)
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
            } else if (method == "DISPD") {
                DISPDSolver solver = DISPDSolver(odeFunction, initialStep);
                solver.Solve(t0, y0, tEnd, storage, tolerance);
            } else if (method == "DISPF") {
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
