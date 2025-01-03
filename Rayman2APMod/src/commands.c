#include "framework.h"
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
		fn_vPrint("Usage: ap <connect|disconnect|check>");
		return;
	}

	char* command = d_szArgs[0];
	if (command == "connect") {
		if (lNbArgs < 4) {
			fn_vPrint("Usage: ap connect <ip> <game> <slot> [password]");
			return;
		}

		char* ip = d_szArgs[1];
		char* game = d_szArgs[2];
		char* slot = d_szArgs[3];
		char* password = "";
		if (lNbArgs > 3) {
			password = d_szArgs[4];
		}


	} else if (command == "disconnect") {

	} else if (command == "check") {

	} else {
		fn_vPrint("Usage: ap <connect|disconnect|check>");
	}
}

// Toggles whether death link is currently enabled.
void fn_vDeathlinkCommand(int lNbArgs, char** d_szArgs) {
	MOD_SendMessage(MESSAGE_TYPE_UPDATE_DEATHLINK, MOD_GetDeathLink() ? "0" : "1");
}

// Triggers a test message.
void fn_vTestCommand(int lNbArgs, char** d_szArgs) {
	MOD_SendMessage(MESSAGE_TYPE_TEST, "");
}