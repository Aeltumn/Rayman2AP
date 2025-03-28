#include "connector.h"
#include "mod.h"
#include <windows.h>
#include <stdio.h>

// Store information for the connector itself
HANDLE hChildStdOutRead, hChildStdOutWrite, hChildStdInRead, hChildStdInWrite, threadHandle, readyEvent, job, messageMutex;
SECURITY_ATTRIBUTES saAttr;
PROCESS_INFORMATION pi;
DWORD threadId;

// Store messages that were received, we handle them the next tick
typedef struct {
    int type;
    char* text;
} incomingMessage;

incomingMessage* incomingMessages;
int incomingMessageCount = 0;
int incomingMessagesSize = 0;

/** Task run in a separate thread to handle incoming packets. */
DWORD WINAPI MOD_ReadInput(LPVOID param) {
    // Start reading from the child process's stdout
    char buffer[1];
    BOOL ready = FALSE;
    DWORD bytesRead;
    while (ReadFile(hChildStdOutRead, buffer, 1, &bytesRead, NULL) && bytesRead > 0) {
        // Ignore all input until we find our special character!
        if (buffer[0] != (char)26) continue;

        // Read the length of the message that will follow
        char lengthChar[7];
        ReadFile(hChildStdOutRead, lengthChar, 6, &bytesRead, NULL);
        if (bytesRead == 0) continue;
        lengthChar[6] = '\0';
        int messageLength = atoi(lengthChar);
        if (messageLength <= 0) continue;

        // Determine the type of the message
        char typeChar[1];
        ReadFile(hChildStdOutRead, typeChar, 1, &bytesRead, NULL);
        if (bytesRead == 0) continue;
        int type = typeChar[0] - '0';

        // Read out the message
        char* messageBuffer = malloc(messageLength + 1);
        if (!messageBuffer) {
            MOD_Print("Failed to allocate memory for incoming message");
            return;
        }
        ReadFile(hChildStdOutRead, messageBuffer, messageLength, &bytesRead, NULL);
        if (bytesRead == 0) {
            free(messageBuffer);
            continue;
        }
        messageBuffer[messageLength] = '\0';

        // If we haven't handled any packets yet this is the initial packet and we are now ready
        // and can continue starting the game.
        if (!ready) {
            ready = TRUE;
            SetEvent(readyEvent);
        }

        // Wait for mutex access
        WaitForSingleObject(messageMutex, 50);

        __try {
            // Queue up the message for next tick
            if (incomingMessageCount >= incomingMessagesSize) {
                incomingMessagesSize = max(2, incomingMessagesSize * 2);
                incomingMessage* reallocated = realloc(incomingMessages, incomingMessagesSize * sizeof(incomingMessage));
                if (!reallocated) {
                    MOD_Print("Failed to extend array of incoming messages");
                    free(messageBuffer);
                    return;
                }
                incomingMessages = reallocated;
            }
            incomingMessage message = { 0 };
            message.type = type;
            message.text = messageBuffer;

            // Print all incoming messages to the output log
            FILE* pFile = fopen("output_log.txt", "a");
            if (pFile != NULL) {
                fprintf(pFile, messageBuffer);
                fprintf(pFile, "\n");
                fclose(pFile);
            }
        
            // Store the incoming message last
            incomingMessages[incomingMessageCount++] = message;
        }
        __finally {
            // Release the mutex
            ReleaseMutex(messageMutex);
        }
    }
}

/** Handles all pending messages that were received since the last tick. */
void MOD_RunPendingMessages() {
    // Ignore if there are no messages pending!
    if (incomingMessageCount == 0) return;

    // Wait for mutex access
    WaitForSingleObject(messageMutex, 50);

    __try {
        for (int i = 0; i < incomingMessageCount; i++) {
            incomingMessage entry = incomingMessages[i];
            MOD_HandleMessage(entry.type, entry.text);
            free(entry.text);
        }
        incomingMessageCount = 0;
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
        MOD_Print("Error creating message sync mutex, error code %lu", GetLastError());
        return 1;
    }

    // Set up the security attributes to allow the child process to inherit the pipe handles
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create the pipes for communication
    if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0)) {
        MOD_Print("Error creating stdout pipe, error code %lu", GetLastError());
        return 1;
    }
    if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0)) {
        MOD_Print("Error creating stdin pipe, error code %lu", GetLastError());
        return 1;
    }

    // Set the pipe handles to be inherited by the child process
    SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

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
        MOD_Print("Error creating job object, error code %lu", GetLastError());
        return 1;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) == 0) {
        MOD_Print("Error configuring job object, error code %lu", GetLastError());
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
        MOD_Print("Error creating process, error code %lu", GetLastError());
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
        MOD_Print("Error creating event, error code %lu", GetLastError());
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
        MOD_Print("Error creating thread, error code %lu", GetLastError());
        return 1;
    }

    // Wait for the handler to be ready
    WaitForSingleObject(readyEvent, INFINITE);
    return 0;
}

/** Shuts down the connector. */
void MOD_StopConnector() {
    // Disconnect if we have to
    MOD_SendMessageE(MESSAGE_TYPE_SHUTDOWN);

    // Clean up everything
    CloseHandle(threadHandle);
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Destroy the child process
    TerminateJobObject(job, 0);
    CloseHandle(job);

    CloseHandle(messageMutex);
}

/** Sends out a connector message with no metadata. */
void MOD_SendMessageE(int type) {
    MOD_SendMessage(type, "");
}

/** Sends out a connector message. */
void MOD_SendMessage(int type, const char* data) {
    DWORD bytesWritten;
    int length = strlen(data) + 2;
    char* message = malloc(length);
    message[0] = type + '0';
    if (length > 2) {
        strcpy(message + 1, data);
    }
    message[length - 1] = '\n';
    if (!WriteFile(hChildStdInWrite, message, length, &bytesWritten, NULL)) {
        MOD_Print("Encountered error while writing to connector %lu", GetLastError());
    }
    free(message);
}

/** Handles a connector message. */
void MOD_HandleMessage(int type, const char* data) {
    switch (type) {
    case MESSAGE_TYPE_DEATH:
        // Ignore a death if death link is currently not enabled
        if (!MOD_GetDeathLink()) return;

        // Trigger a death for th player
        MOD_TriggerDeath();
        MOD_ShowScreenText(data);
        break;
    case MESSAGE_TYPE_MESSAGE:
        MOD_Print(data);
        break;
    default:
        MOD_Print("[parent] Received type %d: %s", type, data);
        break;
    }
}