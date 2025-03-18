#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include "../APCpp/Archipelago.h"
#pragma comment (lib, "crypt32")

#define MESSAGE_TYPE_DEBUG 0
#define MESSAGE_TYPE_CHECK 1
#define MESSAGE_TYPE_ITEM 2
#define MESSAGE_TYPE_DEATH 3
#define MESSAGE_TYPE_SHUTDOWN 4
#define MESSAGE_TYPE_CONNECT 5
#define MESSAGE_TYPE_DISCONNECT 6
#define MESSAGE_TYPE_MESSAGE 7
#define MESSAGE_TYPE_COMPLETE 8

class Connector {
public:
	static void waitForInput();
	static void waitForAP();

	void init();
	void send(int type, std::string data);
	void handle(int type, std::string data);
	bool connect(std::string ip, std::string slot, std::string password);
	bool disconnect();
	bool isConnected();
};

