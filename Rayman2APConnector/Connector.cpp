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
        // Send the message up to Archipelago server.
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
        // Disconnect if connected, used on shutdown.
        if (isConnected()) {
            disconnect();
        }
        break;
    case MESSAGE_TYPE_DISCONNECT:
        // Disconnect from the Archipelago server.
        if (disconnect()) {
            send(MESSAGE_TYPE_MESSAGE, "Succesfully disconnected from " + lastIp);
        } else {
            send(MESSAGE_TYPE_MESSAGE, "You are not connected to any Archipelago server");
        }
        break;
    case MESSAGE_TYPE_CHECK:
        // Check if the connection is valid.
        if (isConnected()) {
            send(MESSAGE_TYPE_MESSAGE, "You are currently connected to " + lastIp);
        } else {
            send(MESSAGE_TYPE_MESSAGE, "You are not connected to any Archipelago server");
        }
        break;
    case MESSAGE_TYPE_ITEM:
    {
        // Communicate up that an item is collected.
        std::istringstream f(data);
        std::string id;
        std::getline(f, id, ' ');
        if (isConnected()) {
            AP_SendItem(std::stoi(id));
        }
    }
        break;
    case MESSAGE_TYPE_COMPLETE:
        // The game is done; pass it on!
        if (isConnected()) {
            AP_StoryComplete();
        }
        break;
    default:
        send(MESSAGE_TYPE_DEBUG, "[child] Received type " + std::to_string(type) + ": " + data);
    }
}

/** Handles clearing cached item checks. */
void handleItemClear() {
    instance->send(MESSAGE_TYPE_DEBUG, "[child] AP item clear");
}

/** Handles an item being checked. */
void handleItem(int64_t id, bool notify) {
    instance->send(MESSAGE_TYPE_DEBUG, "[child] AP item check: " + std::to_string(id));
}

/** Handles a location being checked. */
void handleLocation(int64_t id) {
    instance->send(MESSAGE_TYPE_DEBUG, "[child] AP location check: " + std::to_string(id));
}

/** Handles level swap data being delivered. */
void handleLevelSwaps(std::string data) {
    instance->send(MESSAGE_TYPE_DEBUG, "[child] AP level_swaps: " + data);
}

/** Handles lum gate thresholds being delivered. */
void handleLumGates(std::string data) {
    instance->send(MESSAGE_TYPE_DEBUG, "[child] AP lum_gates: " + data);
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
    AP_SetItemClearCallback(handleItemClear);
    AP_SetItemRecvCallback(handleItem);
    AP_SetLocationCheckedCallback(handleLocation);
    AP_SetDeathLinkRecvCallback(handleDeathLink);
    AP_RegisterSlotDataRawCallback("level_swaps", handleLevelSwaps);
    AP_RegisterSlotDataRawCallback("lum_gates", handleLumGates);
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