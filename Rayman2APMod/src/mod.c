#include "mod.h"

#define SCREEN_TEXT_FADE_TIME 8
#define TEXT_MARGIN 2
#define LEVEL_COUNT 56
#define MAX_LENGTH 32

BOOL MOD_Connected = FALSE;
int MOD_Lums = 0;
int MOD_Cages = 0;
int MOD_Masks = 0;
int MOD_Upgrades = 0;
BOOL MOD_DeathLink = FALSE;
BOOL MOD_DeathLinkOverride = FALSE;
int MOD_EndGoal = 1;
BOOL MOD_Elixir = FALSE;
int* MOD_LumGates[6];
BOOL MOD_IgnoreDeath = FALSE;
char MOD_ScreenText[10][128];
time_t MOD_ScreenTextStart[10];
int MOD_ScreenTextLatest = -1;
BitSet MOD_LastCollected;
BOOL MOD_TreasureComplete = FALSE;

// Copied from https://github.com/raytools/ACP_Ray2/blob/master/src/Ray2x/SPTXT/SPTXT.c
long SPTXT_fn_lGetFmtStringLength(char const* szFmt, va_list args) {
	long lSize = vsnprintf(NULL, 0, szFmt, args);
	return lSize + 1;
}

long SPTXT_fn_lGetCharWidth(MTH_tdxReal xSize)
{
	MTH_tdxReal size = 15.0f - xSize;
	MTH_tdxReal width = 46.0f - size * 4.0f;
	return (long)width;
}

long SPTXT_fn_lGetCharHeight(MTH_tdxReal xSize) {
	MTH_tdxReal size = 15.0f - xSize;
	MTH_tdxReal height = 38.0f - size * 2.5f;
	return (long)height + TEXT_MARGIN + TEXT_MARGIN;
}

void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame) {
	MOD_Print("GAM_fn_vAskToChangeLevel: %s", szLevelName);
	GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
}

void MOD_SetLevel(const char* szName) {
	MOD_Print("GAM_fn_vSetLevelName: %s", szName);
	GAM_fn_vSetLevelName(szName);
}

void MOD_SetNextLevel(const char* szName) {
	MOD_Print("GAM_fn_vSetNextLevelName: %s", szName);
	GAM_fn_vSetNextLevelName(szName);
}

void MOD_SetFirstLevel(const char* szName) {
	MOD_Print("GAM_fn_vSetFirstLevelName: %s", szName);
	GAM_fn_vSetFirstLevelName(szName);
}

/** Sets the value of a DSG variable. */
BOOL AI_fn_bSetDsgVar(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, void** p_pValue_In) {
	if (!HIE_M_bSuperObjectIsActor(p_stSuperObj))
		return FALSE;

	AI_tdstMind* hMind = AI_M_hGetMindOfSuperObj(p_stSuperObj);

	if (ucDsgVarId > AI_M_ucGetNbDsgVar(hMind))
		return FALSE;

	AI_tdstDsgVarInfo* p_stDsgInfo = AI_M_p_stGetDsgVarInfo(hMind, ucDsgVarId);

	// Update the value at the pointer
	*(AI_M_p_cGetDsgMemBuffer(hMind) + p_stDsgInfo->ulOffsetInDsgMem) = *p_pValue_In;

	return TRUE;
}

/** Sets the value of the boolean array. */
void AI_fn_bSetBooleanInArray(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, unsigned int ulIndex, ACP_tdxBool value) {
	AI_tdstArray* p_stArray;
	AI_fn_bGetDsgVar(p_stSuperObj, ucDsgVarId, NULL, &p_stArray);

	ulIndex--;
	unsigned long ulIndexFirstLong = (ulIndex >> 5);
	unsigned long ulIndexFirstBit = (ulIndex & 31);
	unsigned long mask = 1 << ulIndexFirstBit;
	long* pValue = &AI_M_pArrayElement(p_stArray, ulIndexFirstLong)->lValue;
	if (value) {
		*pValue |= mask;
	} else {
		*pValue &= ~mask;
	}

	// Also update the last collected bitset so we don't trigger an update next tick!
	setBitSet(&MOD_LastCollected, ulIndex + 1, value);
}

/** Checks if any lums/cages have been collected since last frame. */
void MOD_CheckVariables() {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal) {
		// Check if any items have been collected
		for (int i = 1; i <= 1400; i++) {
			unsigned char last = getBitSet(&MOD_LastCollected, i);
			ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, i);
			
			// If there's a desync between the local storage and the DSG variables
			// we collected an item, send it across!
			if (last != dsg) {
				setBitSet(&MOD_LastCollected, i, dsg);

				// Only if the item is now collected, send a check!
				if (dsg) {
					// While testing we show collected items on screen
					MOD_vShowScreenText("Collected %d", i);
					MOD_Print("Collected %d", i);
					
					// Send up the id of the item directly
					char str[6];
					sprintf(str, "%d", i);
					MOD_SendMessage(MESSAGE_TYPE_COLLECTED, str);

					// If this is 1146 the game was completed!
					if (i == 1146) {
						if (MOD_EndGoal == 1) {
							// If the goal is the crow's nest, you got it!
							MOD_Print("Game completed!");
							MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
						} else if (MOD_EndGoal == 3) {
							// If the goal is 100% we also require having everything!
							int hasEnoughLums = MOD_Lums >= 1000;
							int hasEnoughCages = MOD_Cages >= 80;
							if (!hasEnoughLums) {
								MOD_Print("Game is not complete, not enough lums!");
							} else if (!hasEnoughCages) {
								MOD_Print("Game is not complete, not enough cages!");
							} else {
								MOD_Print("Game completed 100%!");
								MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
							}
						}
					}
				}
			}
		}

		// If the end goal is treasure% we have to detect if you are in the treasure area
		// as there is no check for it.
		if (MOD_EndGoal == 2 && !MOD_TreasureComplete) {
			const char* szLevelName = GAM_fn_p_szGetLevelName();
			if (_stricmp(szLevelName, "vulca_20") == 0) {
				HIE_tdstSuperObject* pMain = HIE_fn_p_stFindObjectByName("StdCamer");
				if (pMain) {
					MTH3D_tdstVector* pCoords = &pMain->p_stGlobalMatrix->stPos;
					MTH_tdxReal dx = pCoords->x + 185.04;
					if (dx < 0) dx = -dx;
					MTH_tdxReal dy = pCoords->y - 120.24;
					if (dy < 0) dy = -dy;
					MTH_tdxReal dz = pCoords->z + 298.92;
					if (dz < 0) dz = -dz;

					if (dx <= 10 && dy <= 10 && dz <= 10) {
						MOD_TreasureComplete = TRUE;
						MOD_Print("Treasure ending complete!");
						MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
					}
				}
			}
		}

		// Set the collected cages for health to the custom value so health is overridden
		unsigned char cages = MOD_Cages;
		AI_fn_bSetDsgVar(pGlobal, 46, &cages);

		// Set the silver lum states based on the amount of upgrades
		if (MOD_Upgrades == 0) {
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1095, FALSE);
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1143, FALSE);
		} else if (MOD_Upgrades == 1) {
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1095, TRUE);
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1143, FALSE);
		} else {
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1095, TRUE);
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1143, TRUE);
		}

		// Set whether you have the elixir
		AI_fn_bSetBooleanInArray(pGlobal, 42, 1188, MOD_Elixir);

		// Ensure you can always complete the game
		if (MOD_EndGoal != 2) {
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1146, FALSE);
		}
	}
}

/** Ticked by the engine every frame, runs all messages received since last tick. */
void MOD_EngineTick() {
	MOD_RunPendingMessages();
	if (true || MOD_Connected) {
		MOD_CheckVariables();
	}
	GAM_fn_vEngine();
}

/** Handles the player dying and triggers a death link. */
void MOD_Init() {
	// Test if the player has died, this gets triggered once on death
	if (GAM_fn_ucGetEngineMode() == 7) {
		if (MOD_DeathLink && !MOD_DeathLinkOverride && !MOD_IgnoreDeath) {
			MOD_SendMessage(MESSAGE_TYPE_DEATH, "Rayman died");
		}
		MOD_IgnoreDeath = FALSE;
	}

	GAM_fn_vChooseTheGoodInit();
}

/** Updates the current progression state. */
void MOD_UpdateState(BOOL connected, int lums, int cages, int masks, int upgrades, BOOL deathLink, int endGoal, BOOL elixir, int* lumGates) {
	if (MOD_Connected != connected) {
		// Clear the collection cache whenever we reconnect so we resend all the information!
		clearBitSet(&MOD_LastCollected);
	}
	MOD_Connected = connected;
	MOD_Lums = lums;
	MOD_Cages = cages;
	MOD_Masks = masks;
	MOD_Upgrades = upgrades;
	MOD_DeathLink = deathLink;
	MOD_EndGoal = endGoal;
	MOD_Elixir = elixir;
	for (int i = 0; i < 6; i++) {
		MOD_LumGates[i] = lumGates[i];
	}
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

	long lSize = SPTXT_fn_lGetFmtStringLength(text, args);
	char* szBuffer = _alloca(lSize);

	if (szBuffer) {
		vsprintf(szBuffer, text, args);

		// Print the message to the cosole
#ifndef DISABLE_CONSOLE_PRINT
		fn_vPrint(szBuffer);
#endif

		// Print all messages to a log file
		FILE* pFile = fopen("parent_log.txt", "a");
		if (pFile != NULL) {
			fprintf(pFile, szBuffer);
			fprintf(pFile, "\n");
			fclose(pFile);
		}
	}
	va_end(args);
}

/** Prints a message to the console. */
void MOD_vShowScreenText(char* text, ...) {
	va_list args;
	va_start(args, text);
	long lSize = SPTXT_fn_lGetFmtStringLength(text, args);
	char* szBuffer = _alloca(lSize);
	if (szBuffer) {
		vsprintf(szBuffer, text, args);
		MOD_ShowScreenText(szBuffer);
	}
	va_end(args);
}

/** Shows the given text on the screen for the next 8 seconds. */
void MOD_ShowScreenText(char* text) {
	// Determine where to put this text
	MOD_ScreenTextLatest++;
	if (MOD_ScreenTextLatest >= 10) {
		MOD_ScreenTextLatest = 0;
	}

	// Determine the text to show
	MOD_ScreenTextStart[MOD_ScreenTextLatest] = time(NULL);
	char* screen = MOD_ScreenText[MOD_ScreenTextLatest];
	int size = min(127, strlen(text));
	strncpy(screen, text, size);

	// Filter out invalid characters that cannot be rendered
	for (int i = 0; i < size; i++) {
		if (screen[i] == '\n' || screen[i] == '\r') {
			screen[i] = ' ';
		}
	}
	screen[size] = 0;
}

/** Returns whether death link is currently enabled. */
BOOL MOD_GetDeathLink() {
	return MOD_DeathLink;
}

/** Updates the current state of the death link setting. */
void MOD_ToggleDeathLink() {
	MOD_DeathLinkOverride = !MOD_DeathLinkOverride;
	if (!MOD_DeathLinkOverride) {
		MOD_Print("Deathlink is now enabled again, be careful!");
	} else {
		MOD_Print("Deathlink has been disabled, you're safe now!");
	}
}

/** Draws text to the screen with the recent screen text and progression statistics. */
void CALLBACK MOD_vTextCallback(SPTXT_tdstTextInfo* pInfo) {
	// Determine the current time
	time_t currentTime = time(NULL);

	// Draw the screen text to the screen
	pInfo->xSize = 11;
	pInfo->bFrame = TRUE;
	pInfo->X = 10;
	pInfo->Y = 990;

	long lineHeight = SPTXT_fn_lGetCharHeight(pInfo->xSize);
	for (int i = 9; i >= 0; i--) {
		// Ignore any lines that have finished fading out
		int j = MOD_ScreenTextLatest - i;
		if (j < 0) {
			j += 10;
		}
		time_t startTime = MOD_ScreenTextStart[j];
		int timePassed = currentTime - startTime;
		if (timePassed > SCREEN_TEXT_FADE_TIME) continue;

		// Write the line and then move the Y up
		char* screen = MOD_ScreenText[j];
		pInfo->Y = pInfo->Y - lineHeight;
		SPTXT_vPrint(screen);
	}

	// Draw the current Archipelago progression to the bottom in the hall of doors or on the pause screen
	const char* szLevelName = GAM_fn_p_szGetLevelName();
	if (MOD_Connected && (_stricmp(szLevelName, "mapmonde") == 0 || *AI_g_bInGameMenu)) {
		pInfo->xSize = 6;
		pInfo->bRightAlign = TRUE;
		pInfo->X = 995;
		pInfo->Y = 990 - 3 * lineHeight;
		SPTXT_vPrintFmtLine("/o200:Archipelago Received");
		pInfo->Y = 990 - 2 * lineHeight;
		SPTXT_vPrintFmtLine("/o400:Lums /o0:%d of 1000/o400:, Cages /o0:%d of 80", MOD_Lums, MOD_Cages);
		pInfo->Y = 990 - lineHeight;
		SPTXT_vPrintFmtLine("/o400:Masks /o0:%d of 4/o400:, /o400:Power /o0:%d of 2/o400:, Elixir %s", MOD_Masks, MOD_Upgrades, MOD_Elixir ? "/o0:Yes" : "/o200:No");
	}
	SPTXT_vResetTextInfo(pInfo);
}

void MOD_Main(void) {
	// Seed the random number generator
	srand(time(NULL));

	// Clear the collection bitset
	clearBitSet(&MOD_LastCollected);

	// Initialize commands
	MOD_InitCommands();

	// Initialize on-screen text
	SPTXT_vInit();
	SPTXT_vAddTextCallback(MOD_vTextCallback);
}