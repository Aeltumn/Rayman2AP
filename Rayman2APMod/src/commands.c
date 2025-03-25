#include "framework.h"
#include "mod.h"
#include "connector.h"

tdfnCommand fn_vApCmd;
tdfnCommand fn_vDeathlinkCommand;
tdfnCommand fn_vSayCommand;

void MOD_InitCommands(void) {
	fn_vRegisterCommand("ap", fn_vApCmd);
	fn_vRegisterCommand("deathlink", fn_vDeathlinkCommand);
	fn_vRegisterCommand("say", fn_vSayCommand);
}

// Adds /ap which lets you connect to the AP server
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

		char result[C_MaxLine];
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

// Toggles whether death link is currently enabled.
void fn_vDeathlinkCommand(int lNbArgs, char** d_szArgs) {
	MOD_SetDeathLink(MOD_GetDeathLink() ? FALSE : TRUE);
}

// Allows sending messages to the Archipelago server.
void fn_vSayCommand(int lNbArgs, char** d_szArgs) {
	// Add a space between arguments as it gets removed by the argument parser but we want to greedily include all arguments
	char result[C_MaxLine];
	int pos = 0;
	for (int i = 0; i < lNbArgs; i++) {
		if (i > 0) {
			result[pos++] = " ";
		}
		strcpy(result + pos, d_szArgs[i]);
		pos += strlen(d_szArgs[i]);
	}
	MOD_SendMessageE(MESSAGE_TYPE_MESSAGE, result);
}