#include <iostream>
#include <string>
#include <utility>
#include <ctime>
#include <cstdlib>
#include <thread>
#include "json.hpp"
#include "gateways.h"
#include "safeInput.h"

//For convenience
using json = nlohmann::json;


GatewayWrapper::GatewayWrapper(char** argv) {
	//Wrapper prompts user to choose the type of Gateway he is going to use
	//And then creates accordingly
	srand(time(0));

	while (1) {
		std::cout << "------------------------------------------\n";
		std::cout << "WEndex.Taxi is glad to see you! \n";
		std::cout << "Are you a (P)assenger or (D)river? \n";
		char c = inputChar("PpDd", "I don't know such an entity. Please try again!");

		if (c == 'P' || c == 'p') {
			PassengerGateway passengerGateway(argv);
		}
		else if (c == 'D' || c == 'd') {
			DriverGateway driverGateway(argv);
		}
	}
}

Gateway::Gateway(char** argv) : client(argv), loggedIn(false), failed(false) {
	//This performs login (the same for both the passenger and the driver)

	json loginData;

	std::cout << "Do you want to (L)ogin or (S)ign up?\n";
	char c = inputChar("LlSs");

	std::cout << "Email: ";
	std::string s = inputString(true);

	loginData["email"] = s;
	email = s;

	std::cout << "Password: ";
	s = inputString(true);

	loginData["password"] = s;
	userData["password"] = s;


	if (c == 'L' || c == 'l') {
		json resp = client.request({ {"dataType", "login"}, {"data", loginData} });

		if (resp["dataType"] == "error") {
			std::cout << resp["message"].get<std::string>() << '\n';
			failed = true;
		}
		else {
			userData = resp["data"];
			loggedIn = true;
		}
	}

	//The rest is done in inherited gateways as the logic differs
}

PassengerGateway::PassengerGateway(char** argv) : Gateway(argv) {
	//This performs sign up for passenger
	//Asks user for his name and payment method

	if (failed)
		return;

	if (!loggedIn) {
		userData["userType"] = "P";
		userData["rating"] = (double)(rand() % 50) / 10;

		std::cout << "What is you full name?\n";
		userData["name"] = inputString(false);

		std::cout << "You need to specify one payment method.\n";
		auto res = askPayMethUpd();
		userData["payments"][res.first] = res.second;

		json resp = client.request({ { "dataType", "signup" }, { "data", {{"email", email}, {"data", userData}} } });

		if (resp["dataType"] == "error") {
			std::cout << resp["message"].get<std::string>() << '\n';
			return;
		}
		else {
			userData = resp["data"];
			loggedIn = true;
		}
	}

	//Starts infinite event catcher for passenger
	while (1) {
		eventCatcher();
	}
}

DriverGateway::DriverGateway(char** argv) : Gateway(argv) {
	//This performs sign up for driver
	//Asks for information about his car

	if (failed)
		return;

	if (!loggedIn) {
		userData["userType"] = "D";
		userData["rating"] = (double)(rand() % 50) / 10;

		std::cout << "What is you full name?\n";
		userData["name"] = inputString(false);

		json car;

		std::cout << "You need to specify characteristics of your car. \n";
		std::cout << "Model: ";
		car["model"] = inputString(false);

		std::cout << "Color: ";
		car["color"] = inputString(false);

		std::cout << "Plate number: ";

		car["plateNumber"] = inputString(true);
		car["carType"] = rand() % 4;
		car["position"] = { {"latitude", 35}, {"longitude", -120} };

		userData["status"] = "Idle";

		json resp = client.request({ { "dataType", "signup" }, { "data", {{"email", email}, {"data", userData}, {"car", car}} } });

		if (resp["dataType"] == "error") {
			std::cout << resp["message"].get<std::string>() << '\n';
			return;
		}
		else {
			userData = resp["data"];
			loggedIn = true;
		}
	}

	//Starts infinite event catcher for driver
	while (1) {
		eventCatcher();
	}
}

void PassengerGateway::eventCatcher() {
	std::cout << "Type in a command or type /help\n";

	std::string s = inputString(true);

	if (s == "/help") {
		std::cout << "/reqorder - Start an order\n";
		std::cout << "/showorder - Get information on the order by ID\n";
		std::cout << "/carloc - Get location of the car on active order\n";
		std::cout << "/updpayment - Update payment methods\n";
		std::cout << "/updaddr - Update pinned addresses\n";
		std::cout << "/showbill - Get the bill of the order by ID\n";
		std::cout << "/orderhist - Get IDs of all your orders\n";
	}
	else if (s == "/reqorder") reqOrder();
	else if (s == "/showorder") showOrder();
	else if (s == "/carloc") carLoc();
	else if (s == "/updpayment") updPayment();
	else if (s == "/updaddr") updAddr();
	else if (s == "/showbill") showBill();
	else if (s == "/orderhist") orderHist();
}



void DriverGateway::eventCatcher() {
	std::cout << "Type in a command or type /help\n";

	std::string s = inputString(true);

	if (s == "/help") {
		std::cout << "/mycar - Get information on your car\n";
		std::cout << "/updstatus - Update your status\n";
		std::cout << "/availorders - Check available orders\n";
		std::cout << "/acceptorder - Accept order by ID\n";
		std::cout << "/showorder - Get information on the order by ID\n";
		std::cout << "/orderhist - Get IDs of all your orders\n";
	}
	else if (s == "/showorder") showOrder();
	else if (s == "/orderhist") orderHist();
	else if (s == "/mycar") myCar();
	else if (s == "/updstatus") updStatus();
	else if (s == "/availorders") availOrders();
	else if (s == "/acceptorder") acceptOrder();
}



void PassengerGateway::reqOrder() {
	json j, resp;

	while (1) {
		std::cout << "Specify your location (or type /quit to cancel the procedure): ";
		std::string s = inputString(false);

		if (s == "/quit") {
			return;
		}

		j["from"] = s;


		std::cout << "Specify destination: ";
		j["to"] = inputString(false);

		//Sends the request for server to check whether the cities are correct
		resp = client.request({ {"dataType", "orderRequest"}, {"data", j} });

		if (resp["dataType"] == "error") {
			std::cout << resp["message"].get<std::string>();
		}
		else {
			break;
		}
	}

	//If the cities are correct then print the available options to choose from

	std::cout << resp["message"].get<std::string>();


	char c = inputChar("EeCcBb+");
	json carTypes = { {"E", 0}, {"e", 0}, {"C", 1}, {"c", 1}, {"B", 2}, {"b", 2}, {"+", 3} };

	j["carType"] = carTypes[std::string(1, c)];


	j["freeBottleOfWater"] = 0;
	j["parkRightInFrontOfTheEntrance"] = false;

	//If the car is any except the Economy then ask the user about additional bottles of water
	if (c != 'E' && c != 'e') {
		std::cout << "Do you want additional bottles of water (each for "
			<< resp["data"]["freeBottleOfWaterPrice"].get<int>() << ")? (Y) or (N)\n";

		char cc = inputChar("YyNn");


		if (cc == 'Y' || cc == 'y') {
			std::cout << "And how many?\n";

			j["freeBottleOfWater"] = inputNumber();
		}
	}

	//If the car is Business then ask the user about parking right in front of the entrance option
	if (c == 'B' || c == 'b') {
		std::cout << "Do you want the car to park right in front of the entrance? Cost of the option is "
			<< resp["data"]["parkRightInFrontOfTheEntrancePrice"].get<int>() << " (Y) or (N)\n";

		char cc = inputChar("YyNn");

		if (c == 'Y' || c == 'y') {
			j["parkRightInFrontOfTheEntrance"] = true;
		}
	}

	//Asks user about payment method
	//He can then choose the default one stored in database
	std::cout << "Please specify payment method:\n";
	std::cout << "(W)Endex.Money\n";
	std::cout << "(Q)uiui\n";
	std::cout << "(T)inkon\n";
	std::cout << "(S)derbank\n";

	j["paymentMethod"] = std::string(1, inputChar("WQTSwqts"));

	std::cout << "Do you want to (U)se your saved wallet or (C)reate new (or replace the old in case you have it)?\n";
	c = inputChar("UuCc");

	std::string wallet = "default";

	if (c == 'C' || c == 'c') {
		std::cout << "Specify wallet number: ";
		wallet = inputString(true);
	}

	j["paymentValue"] = wallet;


	j["email"] = email;
	resp = client.request({ {"dataType", "createOrder"}, {"data", j} });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Successfully created an order! It was assigned an ID " << resp["data"].get<int>() << "\n";
	}
}

void PassengerGateway::carLoc() {
	json resp = client.request({ {"dataType", "coordRequest"}, {"data", {{"email", email}}} });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << "\n";
	}
	else {
		std::cout << "Car that is on your active order is now on " << resp["data"]["carPos"]["latitude"]
			<< ", " << resp["data"]["carPos"]["longitude"] << '\n';
	}
}

void PassengerGateway::updPayment() {
	auto res = askPayMethUpd();

	json resp = client.request({ {"dataType", "updatePaymentMethod"}, {"data",
		{{"email", email}, {"paymentMethod", res.first}, {"paymentValue", res.second}} } });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Successfully updated payment method!\n";
	}
}

void PassengerGateway::updAddr() {
	std::cout << "Specify the number of order which you want to change" <<
		" (or specify the position past the last order to create a new one)\n";

	//The position of the address in the database
	int addressID = inputNumber();


	std::cout << "Specify address: ";
	std::string addressValue = inputString(false);

	json resp = client.request({ {"dataType", "updatePinnedAddresses"},
		{"data", {{"email", email}, {"addressID", addressID}, {"addressValue", addressValue}}} });


	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Successfully updated pinned address!\n";
	}
}

//This method print the information of the order related to payment (price, payment method and wallet number)
void PassengerGateway::showBill() {
	std::cout << "Please, specify an ID of the order you want to get the bill of: ";
	int res = inputNumber();


	json resp = client.request({ {"dataType", "showOrder"}, {"data", {{"orderID", res}}} });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << "\n";
	}
	else {
		std::cout << "Price: " << resp["data"]["price"] << '\n';
		std::cout << "Payment method: " << resp["data"]["paymentMethod"].get<std::string>() << '\n';
		std::cout << "Wallet: " << resp["data"]["paymentValue"].get<std::string>() << '\n';
	}
}



void DriverGateway::myCar() {
	json resp = client.request({ {"dataType", "getCarInfo"}, {"data",
			{{"email", email}} } });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Your car specification: \n";

		json& carData = resp["data"]["carData"];

		std::cout << "Type assigned by WEndex: " << carData["carType"] << '\n';
		std::cout << "Model: " << carData["model"].get<std::string>() << '\n';
		std::cout << "Licence plate number: " << carData["plateNumber"].get<std::string>() << '\n';
		std::cout << "Color: " << carData["color"].get<std::string>() << '\n';

		std::cout << "Your car is now located on: " << carData["position"]["latitude"] << ", " <<
			carData["position"]["longitude"] << '\n';
	}
}

void DriverGateway::updStatus() {
	std::cout << "Which status do you want to assign to yourself? (I)dle or (W)orking? \n";

	char c = inputChar("IiWw");


	json resp = client.request({ {"dataType", "updateStatus"}, {"data",
		{{"email", email}, {"status", std::string(1, c)}} } });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Successfully updated status to '" << resp["data"].get<std::string>() << '\'' << '\n';
	}
}

void DriverGateway::availOrders() {
	json resp = client.request({ {"dataType", "findAvailableOrders"}, {"data",
			{{"email", email}} } });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Orders available to you: \n";
		for (auto& i : resp["data"]) {
			std::cout << i.get<std::string>() << '\n';
		}
	}
}

void DriverGateway::acceptOrder() {
	std::cout << "Specify the number of the order: ";
	int orderID = inputNumber();

	json resp = client.request({ {"dataType", "acceptOrder"}, {"data",
		{{"email", email}, {"orderID", orderID}} } });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Successfully accepted the order " << orderID << '\n';

		//When the order is accepted
		//The DriverGateway starts to send request to the server every 1 second
		//These reqeust simulate the movement of the car
		while (1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			json resp = client.request({ {"dataType", "drive"}, {"data",
				{{"email", email}} } });

			if (resp["dataType"] == "error") {
				std::cout << resp["message"].get<std::string>() << '\n';
				break;
			}
			else {
				std::cout << "Current location: " << resp["data"]["position"]["latitude"] << ", " <<
					resp["data"]["position"]["longitude"] << '\n';
			}
		}
	}
}



void Gateway::showOrder() {
	std::cout << "Please, specify an ID of the order you want to look up: ";
	int res = inputNumber();


	json resp = client.request({ {"dataType", "showOrder"}, {"data", {{"orderID", res}}} });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << "\n";
	}
	else {
		std::cout << "Source location: " << resp["data"]["from"].get<std::string>() << '\n';
		std::cout << "Destination: " << resp["data"]["to"].get<std::string>() << '\n';
		std::cout << "Car type: " << resp["data"]["carType"].get<int>() << '\n';
		std::cout << "Passenger name: " << resp["data"]["passengerName"].get<std::string>() << '\n';
		std::cout << "Status: " << resp["data"]["status"].get<std::string>() << '\n';
		std::cout << "Price: " << resp["data"]["price"].get<double>() << '\n';
		std::cout << "Payment method: " << resp["data"]["paymentMethod"].get<std::string>() << '\n';
		std::cout << "Wallet: " << resp["data"]["paymentValue"].get<std::string>() << '\n';
		std::cout << "Number of free bottles of water: " << resp["data"]["freeBottleOfWater"].get<int>() << '\n';
		std::cout << "Option: park right in front of the entrance: " << resp["data"]["parkRightInFrontOfTheEntrance"] << '\n';
	}
}

void Gateway::orderHist() {
	json resp = client.request({ {"dataType", "listOrders"}, {"data",
			{{"email", email}} } });

	if (resp["dataType"] == "error") {
		std::cout << resp["message"].get<std::string>() << '\n';
	}
	else {
		std::cout << "Your orders: \n";
		for (auto& i : resp["data"]) {
			std::cout << i.get<std::string>() << '\n';
		}
	}
}


std::pair<std::string, std::string> PassengerGateway::askPayMethUpd() {
	std::cout << "Which one would you specify? \n";
	std::cout << "(W)Endex.Money\n";
	std::cout << "(Q)uiui\n";
	std::cout << "(T)inkon\n";
	std::cout << "(S)derbank\n";

	char c = inputChar("WQTSwqts");

	std::cout << "Specify wallet number: ";
	std::string wallet = inputString(true);

	return { std::string(1, c), wallet };
}