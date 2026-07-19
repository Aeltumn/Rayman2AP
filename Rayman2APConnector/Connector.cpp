#include "Connector.h"
#include <iomanip>
#include <unordered_map>
#include <thread>
#include <windows.h>
#include <algorithm>

Connector *instance;

// Current archipelago item state
int lums = 0;
int cages = 0;
int masks = 0;
int upgrades = 0;
bool elixir = false;
bool knowledge = false;
bool fragmented = false;
bool hover = true;
bool ledge = true;
bool swim = true;
bool lavaHover = true;

// Current archipelago settings
bool deathLink = false;
int endGoal = 1;
bool lumsanity = false;
bool roomRandomisation = false;
bool accessiblePortals = false;
int deathLinkAmnesty = 1;
bool betterLevelPortals = false;
int lumBundleSize = 0;
bool damageLink = false;
int previousCommunicatedLumBundle = 0;
int lumGates[6] = {100, 300, 475, 550, 60, 450};
std::unordered_map<std::string, std::vector<std::string>> levelChains;
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
                if (message->type == AP_MessageType::Chat) {
                    AP_ChatMessage* chatMessage = static_cast<AP_ChatMessage*>(message);
                    std::string formattedMessage = "/o400:" + chatMessage->player + " /o0:- " + chatMessage->message;
					instance->send(MESSAGE_TYPE_CHAT, formattedMessage);
                } else if (message->type == AP_MessageType::ServerChat) {
                    AP_ServerChatMessage* serverChatMessage = static_cast<AP_ServerChatMessage*>(message);
					std::string formattedMessage = "/o200:Server /o0:- " + serverChatMessage->message;
                    instance->send(MESSAGE_TYPE_CHAT, formattedMessage);
                } else {
                    instance->send(MESSAGE_TYPE_MESSAGE, message->text);
                }
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
        send(MESSAGE_TYPE_MESSAGE, data);
		break;
    }
    case MESSAGE_TYPE_CHAT: {
        // Send the chat message up to Archipelago server.
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
        } else if (!connect(result[0], result[1], result.size() > 2 ? result[2] : "")) {
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
            send(MESSAGE_TYPE_MESSAGE, "Successfully disconnected from " + lastIp);
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
    case MESSAGE_TYPE_LUMS: {
        // Communicate up that we got a lum bundle if applicable.
        if (isConnected() && lumBundleSize > 1) {
            // Send back down that we received the data
            int value = atoi(data.c_str());
            int lumBundlesObtained = value / lumBundleSize;
            if (value >= 710 && (value % lumBundleSize) != 0) {
                // Send the leftover lum bundle!
                AP_SendItem(1653615);
            }

            // Send any regular lum bundles!
            while (lumBundlesObtained > previousCommunicatedLumBundle) {
                AP_SendItem(1653616 + previousCommunicatedLumBundle);
                previousCommunicatedLumBundle++;
            }
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
    oss << fragmented << ",";
    oss << hover << ",";
    oss << ledge << ",";
    oss << swim << ",";
    oss << lavaHover << ",";
    instance->send(MESSAGE_TYPE_STATE, oss.str());
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
    oss << accessiblePortals << ",";
    oss << deathLinkAmnesty << ",";
    oss << betterLevelPortals << ",";
    oss << lumBundleSize << ",";
    oss << damageLink << ",";

    for (int i = 0; i < 6; i++) {
        oss << lumGates[i] << ",";
    }

    // Sort the level ids alphabetically so the game can just use indices
    std::vector<std::string> keys;
    keys.reserve(levelChains.size());
    for (const auto& pair : levelChains) {
        keys.push_back(pair.first);
    }
    std::sort(keys.begin(), keys.end());

    for (const auto& key : keys) {
        for (const auto second : levelChains[key]) {
            oss << second << "|";
        }
        oss << ";";
    }
    instance->send(MESSAGE_TYPE_SETTINGS, oss.str());
}


/** Handles clearing cached item checks. */
void handleItemClear() {
    lums = 0;
    cages = 0;
    upgrades = 0;
    masks = 0;
    elixir = false;
    knowledge = false;
    fragmented = false;
    hover = true;
    ledge = true;
    swim = true;
    lavaHover = true;
    previousCommunicatedLumBundle = 0;
    sendStateUpdate(connected);
}

/** Resets received settings. */
void handleReset() {
    bool wasConnected = connected;
    connected = false;
    deathLink = false;
    damageLink = false;
    endGoal = 1;
    lumsanity = false;
    roomRandomisation = false;
    accessiblePortals = false;
    deathLinkAmnesty = 1;
    betterLevelPortals = false;
    lumBundleSize = 0;
    lumGates[0] = 100;
    lumGates[1] = 300;
    lumGates[2] = 475;
    lumGates[3] = 550;
    lumGates[4] = 60;
    lumGates[5] = 450;
    levelChains.clear();
    sendSettings(wasConnected);
}

/** Handles an item being checked. */
void handleItem(int64_t id, bool notify) {
    // Parse the custom item ids into the actual item types and handle them.
    const char* type;
    switch (id) {
    case 1651615:
        type = "Lum";
        lums++;
        break;
    case 1651616:
        type = "Super Lum";
        lums += 5;
        break;
    case 1651617:
        type = "Cage";
        cages++;
        break;
    case 1651618:
        type = "Water Mask";
        masks++;
        break;
    case 1651619:
        type = "Earth Mask";
        masks++;
        break;
    case 1651620:
        type = "Fire Mask";
        masks++;
        break;
    case 1651621:
        type = "Air Mask";
        masks++;
        break;
    case 1651622:
        type = "Silver Lum";
        upgrades++;
        break;
    case 1651623:
        type = "Elixir of Life";
        elixir = true;
        break;
    case 1651624:
        type = "Knowledge of the Cave of Bad Dreams";
        knowledge = true;
        break;
    case 1651625:
        type = "Lum Bundle";
        lums += lumBundleSize;
        break;
    case 1651626:
        type = "Leftover Lum Bundle";
        lums += 710 % lumBundleSize;
        break;
    case 1651627:
        type = "Hover";
        hover = true;
        break;
    case 1651628:
        type = "Ledge Grab";
        ledge = true;
        break;
    case 1651629:
        type = "Swim";
        swim = true;
        break;
    case 1651630:
        type = "Lava Hover";
        lavaHover = true;
        break;
    case 1651631:
        type = "Fairy Glade Revisit Swing";
        upgrades &= 1;
        break;
    case 1651632:
        type = "Cave of Bad Dreams 1 Swings";
        upgrades &= 2;
        break;
    case 1651633:
        type = "Cave of Bad Dreams 2 Swings";
        upgrades &= 4;
        break;
    case 1651634:
        type = "Stone and Fire Side Temple Swing";
        upgrades &= 8;
        break;
    case 1651635:
        type = "Fairy Glade 4 Swing";
        upgrades &= 16;
        break;
    case 1651636:
        type = "Fairy Glade 5 Swing";
        upgrades &= 32;
        break;
    case 1651637:
        type = "Bayou 1 Swings";
        upgrades &= 64;
        break;
    case 1651638:
        type = "Bayou 2 Swing";
        upgrades &= 128;
        break;
    case 1651639:
        type = "Water and Ice 2 Swings";
        upgrades &= 256;
        break;
    case 1651640:
        type = "Menhir Hills 2 Swings";
        upgrades &= 512;
        break;
    case 1651641:
        type = "Menhir Hills 3 Swing";
        upgrades &= 1024;
        break;
    case 1651642:
        type = "Canopy 3 Swing";
        upgrades &= 2048;
        break;
    case 1651643:
        type = "Whale Bay 1 Swing";
        upgrades &= 4096;
        break;
    case 1651644:
        type = "Stone and Fire 1 Swings";
        upgrades &= 8192;
        break;
    case 1651645:
        type = "Stone and Fire 2 Swings";
        upgrades &= 16384;
        break;
    case 1651646:
        type = "Precipice 1 Swings";
        upgrades &= 32768;
        break;
    case 1651647:
        type = "Rock and Lava 1 Swing";
        upgrades &= 65536;
        break;
    case 1651648:
        type = "Beneath Rock and Lava 3 Swing";
        upgrades &= 131072;
        break;
    case 1651649:
        type = "Tomb of the Ancients 2 Swings";
        upgrades &= 262144;
        break;
    case 1651650:
        type = "Iron Mountains 1 Swings";
        upgrades &= 524288;
        break;
    case 1651651:
        type = "Iron Mountains 3 Swings";
        upgrades &= 1048576;
        break;
    case 1651652:
        type = "Powered Shots";
        upgrades &= 2097152;
        break;
    default:
        // The item type is invalid, send a debug log!
        instance->send(MESSAGE_TYPE_MESSAGE, "Received invalid item: " + std::to_string(id));
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

/** Handles level chain data being delivered. */
void handleLevelChains(std::string data) {
    try {
        // Parses the level swaps from the archipelago input
        levelChains.clear();
        if (data.length() < 4) return;
        std::string inner = data.substr(1, data.length() - 3);
        std::stringstream stream(inner);
        std::string token;
        bool first = true;
        while (std::getline(stream, token, ']')) {
            size_t colonPos = token.find(":");
            if (colonPos == std::string::npos) continue;
            std::string level = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            std::vector<std::string> levels;

            std::stringstream innerStream(value.substr(1, value.length() - 1));
            std::string innerToken;
            while (std::getline(innerStream, innerToken, ',')) {
                levels.push_back(innerToken.substr(1, innerToken.length() - 2));
            }

            auto levelId = level.substr(first ? 1 : 2, first ? level.length() - 2 : level.length() - 3);
            levelChains[levelId] = levels;
            first = false;
        }
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLevelChains] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLevelChains] Caught an unknown exception!");
    }
}

/** Handles lum gate thresholds being delivered. */
void handleLumGates(std::string data) {
    try {
        // Parse the lum gates from the archipelago input
        if (data.length() < 4) return;
        std::string inner = data.substr(1, data.length() - 3);
        std::stringstream stream(inner);
        std::string token;
        int i = 0;
        while (std::getline(stream, token, ',') && i < 6) {
            lumGates[i++] = std::stoi(token);
        }
    } catch (const std::exception& e) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLumGates] Caught exception: " + std::string(e.what()));
    } catch (...) {
        instance->send(MESSAGE_TYPE_MESSAGE, "[handleLumGates] Caught an unknown exception!");
    }
}

/** Handles slot information ingestion. */
void handleDeathLinkEnabled(std::string data) {
    deathLink = std::stoi(data) == 1;
    sendSettings(false);
}
void handleDamageLink(std::string data) {
    damageLink = std::stoi(data) == 1;
    sendSettings(false);
}
void handleAutomaticMovement(std::string data) {
    auto value = std::stoi(data);
    if ((value & 1) > 1) hover = true;
    if ((value & 2) > 1) swim = true;
    if ((value & 4) > 1) ledge = true;
    if ((value & 8) > 1) lavaHover = true;
    sendStateUpdate(false);
}
void handleFragmentedLums(std::string data) {
    fragmented = std::stoi(data) == 1;
    sendStateUpdate(false);
}
void handleDeathLinkAmnesty(std::string data) {
    deathLinkAmnesty = std::stoi(data) == 1;
    sendSettings(false);
}
void handleBetterLevelPortals(std::string data) {
    betterLevelPortals = std::stoi(data) == 1;
    sendSettings(false);
}
void handleLumBundleSize(std::string data) {
    lumBundleSize = std::stoi(data);
    sendSettings(false);
}
void handleEndGoal(std::string data) {
    endGoal = std::stoi(data);
    sendSettings(false);
}
void handleLumsanity(std::string data) {
    lumsanity = std::stoi(data);
    sendSettings(false);
    sendStateUpdate(false);
}
void handleAccessiblePortals(std::string data) {
    accessiblePortals = std::stoi(data);
    sendSettings(false);
}
void handleRoomRandomisation(std::string data) {
    roomRandomisation = std::stoi(data);

    // Only now do we call it connected!
    if (!connected) {
        connected = true;
        instance->send(MESSAGE_TYPE_MESSAGE, "Successfully connected to Archipelago server!");
    }
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
    AP_RegisterSlotDataRawCallback("level_chains", handleLevelChains);
    AP_RegisterSlotDataRawCallback("lum_gates", handleLumGates);
    AP_RegisterSlotDataRawCallback("death_link", handleDeathLinkEnabled);
    AP_RegisterSlotDataRawCallback("death_link_amnesty", handleDeathLinkAmnesty);
    AP_RegisterSlotDataRawCallback("better_level_portals", handleBetterLevelPortals);
    AP_RegisterSlotDataRawCallback("end_goal", handleEndGoal);
    AP_RegisterSlotDataRawCallback("room_randomisation", handleRoomRandomisation);
    AP_RegisterSlotDataRawCallback("accessible_portals", handleAccessiblePortals);
    AP_RegisterSlotDataRawCallback("lumsanity", handleLumsanity);
    AP_RegisterSlotDataRawCallback("lum_bundle_size", handleLumBundleSize);
    AP_RegisterSlotDataRawCallback("damage_link", handleDamageLink);
    AP_RegisterSlotDataRawCallback("fragmented_lums", handleFragmentedLums);
    AP_RegisterSlotDataRawCallback("automatic_movement", handleAutomaticMovement);
    AP_Start();
    return true;
}

bool Connector::disconnect() {
    if (!AP_IsInit()) return false;
    AP_Shutdown();
    handleItemClear();
    handleReset();
    instance->send(MESSAGE_TYPE_RESET, "");
    return true;
}

bool Connector::isConnected() {
    return AP_IsInit();
}

void Connector::init() {
    // Store the instance so we can use it in the death link
    instance = this;

    // Send an initial message to inform the client
    instance->send(MESSAGE_TYPE_MESSAGE, "Rayman2APConnector v" + std::string(CURRENT_VERSION) + " has started and is ready to use");

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
            char indicator;
            DWORD bytesRead;
            
            if (!ReadFile(hStdIn, &indicator, 1, &bytesRead, NULL)) {
                throw std::runtime_error("Failed to read message");
            }
            if (bytesRead == 0) continue;

            // Ignore all input until we find our special character!
            if (indicator != (char)26) continue;

            // Read the length of the message that will follow
            char lengthChar[7];
            if (!ReadFile(hStdIn, lengthChar, 6, &bytesRead, NULL)) {
                throw std::runtime_error("Failed to read message");
            }
            if (bytesRead == 0) continue;
            lengthChar[6] = '\0';
            int messageLength = atoi(lengthChar);
            if (messageLength < 0) continue;

            // Determine the type of the message
            char typeChar;
            if (!ReadFile(hStdIn, &typeChar, 1, &bytesRead, NULL)) {
                throw std::runtime_error("Failed to read message");
            }
            if (bytesRead == 0) continue;
            int type = typeChar - '0';

            // Read out the message
            char* messageBuffer = static_cast<char*>(std::malloc(messageLength + 1));
            if (!messageBuffer) {
                throw std::runtime_error("Failed to allocate memory for incoming message");
            }
            if (messageLength > 0) {
                if (!ReadFile(hStdIn, messageBuffer, messageLength, &bytesRead, NULL)) {
                    throw std::runtime_error("Failed to read message");
                }
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