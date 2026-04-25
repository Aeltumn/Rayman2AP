#include "Connector.h"
#include <iomanip>
#include <unordered_map>
#include <thread>
#include <windows.h>

// Stores the ids of all super lums.
const int COBD_KNOWLEDGE_ID = 1101;
const int ELIXIR_ID = 1123;
const int MASK_IDS[] = { 1112, 1113, 1114, 1115 };
const int SILVER_LUM_IDS[] = { 1095, 1143 };
const int SUPER_LUM_IDS[] = { 762, 776, 781, 786, 791, 796, 1, 13, 19, 66, 81, 86, 91, 71, 61, 51, 96, 172, 161, 206, 201, 211, 292, 315, 333, 328, 310, 364, 359, 369, 380, 375, 406, 401, 416, 411, 491, 496, 556, 646, 618, 613, 631, 636, 686, 681, 661, 666, 671, 676, 721, 736, 731, 746, 741, 1354, 1389, 1311 };
const int THOUSANDTH_LUM_ID = 1014;

Connector *instance;

// Current archipelago item state
int lums = 0;
int cages = 0;
int masks = 0;
int upgrades = 0;
bool elixir = false;
bool knowledge = false;

// Current archipelago settings
bool deathLink = false;
int endGoal = 1;
bool lumsanity = false;
bool roomRandomisation = false;
int lumGates[6] = {100, 300, 475, 550, 60, 450};
std::unordered_map<std::string, std::string> levelSwaps;
std::string lastIp;
bool connected = false;
std::chrono::steady_clock::time_point connectStart;
std::chrono::seconds timeout = std::chrono::seconds(5);

// Dummy method so we don't have to add the gifting module which requires C++17
void handleGiftAPISetReply(const AP_SetReply& reply) {}

/** Prints the current connection status. */
void printConnectionStatus() {
    if (!AP_IsInit()) {
        instance->send(MESSAGE_TYPE_MESSAGE, "AP Status: Not Connected");
    } else {
        if (!connected) {
            instance->send(MESSAGE_TYPE_MESSAGE, "AP Status: Connecting");
        } else {
            switch (AP_GetConnectionStatus()) {
            case AP_ConnectionStatus::Disconnected:
                instance->send(MESSAGE_TYPE_MESSAGE, "AP Status: Disconnected");
                break;
            case AP_ConnectionStatus::Connected:
                instance->send(MESSAGE_TYPE_MESSAGE, "AP Status: Connected");
                break;
            case AP_ConnectionStatus::Authenticated:
                instance->send(MESSAGE_TYPE_MESSAGE, "AP Status: Authenticated");
                break;
            case AP_ConnectionStatus::ConnectionRefused:
                instance->send(MESSAGE_TYPE_MESSAGE, "AP Status: Connection Refused");
                break;
            }
        }
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
            if (AP_IsInit() && !connected) {
                // Wait for at most 5 seconds for the connection to establish, otherwise
                // we deem it failed and force a disconnect!
                auto now = std::chrono::steady_clock::now();
                auto elapsed = now - connectStart;
                if (elapsed >= timeout) {
                    instance->send(MESSAGE_TYPE_MESSAGE, "Connection failed: host timed out");
                    disconnect();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[waitForAP] Caught exception: " + std::string(e.what()));
    }
    catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[waitForAP] Caught an unknown exception!");
    }
}

void Connector::handle(int type, std::string data) {
    switch (type) {
    case MESSAGE_TYPE_DEATH: {
        // Trigger a death link!
        AP_DeathLinkSend();
        break;
    }
    case MESSAGE_TYPE_MESSAGE: {
        // Send the message up to Archipelago server.
        AP_Say(data);
        break;
    }
    case MESSAGE_TYPE_CONNECT: {
        // Parse out the input to find the separate arguments
        std::vector<std::string> result;
        std::string current;
        bool inQuotes = false;
        for (size_t i = 0; i < data.size(); ++i) {
            char c = data[i];

            if (c == '"') {
                inQuotes = !inQuotes;
                continue;
            }

            if (!inQuotes && std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    result.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            result.push_back(current);
        }

        // Process the input and start connecting
        if (result.size() < 2) {
            send(MESSAGE_TYPE_MESSAGE, "Usage: ap connect <ip> <slot> [password]");
        } else if (!connect(result[0], result[1], result.size() >= 2 ? "" : result[2])) {
            send(MESSAGE_TYPE_MESSAGE, "You are already connected to an Archipelago server");
        }
        break;
    }
    case MESSAGE_TYPE_SHUTDOWN: {
        // Disconnect if connected, used on shutdown.
        if (isConnected()) {
            disconnect();
        }
        send(MESSAGE_TYPE_MESSAGE, "Shutdown completed");
        break;
    }
    case MESSAGE_TYPE_DISCONNECT: {
        // Disconnect from the Archipelago server.
        if (disconnect()) {
            send(MESSAGE_TYPE_MESSAGE, "Succesfully disconnected from " + lastIp);
        }
        else {
            send(MESSAGE_TYPE_MESSAGE, "You are not connected to any Archipelago server");
        }
        break;
    }
    case MESSAGE_TYPE_CHECK: {
        // Check if the connection is valid.
        if (isConnected()) {
            send(MESSAGE_TYPE_MESSAGE, "You are currently connected to " + lastIp);
            printConnectionStatus();
        }
        else {
            send(MESSAGE_TYPE_MESSAGE, "You are not connected to any Archipelago server");
        }
        break;
    }
    case MESSAGE_TYPE_COLLECTED: {
        // Communicate up that an item is collected.
        if (isConnected()) {
            // Send back down that we received the data
            int value = atoi(data.c_str());
            AP_SendItem(1651615 + value);
        }
        break;
    }
    case MESSAGE_TYPE_COMPLETE: {
        // The game is done; pass it on!
        if (isConnected()) {
            AP_StoryComplete();
        }
        break;
    }
    default: {
        send(MESSAGE_TYPE_MESSAGE, "Received unknown " + std::to_string(type) + ": " + data);
        break;
    }
    }
}

/** Sends a state update to the game client. */
void sendStateUpdate(bool force) {
    if (!force && !instance->isConnected()) return;

    std::ostringstream oss;
    oss << lums << ",";
    oss << cages << ",";
    oss << masks << ",";
    oss << upgrades << ",";
    oss << elixir << ",";
    oss << knowledge << ",";
    instance->send(MESSAGE_TYPE_STATE, oss.str());
    instance->send(MESSAGE_TYPE_MESSAGE, "Sent: `state`");
}

/** Sends the current settings to game client. */
void sendSettings(bool force) {
    if (!force && !instance->isConnected()) return;

    std::ostringstream oss;
    oss << instance->isConnected() << ",";
    oss << deathLink << ",";
    oss << endGoal << ",";
    oss << lumsanity << ",";
    oss << roomRandomisation << ",";

    for (int i = 0; i < 6; i++) {
        oss << lumGates[i] << ",";
    }

    for (const auto& pair : levelSwaps) {
        oss << pair.first << "|" << pair.second << ";";
    }
    instance->send(MESSAGE_TYPE_SETTINGS, oss.str());
    instance->send(MESSAGE_TYPE_MESSAGE, "Sent: `settings`");
}


/** Handles clearing cached item checks. */
void handleItemClear() {
    lums = 0;
    cages = 0;
    upgrades = 0;
    masks = 0;
    elixir = false;
    knowledge = false;
    sendStateUpdate(connected);
}

/** Resets received settings. */
void handleReset() {
    bool wasConnected = connected;
    connected = false;
    deathLink = false;
    endGoal = 1;
    lumsanity = false;
    roomRandomisation = false;
    lumGates[0] = 100;
    lumGates[1] = 300;
    lumGates[2] = 475;
    lumGates[3] = 550;
    lumGates[4] = 60;
    lumGates[5] = 450;
    levelSwaps.clear();
    sendSettings(wasConnected);
}

/** Handles an item being checked. */
void handleItem(int64_t id, bool notify) {
    // Archipelago increments all ids by 1651615, so we subtract that to get the ID used
    // by Rayman 2.
    int64_t r2Id = id - 1651615;

    // Determine what type of item was collected
    const char* type;
    if (r2Id == ELIXIR_ID) {
        type = "Elixir of Life";
        elixir = true;
    } else if (std::find(std::begin(MASK_IDS), std::end(MASK_IDS), r2Id) != std::end(MASK_IDS)) {
        type = "Mask";
        masks++;
    } else if (std::find(std::begin(SILVER_LUM_IDS), std::end(SILVER_LUM_IDS), r2Id) != std::end(SILVER_LUM_IDS)) {
        type = "Silver Lum";
        upgrades++;
    } else if (r2Id >= 840 && r2Id <= 919) {
        type = "Cage";
        cages++;
    } else if ((r2Id >= 1 && r2Id <= 800) || (r2Id >= 1201 && r2Id <= 1400)) {
        if (std::find(std::begin(SUPER_LUM_IDS), std::end(SUPER_LUM_IDS), r2Id) != std::end(SUPER_LUM_IDS)) {
            type = "Super Lum";
            lums += 5;
        } else {
            type = "Lum";
            lums++;
        }
    } else if (r2Id == COBD_KNOWLEDGE_ID) {
        type = "Knowledge of the Cave of Bad Dreams";
        knowledge = true;
    } else if (r2Id == THOUSANDTH_LUM_ID) {
        type = "1000th Lum";
        lums++;
    } else {
        // The item type is invalid, send a debug log!
        instance->send(MESSAGE_TYPE_MESSAGE, "Received invalid item: " + std::to_string(r2Id));
        return;
    }

    // Send an update to the client with the new information
    sendStateUpdate(false);

    // If we have to notify the player we send a second message with the information that
    // they received the item externally!
    if (notify) {
        instance->send(MESSAGE_TYPE_COLLECTED, std::string("Received ") + type);
    }
}

/** Handles a location being checked. */
void handleLocation(int64_t id) {
    // We currently don't support checking off locations.
}

/** Handles level swap data being delivered. */
void handleLevelSwaps(std::string data) {
    instance->send(MESSAGE_TYPE_MESSAGE, "Receive: `level_swaps`");
    try {
        // Parses the level swaps from the archipelago input
        levelSwaps.clear();
        if (data.length() < 4) return;
        std::string inner = data.substr(1, data.length() - 3);
        std::stringstream stream(inner);
        std::string token;
        while (std::getline(stream, token, ',')) {
            size_t colonPos = token.find(":");
            if (colonPos == std::string::npos) continue;
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            levelSwaps[key.substr(1, key.length() - 2)] = value.substr(1, value.length() - 2);
        }
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLevelSwaps] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLevelSwaps] Caught an unknown exception!");
    }
}

/** Handles lum gate thresholds being delivered. */
void handleLumGates(std::string data) {
    instance->send(MESSAGE_TYPE_MESSAGE, "Receive: `lum_gates`");
    try {
        // Parse the lum gates from the archipelago input
        if (data.length() < 3) return;
        std::string inner = data.substr(1, data.length() - 2);
        std::stringstream stream(inner);
        std::string token;
        int i = 0;
        while (std::getline(stream, token, ',')) {
            lumGates[i++] = std::stoi(token);
        }
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLumGates] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLumGates] Caught an unknown exception!");
    }
}

/** Handles information on whether death link is enabled. */
void handleDeathLinkEnabled(std::string data) {
    instance->send(MESSAGE_TYPE_MESSAGE, "Receive: `death_link`");
    deathLink = std::stoi(data) == 1;
    sendSettings(false);
}

/** Handles information on the selected end goal. */
void handleEndGoal(std::string data) {
    instance->send(MESSAGE_TYPE_MESSAGE, "Receive: `end_goal`");
    endGoal = std::stoi(data);

    // Only now do we call it connected!
    if (!connected) {
        connected = true;
        instance->send(MESSAGE_TYPE_MESSAGE, "Succesfully connected to Archipelago server!");
    }

    // Send settings after everything is updated
    sendSettings(false);
}

/** Handles information on whether lumsanity is being used. */
void handleLumsanity(std::string data) {
    instance->send(MESSAGE_TYPE_MESSAGE, "Receive: `lumsanity`");
    lumsanity = std::stoi(data);
    sendSettings(false);
    sendStateUpdate(false);
}

/** Handles information on whether room randomisation is on. */
void handleRoomRandomisation(std::string data) {
    instance->send(MESSAGE_TYPE_MESSAGE, "Receive: `room_randomisation`");
    roomRandomisation = std::stoi(data);
    sendSettings(false);
}

/** Handles an incoming death link from other games. */
void handleDeathLink() {
    instance->send(MESSAGE_TYPE_DEATH, "");
}

bool Connector::connect(std::string ip, std::string slot, std::string password) {
    if (AP_IsInit()) {
        // If we are properly connected require a formal disconnect, otherwise
        // we internally disconnect and let you switch IPs.
        if (connected) return false;
        disconnect();
    }

    connectStart = std::chrono::steady_clock::now();
    lastIp = ip;
    instance->send(MESSAGE_TYPE_MESSAGE, "Attempting to connect to `" + ip + "` on slot `" + slot + "`...");
    AP_Init(ip.c_str(), "Rayman 2", slot.c_str(), password.c_str());
    AP_SetDeathLinkSupported(true);
    AP_SetItemClearCallback(handleItemClear);
    AP_SetItemRecvCallback(handleItem);
    AP_SetLocationCheckedCallback(handleLocation);
    AP_SetDeathLinkRecvCallback(handleDeathLink);
    AP_RegisterSlotDataRawCallback("level_swaps", handleLevelSwaps);
    AP_RegisterSlotDataRawCallback("lum_gates", handleLumGates);
    AP_RegisterSlotDataRawCallback("death_link", handleDeathLinkEnabled);
    AP_RegisterSlotDataRawCallback("end_goal", handleEndGoal);
    AP_RegisterSlotDataRawCallback("lumsanity", handleLumsanity);
    AP_RegisterSlotDataRawCallback("room_randomisation", handleRoomRandomisation);
    AP_Start();
    return true;
}

bool Connector::disconnect() {
    if (!AP_IsInit()) return false;
    AP_Shutdown();
    handleItemClear();
    handleReset();
    return true;
}

bool Connector::isConnected() {
    return AP_IsInit();
}

void Connector::init() {
    // Store the instance so we can use it in the death link
    instance = this;

    // Send an initial message to inform the client
    instance->send(MESSAGE_TYPE_MESSAGE, "Rayman2APConnector has started and is ready to use");

    // Immediately send a state update so lum gates are set on boot
    sendSettings(true);
    sendStateUpdate(true);
}

void Connector::waitForInput() {
    try {
        // Get the std::in handle
        HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
        if (!hStdIn) {
            throw std::runtime_error("Could not get std::in handle");
        }

        while (true) {
            // Start reading from the child process's stdout
            char buffer[1];
            DWORD bytesRead;
            while (ReadFile(hStdIn, buffer, 1, &bytesRead, NULL) && bytesRead > 0) {
                // Ignore all input until we find our special character!
                if (buffer[0] != (char)26) continue;

                // Read the length of the message that will follow
                char lengthChar[7];
                ReadFile(hStdIn, lengthChar, 6, &bytesRead, NULL);
                if (bytesRead == 0) continue;
                lengthChar[6] = '\0';
                int messageLength = atoi(lengthChar);
                if (messageLength < 0) continue;

                // Determine the type of the message
                char typeChar;
                ReadFile(hStdIn, &typeChar, 1, &bytesRead, NULL);
                if (bytesRead == 0) continue;
                int type = typeChar - '0';

                // Read out the message
                char* messageBuffer = static_cast<char*>(std::malloc(messageLength + 1));
                if (!messageBuffer) {
                    throw std::runtime_error("Failed to allocate memory for incoming message");
                    continue;
                }
                if (messageLength > 0) {
                    ReadFile(hStdIn, messageBuffer, messageLength, &bytesRead, NULL);
                    if (bytesRead == 0) {
                        free(messageBuffer);
                        continue;
                    }
                }
                messageBuffer[messageLength] = '\0';
                handle(type, std::string(messageBuffer));
                std::free(messageBuffer);
            }
        }
    }
    catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[waitForInput] Caught exception: " + std::string(e.what()));
    }
    catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[waitForInput] Caught an unknown exception!");
    }
}

/** Returns the number as a padded string. */
std::string paddedString(int num) {
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << num;
    return oss.str();
}

/** Sends the given data of the given type to the game. */
void Connector::send(int type, std::string data) {
    DWORD bytesWritten;
    int length = data.length();
    std::string message;
    message.reserve(length + 8);

    message.push_back(26);
    message += paddedString(length);
    message.push_back(type + '0');
    if (length > 0) {
        message += data;
    }

    // Get the std::out handle
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!hStdOut || !WriteFile(hStdOut, message.data(), length + 8, &bytesWritten, NULL)) {
        throw std::runtime_error("Encountered error while writing to game " + std::to_string(GetLastError()));
    }
}