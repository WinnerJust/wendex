#pragma once
#include <string>

#define try "Please try again.\n"

char inputChar(std::string allowed, std::string errMes = try);
std::string inputString(bool noSpaces, std::string errMes = try);
int inputNumber(std::string errMes = try);