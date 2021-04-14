#pragma once

#include <winsock2.h>
#include <functional>
#include "json.hpp"

using json = nlohmann::json;

class Server {
    std::function<json(json)> onData = [](json j) -> json { return ""; };

    void start();

public:

    Server();

    void setOnData(std::function<json(json)>);
    int handleConnection(SOCKET& ClientSocket);
};