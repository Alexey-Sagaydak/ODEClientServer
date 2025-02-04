#include "Routers.hpp"

void route::RegisterResources(hv::HttpService& router)
{
    router.GET("/", [](HttpRequest* req, HttpResponse* resp)
    {
        resp->SetBody("Server is working fine!");
        resp->content_type = TEXT_PLAIN;
        return 200;
    });

    router.POST("/solve", [](HttpContextPtr const &ctx)
    {
        try
        {
            auto body = nlohmann::json::parse(ctx->request->body);
            std::cout << body << std::endl;
            std::string taskName = body["Equation"].get<std::string>();
            std::string method = body["Method"].get<std::string>();
            nlohmann::json parameters = body["Parameters"];
            
            std::thread([ctx, taskName, method, parameters]()
            {
                try
                {
                    double t0 = parameters["t0"].get<double>();
                    double tEnd = parameters["t1"].get<double>();
                    double initialStep = 0.001;
                    double tolerance = parameters["tolerance"].get<double>();

                    Storage storage;
                    TaskManager taskManager;
                    taskManager.LoadParameters(parameters);
                    std::vector<double> y0 = ExtractInitialConditions(taskManager.parameters);
                    auto odeFunction = taskManager.GetTask(taskName);

                    if (method == "ExplicitEuler")
                    {
                        EulerSolver solver(odeFunction, initialStep);
                        solver.Solve(t0, y0, tEnd, storage, tolerance);
                    }
                    else if (method == "RungeKutta2")
                    {
                        RK2Solver solver(odeFunction, initialStep);
                        solver.Solve(t0, y0, tEnd, storage, tolerance);
                    }
                    else if (method == "RK23S")
                    {
                        RK23SSolver solver(odeFunction, initialStep);
                        solver.Solve(t0, y0, tEnd, storage, tolerance);
                    }
                    else if (method == "STEKS")
                    {
                        STEKSSolver solver(odeFunction, initialStep);
                        solver.Solve(t0, y0, tEnd, storage, tolerance);
                    }
                    else if (method == "DISPD")
                    {
                        DISPDSolver solver(odeFunction, initialStep);
                        solver.Solve(t0, y0, tEnd, storage, tolerance);
                    }
                    else if (method == "DISPF")
                    {
                        throw std::runtime_error("Unsupported yet: " + method);
                    }
                    else
                    {
                        throw std::runtime_error("Unknown method: " + method);
                    }

                    // Начинаем потоковую передачу
                    ctx->writer->Begin();
                    ctx->response->content_type = APPLICATION_JSON;
                    ctx->writer->WriteHeader("Transfer-Encoding", "chunked");
                    ctx->writer->EndHeaders();

                    const size_t CHUNK_SIZE = 4096;
                    std::string buffer;
                    buffer.reserve(CHUNK_SIZE * 2);

                    auto flush_buffer = [&]()
                    {
                        if (!buffer.empty() && ctx->writer->isConnected())
                        {
                            ctx->writer->WriteBody(buffer);
                            buffer.clear();
                        }
                    };

                    // Старт JSON
                    buffer += R"({"status":"success","results":)";
                    flush_buffer();

                    if (storage.Size() > 0)
                    {
                        buffer += "[";
                        for (size_t i = 0; i < storage.Size(); ++i)
                        {
                            if (!ctx->writer->isConnected()) break;

                            if (i > 0) buffer += ",";

                            nlohmann::json result;
                            result["t"] = storage[i].first;
                            result["values"] = storage[i].second;
                            buffer += result.dump();

                            if (buffer.size() >= CHUNK_SIZE)
                            {
                                flush_buffer();
                            }
                        }
                        buffer += "]";
                    }
                    else
                    {
                        buffer += "[]";
                    }

                    // Завершаем JSON
                    buffer += "}";
                    flush_buffer();

                    ctx->writer->End();
                }
                catch (const std::exception& e)
                {
                    if (ctx->writer->isConnected())
                    {
                        ctx->writer->WriteStatus(HTTP_STATUS_INTERNAL_SERVER_ERROR);
                        ctx->writer->WriteHeader("Content-Type", TEXT_PLAIN);
                        ctx->writer->WriteBody("Error: " + std::string(e.what()));
                        ctx->writer->End();
                    }
                }
            }).detach();

            return static_cast<int>(HTTP_STATUS_UNFINISHED);
        }
        catch (const std::exception& e)
        {
            ctx->response->status_code = HTTP_STATUS_BAD_REQUEST;
            ctx->response->SetBody(std::string("Error: ") + e.what());
            ctx->response->content_type = TEXT_PLAIN;
            return static_cast<int>(HTTP_STATUS_BAD_REQUEST);
        }
    });
}
