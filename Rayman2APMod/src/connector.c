#include "connector.h"
#include "mod.h"
#include <windows.h>
#include <stdio.h>

// Store information for the connector itself
HANDLE hChildStdOutRead, hChildStdOutWrite;
HANDLE hChildStdInRead, hChildStdInWrite;
SECURITY_ATTRIBUTES saAttr;
PROCESS_INFORMATION pi;
HANDLE threadHandle, readyEvent, job, messageMutex;
DWORD threadId;

// Store messages that were received, we handle them the next tick
typedef struct {
    int type;
    char* text;
} incomingMessage;

incomingMessage* incomingMessages;
int incomingMessageCount;
int incomingMessagesSize;

/** Task run in a separate thread to handle incoming packets. */
DWORD WINAPI MOD_ReadInput(LPVOID param) {
    // Start reading from the child process's stdout
    char buffer[128];
    BOOL ready = FALSE;
    DWORD bytesRead;
    while (ReadFile(hChildStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        // Replace the \n with the null terminator otherwise add it after
        if (buffer[bytesRead - 2] == '\r') {
            buffer[bytesRead - 2] = '\0';
        } else if (buffer[bytesRead - 1] == '\n') {
            buffer[bytesRead - 1] = '\0';
        } else {
            buffer[bytesRead] = '\0';
        }
        int type = buffer[0] - '0';
        memmove(buffer, buffer + 1, strlen(buffer));

        // Wait for mutex access
        WaitForSingleObject(messageMutex, INFINITE);

        __try {
            // Queue up the message for next tick
            incomingMessage message = { type, buffer };

            incomingMessageCount++;
            if (incomingMessageCount > incomingMessagesSize) {
                incomingMessagesSize = max(2, incomingMessagesSize * 2);
                if (incomingMessages) {
                    incomingMessage* reallocated = realloc(incomingMessages, incomingMessagesSize * sizeof(incomingMessage));
                    if (reallocated) {
                        incomingMessages = reallocated;
                    }
                    else {
                        MOD_Print("Failed to extend array of incoming messages");
                    }
                }
                else {
                    incomingMessages = malloc(incomingMessagesSize * sizeof(incomingMessage));
                }
            }
            incomingMessages[incomingMessageCount - 1] = message;

            // If we haven't handled any packets yet this is the initial packet and we are now ready
            // and can continue starting the game.
            if (!ready) {
                ready = TRUE;
                MOD_Print("[parent] Ready to handle input after receiving first packet");
                SetEvent(readyEvent);
            }
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
    WaitForSingleObject(messageMutex, INFINITE);

    __try {
        incomingMessage* pending = incomingMessages;
        int length = incomingMessageCount;
        incomingMessages = malloc(2 * sizeof(incomingMessage));
        incomingMessagesSize = 2;
        incomingMessageCount = 0;
        for (int i = 0; i < length; i++) {
            incomingMessage entry = pending[i];
            MOD_HandleMessage(entry.type, entry.text);
        }
        free(pending);
    }
    __finally {
        // Release the mutex
        ReleaseMutex(messageMutex);
    }
}

/** Starts up the connector which runs Rayman2APConnector in a subprocess. */
int MOD_StartConnector() {
    // Set up the mutex for synchronization
    messageMutex = CreateMutex(NULL, FALSE, NULL);
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

/** Sends out a connector message. */
void MOD_SendMessage(int type, const char* data) {
    DWORD bytesWritten;
    int length = strlen(data) + 3;
    char* message = malloc(length);
    message[0] = type + '0';
    strcpy(message + 1, data);
    message[length - 2] = '\n';
    message[length - 1] = '\0';
    if (!WriteFile(hChildStdInWrite, message, strlen(message), &bytesWritten, NULL)) {
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
    case MESSAGE_TYPE_UPDATE_DEATHLINK:
        // Update the death link state
        MOD_SetDeathLink(data[0] - '0' ? TRUE : FALSE);
        break;
    default:
        MOD_Print("[parent] Received type %d: %s", type, data);
        break;
    }
}