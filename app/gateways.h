#pragma once

#include "client.h"

class GatewayWrapper {
public:
	GatewayWrapper(char** argv);
};

class Gateway {
protected:
	Client client;
	json userData;
	bool loggedIn;
	bool failed;
	std::string email;

	void showOrder();
	void orderHist();

public:
	Gateway(char** argv);
};

class PassengerGateway : public Gateway {
	void eventCatcher();

	void reqOrder();
	void carLoc();
	void updPayment();
	void updAddr();
	void showBill();

	std::pair<std::string, std::string> askPayMethUpd();

public:
	PassengerGateway(char** argv);
};

class DriverGateway : public Gateway {
	void eventCatcher();

	void myCar();
	void updStatus();
	void availOrders();
	void acceptOrder();
public:
	DriverGateway(char** argv);
};