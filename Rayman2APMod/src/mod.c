#include "mod.h"

#define SCREEN_TEXT_FADE_TIME 8
#define TEXT_MARGIN 2
#define LEVEL_COUNT 56
#define MAX_LENGTH 32

BOOL MOD_DeathLink = TRUE;
BOOL MOD_IgnoreDeath = FALSE;
char MOD_ScreenText[10][128];
time_t MOD_ScreenTextStart[10];

// TODO First areas of each level will need to randomise between one another otherwise you can't exit properly!
const char* MOD_LevelNames[] = {
	"Astro_00",
	"Astro_10",
	"Bast_09", // Iron Mountains intro cutscene.
	"Bast_10",
	"Bast_20",
	"Bast_22",
	"Boat01",
	"Boat02",
	"Cask_10",
	"Cask_30",
	"Chase_10",
	"Chase_22",
	"Earth_10",
	"Earth_20",
	"Earth_30",
	"GLob_10",
	"GLob_20",
	"GLob_30",
	"Helic_10",
	"Helic_20",
	"Helic_30",
	"Ile_10",
	"Learn_10",
	"Learn_30",
	"Learn_31",
	"Learn_40",
	"Learn_60",
	"Ly_10",
	"Ly_20",
	"Mine_10",
	"Morb_00",
	"Morb_10",
	"Morb_20",
	"Nave_10",
	"Nave_15",
	"Nave_20",
	"Plum_00$01$00",
	"Plum_10",
	"Plum_20",
	"Rhop_10", // Ending Boss Fight
	"Rodeo_10",
	"Rodeo_40",
	"Rodeo_60",
	"Seat_10",
	"Seat_11",
	"Ski_10",
	"Ski_60",
	"Vulca_10",
	"Vulca_20",
	"Water_10",
	"Water_20",
	"Whale_00",
	"Whale_05",
	"Whale_10"
};
char* MOD_LevelNamesShuffled[LEVEL_COUNT];

// Utility method for shuffling array.
void shuffle(char* array[], size_t n) {
	if (n > 1) {
		size_t i;
		for (i = 0; i < n - 1; i++)	{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			char* t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

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

// Hook into level transitions and randomize them
void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame) {
	// Ignore going back to the main menu
	if (_stricmp(szLevelName, "menu") == 0 || _stricmp(szLevelName, "mapmonde") == 0) {
		GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
		MOD_Print("GAM_fn_vAskToChangeLevel: %s", szLevelName);
		return;
	}

	// Find which map to send the player to
	int oldId = -1;
	for (int i = 0; i < LEVEL_COUNT; i++) {
		if (_stricmp(szLevelName, MOD_LevelNames[i]) == 0) {
			oldId = i;
		}
	}
	if (oldId == -1) {
		GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
		return;
	}

	// Get the new target and send them there
	char* targetLevelName = MOD_LevelNamesShuffled[oldId];
	MOD_Print("GAM_fn_vAskToChangeLevel: %s -> %s", szLevelName, targetLevelName);
	GAM_fn_vAskToChangeLevel(targetLevelName, bSaveGame);
}

void MOD_SetLevel(const char* szName) {
	MOD_Print("GAM_fn_vSetLevelName: %s", szName);
	GAM_fn_vSetLevelName(szName);
}

void MOD_SetNextLevel(const char* szName) {
	// TODO: This needs to happen earlier!
	// Whenever we go back to the hall of doors we need to restore the original level name
	if (_stricmp(szName, "mapmonde") == 0) {
		char* currentMap = GAM_fn_p_szGetLevelName();
		int oldId = -1;
		for (int i = 0; i < LEVEL_COUNT; i++) {
			if (_stricmp(currentMap, MOD_LevelNamesShuffled[i]) == 0) {
				oldId = i;
			}
		}
		if (oldId != -1) {
			GAM_fn_vSetLevelName(MOD_LevelNames[oldId]);
			MOD_Print("Restored level name to %s", MOD_LevelNames[oldId]);
		}
	}

	MOD_Print("GAM_fn_vSetNextLevelName: %s", szName);
	GAM_fn_vSetNextLevelName(szName);
}

void MOD_SetFirstLevel(const char* szName) {
	MOD_Print("GAM_fn_vSetFirstLevelName: %s", szName);
	GAM_fn_vSetFirstLevelName(szName);
}

/** Ticked by the engine every frame, runs all messages received since last tick. */
void MOD_EngineTick() {
	MOD_RunPendingMessages();
	GAM_fn_vEngine();
}

/** Handles the player dying and triggers a death link. */
void MOD_Init() {
	// Test if the player has died, this gets triggered once on death
	if (GAM_fn_ucGetEngineMode() == 7) {
		if (MOD_DeathLink && !MOD_IgnoreDeath) {
			MOD_SendMessage(MESSAGE_TYPE_DEATH, "Rayman died");
		}
		MOD_IgnoreDeath = FALSE;
	}

	GAM_fn_vChooseTheGoodInit();
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
#ifdef DEBUG_PRINT
	FILE* pFile = fopen("output_log.txt", "a");
	if (pFile != NULL) {
		fprintf(pFile, "\n");
		va_list args;
		va_start(args, text);
		vfprintf(pFile, text, args);
		va_end(args);
		fclose(pFile);
	}
#else
	// Remove any carriage returns from the input as they crash the game
	int length = strlen(text);
	int nullIndex = length;
	for (int i = length - 1; i >= 0; i--) {
		if (text[i] == '\r') {
			// Move the null terminator forward to remove trailing \r's
			if (i == nullIndex - 1) {
				text[i] = 0;
				nullIndex--;
			} else {
				text[i] = ' ';
			}
		}
	}

	va_list args;
	va_start(args, text);

	long lSize = SPTXT_fn_lGetFmtStringLength(text, args);
	char* szBuffer = _alloca(lSize);

	if (szBuffer) {
		vsprintf(szBuffer, text, args);
		fn_vPrint(szBuffer);
	}

	va_end(args);
#endif
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
	// Seed the random number generator
	srand(time(NULL));

	// Shuffle the list of levels to create a unique mapping
	for (int i = 0; i < LEVEL_COUNT; i++) {
		MOD_LevelNamesShuffled[i] = (char*)MOD_LevelNames[i];
	}
	shuffle(MOD_LevelNamesShuffled, LEVEL_COUNT);

	// Initialize commands
	MOD_InitCommands();

	// Initialize on-screen text
	SPTXT_vInit();
	SPTXT_vAddTextCallback(MOD_vTextCallback);
}