#pragma once

#include <string>
#include "server.h"

class System {
	Server server;
	json db;

	void dumpDB();
	std::string findClosestCityName(std::string);
	std::string generateOrder(std::string from, std::string to);

public:
	System();
};