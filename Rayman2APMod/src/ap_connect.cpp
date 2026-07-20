#include "ap_connect.h"
#include "mod.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <thread>
#include <windows.h>
#include <algorithm>
#include "../../APCpp/Archipelago.h"

// Current Archipelago item state
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

// Current Archipelago settings
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
int lumGates[6] = { 100, 300, 475, 550, 60, 450 };
std::unordered_map<std::string, std::vector<std::string>> levelChains;
std::string lastIp;
bool connected = false;
std::chrono::steady_clock::time_point connectStart;
std::chrono::seconds timeout = std::chrono::seconds(5);

const char* levelIds[LEVEL_COUNT];
int levelChainLengths[CHAIN_COUNT];
int* levelChainContents[CHAIN_COUNT];

/** Sends a state update to the game client. */
void sendStateUpdate(bool force) {
    if (!force && !AP_IsConnected()) return;
    MOD_UpdateState(lums, cages, masks, upgrades, elixir, knowledge, fragmented, hover, ledge, swim, lavaHover);
}

/** Sends the current settings to game client. */
void sendSettings(bool force) {
    if (!force && !AP_IsConnected()) return;
    MOD_UpdateSettings(AP_IsConnected(), deathLink, damageLink, endGoal, lumsanity, roomRandomisation, accessiblePortals, deathLinkAmnesty, betterLevelPortals, lumBundleSize, lumGates, levelIds, levelChainLengths, levelChainContents);
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
    for (int i = 0; i < LEVEL_COUNT; i++) {
        levelIds[i] = "";
    }
    for (int i = 0; i < CHAIN_COUNT; i++) {
        levelChainLengths[i] = 0;
        levelChainContents[i] = NULL;
    }
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
        upgrades |= 1;
        break;
    case 1651632:
        type = "Cave of Bad Dreams 1 Swings";
        upgrades |= 2;
        break;
    case 1651633:
        type = "Cave of Bad Dreams 2 Swings";
        upgrades |= 4;
        break;
    case 1651634:
        type = "Stone and Fire Side Temple Swing";
        upgrades |= 8;
        break;
    case 1651635:
        type = "Fairy Glade 4 Swing";
        upgrades |= 16;
        break;
    case 1651636:
        type = "Fairy Glade 5 Swing";
        upgrades |= 32;
        break;
    case 1651637:
        type = "Bayou 1 Swings";
        upgrades |= 64;
        break;
    case 1651638:
        type = "Bayou 2 Swing";
        upgrades |= 128;
        break;
    case 1651639:
        type = "Water and Ice 2 Swings";
        upgrades |= 256;
        break;
    case 1651640:
        type = "Menhir Hills 2 Swings";
        upgrades |= 512;
        break;
    case 1651641:
        type = "Menhir Hills 3 Swing";
        upgrades |= 1024;
        break;
    case 1651642:
        type = "Canopy 3 Swing";
        upgrades |= 2048;
        break;
    case 1651643:
        type = "Whale Bay 1 Swing";
        upgrades |= 4096;
        break;
    case 1651644:
        type = "Stone and Fire 1 Swings";
        upgrades |= 8192;
        break;
    case 1651645:
        type = "Stone and Fire 2 Swings";
        upgrades |= 16384;
        break;
    case 1651646:
        type = "Precipice 1 Swings";
        upgrades |= 32768;
        break;
    case 1651647:
        type = "Rock and Lava 1 Swing";
        upgrades |= 65536;
        break;
    case 1651648:
        type = "Beneath Rock and Lava 3 Swing";
        upgrades |= 131072;
        break;
    case 1651649:
        type = "Tomb of the Ancients 2 Swings";
        upgrades |= 262144;
        break;
    case 1651650:
        type = "Iron Mountains 1 Swings";
        upgrades |= 524288;
        break;
    case 1651651:
        type = "Iron Mountains 3 Swings";
        upgrades |= 1048576;
        break;
    case 1651652:
        type = "Powered Shots";
        upgrades |= 2097152;
        break;
    default:
        // The item type is invalid, send a debug log!
        MOD_Print("Received invalid item: %d", id);
        return;
    }

    // Send an update to the client with the new information
    sendStateUpdate(false);

    // If we have to notify the player we send a second message with the information that
    // they received the item externally!
    if (notify) {
        MOD_Notify((std::string("Received ") + type).c_str());
    }
}

/** Handles a location being checked. */
void handleLocation(int64_t id) {
    // We currently don't support checking off locations.
}

/** Handles level chain data being delivered. */
void handleLevelChains(std::string data) {
    // Parses the level swaps from the archipelago input
    try {
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
        MOD_Print("[handleLevelChains] Caught exception: %s", e.what());
    } catch (...) {
        MOD_Print("[handleLevelChains] Caught an unknown exception!");
    }

    // Sort the level ids alphabetically so the game can just index from there
    std::vector<std::string> keys;
    keys.reserve(levelChains.size());
    for (const auto& pair : levelChains) {
        keys.push_back(pair.first);
    }
    std::sort(keys.begin(), keys.end());

    int chainId = 0;
    int levelId = 0;
    for (const auto& key : keys) {
        auto& chain = levelChains[key];
        int* chainArray = levelChainContents[chainId];
        int chainLength = 0;
        for (const auto& second : chain) {
            chainLength++;
            chainArray = (int*) realloc(chainArray, chainLength * sizeof(int));

            levelIds[levelId] = second.c_str();
            chainArray[chainLength - 1] = levelId;
            levelId++;
        }
        levelChainContents[chainId] = chainArray;
        levelChainLengths[chainId] = chainLength;
        chainId++;
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
        MOD_Print("[handleLumGates] Caught exception: %s", e.what());
    } catch (...) {
        MOD_Print("[handleLumGates] Caught an unknown exception!");
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
    if ((value & 1) > 0) hover = false;
    if ((value & 2) > 0) swim = false;
    if ((value & 4) > 0) ledge = false;
    if ((value & 8) > 0) lavaHover = false;
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
        MOD_Print("Successfully connected to Archipelago server!");
    }
    sendSettings(false);
}

/** Handles an incoming death link from other games. */
void handleDeathLink(std::string source, std::string cause) {
    MOD_TriggerDeath((source + " died: " + cause).c_str());
}

/** Disconnects from Archipelago. */
bool disconnect() {
    if (!AP_IsInit()) return false;
    AP_Shutdown();
    handleItemClear();
    handleReset();
    MOD_Reset();
    return true;
}

/** Connects to Archipelago. */
bool connect(std::string ip, std::string slot, std::string password) {
    if (AP_IsInit()) {
        // If we are properly connected require a formal disconnect, otherwise
        // we internally disconnect and let you switch IPs.
        if (connected) return false;
        disconnect();
    }

    connectStart = std::chrono::steady_clock::now();
    lastIp = ip;
    MOD_Print(("Attempting to connect to `" + ip + "` on slot `" + slot + "`...").c_str());
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

/** Returns whether the AP connector is connected. */
bool AP_IsConnected() {
    return AP_IsInit();
}

/** Connects to an Archipelago server with the given arguments. */
void AP_Connect(char* args) {
    // Parse out the input to find the separate arguments
    std::vector<std::string> result;
    std::string current;
    bool inQuotes = false;
    std::string data = std::string(args);
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
        MOD_Print("Usage: ap connect <ip> <slot> [password]");
    } else if (!connect(result[0], result[1], result.size() > 2 ? result[2] : "")) {
        MOD_Print("You are already connected to an Archipelago server");
    }
}

/** Disconnects from the Archipelago server if connected. */
void AP_Disconnect() {
    if (AP_IsConnected()) {
        disconnect();
        MOD_Print("Successfully disconnected from %s", lastIp);
    } else {
        MOD_Print("You are not connected to any Archipelago server");
    }

}

/** Prints the current connection status. */
void AP_PrintConnectionStatus() {
    if (!AP_IsInit()) {
        MOD_Print("Not connected to any Archipelago server");
    } else {
        if (!connected) {
            MOD_Print("Still connecting to Archipelago server...");
        } else {
            switch (AP_GetConnectionStatus()) {
            case AP_ConnectionStatus::Disconnected:
                MOD_Print("Not connected to any Archipelago server");
                break;
            case AP_ConnectionStatus::Connected:
                MOD_Print("Currently connected to %s", lastIp);
                break;
            case AP_ConnectionStatus::Authenticated:
                MOD_Print("Authenticating with Archipelago server");
                break;
            case AP_ConnectionStatus::ConnectionRefused:
                MOD_Print("Connection to Archipelago server was refused");
                break;
            }
        }
    }
}

/** Sends the given chat message. */
void AP_SendChat(char* message) {
    if (AP_IsConnected()) {
        AP_Say(message);
    }
}

/** Sends out a death link. */
void AP_SendDeathLink(char* message) {
    if (AP_IsConnected()) {
        AP_DeathLinkSend(std::string(message));
    }
}

/** Marks an item as collected. */
void AP_MarkCollected(int id) {
    // Communicate up that an item is collected.
    if (AP_IsConnected()) {
        // Send back down that we received the data
        AP_SendItem(1651615 + id);
    }
}

/** Updates the collected lum count. */
void AP_UpdateLums(int collected) {
    // Communicate up that we got a lum bundle if applicable.
    if (AP_IsConnected() && lumBundleSize > 1) {
        // Send back down that we received the data
        int lumBundlesObtained = collected / lumBundleSize;
        if (collected >= 710 && (collected % lumBundleSize) != 0) {
            // Send the leftover lum bundle!
            AP_SendItem(1653615);
        }

        // Send any regular lum bundles!
        while (lumBundlesObtained > previousCommunicatedLumBundle) {
            AP_SendItem(1653616 + previousCommunicatedLumBundle);
            previousCommunicatedLumBundle++;
        }
    }
}

/** Completes the story. */
void AP_Complete() {
    if (AP_IsConnected()) {
        AP_StoryComplete();
    }
}

/** Starts the thread listening to new Archipelago messages. */
bool AP_StartArchipelagoConnector() {
    // Prepare variables so they are properly empty
    handleItemClear();
    handleReset();

    // Start a new thread to listen to Archipelago
    auto listener = std::make_shared<APListener>();
    std::thread(&APListener::wait, listener.get()).detach();

    // Immediately send a state update so every variable is initialized on boot
    sendSettings(true);
    sendStateUpdate(true);
    return false;
}

/** Stops the Archipelago connection. */
void AP_StopArchipelagoConnector() {
    if (AP_IsConnected()) disconnect();

    MOD_Print("Shutdown completed");
}

void APListener::wait() {
    try {
        // Keep waiting for AP to have messages
        while (true) {
            if (AP_IsInit() && AP_IsMessagePending()) {
                AP_Message* message = AP_GetLatestMessage();
                if (message->type == AP_MessageType::Chat) {
                    AP_ChatMessage* chatMessage = static_cast<AP_ChatMessage*>(message);
                    std::string formattedMessage = "/o400:" + chatMessage->player + " /o0:- " + chatMessage->message;
                    MOD_Chat(formattedMessage.c_str());
                } else if (message->type == AP_MessageType::ServerChat) {
                    AP_ServerChatMessage* serverChatMessage = static_cast<AP_ServerChatMessage*>(message);
                    std::string formattedMessage = "/o200:Server /o0:- " + serverChatMessage->message;
                    MOD_Chat(formattedMessage.c_str());
                } else {
                    MOD_Print(message->text.c_str());
                }
                AP_ClearLatestMessage();
            }
            if (AP_IsInit() && !connected) {
                // Wait for at most 5 seconds for the connection to establish, otherwise
                // we deem it failed and force a disconnect!
                auto now = std::chrono::steady_clock::now();
                auto elapsed = now - connectStart;
                if (elapsed >= timeout) {
                    MOD_Print("Connection failed: host timed out");
                    disconnect();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } catch (const std::exception& e) {
        MOD_Print("[waitForAP] Caught exception: %s", e.what());
    } catch (...) {
        MOD_Print("[waitForAP] Caught an unknown exception!");
    }
}
