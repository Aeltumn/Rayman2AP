#include "connector.h"
#include "mod.h"
#include <windows.h>
#include <stdio.h>

HANDLE hChildStdOutRead, hChildStdOutWrite;
HANDLE hChildStdInRead, hChildStdInWrite;
SECURITY_ATTRIBUTES saAttr;
PROCESS_INFORMATION pi;
HANDLE threadHandle, readyEvent, job;
DWORD threadId;

DWORD WINAPI MOD_ReadInput(LPVOID param) {
    // Start reading from the child process's stdout
    char buffer[128];
    int ready = 0;
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
        MOD_HandleMessage(type, buffer);

        if (ready == 0) {
            ready = 1;
            MOD_Print("[parent] Ready to handle input after receiving first packet");
            SetEvent(readyEvent);
        }
    }
}

int MOD_StartConnector() {
    // Set up the security attributes to allow the child process to inherit the pipe handles
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create the pipes for communication
    if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0)) {
        MOD_Print("Error creating stdout pipe");
        return 1;
    }
    if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0)) {
        MOD_Print("Error creating stdin pipe");
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
        MOD_Print("Error creating job object");
        return 1;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) == 0) {
        MOD_Print("Error configuring job object");
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
        MOD_Print("Error creating process");
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
        MOD_Print("Error creating event");
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
        MOD_Print("Error creating thread");
        return 1;
    }

    // Wait for the handler to be ready
    WaitForSingleObject(readyEvent, INFINITE);

    return 0;
}

int MOD_StopConnector() {
    // Clean up everything
    CloseHandle(threadHandle);
    CloseHandle(hChildStdOutRead);
    CloseHandle(hChildStdInWrite);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Destroy the child process
    TerminateJobObject(job, 0);
    CloseHandle(job);
    return 0;
}

int MOD_SendMessage(int type, const char* data) {
    DWORD bytesWritten;
    int length = strlen(data) + 3;
    char* message = malloc(length);
    message[0] = type + '0';
    strcpy(message + 1, data);
    message[length - 2] = '\n';
    message[length - 1] = '\0';
    MOD_Print("[parent] Sending type %d with data %s to connector", type, data);
    if (!WriteFile(hChildStdInWrite, message, strlen(message), &bytesWritten, NULL)) {
        MOD_Print("Encountered error while writing to connector %lu", GetLastError());
    }
    free(message);
}

int MOD_HandleMessage(int type, const char* data) {
    MOD_Print("[parent] Received type %d: %s", type, data);

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
    }
}