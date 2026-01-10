#include <iostream>
#include <thread>
#include "Connector.h"

int main() {
	Connector connector = Connector();
	connector.init();

	std::thread awaitMainApp(&Connector::waitForInput, &connector);
	std::thread awaitAPInfo(&Connector::waitForAP, &connector);

	awaitMainApp.join();
	awaitAPInfo.join();
	return 0;
}