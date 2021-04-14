#include <iostream>
#include <string>
#include <algorithm>

//Ask to type in a value until one of the chars from allowed string in not typed
char inputChar(std::string allowed, std::string errMes) {
	std::string c;
	while (1) {
		std::getline(std::cin, c);

		if (allowed.find(c) != -1) break;
		else std::cout << errMes;
	}

	return c[0];
}

//noSpaces = false - Inputs any non-empty string
//noSpaces = true - Inputs any non-empty string without spaces
std::string inputString(bool noSpaces, std::string errMes) {
	std::string s;

	while (1) {
		std::getline(std::cin, s);

		if (s.size() > 0 && (std::find(s.begin(), s.end(), ' ') == s.end() || !noSpaces)) break;
		else std::cout << errMes;
	}

	return s;
}

//Ask to type in until the number is received
int inputNumber(std::string errMes) {
	std::string c;
	long res;

	while (1) {
		std::getline(std::cin, c);

		if (c.empty()) {
			std::cout << errMes;
			continue;
		}

		char* remaining;
		res = std::strtol(c.c_str(), &remaining, 10);
		if (*remaining) std::cout << errMes;
		else break;
	}

	return res;
}