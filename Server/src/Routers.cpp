#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "Routers.hpp"

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
        std::string body = req->Body();
        if (body.empty()) {
            resp->status_code = HTTP_STATUS_BAD_REQUEST;
            resp->SetBody("Empty JSON body");
            resp->content_type = TEXT_PLAIN;
            return 400;
        }
        std::cout << "Received JSON: " << body << std::endl;

        nlohmann::json response;
        response["status"] = "success";
        response["message"] = "Data received successfully";
        resp->SetBody(response.dump());
        resp->content_type = APPLICATION_JSON;
        return 200;
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
