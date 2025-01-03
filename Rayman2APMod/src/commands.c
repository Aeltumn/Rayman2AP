#include "framework.h"

tdfnCommand fn_vApCmd;

void MOD_InitCommands(void) {
	fn_vRegisterCommand("ap", fn_vApCmd);
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
		fn_vPrint("Usage: ap <connect|disconnect|check> [ip] [game] [slot] [password]");
	}
}