#include "framework.h"
#include "mod.h"
#include "connector.h"

tdfnCommand fn_vApCmd;
tdfnCommand fn_vDeathlinkCommand;
tdfnCommand fn_vSayCommand;
tdfnCommand fn_vDsgCommand;
tdfnCommand fn_vHallOfDoors;

void MOD_InitCommands(void) {
	fn_vRegisterCommand("ap", fn_vApCmd);
	fn_vRegisterCommand("deathlink", fn_vDeathlinkCommand);
	fn_vRegisterCommand("say", fn_vSayCommand);
	fn_vRegisterCommand("dsg", fn_vDsgCommand);
	fn_vRegisterCommand("hallofdoors", fn_vHallOfDoors);
}

/** Adds /ap which lets you connect to the AP server. */
void fn_vApCmd(int lNbArgs, char** d_szArgs) {
	if (lNbArgs < 1) {
		MOD_Print("Usage: ap <connect|disconnect|check>");
		return;
	}

	char* command = d_szArgs[0];
	if (_stricmp(command, "connect") == 0) {
		if (lNbArgs < 3) {
			MOD_Print("Usage: ap connect <ip> <slot> [password]");
			return;
		}

		char* ip = d_szArgs[1];
		char* slot = d_szArgs[2];
		char* password = "";
		if (lNbArgs > 3) {
			password = d_szArgs[3];
		}

		char result[72];
		sprintf(result, "%s %s %s", ip, slot, password);
		MOD_SendMessage(MESSAGE_TYPE_CONNECT, result);
	} else if (_stricmp(command, "disconnect") == 0) {
		MOD_SendMessageE(MESSAGE_TYPE_DISCONNECT);
	} else if (_stricmp(command, "check") == 0) {
		MOD_SendMessageE(MESSAGE_TYPE_CHECK);
	} else {
		MOD_Print("Usage: ap <connect|disconnect|check>");
	}
}

/** Toggles whether death link is currently enabled. */
void fn_vDeathlinkCommand(int lNbArgs, char** d_szArgs) {
	if (!MOD_GetDeathLink(TRUE)) {
		MOD_Print("Death linking is not enabled currently");
		return;
	}
	MOD_ToggleDeathLink();
}

/** Allows sending messages to the Archipelago server. */
void fn_vSayCommand(int lNbArgs, char** d_szArgs) {
	int totalLength = 0;
	for (int i = 0; i < lNbArgs; i++) {
		totalLength += strlen(d_szArgs[i]);
	}
	totalLength += lNbArgs - 1;
	char* result = malloc(totalLength + 1);
	result[0] = '\0';
	for (int i = 0; i < lNbArgs; i++) {
		if (i > 0) {
			strcat(result, " ");
		}
		strcat(result, d_szArgs[i]);
	}
	MOD_SendMessage(MESSAGE_TYPE_MESSAGE, result);
	free(result);
}

/** Allows editing DSG variables on the global object for testing. */
void fn_vDsgCommand(int lNbArgs, char** d_szArgs) {
	if (lNbArgs < 1) {
		MOD_Print("Usage: dsg <var> [value]");
		return;
	}

	// Parse the variable number
	unsigned int var = (unsigned int) atoi(d_szArgs[0]);
	if (var < 0 || var > 1400) {
		MOD_Print("Failed to parse variable id");
		return;
	}

	BOOL setMode = lNbArgs >= 2;
	ACP_tdxBool realValue = FALSE;
	if (setMode) {
		// In set mode you have to pass along false or true to change the value!
		char* value = d_szArgs[1];
		if (strcmp(value, "true") == 0) {
			realValue = TRUE;
		} else if (strcmp(value, "false") == 0) {
			realValue = FALSE;
		} else {
			MOD_Print("Unknown value type: '%s', only `true` and `false are known", value);
			return;
		}
	}
	
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (!pGlobal) {
		MOD_Print("Could not find global object");
		return;
	}
	if (setMode) {
		AI_fn_bSetBooleanInArray(pGlobal, 42, var, realValue);
	} else {
		MOD_Print("The value of DSG variable %d is: %s", var, AI_fn_bGetBooleanInArray(pGlobal, 42, var) ? "TRUE" : "FALSE");
	}
}

/** Sends you to the hall of doors. */
void fn_vHallOfDoors(int lNbArgs, char** d_szArgs) {
	GAM_fn_vAskToChangeLevel("mapmonde", TRUE);
}