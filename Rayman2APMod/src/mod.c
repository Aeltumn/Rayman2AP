#include "mod.h"

BOOL MOD_DeathLink = 0;

// Copied from https://github.com/raytools/ACP_Ray2/blob/master/src/Ray2x/SPTXT/SPTXT.c
long SPTXT_fn_lGetFmtStringLength(char const* szFmt, va_list args) {
	long lSize = vsnprintf(NULL, 0, szFmt, args);
	return lSize + 1;
}

void MOD_TriggerDeath() {
	HIE_tdstEngineObject* pRayman = HIE_M_hSuperObjectGetActor(HIE_M_hGetMainActor());
	if (pRayman) {
		pRayman->hCollSet->stColliderInfo.ucColliderType = 52;
		pRayman->hCollSet->stColliderInfo.ucColliderPriority = 255;
		pRayman->hStandardGame->ucHitPoints--;
	}
}

void MOD_Print(char* text, ...) {
	va_list args;
	va_start(args, text);

	long lSize = SPTXT_fn_lGetFmtStringLength(text, args);
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

BOOL MOD_GetDeathLink() {
	return MOD_DeathLink;
}

void MOD_SetDeathLink(BOOL value) {
	// Ignore if no changes were made
	if (MOD_DeathLink == value) return;

	MOD_DeathLink = value;
	if (value) {
		MOD_Print("Deathlink is now enabled, be careful!");
	} else {
		MOD_Print("Deathlink has been disabled, you're safe now!");
	}
}
