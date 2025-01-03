#include <windows.h>
#include <stdio.h>
#include "connector.h"

// Copy the definition of the injection function from Twofold
#define LDR_C_InitProc "ModMain"

typedef int (*td_pfn_lInitProc)(BOOL bInit, void* reserved);

int main() {
    if (MOD_StartConnector()) {
        printf("Error starting connector\n");
        return 1;
    }
    MOD_SendMessage(MESSAGE_TYPE_DEATH, "Rayman died");
    return 0;
}

void runDLL() {
    // Load in the DLL
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    strcat(buffer, "\\Rayman2APMod.dll");
    HMODULE hDll = LoadLibrary(buffer);
    if (hDll == NULL) {
        DWORD error = GetLastError();
        printf("Error loading DLL %s with error code: %lu\n", buffer, error);
        return 1;
    }

    // Load the main function from the DLL
    td_pfn_lInitProc pInitProc = (td_pfn_lInitProc)GetProcAddress(hDll, LDR_C_InitProc);
    if (pInitProc == NULL) {
        printf("Error loading main function\n");
        FreeLibrary(hDll);
        return 1;
    }

    // Run the DLL
    pInitProc(1, NULL);
    FreeLibrary(hDll);
    return 0;
}