#include "connector.h"
#include "mod.h"
#include <windows.h>
#include <stdio.h>

void removeSubstring(char const* string, char const* substring) {
    char* _substr = strstr(string, substring);
    while (_substr != NULL && strcmp(substring, "") != 0) {
        sprintf(_substr, "%s%s", "", _substr + strlen(substring));
        _substr = strstr(string, substring);
    }
    return _substr;
}

// Store information for the connector itself
HANDLE hChildStdOutRead, hChildStdOutWrite, hChildStdInRead, hChildStdInWrite, threadHandle, readyEvent, job, messageMutex;
SECURITY_ATTRIBUTES saAttr;
PROCESS_INFORMATION pi;
DWORD threadId;
BOOL MOD_Running;
BOOL MOD_Shutdown;

// Store messages that were received, we handle them the next tick
typedef struct {
    int type;
    char* text;
} incomingMessage;

incomingMessage* incomingMessages;
int incomingMessageCount = 0;
int incomingMessageReadIndex = 0;
int incomingMessageWriteIndex = 0;
int incomingMessagesSize = 0;

/** Handles a connector message. */
void MOD_HandleMessage(int type, const char* data) {
    switch (type) {
    case MESSAGE_TYPE_SETTINGS: {
        // Parse all data from the input message in order
        char* copy = strdup(data);
        if (!copy) break;

        char* token;
        int index = 0;

        BOOL connected = FALSE;
        BOOL deathLink = FALSE;
        BOOL damageLink = FALSE;
        int endGoal = 1;
        BOOL lumsanity = FALSE;
        BOOL roomRandomisation = FALSE;
        BOOL accessiblePortals = FALSE;
        int deathLinkAmnesty = 1;
        BOOL betterLevelPortals = FALSE;
        int lumBundleSize = 0;
        int lumGates[6];

        char* levelIds[LEVEL_COUNT];
        int chainLengths[CHAIN_COUNT];
        int* chainContents[CHAIN_COUNT];

        // Ensure values are properly empty!
        for (int i = 0; i < LEVEL_COUNT; i++) {
            levelIds[i] = "";
        }
        for (int i = 0; i < CHAIN_COUNT; i++) {
            chainLengths[i] = 0;
            chainContents[i] = NULL;
        }

        token = strtok(copy, ",");
        while (token) {
            switch (index) {
            case 0: connected = atoi(token); break;
            case 1: deathLink = atoi(token); break;
            case 2: endGoal = atoi(token); break;
            case 3: lumsanity = atoi(token); break;
            case 4: roomRandomisation = atoi(token); break;
            case 5: accessiblePortals = atoi(token); break;
            case 6: deathLinkAmnesty = atoi(token); break;
            case 7: betterLevelPortals = atoi(token); break;
            case 8: lumBundleSize = atoi(token); break;
            case 9: damageLink = atoi(token); break;
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                lumGates[index - 10] = atoi(token);
                break;
            }

            // When we reach the final index we switch processing!
            if (index == 16) {
                int idx = 0;
                int levelId = 0;
                char* context1 = NULL;
                char* chain = strtok_s(token, ";", &context1);

                while (chain) {
                    int* chainArray = chainContents[idx];
                    int chainLength = 0;

                    char* context2 = NULL;
                    char* zone = strtok_s(chain, "|", &context2);

                    while (zone) {
                        chainLength++;
                        chainArray = realloc(chainArray, chainLength * sizeof(int));

                        // Store this zone name under an id, then store only the
                        // id in this chain so there's less memory to re-allocate.
                        levelIds[levelId] = zone;
                        chainArray[chainLength - 1] = levelId;
                        levelId++;

                        zone = strtok_s(NULL, "|", &context2);
                    }

                    chainLengths[idx] = chainLength;
                    chainContents[idx] = chainArray;

                    idx++;
                    chain = strtok_s(NULL, ";", &context1);
                }
                break;
            }

            index++;
            token = strtok(NULL, ",");
        }
        free(copy);

        // Send this data across to the main mod file
        MOD_UpdateSettings(connected, deathLink, damageLink, endGoal, lumsanity, roomRandomisation, accessiblePortals, deathLinkAmnesty, betterLevelPortals, lumBundleSize, lumGates, levelIds, chainLengths, chainContents);
        break;
    }
    case MESSAGE_TYPE_STATE: {
        // Parse all data from the input message in order
        char* copy = strdup(data);
        if (!copy) break;

        char* token;
        int index = 0;
        int gateIndex = 0;

        int lums = 0;
        int cages = 0;
        int masks = 0;
        int upgrades = 0;
        BOOL elixir = FALSE;
        BOOL knowledge = FALSE;
        BOOL fragmented = FALSE;
        BOOL hover = FALSE;
        BOOL ledge = FALSE;
        BOOL swim = FALSE;
        BOOL lavaHover = FALSE;

        token = strtok(copy, ",");
        while (token) {
            switch (index) {
            case 0: lums = atoi(token); break;
            case 1: cages = atoi(token); break;
            case 2: masks = atoi(token); break;
            case 3: upgrades = atoi(token); break;
            case 4: elixir = atoi(token); break;
            case 5: knowledge = atoi(token); break;
            case 6: fragmented = atoi(token); break;
            case 7: hover = atoi(token); break;
            case 8: ledge = atoi(token); break;
            case 9: swim = atoi(token); break;
            case 10: lavaHover = atoi(token); break;
            }

            index++;
            token = strtok(NULL, ",");
        }
        free(copy);

        // Send this data across to the main mod file
        MOD_UpdateState(lums, cages, masks, upgrades, elixir, knowledge, fragmented, hover, ledge, swim, lavaHover);
        break;
    }
    case MESSAGE_TYPE_DEATH: {
        MOD_TriggerDeath(data);
        break;
    }
    case MESSAGE_TYPE_COLLECTED: {
        // If we receive collected from another source it means another
        // player obtained something we should be informed about on the screen
        MOD_ShowScreenText(data);
        break;
    }
    case MESSAGE_TYPE_MESSAGE: {
        MOD_Print(data);
        break;
    }
    case MESSAGE_TYPE_RESET: {
        MOD_Reset();
        break;
    }
    case MESSAGE_TYPE_CHAT: {
        MOD_ShowScreenText(data);
        
        // Filter color codes out of text before printing!
        removeSubstring(data, "/o400:");
        removeSubstring(data, "/o200:");
        removeSubstring(data, "/o0:");
        MOD_Print(data);
        break;
	}
    default: {
        MOD_Print("[parent] Received type %d: %s", type, data);
        break;
    }
    }
}

/** Handles a recent error from running [action]. */
void MOD_HandleError(char* action) {
    DWORD errorCode = GetLastError();
    MOD_Print("Encountered error while %s, code %lu", action, errorCode);

    // The child process must have gone missing!
    if (errorCode == ERROR_BROKEN_PIPE) {
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        MOD_Print("Broken pipe occurred, child exit code %lu", exitCode);
    }
}

/** Task run in a separate thread to handle incoming packets. */
DWORD WINAPI MOD_ReadInput(LPVOID param) {
    // Start reading from the child process's stdout
    char buffer[1];
    BOOL ready = FALSE;
    DWORD bytesRead;
    while (MOD_Running) {
        if (!ReadFile(hChildStdOutRead, buffer, 1, &bytesRead, NULL)) {
            MOD_HandleError("reading message buffer");
            continue;
        }
        if (bytesRead == 0) continue;

        // Ignore all input until we find our special character!
        if (buffer[0] != (char)26) continue;

        // Read the length of the message that will follow
        char lengthChar[7];
        if (!ReadFile(hChildStdOutRead, lengthChar, 6, &bytesRead, NULL)) {
            MOD_HandleError("reading message buffer");
            continue;
        }
        if (bytesRead == 0) continue;
        lengthChar[6] = '\0';
        int messageLength = atoi(lengthChar);
        if (messageLength < 0) continue;

        // Determine the type of the message
        char typeChar[1];
        if (!ReadFile(hChildStdOutRead, typeChar, 1, &bytesRead, NULL)) {
            MOD_HandleError("reading message buffer");
            continue;
        }
        if (bytesRead == 0) continue;
        int type = typeChar[0] - '0';

        // Read out the message
        char* messageBuffer = malloc(messageLength + 1);
        if (!messageBuffer) {
            MOD_Print("Failed to allocate memory for incoming message");
            return;
        }
        if (messageLength > 0) {
            if (!ReadFile(hChildStdOutRead, messageBuffer, messageLength, &bytesRead, NULL)) {
                MOD_HandleError("reading message buffer");
                continue;
            }
            if (bytesRead == 0) {
                free(messageBuffer);
                continue;
            }
        }
        messageBuffer[messageLength] = '\0';

        // If we haven't handled any packets yet this is the initial packet and we are now ready
        // and can continue starting the game.
        if (!ready) {
            ready = TRUE;
            SetEvent(readyEvent);
        }

        // Wait for mutex access
        WaitForSingleObject(messageMutex, INFINITE);

        __try {
            // Handle allocating more memory if we've caught up to the read index
            if (incomingMessageCount >= incomingMessagesSize) {
                int oldSize = incomingMessagesSize;
                incomingMessagesSize = max(128, incomingMessagesSize * 2);
                incomingMessage* reallocated = malloc(incomingMessagesSize * sizeof(incomingMessage));
                if (!reallocated) {
                    MOD_Print("Failed to extend array of incoming messages");
                    free(messageBuffer);
                    return;
                }

                // Copy over the old buffer but re-order elements to be 0-indexed again.
                for (int i = 0; i < oldSize; i++) {
                    int oldIndex = incomingMessageReadIndex + i;
                    if (oldIndex >= oldSize) {
                        oldIndex -= oldSize;
                    }
                    reallocated[i] = incomingMessages[oldIndex];
                }

                // Set the new variable
                if (oldSize > 0) free(incomingMessages);
                incomingMessageReadIndex = 0;
                incomingMessageWriteIndex = oldSize;
                incomingMessages = reallocated;
            }

            // Queue up the message for the engine to process
            incomingMessage message = { 0 };
            message.type = type;
            message.text = messageBuffer;

            // Print all incoming messages to the output log
            FILE* pFile = fopen("ap_log_connector.txt", "a");
            if (pFile != NULL) {
                fprintf(pFile, "%s", messageBuffer);
                fprintf(pFile, "\n");
                fclose(pFile);
            }
        
            // Store the incoming message last
            incomingMessages[incomingMessageWriteIndex++] = message;
            incomingMessageCount++;
            if (incomingMessageWriteIndex >= incomingMessagesSize) {
                incomingMessageWriteIndex = 0;
            }
        }
        __finally {
            // Release the mutex
            ReleaseMutex(messageMutex);
        }
    }

    // Log when the connector is closed
    MOD_Print("Closed internal AP client");

    // Clean up everything
    MOD_Shutdown = TRUE;
    CloseHandle(threadHandle);
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Destroy the child process
    TerminateJobObject(job, 0);
    CloseHandle(job);
    CloseHandle(messageMutex);

    // Ensure the child process is destroyed!
    TerminateProcess(pi.hProcess, 1);
}

/** Handles all pending messages that were received since the last tick. */
void MOD_RunPendingMessages() {
    // Ignore if there are no messages pending!
    if (incomingMessageCount <= 0) return;

    // Wait for mutex access, if it's taken, ignore it completely!
    DWORD result = WaitForSingleObject(messageMutex, 0);
    if (result != WAIT_OBJECT_0) return;

    __try {
        // Limit the maximum amount of messages processed per tick!
        int toProcess = incomingMessageCount;
        if (toProcess > MAX_MESSAGES_PROCESSED_PER_FRAME) {
            toProcess = MAX_MESSAGES_PROCESSED_PER_FRAME;
        }
        while (toProcess > 0) {
            incomingMessage entry = incomingMessages[incomingMessageReadIndex];
            MOD_HandleMessage(entry.type, entry.text);
            free(entry.text);

            // Increment the index and loop it around the circular buffer
            incomingMessageReadIndex++;
            if (incomingMessageReadIndex >= incomingMessagesSize) {
                incomingMessageReadIndex = 0;
            }
            incomingMessageCount--;
            toProcess--;
        }
    }
    __finally {
        // Release the mutex
        ReleaseMutex(messageMutex);
    }
}

/** Starts up the connector which runs Rayman2APConnector in a subprocess. */
int MOD_StartConnector() {
    // Set up the mutex for synchronization
    messageMutex = CreateMutexA(NULL, FALSE, NULL);
    if (!messageMutex) {
        MOD_HandleError("creating message sync mutex");
        return 1;
    }

    // Start the reading thread later
    MOD_Running = true;

    // Set up the security attributes to allow the child process to inherit the pipe handles
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create the pipes for communication
    if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0)) {
        MOD_HandleError("creating stdout pipe");
        return 1;
    }
    if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0)) {
        MOD_HandleError("creating stdin pipe");
        return 1;
    }

    // Set the pipe handles to be inherited by the child process
    SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStdInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStdOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    // Set up the STARTUPINFO structure
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdInput = hChildStdInRead;
    si.hStdOutput = hChildStdOutWrite;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Set up the PROCESS_INFORMATION structure
    ZeroMemory(&pi, sizeof(pi));

    // Create the job to share with the child process so we can close it
    job = CreateJobObjectA(NULL, NULL);
    if (!job) {
        MOD_HandleError("creating job object");
        return 1;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) == 0) {
        MOD_HandleError("configuring job object");
        return 1;
    }

    // Create the child process
    char path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, path);
    strcat(path, "\\Rayman2APConnector.exe");
    if (!CreateProcessA(
        NULL,                // Application name
        path,                // Command line (your .exe)
        NULL,                // Process security attributes
        NULL,                // Thread security attributes
        TRUE,                // Inherit handles
        CREATE_NO_WINDOW,    // Creation flags
        NULL,                // Environment
        NULL,                // Current directory
        &si,                 // Startup information
        &pi                  // Process information
    )) {
        MOD_HandleError("creating process");
        return 1;
    }

    // Assign the child to the job
    AssignProcessToJobObject(job, pi.hProcess);

    // Close the write ends of the pipes in the parent process
    CloseHandle(hChildStdOutWrite);
    CloseHandle(hChildStdInRead);

    // Start a thread to read data from the connector
    readyEvent = CreateEvent(
        NULL,    // Default security attributes
        FALSE,   // Manual reset (FALSE means auto-reset)
        FALSE,   // Initial state is non-signaled
        NULL     // No name
    );

    if (readyEvent == NULL) {
        MOD_HandleError("creating event");
        return 1;
    }

    threadHandle = CreateThread(
        NULL,                // Default security attributes
        0,                   // Default stack size
        MOD_ReadInput,       // Function to run in the thread
        NULL,                // Parameter passed to the thread function (NULL)
        0,                   // Default creation flags
        &threadId            // Receives the thread ID
    );
    if (threadHandle == NULL) {
        MOD_HandleError("creating thread");
        return 1;
    }

    // Wait for the handler to be ready
    WaitForSingleObject(readyEvent, INFINITE);
    return 0;
}

/** Shuts down the connector. */
void MOD_StopConnector() {
    // Print a debug line to the logs
    MOD_Print("Shutting down connector");

    // Stop waiting for messages
    MOD_Running = false;

    // Send a disconnect which sends back something
    // that will end the waiting thread
    MOD_SendMessageE(MESSAGE_TYPE_SHUTDOWN);
}

/** Sends out a connector message with no metadata. */
void MOD_SendMessageE(int type) {
    MOD_SendMessage(type, "");
}

/** Sends out a connector message. */
void MOD_SendMessage(int type, const char* data) {
    if (MOD_Shutdown) {
        MOD_Print("AP connector has shut down, please restart your game!");
        return;
    }
    DWORD bytesWritten;
    int length = strlen(data);
    char* message = malloc(length + 8);
    message[0] = 26;
    snprintf(message + 1, 7, "%06d", length);
    message[7] = type + '0';
    if (length > 0) {
        memcpy(message + 8, data, length);
    }
    if (!WriteFile(hChildStdInWrite, message, length + 8, &bytesWritten, NULL)) {
        MOD_HandleError("writing to connector");
    }
    free(message);
}