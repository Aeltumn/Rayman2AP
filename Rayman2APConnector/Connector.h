#pragma once
#include <string>

#define MESSAGE_TYPE_DEBUG 0
#define MESSAGE_TYPE_CHECK 1
#define MESSAGE_TYPE_ITEM 2
#define MESSAGE_TYPE_DEATH 3

class Connector {
public:
	void wait();
	void send(int type, std::string data);
	void handle(int type, std::string data);
};

