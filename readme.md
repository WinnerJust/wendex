# WEndex.taxi

## Description

The backend for WEndex.taxi for Windows only

Consists of:

1. The system (the server) - the backend itself
2. The console app (the client) - showing possible usage of the server

## Installation

You will need to build the program from source code.

In order to run the program you need to provide a database. Or you can use the sample `app/db.json`

Make sure to follow the name convention and the structure:

```cpp
{
   "speed":0.1,
   "parkRightInFrontOfTheEntrancePrice":300,
   "freeBottleOfWaterPrice":100,
   "prices":[
      {
         "name":"(E)conomy",
         "price":1000
      },
      {
         "name":"(C)omfort",
         "price":1500
      },
      {
         "name":"Comfort(+)",
         "price":1700
      },
      {
         "name":"(B)usiness",
         "price":4000
      }
   ],
   "cities":{
      "Challenge-Brownsville":{
         "latitude":39.46447,
         "longitude":-121.26338
      },
      "Florence-Graham":{
         "latitude":33.96772,
         "longitude":-118.24438
      }
   }
}
```

## How to run

**System**

You should double check the correctness of the format of your database, then initialize the `System`:

```cpp
int main() {
	System system;

	return 0;
}
```

Now the `System` is ready to use.

**Console app**

You should have your System running to use the console app.

Initialize `GatewayWrapper` class passing the IP of the server to it.

If the Server and the Client are to be used on the same machine you can just pass `localhost`

```cpp
char IP[] = "localhost";
GatewayWrapper gatewayWrapper(IP);
```

The sample Console app uses second argument passed to it as the IP, so you can run it as:

```cpp
app.exe localhost
```

## Requests supported by the System

**signup** - creates a new account

**login** - authorizes a user

**orderRequest** - check the validity of source and destination cities

**createOrder** - creates an order and makes it available to be accepted by a driver

**showOrder** - returns information about the order

**listOrders** - returns IDs of all orders of a user

**findAvailableOrders** - returns IDs of orders suitable for a driver

**acceptOrder** - allows driver to accept an order

**drive** - simulates an event of moving of a car

**coordRequest** - returns coordinates of a car

**updatePaymentMethod** - updates payment methods of a user

**updatePinnedAddresses** - updates pinned addresses of a user

**getCarInfo** - returns information about a car of a driver

**updateStatus** - updates status of a driver

## Sample console app demonstration

```cpp
------------------------------------------
WEndex.Taxi is glad to see you!
Are you a (P)assenger or (D)river?
P
Successfully connected to server
Do you want to (L)ogin or (S)ign up?
S
Email: test@email.com
Password: 123
What is you full name?
Bob
You need to specify one payment method.
Which one would you specify?
(W)Endex.Money
(Q)uiui
(T)inkon
(S)derbank
T
Specify wallet number: 1337
Type in a command or type /help
/reqorder
Specify your location (or type /quit to cancel the procedure): Yreka
Specify destination: Vincent
Route: Yreka-Vincent
Estimated time: 90.174849
Prices:
(E)conomy: 9017.484946
(C)omfort: 13526.227419
Comfort(+): 15329.724409
(B)usiness: 36069.939785
E
Please specify payment method:
(W)Endex.Money
(Q)uiui
(T)inkon
(S)derbank
T
Do you want to (U)se your saved wallet or (C)reate new (or replace the old in case you have it)?
U
Successfully created an order! It was assigned an ID 0
Type in a command or type /help
```

## Other information

This is not stable release of a program. It was successfully compiled using MS VS 2019.

It was successfully run on Windows 10 x64 2004.

You are running this program at your own risk.

Copyright © 2021. All rights reserved.