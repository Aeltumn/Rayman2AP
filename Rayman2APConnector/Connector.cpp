#include "Connector.h"

Connector *instance;
std::string lastIp;

 void Connector::waitForInput() {
    // Keep reading characters until the null terminator is encountered
    std::string input;
    while (std::getline(std::cin, input)) {
        int type = input[0] - '0';
        input.erase(0, 1);
        instance->handle(type, input);
    }
}

void Connector::waitForAP() {
    // Keep waiting for AP to have messages
    while (true) {
        if (AP_IsInit() && AP_IsMessagePending()) {
            AP_Message* message = AP_GetLatestMessage();
            AP_ClearLatestMessage();
            instance->send(MESSAGE_TYPE_MESSAGE, message->text);
        }
    }
}

void Connector::init() {
    // Store the instance so we can use it in the death link
    instance = this;
}

void Connector::send(int type, std::string data) {
    char typeChar = (type + '0');
	std::cout << '4' << typeChar << data << std::endl;
}

void Connector::handle(int type, std::string data) {
    switch (type) {
    case MESSAGE_TYPE_DEATH:
        // Trigger a death link!
        AP_DeathLinkSend();
        break;
    case MESSAGE_TYPE_MESSAGE:
        // Send the message up to AP.
        AP_Say(data);
        break;
    case MESSAGE_TYPE_CONNECT:
    {
        // Read out the information from the input
        std::istringstream f(data);
        std::string ip;
        std::string slot;
        std::string password;
        std::getline(f, ip, ' ');
        std::getline(f, slot, ' ');
        std::getline(f, password, ' ');

        if (connect(ip, slot, password)) {
            send(MESSAGE_TYPE_MESSAGE, "You are now connected to " + lastIp);
        }
        else {
            send(MESSAGE_TYPE_MESSAGE, "You are already connected to an Archipelago server");
        }
    }
        break;
    case MESSAGE_TYPE_SHUTDOWN:
        if (isConnected()) {
            disconnect();
        }
        break;
    case MESSAGE_TYPE_DISCONNECT:
        if (disconnect()) {
            send(MESSAGE_TYPE_MESSAGE, "Succesfully disconnected from " + lastIp);
        } else {
            send(MESSAGE_TYPE_MESSAGE, "You are not connected to any Archipelago server");
        }
        break;
    case MESSAGE_TYPE_CHECK:
        if (isConnected()) {
            send(MESSAGE_TYPE_MESSAGE, "You are currently connected to " + lastIp);
        } else {
            send(MESSAGE_TYPE_MESSAGE, "You are not connected to any Archipelago server");
        }
        break;
    default:
        send(MESSAGE_TYPE_DEBUG, "[child] Received type " + std::to_string(type) + ": " + data);
    }
}

/** Handles an incoming death link from other games. */
void handleDeathLink() {
    instance->send(MESSAGE_TYPE_DEATH, "");
}

bool Connector::connect(std::string ip, std::string slot, std::string password) {
    if (AP_IsInit()) return false;
    lastIp = ip;
    AP_Init(ip.c_str(), "Rayman 2", slot.c_str(), password.c_str());
    AP_SetDeathLinkSupported(true);
    AP_SetDeathLinkRecvCallback(handleDeathLink);
    AP_Start();
    return true;
}

bool Connector::disconnect() {
    if (!AP_IsInit()) return false;
    AP_Shutdown();
    return true;
}

bool Connector::isConnected() {
    return AP_IsInit();
}