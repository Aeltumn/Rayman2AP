#include "framework.h"
#include "mod.h"
#include "connector.h"

tdfnCommand fn_vApCmd;
tdfnCommand fn_vDeathlinkCommand;
tdfnCommand fn_vTestCommand;

void MOD_InitCommands(void) {
	fn_vRegisterCommand("ap", fn_vApCmd);
	fn_vRegisterCommand("deathlink", fn_vDeathlinkCommand);
	fn_vRegisterCommand("test", fn_vTestCommand);
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

// Triggers a test message.
void fn_vTestCommand(int lNbArgs, char** d_szArgs) {
	MOD_SendMessageE(MESSAGE_TYPE_TEST);
}