#include "mod.h"

#define SCREEN_TEXT_FADE_TIME 8
#define TEXT_MARGIN 2
#define MAX_LENGTH 32

#define MOD_PrintConsolePlusScreen(txt, ...)              \
    do {                                 \
        MOD_Print((txt), __VA_ARGS__); \
        MOD_ShowScreenText((txt), __VA_ARGS__); \
    } while (0)

// Store the base game lum amounts and super lum ids
int* BASE_GAME_LUMS[6] = { 100, 300, 475, 550, 60, 450 };
int* SUPER_LUM_IDS[290] = { 1, 2, 3, 4, 5, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 51, 52, 53, 54, 55, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 161, 162, 163, 164, 165, 172, 173, 174, 175, 176, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 292, 293, 294, 295, 296, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 518, 519, 520, 521, 522, 556, 557, 558, 559, 560, 613, 614, 615, 616, 617, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 646, 647, 648, 649, 650, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 721, 722, 723, 724, 725, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 762, 763, 764, 765, 766, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 1311, 1312, 1313, 1314, 1315, 1354, 1355, 1356, 1357, 1358, 1389, 1390, 1391, 1392, 1393 };

// Store hardcoded level gate identifiers
int* NO_LUM_GATE_LEVELS[4] = { 960, 961, 964, 967 };
int* LUM_GATE_ONE_LEVELS[4] = { 970, 972, 976, 979};
int* LUM_GATE_TWO_LEVELS[5] = { 981, 975, 985, 988, 990 };
int* LUM_GATE_THREE_LEVELS[2] = { 993, 1007 };
int* LUM_GATE_FOUR_LEVELS[2] = { 1000, 1002 };
int COBD_LEVEL = 966;
int FINAL_LEVEL = 1005;

// Store archipelago progression
int MOD_Lums = 0;
int MOD_Cages = 0;
int MOD_Masks = 0;
int MOD_Upgrades = 0;
int MOD_EndGoal = 1;
BOOL MOD_Elixir = FALSE;
BOOL MOD_Knowledge = FALSE;

// Store archipelago settings
BOOL MOD_Connected = FALSE;
BOOL MOD_DeathLink = FALSE;
BOOL MOD_Lumsanity = FALSE;
BOOL MOD_RoomRandomisation = FALSE;
int* MOD_LumGates[6];
char MOD_LevelSwapSource[LEVEL_COUNT][MAX_LENGTH];
char MOD_LevelSwapTarget[LEVEL_COUNT][MAX_LENGTH];

// Store tracking variables used at runtime
char* MOD_LastEntered;
BOOL MOD_DeathLinkOverride = FALSE;
BOOL MOD_IgnoreDeath = FALSE;
BOOL MOD_TreasureComplete = FALSE;
BOOL MOD_InLumGate = FALSE;
int MOD_CurrentLumGate = -1;
BOOL MOD_KnowledgeSent = FALSE;

// Store variables for the screen text system
char MOD_ScreenText[10][128];
time_t MOD_ScreenTextStart[10];
int MOD_ScreenTextLatest = -1;

// Store sets with collected objects for comparison
BitSet MOD_LastCollected;
BitSet MOD_DevCollected;
BitSet MOD_RealCollected;

// Add mappings for custom level ids that differ from remote ids
#define MOD_CustomLevelCount 3
char* MOD_CustomLevelIdsStart[MOD_CustomLevelCount] = {"morb_10$01$00", "rodeo_40$01", "plum_00$01$00"};
char* MOD_CustomLevelIdsTarget[MOD_CustomLevelCount] = {"morb_10", "rodeo_40", "plum_00"};

// Store whether dev mode is enabled
BOOL MOD_DevMode = FALSE;

/** Compares to integers in an array. */
static int cmpInt(const void* a, const void* b) {
	int x = *(const int*)a;
	int y = *(const int*)b;
	return (x > y) - (x < y);
}

/** Returns whether the given integer is a super lum's id. */
bool isSuperLum(int x) {
	return bsearch(&x, SUPER_LUM_IDS, 290, sizeof(int), cmpInt) || bsearch(&(int) { x - 1 }, SUPER_LUM_IDS, 290, sizeof(int), cmpInt);
}

// https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
int compareStringCaseInsensitive(char const* a, char const* b) {
	return _stricmp(a, b);
}

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
	// In dev mode print which locations we switch to
	if (MOD_DevMode) {
		MOD_PrintConsolePlusScreen("Changing level to %s", szLevelName);
	}

	if (!MOD_Connected || compareStringCaseInsensitive(szLevelName, "menu") == 0 || compareStringCaseInsensitive(szLevelName, "mapmonde") == 0) {
		// If we have a level we previously marked as having entered, set the exit portal id!
		if (MOD_LastEntered) {
			GAM_tdstEngineStructure* structure = GAM_g_stEngineStructure;
			if (compareStringCaseInsensitive(MOD_LastEntered, "Learn_10") == 0) {
				structure->ucPreviousLevel = 3;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "Learn_30") == 0) {
				structure->ucPreviousLevel = 10;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "Ski_10") == 0) {
				structure->ucPreviousLevel = 25;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "vulca_10") == 0) {
				structure->ucPreviousLevel = 135;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "chase_10") == 0) {
				structure->ucPreviousLevel = 15;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "Ly_10") == 0) {
				structure->ucPreviousLevel = 20;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "rodeo_10") == 0) {
				structure->ucPreviousLevel = 55;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "water_10") == 0) {
				structure->ucPreviousLevel = 160;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "glob_30") == 0) {
				structure->ucPreviousLevel = 210;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "whale_00") == 0) {
				structure->ucPreviousLevel = 45;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "plum_00") == 0) {
				structure->ucPreviousLevel = 195;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "bast_09") == 0 ||
				compareStringCaseInsensitive(MOD_LastEntered, "bast_10") == 0) {
				structure->ucPreviousLevel = 130;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "nave_10") == 0) {
				structure->ucPreviousLevel = 80;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "seat_10") == 0) {
				structure->ucPreviousLevel = 40;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "earth_10") == 0) {
				structure->ucPreviousLevel = 95;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "Ly_20") == 0) {
				structure->ucPreviousLevel = 115;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "helic_10") == 0) {
				structure->ucPreviousLevel = 105;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "morb_00") == 0) {
				structure->ucPreviousLevel = 118;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "learn_40") == 0) {
				structure->ucPreviousLevel = 12;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "Boat01") == 0) {
				structure->ucPreviousLevel = 140;
			} else if (compareStringCaseInsensitive(MOD_LastEntered, "Rhop_10") == 0) {
				structure->ucPreviousLevel = 145;
			} else {
				// Fallback is woods of light!
				structure->ucPreviousLevel = 3;
			}
			if (MOD_DevMode) MOD_Print("Restored last entry level from %s which is id %d", MOD_LastEntered, structure->ucPreviousLevel);
			MOD_LastEntered = NULL;
		}

		if (MOD_DevMode) MOD_Print("GAM_fn_vAskToChangeLevel (ignore): %s", szLevelName);
		GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
		return;
	}
	
	// If it's the final level, check if you have enough masks to enter!
	if (compareStringCaseInsensitive(szLevelName, "Rhop_10") == 0) {
		if (MOD_Masks < 4) {
			if (MOD_DevMode) MOD_Print("GAM_fn_vAskToChangeLevel (not enough masks, redirecting to Pirate Ship)");
			szLevelName = "Boat_10";
			MOD_ShowScreenText("Not enough masks collected, you have %d out of 4!", MOD_Masks);
		} else {
			if (MOD_DevMode) MOD_Print("GAM_fn_vAskToChangeLevel (enough masks): %s", szLevelName);
			GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
			return;
		}
	}

	// When entering the ending credits we check off winning!
	if (compareStringCaseInsensitive(szLevelName, "end_10") == 0) {
		if (MOD_EndGoal == 1) {
			// If the goal is the crow's nest, you got it!
			MOD_PrintConsolePlusScreen("Game completed!");
			MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
		} else if (MOD_EndGoal == 3) {
			// If the goal is 100% we also require having everything!
			int hasEnoughLums = MOD_Lums >= 1000;
			int hasEnoughCages = MOD_Cages >= 80;
			if (!hasEnoughLums) {
				MOD_PrintConsolePlusScreen("Game is not complete, not enough lums!");
			} else if (!hasEnoughCages) {
				MOD_PrintConsolePlusScreen("Game is not complete, not enough cages!");
			} else {
				MOD_PrintConsolePlusScreen("Game completed 100%!");
				MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
			}
		}
	}

	// When we enter a level, store which one we wanted to enter!
	if (!MOD_LastEntered) {
		if (compareStringCaseInsensitive(szLevelName, "nego_10") != 0 &&
			compareStringCaseInsensitive(szLevelName, "batam_10") != 0 &&
			compareStringCaseInsensitive(szLevelName, "batam_20") != 0) {
			MOD_LastEntered = szLevelName;
		}
	}

	// First try to map this level id if it's a broken one
	char* compareLevel = szLevelName;
	for (int i = 0; i < MOD_CustomLevelCount; i++) {
		if (compareStringCaseInsensitive(compareLevel, MOD_CustomLevelIdsStart[i]) == 0) {
			compareLevel = MOD_CustomLevelIdsTarget[i];
		}
	}

	// Find which map to send the player to instead of the basic one
	int oldId = -1;
	for (int i = 0; i < LEVEL_COUNT; i++) {
		if (compareStringCaseInsensitive(compareLevel, MOD_LevelSwapSource[i]) == 0) {
			oldId = i;
		}
	}
	if (oldId == -1) {
		if (MOD_DevMode) MOD_Print("GAM_fn_vAskToChangeLevel (old id -1): %s", szLevelName);
		GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
		return;
	}

	// Get the new target and send them there
	char* targetLevelName = MOD_LevelSwapTarget[oldId];
	if (MOD_DevMode) MOD_Print("GAM_fn_vAskToChangeLevel (modified): %s -> %s", szLevelName, targetLevelName);
	GAM_fn_vAskToChangeLevel(targetLevelName, bSaveGame);
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

/** Clears lum gate overrides. */
void clearLumGateOverrides() {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal && MOD_InLumGate) {
		MOD_InLumGate = false;
		MOD_CurrentLumGate = -1;
		for (int i = 1; i <= 1400; i++) {
			AI_fn_bSetBooleanInArray(pGlobal, 42, i, getBitSet(&MOD_RealCollected, i));
		}
	}
}

/** Sets lum values to the lum gates of the given id. */
void setLumGateOverride(int lumGateId) {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");

	if (pGlobal && MOD_CurrentLumGate != lumGateId) {
		if (MOD_InLumGate) {
			// If we were already in a gate, clear overrides first before we re-apply!
			clearLumGateOverrides();
		}

		MOD_InLumGate = true;
		MOD_CurrentLumGate = lumGateId;

		// Determine how many lums we should have and what the base game will
		// expect, then change lums to be at exactly the right value to jank it all together.
		int baseGameLums = BASE_GAME_LUMS[lumGateId];
		int lumGateLums = MOD_LumGates[lumGateId];
		int missingLums = lumGateLums - MOD_Lums;
		if (missingLums < 0) {
			missingLums = 0;
		}
		int finalLums = baseGameLums - missingLums;
		int givenLums = 0;

		// Clear data in case it's leftover from a previous load
		clearBitSet(&MOD_RealCollected);

		for (int i = 1; i <= 1400; i++) {
			// Copy out the data into the real collection
			setBitSet(&MOD_RealCollected, i, AI_fn_bGetBooleanInArray(pGlobal, 42, i));

			// Update the data to set the correct lum count we want
			if ((i >= 1 && i <= 800) || (i >= 1201 && i <= 1400)) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, i, givenLums++ < finalLums);
			}
		}
	}
}

/** Checks if any lums/cages have been collected since last frame. */
void MOD_CheckVariables() {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal) {
		// If we're in the lum gate level we customise everything!
		const char* szLevelName = GAM_fn_p_szGetLevelName();

		// If we're in a lum gate, override the lum gate values.
		if (compareStringCaseInsensitive(szLevelName, "Nego_10") == 0) {
			if (!MOD_InLumGate) {
				HIE_tdstSuperObject* pLums = HIE_fn_p_stFindObjectByName("NIK_DS1_ZyvaEnvoieTesLums");
				if (pLums) {
					int* levelId;
					AI_fn_bGetDsgVar(pLums, 27, NULL, &levelId);
					int lumGateId = 0;
					if (*levelId == 21) {
						lumGateId = 1;
					} else if (*levelId == 33) {
						lumGateId = 2;
					} else if (*levelId == 40) {
						lumGateId = 3;
					}
					setLumGateOverride(lumGateId);
				}
			}
			return;
		}

		// If you get close to the walk of life or power pedestal we update the lum values!
		if (compareStringCaseInsensitive(szLevelName, "chase_10") == 0) {
			HIE_tdstSuperObject* pMain = HIE_fn_p_stFindObjectByName("StdCamer");
			if (pMain) {
				MTH3D_tdstVector* pCoords = &pMain->p_stGlobalMatrix->stPos;
				MTH_tdxReal dx = pCoords->x + 85.0;
				if (dx < 0) dx = -dx;
				MTH_tdxReal dy = pCoords->y + 118.0;
				if (dy < 0) dy = -dy;
				MTH_tdxReal dz = pCoords->z - 31.0;
				if (dz < 0) dz = -dz;

				if (dx <= 20 && dy <= 10 && dz <= 10) {
					setLumGateOverride(4);
					return;
				}
			}
		}
		if (compareStringCaseInsensitive(szLevelName, "earth_10") == 0) {
			HIE_tdstSuperObject* pMain = HIE_fn_p_stFindObjectByName("StdCamer");
			if (pMain) {
				MTH3D_tdstVector* pCoords = &pMain->p_stGlobalMatrix->stPos;
				MTH_tdxReal dx = pCoords->x + 15927.23;
				if (dx < 0) dx = -dx;
				MTH_tdxReal dy = pCoords->y - 3537.60;
				if (dy < 0) dy = -dy;
				MTH_tdxReal dz = pCoords->z + 7279.12;
				if (dz < 0) dz = -dz;

				if (dx <= 20 && dy <= 20 && dz <= 20) {
					setLumGateOverride(5);
					return;
				}
			}
		}

		// When we exit the lum gate, restore the data again!
		clearLumGateOverrides();

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
					// Send up the id of the item directly
					char str[6];
					sprintf(str, "%d", i);
					MOD_SendMessage(MESSAGE_TYPE_COLLECTED, str);
				}
			}
		}

		// If the end goal is treasure% we have to detect if you are in the treasure area
		// as there is no check for it.
		if (MOD_EndGoal == 2 && !MOD_TreasureComplete) {
			if (compareStringCaseInsensitive(szLevelName, "vulca_20") == 0) {
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
						MOD_PrintConsolePlusScreen("Treasure ending complete!");
						MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
					}
				}
			}
		}

		// Check if we are in the second part of Menhir Hills and send out info on the Cave of Bad Dreams's location
		if (!MOD_KnowledgeSent) {
			if (compareStringCaseInsensitive(szLevelName, "rodeo_40$02") == 0) {
				MOD_KnowledgeSent = TRUE;
				MOD_PrintConsolePlusScreen("Learned about the Cave of Bad Dreams's name");
				MOD_SendMessage(MESSAGE_TYPE_COLLECTED, "1101");
			}
		}

		// Set the collected cages for health to the custom value so health is overridden,
		// if the value would be 9 always make it 8 so it doesn't incorrectly give a health
		// increase when a new cage is collected for one frame.
		int fakeCages = MOD_Cages;
		if (fakeCages % 10 == 9) {
			fakeCages -= 1;
		}
		unsigned char cages = fakeCages;
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

		// Set whether you have the elixir and knowledge of the cave of bad dreams
		AI_fn_bSetBooleanInArray(pGlobal, 42, 1123, MOD_Elixir);
		AI_fn_bSetBooleanInArray(pGlobal, 42, 1101, MOD_Knowledge);

		// This ensures the safe file loads into the hall of doors always.
		AI_fn_bSetBooleanInArray(pGlobal, 42, 1133, TRUE);

		// Update which portals are available based on the current checks
		for (int i = 0; i < 4; i++) {
			AI_fn_bSetBooleanInArray(pGlobal, 42, NO_LUM_GATE_LEVELS[i], TRUE);
		}
		if (MOD_Lums >= MOD_LumGates[0]) {
			for (int i = 0; i < 4; i++) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, LUM_GATE_ONE_LEVELS[i], TRUE);
			}
		}
		if (MOD_Lums >= MOD_LumGates[1]) {
			for (int i = 0; i < 5; i++) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, LUM_GATE_TWO_LEVELS[i], TRUE);
			}
		}
		if (MOD_Lums >= MOD_LumGates[2]) {
			for (int i = 0; i < 2; i++) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, LUM_GATE_THREE_LEVELS[i], TRUE);
			}
		}
		if (MOD_Lums >= MOD_LumGates[3]) {
			for (int i = 0; i < 2; i++) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, LUM_GATE_FOUR_LEVELS[i], TRUE);
			}
		}
		AI_fn_bSetBooleanInArray(pGlobal, 42, FINAL_LEVEL, MOD_Masks >= 4);
	}
}

/** Ticked by the engine every frame, runs all messages received since last tick. */
void MOD_EngineTick() {
	MOD_RunPendingMessages();

	// In dev mode we send out messages whenever checks happen!
	if (MOD_DevMode) {
		HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
		if (pGlobal) {
			for (int i = 1; i <= 1400; i++) {
				unsigned char last = getBitSet(&MOD_DevCollected, i);
				ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, i);

				// If there's a desync between the local storage and the DSG variables
				// we collected an item, send it across!
				if (last != dsg) {
					setBitSet(&MOD_DevCollected, i, dsg);

					// Only if the item is now collected, send a check!
					if (dsg) {
						// While testing we show collected items on screen
						MOD_PrintConsolePlusScreen("Collected %d", i);
					}
				}
			}
		}
	}

	if (MOD_Connected) {
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

/** Updates the current archipelago settings. */
void MOD_UpdateSettings(BOOL connected, BOOL deathLink, int endGoal, BOOL lumsanity, BOOL roomRandomisation, int* lumGates, char** levelSwapKeys, char** levelSwapTargets) {
	if (MOD_Connected != connected) {
		// Clear the collection cache whenever we reconnect so we resend all the information!
		clearBitSet(&MOD_LastCollected);
	}
	MOD_Connected = connected;
	MOD_DeathLink = deathLink;
	MOD_EndGoal = endGoal;
	MOD_Lumsanity = lumsanity;
	MOD_RoomRandomisation = roomRandomisation;
	for (int i = 0; i < 6; i++) {
		MOD_LumGates[i] = lumGates[i];
	}
	for (int i = 0; i < LEVEL_COUNT; i++) {
		strncpy(MOD_LevelSwapSource[i], levelSwapKeys[i], MAX_LENGTH - 1);
	}
	for (int i = 0; i < LEVEL_COUNT; i++) {
		strncpy(MOD_LevelSwapTarget[i], levelSwapTargets[i], MAX_LENGTH - 1);
	}
}

/** Updates the current progression state. */
void MOD_UpdateState(int lums, int cages, int masks, int upgrades, BOOL elixir, BOOL knowledge) {
	MOD_Lums = lums;
	MOD_Cages = cages;
	MOD_Masks = masks;
	MOD_Upgrades = upgrades;
	MOD_Elixir = elixir;
	MOD_Knowledge = knowledge;

	// If we're not on lumsanity we need to include any non-super lums collected locally!
	if (!MOD_Lumsanity) {
		HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
		if (pGlobal) {
			for (int i = 1; i <= 800; i++) {
				ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, i);
				if (dsg) {
					// Ignore any super lums!
					if (isSuperLum(i)) continue;
					MOD_Lums++;
				}
			}
			for (int i = 1201; i <= 1400; i++) {
				ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, i);
				if (dsg) {
					// Ignore any super lums!
					if (isSuperLum(i)) continue;
					MOD_Lums++;
				}
			}
		}
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

/** Shows the given text on the screen for the next 8 seconds. */
void MOD_ShowScreenTextInternal(char* text) {
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

/** Prints a message to the console. */
void MOD_ShowScreenText(char* text, ...) {
	va_list args;
	va_start(args, text);
	long lSize = SPTXT_fn_lGetFmtStringLength(text, args);
	char* szBuffer = _alloca(lSize);
	if (szBuffer) {
		vsprintf(szBuffer, text, args);
		MOD_ShowScreenTextInternal(szBuffer);
	}
	va_end(args);
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

	long bigLineHeight = SPTXT_fn_lGetCharHeight(pInfo->xSize);
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
		pInfo->Y = pInfo->Y - bigLineHeight;
		SPTXT_vPrint(screen);
	}

	// Draw the current Archipelago progression to the bottom in the hall of doors or on the pause screen
	const char* szLevelName = GAM_fn_p_szGetLevelName();
	if (MOD_Connected && (compareStringCaseInsensitive(szLevelName, "mapmonde") == 0 || *AI_g_bInGameMenu)) {
		pInfo->xSize = 6;
		pInfo->bRightAlign = TRUE;
		long lineHeight = SPTXT_fn_lGetCharHeight(pInfo->xSize);

		pInfo->X = 995;
		pInfo->Y = 990 - 4 * lineHeight;
		SPTXT_vPrintFmtLine("/o200:Archipelago Received");
		pInfo->Y = 990 - 3 * lineHeight;
		SPTXT_vPrintFmtLine("/o400:Lums /o0:%d of 1000/o400:, Cages /o0:%d of 80", MOD_Lums, MOD_Cages);
		pInfo->Y = 990 - 2 * lineHeight;
		SPTXT_vPrintFmtLine("/o400:Masks /o0:%d of 4/o400:, Power /o0:%d of 2", MOD_Masks, MOD_Upgrades);
		pInfo->Y = 990 - lineHeight;
		SPTXT_vPrintFmtLine("/o400:Elixir %s/o400:, Knowledge %s", MOD_Elixir ? "/o0:Yes" : "/o200:No", MOD_Knowledge ? "/o0:Yes" : "/o200:No");
	}
	SPTXT_vResetTextInfo(pInfo);
}

void MOD_Main(void) {
	// Clear the collection bitset
	clearBitSet(&MOD_LastCollected);

	// Initialize commands
	MOD_InitCommands();

	// Initialize on-screen text
	SPTXT_vInit();
	SPTXT_vAddTextCallback(MOD_vTextCallback);
}