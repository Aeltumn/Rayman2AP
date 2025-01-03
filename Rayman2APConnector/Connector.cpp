#include "Connector.h"
#include <iostream>

void Connector::wait() {
    // Keep reading characters until the null terminator is encountered
    std::string input;
    while (std::getline(std::cin, input)) {
        int type = input[0] - '0';
        input.erase(0, 1);
        handle(type, input);
    }
}

void Connector::send(int type, std::string data ){
    std::string prefix(1, type + '0');
    std::string message = prefix + data;
	std::cout << message << std::endl;
}

void Connector::handle(int type, std::string data) {
    switch (type) {
    case MESSAGE_TYPE_UPDATE_DEATHLINK:
        // TODO: Sync death link state with AP
        send(MESSAGE_TYPE_UPDATE_DEATHLINK, data);
        break;
    case MESSAGE_TYPE_TEST:
        send(MESSAGE_TYPE_DEATH, "");
        break;
    default:
        send(MESSAGE_TYPE_DEBUG, "[child] Received type " + std::to_string(type) + ": " + data);
    }
}