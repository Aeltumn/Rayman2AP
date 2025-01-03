#include <iostream>
#include "Connector.h"

int main() {
	Connector connector = Connector();
	connector.send(MESSAGE_TYPE_DEBUG, "[child] Started connector, awaiting instructions");
	connector.wait();
	return 0;
}