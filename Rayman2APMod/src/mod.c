#include "mod.h"

// Copied from font.c in R2Console
long fn_lGetFmtStringLength(char const* szFmt, va_list args) {
	long lSize = vsnprintf(NULL, 0, szFmt, args);
	return lSize + 1;
}

void MOD_Print(char* text, ...) {
	va_list args;
	va_start(args, text);

	long lSize = fn_lGetFmtStringLength(text, args);
	char* szBuffer = _alloca(lSize);

	if (szBuffer) {
		vsprintf(szBuffer, text, args);
		fn_vPrint(szBuffer);
	}

	va_end(args);
}

void MOD_Main(void) {
	MOD_InitCommands();
}
