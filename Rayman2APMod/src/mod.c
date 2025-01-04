#include "mod.h"

#define SCREEN_TEXT_FADE_TIME 8
#define TEXT_MARGIN 2

BOOL MOD_DeathLink = TRUE;
BOOL MOD_IgnoreDeath = FALSE;
char MOD_ScreenText[10][128];
time_t MOD_ScreenTextStart[10];

// Copied from https://github.com/raytools/ACP_Ray2/blob/master/src/Ray2x/SPTXT/SPTXT.c
long SPTXT_fn_lGetFmtStringLength(char const* szFmt, va_list args) {
	long lSize = vsnprintf(NULL, 0, szFmt, args);
	return lSize + 1;
}

long SPTXT_fn_lGetCharHeight(MTH_tdxReal xSize) {
	MTH_tdxReal size = 15.0f - xSize;
	MTH_tdxReal height = 38.0f - size * 2.5f;
	return (long)height + TEXT_MARGIN + TEXT_MARGIN;
}

/** Ticked by the engine every frame, runs all messages received since last tick. */
void CALLBACK MOD_EngineTick() {
	MOD_RunPendingMessages();
	return GAM_fn_vEngine();
}

/** Handles the player dying and triggers a death link. */
void CALLBACK MOD_Init() {
	// Test if the player has died, this gets triggered once on death
	if (GAM_fn_ucGetEngineMode() == 7) {
		if (MOD_DeathLink && !MOD_IgnoreDeath) {
			MOD_SendMessage(MESSAGE_TYPE_DEATH, "Rayman died");
		}
		MOD_IgnoreDeath = FALSE;
	}

	return GAM_fn_vChooseTheGoodInit();
}

/** Triggers the player to die. */
void MOD_TriggerDeath() {
	// Ignore deaths until the next time the player is alive
	// so we don't trigger a death from this death.
	MOD_IgnoreDeath = TRUE;

	HIE_tdstEngineObject* pRayman = HIE_M_hSuperObjectGetActor(HIE_M_hGetMainActor());
	if (pRayman) {
		pRayman->hCollSet->stColliderInfo.ucColliderType = 52;
		pRayman->hCollSet->stColliderInfo.ucColliderPriority = 255;
		pRayman->hStandardGame->ucHitPoints--;
	}
}

/** Prints a message to the console. */
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

/** Shows the given text on the screen for the next 8 seconds. */
void MOD_ShowScreenText(char* text) {
	time_t currentTime = time(NULL);

	for (int i = 0; i < 10; i++) {
		time_t startFrames = MOD_ScreenTextStart[i];
		if (currentTime - startFrames <= SCREEN_TEXT_FADE_TIME) continue;

		// Move all other text up so we can add this at the end
		for (int j = i + 1; j < 10; j++) {
			if (currentTime - MOD_ScreenTextStart[j] <= SCREEN_TEXT_FADE_TIME) {
				MOD_ScreenTextStart[i] = MOD_ScreenTextStart[j];
				MOD_ScreenTextStart[j] = 0;
				char* targetText = MOD_ScreenText[i];
				char* sourceText = MOD_ScreenText[j];
				strncpy(targetText, sourceText, strlen(sourceText));
				i++;
			}
		}

		// Determine the text to show
		MOD_ScreenTextStart[i] = currentTime;
		char* screen = MOD_ScreenText[i];
		int size = min(127, strlen(text));
		strncpy(screen, text, size);

		// Filter out invalid characters that cannot be rendered
		for (int i = 0; i < size; i++) {
			if (screen[i] == '\n' || screen[i] == '\r') {
				screen[i] = ' ';
			}
		}
		screen[size] = 0;
		break;
	}
}

/** Draws text to the screen with the recent screen text. */
void CALLBACK MOD_vTextCallback(SPTXT_tdstTextInfo* pInfo) {
	// Determine the current time
	time_t currentTime = time(NULL);

	// Draw the text to the screen
	pInfo->xSize = 6;
	pInfo->X = 10;
	pInfo->Y = 1000-10;
	pInfo->bFrame = TRUE;

	for (int i = 0; i < 10; i++) {
		// Ignore any lines that have finished fading out
		time_t startTime = MOD_ScreenTextStart[i];
		int timePassed = currentTime - startTime;
		if (timePassed > SCREEN_TEXT_FADE_TIME) continue;

		// Write the line and then move the Y up
		char* screen = MOD_ScreenText[i];
		pInfo->Y = pInfo->Y - SPTXT_fn_lGetCharHeight(pInfo->xSize);
		SPTXT_vPrint(screen);
	}
	SPTXT_vResetTextInfo(pInfo);
}

/** Returns whether death link is currently enabled. */
BOOL MOD_GetDeathLink() {
	return MOD_DeathLink;
}

/** Updates the current state of the death link setting. */
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

void MOD_Main(void) {
	MOD_InitCommands();

	SPTXT_vInit();
	SPTXT_vAddTextCallback(MOD_vTextCallback);
}
