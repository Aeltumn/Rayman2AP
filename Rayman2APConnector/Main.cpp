#include <iostream>
#include <thread>
#include "Connector.h"

int main() {
	Connector connector = Connector();
	connector.send(MESSAGE_TYPE_MESSAGE, "Rayman2APConnector has started and is ready to use");

	std::thread awaitMainApp(&Connector::waitForInput, &connector);
	std::thread awaitAPInfo(&Connector::waitForAP, &connector);

	awaitMainApp.join();
	awaitAPInfo.join();
	return 0;
}