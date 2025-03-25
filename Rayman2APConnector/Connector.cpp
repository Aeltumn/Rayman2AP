#include "Connector.h"
#include <iomanip>
#include <unordered_map>

Connector *instance;
std::vector<int64_t> unlockedChecks;
std::unordered_map<std::string, std::string> levelSwaps;
std::unordered_map<std::string, int> lumGates;
std::unordered_map<int64_t, std::string> idMap;
std::string lastIp;

/** Prints the current connection status. */
void printConnectionStatus() {
    switch (AP_GetConnectionStatus()) {
    case AP_ConnectionStatus::Disconnected:
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP Status: Disconnected");
        break;
    case AP_ConnectionStatus::Connected:
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP Status: Connected");
        break;
    case AP_ConnectionStatus::Authenticated:
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP Status: Authenticated");
        break;
    case AP_ConnectionStatus::ConnectionRefused:
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP Status: Connection Refused");
        break;
    }
}

/** Returns the number as a padded string. */
std::string paddedString(int num) {
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << num;
    return oss.str();
}

 void Connector::waitForInput() {
     try {
         // Keep reading characters until the null terminator is encountered
         std::string input;
         while (std::getline(std::cin, input)) {
             int type = input[0] - '0';
             input.erase(0, 1);
             instance->handle(type, input);
         }
     } catch (const std::exception& e) {
         instance->send(MESSAGE_TYPE_MESSAGE, "[waitForInput] Caught exception: " + std::string(e.what()));
     } catch (...) {
         instance->send(MESSAGE_TYPE_MESSAGE, "[waitForInput] Caught an unknown exception!");
     }
}

void Connector::waitForAP() {
    try {
        // Keep waiting for AP to have messages
        while (true) {
            if (AP_IsInit() && AP_IsMessagePending()) {
                AP_Message* message = AP_GetLatestMessage();
                instance->send(MESSAGE_TYPE_MESSAGE, message->text);
                AP_ClearLatestMessage();
            }
        }
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[waitForAP] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[waitForAP] Caught an unknown exception!");
    }
}

void Connector::init() {
    // Store the instance so we can use it in the death link
    instance = this;
}

void Connector::send(int type, std::string data) {
    char typeChar = (type + '0');
    int length = data.length();
    if (length < 0 || length > 999999) {
        throw std::runtime_error("Invalidly long strong, length was " + std::to_string(length));
    }
    std::cout << (char)26 << paddedString(length) << typeChar << data;
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

        if (!connect(ip, slot, password)) {
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
            printConnectionStatus();
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
        send(MESSAGE_TYPE_MESSAGE, "[child] Informing AP that story is done");
        if (isConnected()) {
            AP_StoryComplete();
        }
        break;
    default:
        send(MESSAGE_TYPE_MESSAGE, "[child] Received type " + std::to_string(type) + ": " + data);
    }
}

/** Handles clearing cached item checks. */
void handleItemClear() {
    // We don't propagate item state clears because we store within the save file what the client has.
    // Instead we clear the local state of the connector which is what it communicates to the client whenever
    // a new save file is loaded and we need to update the collected objects.
    unlockedChecks.clear();
}

/** Handles an item being checked. */
void handleItem(int64_t id, bool notify) {
    unlockedChecks.push_back(id);

    // TODO Redirect to the game client if a save got selected and send all unlocked checks across when a save is picked, also send level swaps and lum gates when we do!
}

/** Handles a location being checked. */
void handleLocation(int64_t id) {
    // We don't care about external location checks coming in. We don't
    // want to hand out checks because they won't result in a send. We
    // assume Archipelago ignores it if we later check these.
}

/** Handles level swap data being delivered. */
void handleLevelSwaps(std::string data) {
    try {
        // Parses the level swaps from the archipelago input
        levelSwaps.clear();
        if (data.length() < 3) return;
        std::string inner = data.substr(1, data.length() - 2);
        std::stringstream stream(inner);
        std::string token;
        while (std::getline(stream, token, ',')) {
            size_t colonPos = token.find(":");
            if (colonPos == std::string::npos) continue;
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            levelSwaps[key.substr(1, key.length() - 2)] = value.substr(1, value.length() - 2);
        }

        // Debug print to the log
        std::stringstream ss;
        for (auto it = levelSwaps.begin(); it != levelSwaps.end(); ++it) {
            ss << it->first << " = " << it->second;
            if (std::next(it) != levelSwaps.end()) {
                ss << ", ";
            }
        }
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP level_swaps: " + ss.str());
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLevelSwaps] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLevelSwaps] Caught an unknown exception!");
    }
}

/** Handles lum gate thresholds being delivered. */
void handleLumGates(std::string data) {
    try {
        // Parse the lum gates from the archipelago input
        lumGates.clear();
        if (data.length() < 3) return;
        std::string inner = data.substr(1, data.length() - 2);
        std::stringstream stream(inner);
        std::string token;
        while (std::getline(stream, token, ',')) {
            size_t colonPos = token.find(":");
            if (colonPos == std::string::npos) continue;
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            lumGates[key.substr(1, key.length() - 2)] = std::stoi(value);
        }

        // Debug print the lum gates to the log
        std::stringstream ss;
        for (auto it = lumGates.begin(); it != lumGates.end(); ++it) {
            ss << it->first << " = " << std::to_string(it->second);
            if (std::next(it) != lumGates.end()) {
                ss << ", ";
            }
        }
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP lum_gates: " + ss.str());
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLumGates] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLumGates] Caught an unknown exception!");
    }
}

/** Handles id map being delivered. */
void handleIdMap(std::string data) {
    try {
        // Parses the id map from the archipelago input
        idMap.clear();
        if (data.length() < 3) return;
        std::string inner = data.substr(1, data.length() - 2);
        std::stringstream stream(inner);
        std::string token;
        while (std::getline(stream, token, ',')) {
            size_t colonPos = token.find(":");
            if (colonPos == std::string::npos) continue;
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            idMap[std::stol(key.substr(1, key.length() - 2))] = value.substr(1, value.length() - 2);
        }

        // Debug print to the log
        std::stringstream ss;
        for (auto it = idMap.begin(); it != idMap.end(); ++it) {
            ss << std::to_string(it->first) << " = " << it->second;
            if (std::next(it) != idMap.end()) {
                ss << ", ";
            }
        }
        instance->send(MESSAGE_TYPE_MESSAGE, "[child] AP id_map: " + ss.str());
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleIdMap] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleIdMap] Caught an unknown exception!");
    }
}

/** Handles an incoming death link from other games. */
void handleDeathLink() {
    instance->send(MESSAGE_TYPE_DEATH, "");
}

bool Connector::connect(std::string ip, std::string slot, std::string password) {
    if (AP_IsInit()) return false;
    lastIp = ip;
    instance->send(MESSAGE_TYPE_MESSAGE, "[child] Connecting to ip: " + ip + ", game: Rayman 2, slot: " + slot + ", password: " + password);
    AP_Init(ip.c_str(), "Rayman 2", slot.c_str(), password.c_str());
    AP_SetDeathLinkSupported(true);
    AP_SetItemClearCallback(handleItemClear);
    AP_SetItemRecvCallback(handleItem);
    AP_SetLocationCheckedCallback(handleLocation);
    AP_SetDeathLinkRecvCallback(handleDeathLink);
    AP_RegisterSlotDataRawCallback("level_swaps", handleLevelSwaps);
    AP_RegisterSlotDataRawCallback("lum_gates", handleLumGates);
    AP_RegisterSlotDataRawCallback("id_map", handleIdMap);
    AP_Start();
    return true;
}

bool Connector::disconnect() {
    if (!AP_IsInit()) return false;
    AP_Shutdown();
    unlockedChecks.clear();
    levelSwaps.clear();
    lumGates.clear();
    idMap.clear();
    return true;
}

bool Connector::isConnected() {
    return AP_IsInit();
}