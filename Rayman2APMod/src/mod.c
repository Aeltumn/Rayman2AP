#include "mod.h"

#define SCREEN_TEXT_FADE_TIME 8
#define TEXT_MARGIN 2

#define MOD_PrintConsolePlusScreen(txt, ...)              \
    do {                                 \
        MOD_Print((txt), __VA_ARGS__); \
        MOD_ShowScreenText((txt), __VA_ARGS__); \
    } while (0)

// Define the indices of every level chain
int CHAIN_BAYOU = 0;
int CHAIN_BENEATH = 1;
int CHAIN_CANOPY = 2;
int CHAIN_COBD = 3;
int CHAIN_ECHOING = 4;
int CHAIN_FAIRY_GLADE = 5;
int CHAIN_FAIRY_REVISIT = 6;
int CHAIN_IRON_MOUNT = 7;
int CHAIN_MARSHES = 8;
int CHAIN_MENHIR = 9;
int CHAIN_PRECIPICE = 10;
int CHAIN_PRISON = 11;
int CHAIN_SANC_ROCK = 12;
int CHAIN_SANC_STONE = 13;
int CHAIN_SANC_WATER = 14;
int CHAIN_SIDE_TEMPLE = 15;
int CHAIN_TOMB = 16;
int CHAIN_TOP = 17;
int CHAIN_WALK_LIFE = 18;
int CHAIN_WALK_POWER = 19;
int CHAIN_WHALE = 20;

// Track important portal ids
int* NO_LUM_GATE_LEVELS[3] = { 961, 964, 967 };
int* LUM_GATE_ONE_LEVELS[4] = { 970, 972, 976, 979 };
int* LUM_GATE_TWO_LEVELS[5] = { 981, 975, 985, 988, 990 };
int* LUM_GATE_THREE_LEVELS[2] = { 993, 1007 };
int* LUM_GATE_FOUR_LEVELS[2] = { 1000, 1002 };
int PIRATE_SHIP_PORTAL = 1002;
int FINAL_LEVEL = 1005;

// Store the base game lum amounts and super lum ids
int* BASE_GAME_LUMS[6] = { 100, 300, 475, 550, 60, 450 };
int* SUPER_LUM_IDS[290] = { 1, 2, 3, 4, 5, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 51, 52, 53, 54, 55, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 161, 162, 163, 164, 165, 172, 173, 174, 175, 176, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 292, 293, 294, 295, 296, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 556, 557, 558, 559, 560, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 646, 647, 648, 649, 650, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 721, 722, 723, 724, 725, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 762, 763, 764, 765, 766, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 1311, 1312, 1313, 1314, 1315, 1354, 1355, 1356, 1357, 1358, 1389, 1390, 1391, 1392, 1393 };

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
BOOL MOD_AccessiblePortals = FALSE;
int* MOD_LumGates[6];

// Store tracking variables used at runtime
BOOL MOD_DeathLinkOverride = FALSE;
BOOL MOD_IgnoreDeath = FALSE;
BOOL MOD_TreasureComplete = FALSE;
BOOL MOD_InLumGate = FALSE;
int MOD_CurrentLumGate = -1;

// Store variables for the screen text system
char MOD_ScreenText[10][128];
time_t MOD_ScreenTextStart[10];
int MOD_ScreenTextLatest = -1;

// Store sets with collected objects for comparison
BitSet MOD_LastCollected;
BitSet MOD_DevCollected;
BitSet MOD_RealCollected;
int MOD_VariableCheckTicks = 0;

// While in the Menhir Hills store if you had the elixir beforehand
BOOL MOD_InMenhirHills = FALSE;
BOOL MOD_HadElixirPreviously = FALSE;
BOOL MOD_SentKnowledgeOfCOBD = FALSE;

// While sending you back to Marshes we lie about having finshed COBD but we need to put things back in place after
BOOL MOD_InMarshes = FALSE;
BOOL MOD_HadFinishedCOBDPreviously = FALSE;

// When playing The Canopy we have to hide the portal otherwise Globox thinks he's allowed outside
BOOL MOD_InCanopy = FALSE;
BOOL MOD_HasSavedGloboxPreviously = FALSE;

// Level chain info
char MOD_LevelIds[LEVEL_COUNT][MAX_LENGTH];
int MOD_LevelChainsLengths[CHAIN_COUNT];
BOOL MOD_InitLevelChains = FALSE;
int* MOD_LevelChainContents[CHAIN_COUNT];

BOOL MOD_IgnoreRedirect = FALSE;
BOOL MOD_InLevelChain = FALSE;
int MOD_LevelChainActive[CHAIN_COUNT];
int MOD_LevelChainLast[CHAIN_COUNT];
int MOD_LevelChainCurrent = -1;

// Store whether dev mode is enabled
BOOL MOD_DevMode = FALSE;

/** Compares to integers in an array. */
static int cmpInt(const void* a, const void* b) {
	int x = *(const int*)a;
	int y = *(const int*)b;
	return (x > y) - (x < y);
}

/** Returns whether x is a lum or super lum. */
bool isLumLike(int x) {
	return (x >= 1 && x <= 800) || (x >= 1201 && x <= 1400);
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

long SPTXT_fn_lGetCharWidth(MTH_tdxReal xSize) {
	MTH_tdxReal size = 15.0f - xSize;
	MTH_tdxReal width = 46.0f - size * 4.0f;
	return (long)width;
}

long SPTXT_fn_lGetCharHeight(MTH_tdxReal xSize) {
	MTH_tdxReal size = 15.0f - xSize;
	MTH_tdxReal height = 38.0f - size * 2.5f;
	return (long)height + TEXT_MARGIN + TEXT_MARGIN;
}

BOOL MOD_InDevMode() {
	return MOD_DevMode;
}

BOOL MOD_CanProgressChain() {
	// Returns whether there is a next level in the chain to progress
	// to, or if the level has to be manually exited.
	if (MOD_InLevelChain) {
		int chainId = MOD_LevelChainCurrent;
		int currentLevel = MOD_LevelChainActive[chainId] + 1;
		int chainLength = MOD_LevelChainsLengths[chainId];
		return currentLevel > 0 && currentLevel < chainLength;
	}
	return false;
}

BOOL MOD_ProgressLevelChainAndIncrement(int increment) {
	// When we exit the lum gate, restore the data again!
	MOD_ClearLumGateOverrides();

	// If we're in a level chain, continue it!
	if (MOD_InLevelChain) {
		// Determine the current level chain
		int chainId = MOD_LevelChainCurrent;

		// Get the level we are meant to be in and load into it
		int currentLevel = MOD_LevelChainActive[chainId] + increment;
		int chainLength = MOD_LevelChainsLengths[chainId];

		// Save the new index on this chain
		MOD_LevelChainActive[chainId] = currentLevel;

		if (currentLevel < 0 || currentLevel >= chainLength) {
			// If you finish the side temple or revisit we have to move you back to where you were!
			MOD_ExitChain(TRUE);
		} else {
			// Find which level ID this is
			int levelId = MOD_LevelChainContents[chainId][currentLevel];
			char* levelName = MOD_LevelIds[levelId];

			// Learn_32 is the internal ID for the fairy glade revisit!
			if (compareStringCaseInsensitive(levelName, "Learn_31") == 0) {
				// If you go to the normal second zone of fairy glade make sure
				// we send you with the right level id!
				GAM_g_stEngineStructure->ucPreviousLevel = 10;
			} else if (compareStringCaseInsensitive(levelName, "Learn_32") == 0) {
				levelName = "Learn_31";

				// 70 is cask_10 which makes it use the right entrance!
				GAM_g_stEngineStructure->ucPreviousLevel = 70;
			}

			HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
			if (pGlobal) {
				// Before moving into The Canopy we need to ensure Globox is there!
				// 979 is portal but also whether globox is present in the level
				if (compareStringCaseInsensitive(levelName, "glob_10") == 0) {
					if (!MOD_InCanopy) {
						// DSG 957 stores if you opened the gate at the end of Canopy #2, if you
						// have that you have beat this level before and we don't want to repeat the sequence.
						MOD_HasSavedGloboxPreviously = AI_fn_bGetBooleanInArray(pGlobal, 42, 979);
						BOOL FinishedCanopy = AI_fn_bGetBooleanInArray(pGlobal, 42, 957);
						AI_fn_vSetBooleanInArray(pGlobal, 42, 979, FinishedCanopy);
						MOD_InCanopy = TRUE;
					}
				}
				if (compareStringCaseInsensitive(levelName, "glob_20") == 0) {
					if (!MOD_InCanopy) {
						// DSG 873 stores if you freed the teensie at the end of the level, if you
						// have that you have beat this level before and we don't want to repeat the sequence.
						MOD_HasSavedGloboxPreviously = AI_fn_bGetBooleanInArray(pGlobal, 42, 979);
						BOOL FinishedCanopy = AI_fn_bGetBooleanInArray(pGlobal, 42, 873);
						AI_fn_vSetBooleanInArray(pGlobal, 42, 979, FinishedCanopy);
						MOD_InCanopy = TRUE;
					}
				}

				// If we enter a walk level or COBD we spawn their portal which makes them
				// accessible from the HOF and remove the lum check.
				if (chainId == CHAIN_WALK_LIFE) {
					AI_fn_vSetBooleanInArray(pGlobal, 42, 969, TRUE);
				} else if (chainId == CHAIN_WALK_POWER) {
					AI_fn_vSetBooleanInArray(pGlobal, 42, 992, TRUE);
				} else if (chainId == CHAIN_COBD) {
					AI_fn_vSetBooleanInArray(pGlobal, 42, 966, TRUE);
				}
			}

			// Actually enter the level!
			GAM_fn_vAskToChangeLevel(levelName, TRUE);
		}
	}
	return false;
}

BOOL MOD_ProgressLevelChain() {
	MOD_ProgressLevelChainAndIncrement(1);
}

void MOD_EnterLevelChain(int chainId) {
	if (MOD_InLevelChain) {
		// Store on the new chain that we were previously in the previous one.
		MOD_LevelChainLast[chainId] = MOD_LevelChainCurrent + 1;
	}

	// Mark down that we've entered a level chain, then determine what the first level is in this chain!
	MOD_InLevelChain = TRUE;
	MOD_LevelChainCurrent = chainId;
	MOD_ProgressLevelChainAndIncrement(0);
}

void MOD_ExitChain(ACP_tdxBool bSaveGame) {
	// Ignore if we are not in a chain!
	if (!MOD_InLevelChain) {
		GAM_fn_vAskToChangeLevel("mapmonde", bSaveGame);
		return;
	}

	// When we exit the lum gate, restore the data again!
	MOD_ClearLumGateOverrides();

	// Read data and then immediately void it as we exit
	int chainId = MOD_LevelChainCurrent;
	int lastChain = MOD_LevelChainLast[chainId] - 1;
	int currentLevel = MOD_LevelChainActive[chainId];
	int chainLength = MOD_LevelChainsLengths[chainId];

	// If we finish COBD we have to return to whichever chain contains Ski_10!
	// We may have exited to menu from COBD to world in which case we lost the previous chain
	// link.
	if (chainId == CHAIN_COBD && lastChain == -1) {
		for (int i = 0; i <= 20; i++) {
			int thisChainLength = MOD_LevelChainsLengths[i];
			for (int j = 0; j < thisChainLength; j++) {
				int chainLevelId = MOD_LevelChainContents[i][j];
				char* chainLevelName = MOD_LevelIds[chainLevelId];
				if (compareStringCaseInsensitive(chainLevelName, "Ski_10") == 0) {
					lastChain = i;

					// Ensure we return to Ski_10!
					MOD_LevelChainActive[i] = j;
					break;
				}
			}
			if (lastChain != -1) break;
		}
	}

	MOD_LevelChainCurrent = -1;
	MOD_InLevelChain = FALSE;
	MOD_LevelChainLast[chainId] = 0;
	MOD_LevelChainActive[chainId] = 0;

	// Determine if this chain was completed and we should unlock the next level!
	auto completedChain = currentLevel >= chainLength - 1;

	// Prevent completing the prison ship chain without all masks!
	if (chainId == CHAIN_PRISON && MOD_Masks < 4) {
		completedChain = FALSE;
	}

	// If you didn't complete the chain, didn't have a last chain, or it's the walks you return to the hall of doors instead of
	// your previous level you were in.
	if (!completedChain || lastChain == -1 || chainId == CHAIN_WALK_LIFE || chainId == CHAIN_WALK_POWER) {
		int entryLevelId = -1;
		if (chainId == CHAIN_FAIRY_GLADE) {
			entryLevelId = 10;
		} else if (chainId == CHAIN_MARSHES) {
			entryLevelId = 25;
		} else if (chainId == CHAIN_COBD) {
			entryLevelId = 135;
		} else if (chainId == CHAIN_BAYOU) {
			entryLevelId = 15;
		} else if (chainId == CHAIN_WALK_LIFE) {
			entryLevelId = 20;
		} else if (chainId == CHAIN_MENHIR) {
			entryLevelId = 55;
		} else if (chainId == CHAIN_SANC_WATER) {
			entryLevelId = 160;
		} else if (chainId == CHAIN_CANOPY) {
			entryLevelId = 210;
		} else if (chainId == CHAIN_WHALE) {
			entryLevelId = 45;
		} else if (chainId == CHAIN_SANC_STONE) {
			entryLevelId = 195;
		} else if (chainId == CHAIN_ECHOING) {
			entryLevelId = 130;
		} else if (chainId == CHAIN_PRECIPICE) {
			entryLevelId = 80;
		} else if (chainId == CHAIN_TOP) {
			entryLevelId = 40;
		} else if (chainId == CHAIN_SANC_ROCK) {
			entryLevelId = 95;
		} else if (chainId == CHAIN_WALK_POWER) {
			entryLevelId = 115;
		} else if (chainId == CHAIN_BENEATH) {
			entryLevelId = 105;
		} else if (chainId == CHAIN_TOMB) {
			entryLevelId = 118;
		} else if (chainId == CHAIN_IRON_MOUNT) {
			entryLevelId = 12;
		} else if (chainId == CHAIN_PRISON) {
			entryLevelId = 140;
		}

		// If there isn't a last change or the chain wasn't completed, let you go to the hall of doors with a specific exit!
		GAM_tdstEngineStructure* structure = GAM_g_stEngineStructure;
		if (completedChain) {
			// The ids of exit level are different if you completed a level!
			// (These ids are in STH_teleport_GEN_STH.
			structure->ucExitIdToQuitPrevLevel = 1;
			if (chainId == CHAIN_FAIRY_GLADE) {
				structure->ucPreviousLevel = 220;
			} else if (chainId == CHAIN_MARSHES) {
				structure->ucPreviousLevel = 30;
			} else if (chainId == CHAIN_COBD) {
				structure->ucPreviousLevel = 137;
			} else if (chainId == CHAIN_BAYOU) {
				structure->ucPreviousLevel = 18;
			} else if (chainId == CHAIN_WALK_LIFE) {
				structure->ucPreviousLevel = 20;
			} else if (chainId == CHAIN_MENHIR) {
				structure->ucPreviousLevel = 65;
			} else if (chainId == CHAIN_SANC_WATER) {
				structure->ucPreviousLevel = 165;
			} else if (chainId == CHAIN_CANOPY) {
				structure->ucPreviousLevel = 205;
			} else if (chainId == CHAIN_WHALE) {
				structure->ucPreviousLevel = 50;
			} else if (chainId == CHAIN_SANC_STONE) {
				structure->ucPreviousLevel = 90;
			} else if (chainId == CHAIN_ECHOING) {
				structure->ucPreviousLevel = 75;
			} else if (chainId == CHAIN_PRECIPICE) {
				structure->ucPreviousLevel = 85;
			} else if (chainId == CHAIN_TOP) {
				structure->ucPreviousLevel = 41;
			} else if (chainId == CHAIN_SANC_ROCK) {
				structure->ucPreviousLevel = 103;
			} else if (chainId == CHAIN_WALK_POWER) { 
				structure->ucPreviousLevel = 115;
			} else if (chainId == CHAIN_BENEATH) {
				structure->ucPreviousLevel = 110;
			} else if (chainId == CHAIN_TOMB) {
				structure->ucPreviousLevel = 215;
			} else if (chainId == CHAIN_IRON_MOUNT) {
				structure->ucPreviousLevel = 125;
			} else if (chainId == CHAIN_PRISON) {
				structure->ucPreviousLevel = 240;
			}
		} else {
			if (entryLevelId != -1) {
				structure->ucPreviousLevel = entryLevelId;
			}
		}

		HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
		if (pGlobal) {
			// Update the portal the player comes out of when reloading the save!
			if (entryLevelId != -1) {
				int level = entryLevelId;
				AI_fn_bSetDsgVar(pGlobal, 67, &level);
			}
		}

		GAM_fn_vAskToChangeLevel("mapmonde", bSaveGame);
	} else {
		// If there was a specifc chain we are exiting we make sure to
		// set the level ID properly so the right exit is used!
		GAM_tdstEngineStructure* structure = GAM_g_stEngineStructure;
		structure->ucExitIdToQuitPrevLevel = 1;
		if (chainId == CHAIN_COBD) {
			structure->ucPreviousLevel = 137;

			// This triggers the cutscene granting the Elixir of Life after re-entering Ski_10!
			HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
			if (pGlobal && !MOD_InMarshes) {
				MOD_HadFinishedCOBDPreviously = AI_fn_bGetBooleanInArray(pGlobal, 42, 1120);
				AI_fn_vSetBooleanInArray(pGlobal, 42, 1120, TRUE);
				MOD_InMarshes = TRUE;
			}
		} else if (chainId == CHAIN_FAIRY_REVISIT) {
			structure->ucPreviousLevel = 11;
		} else if (chainId == CHAIN_SIDE_TEMPLE) {
			structure->ucPreviousLevel = 190;
		}

		MOD_EnterLevelChain(lastChain);
	}
}

void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame) {
	// Fetch the base game strucure as we'll have to modify it
	GAM_tdstEngineStructure* structure = GAM_g_stEngineStructure;

	// When we exit the lum gate, restore the data again before we make any edits!
	MOD_ClearLumGateOverrides();

	// Re-check for the COBD knowledge & treasure ending check on level switch
	MOD_SentKnowledgeOfCOBD = FALSE;
	MOD_TreasureComplete = FALSE;

	// When changing levels we return temporary overrides for DSG variables that we wanted
	// to last for just one level!
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal) {
		if (MOD_InMarshes) {
			MOD_InMarshes = FALSE;
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1120, MOD_HadFinishedCOBDPreviously);
		}
		if (MOD_InCanopy) {
			MOD_InCanopy = FALSE;
			AI_fn_vSetBooleanInArray(pGlobal, 42, 979, MOD_HasSavedGloboxPreviously);
		}
		if (MOD_InMenhirHills) {
			MOD_InMenhirHills = FALSE;
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1123, MOD_HadElixirPreviously);
		}
	}

	// In dev mode print which locations we switch to
	if (MOD_DevMode) {
		MOD_PrintConsolePlusScreen("Changing level to %s while previous level is %d with exit %d", szLevelName, structure->ucPreviousLevel, structure->ucExitIdToQuitPrevLevel);
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
				MOD_PrintConsolePlusScreen("Game not complete, not enough lums!");
				MOD_ExitChain(bSaveGame);
				return;
			} else if (!hasEnoughCages) {
				MOD_PrintConsolePlusScreen("Game not complete, not enough cages!");
				MOD_ExitChain(bSaveGame);
				return;
			} else {
				MOD_PrintConsolePlusScreen("Game completed 100 percent!");
				MOD_SendMessageE(MESSAGE_TYPE_COMPLETE);
			}
		}
	}

	// When you exit the Pirate Ship from the ending exit we put you back at the default exit unless you have enough masks.
	if (compareStringCaseInsensitive(szLevelName, "mapmonde") == 0) {
		if (MOD_Masks < 4 && structure->ucPreviousLevel == 240) {
			structure->ucPreviousLevel = 140;
		}
	}

	// If we're using room randomisation, change the layout!
	if (MOD_RoomRandomisation) {
		// We ignore exit 99 as that's what is used when moving to the menu and back.
		if (compareStringCaseInsensitive(szLevelName, "mapmonde") == 0 && structure->ucExitIdToQuitPrevLevel != 99) {
			// When entering the menu, update the previous level and exit to put you at the right spot!
			MOD_ExitChain(bSaveGame);
			return;
		} else {
			// When entering a level we have to determine which chain to move you towards!

			// Fairy Glade
			if (compareStringCaseInsensitive(szLevelName, "Learn_30") == 0) {
				MOD_EnterLevelChain(CHAIN_FAIRY_GLADE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "learn_31") == 0) {
				if (structure->ucPreviousLevel == 70) {
					// If the previous level is 70 this is the revisit from Echoing Caves!
					MOD_EnterLevelChain(CHAIN_FAIRY_REVISIT);
					return;
				} else {
					if (MOD_ProgressLevelChain()) return;
				}
			} else if (compareStringCaseInsensitive(szLevelName, "bast_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "bast_22") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "learn_60") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}
			
			// Marhes of Awakening
			if (compareStringCaseInsensitive(szLevelName, "Ski_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
				MOD_EnterLevelChain(CHAIN_MARSHES);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "ski_60") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Bayou
			if (compareStringCaseInsensitive(szLevelName, "chase_10") == 0) {
				MOD_EnterLevelChain(CHAIN_BAYOU);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "chase_22") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Sanctuary of Water and Ice
			if (compareStringCaseInsensitive(szLevelName, "water_10") == 0) {
				MOD_EnterLevelChain(CHAIN_SANC_WATER);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "water_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Menhir Hills
			if (compareStringCaseInsensitive(szLevelName, "rodeo_10") == 0) {
				MOD_EnterLevelChain(CHAIN_MENHIR);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "rodeo_40") == 0 ||
					   compareStringCaseInsensitive(szLevelName, "rodeo_40$01") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "rodeo_60") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Canopy
			if (compareStringCaseInsensitive(szLevelName, "glob_30") == 0) {
				MOD_EnterLevelChain(CHAIN_CANOPY);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "glob_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "glob_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Whale Bay
			if (compareStringCaseInsensitive(szLevelName, "whale_00") == 0) {
				MOD_EnterLevelChain(CHAIN_WHALE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "whale_05") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "whale_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Sanctuary of Stone and Fire
			if (compareStringCaseInsensitive(szLevelName, "plum_00") == 0 ||
				compareStringCaseInsensitive(szLevelName, "plum_00$01$00") == 0) {
				if (MOD_ProgressLevelChain()) return;
				MOD_EnterLevelChain(CHAIN_SANC_STONE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "plum_20") == 0) {
				// The second level of the Sanctuary is the side temple which is it's
				// own chain, not progress through the chain!
				MOD_EnterLevelChain(CHAIN_SIDE_TEMPLE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "plum_10") == 0 ||
					   compareStringCaseInsensitive(szLevelName, "plum_10$01$00") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Echoing Caves
			if (compareStringCaseInsensitive(szLevelName, "bast_10") == 0) {
				MOD_EnterLevelChain(CHAIN_ECHOING);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "cask_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "cask_30") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Precipice
			if (compareStringCaseInsensitive(szLevelName, "nave_10") == 0) {
				MOD_EnterLevelChain(CHAIN_PRECIPICE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "nave_15") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "nave_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Top of the World
			if (compareStringCaseInsensitive(szLevelName, "Seat_10") == 0) {
				MOD_EnterLevelChain(CHAIN_TOP);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "seat_11") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Sanctuary of Rock and Lava
			if (compareStringCaseInsensitive(szLevelName, "earth_10") == 0) {
				MOD_EnterLevelChain(CHAIN_SANC_ROCK);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "earth_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "earth_30") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Beneath the Sanctuary of Rock and Lava
			if (compareStringCaseInsensitive(szLevelName, "helic_10") == 0) {
				MOD_EnterLevelChain(CHAIN_BENEATH);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "helic_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "helic_30") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Tomb of the Ancients
			if (compareStringCaseInsensitive(szLevelName, "morb_00") == 0) {
				MOD_EnterLevelChain(CHAIN_TOMB);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "morb_10") == 0 ||
					   compareStringCaseInsensitive(szLevelName, "morb_10$01$00") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "morb_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Iron Mountains
			if (compareStringCaseInsensitive(szLevelName, "learn_40") == 0) {
				MOD_EnterLevelChain(CHAIN_IRON_MOUNT);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "ile_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "Mine_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Prison Ship
			if (compareStringCaseInsensitive(szLevelName, "boat01") == 0) {
				MOD_EnterLevelChain(CHAIN_PRISON);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "boat02") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "astro_00") == 0) {
				if (MOD_ProgressLevelChain()) return;
			} else if (compareStringCaseInsensitive(szLevelName, "astro_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Walk of Life
			if (compareStringCaseInsensitive(szLevelName, "Ly_10") == 0) {
				MOD_EnterLevelChain(CHAIN_WALK_LIFE);
				return;
			}

			// Cave of Bad Dreams
			if (compareStringCaseInsensitive(szLevelName, "vulca_10") == 0) {
				MOD_EnterLevelChain(CHAIN_COBD);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "vulca_20") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Walk of Power
			if (compareStringCaseInsensitive(szLevelName, "Ly_20") == 0) {
				MOD_EnterLevelChain(CHAIN_WALK_POWER);
				return;
			}
		}
	}

	// Fall back to just the base game level change
	GAM_fn_vAskToChangeLevel(szLevelName, bSaveGame);
}

/** Counts a series of lum collectables. */
int CountCollectibleLums(int* values, int* superValues, int superLums, int count) {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	int total = 0;
	if (pGlobal) {
		for (int i = 0; i < (count - superLums * 5); i++) {
			if (AI_fn_bGetBooleanInArray(pGlobal, 42, values[i])) {
				total++;
			}
		}
		for (int i = 0; i < superLums; i++) {
			if (AI_fn_bGetBooleanInArray(pGlobal, 42, superValues[i])) {
				total += 5;
			}
		}
	}
	return total;
}

/** Counts a series of cage collectables. */
int CountCollectibleCages(int* values, int count) {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	int total = 0;
	if (pGlobal) {
		for (int i = 0; i < count; i++) {
			if (AI_fn_bGetBooleanInArray(pGlobal, 42, values[i])) {
				total++;
			}
		}

	}
	return total;
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
void AI_fn_vSetBooleanInArray(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, unsigned int ulIndex, ACP_tdxBool value) {
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
void MOD_ClearLumGateOverrides() {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal && MOD_InLumGate) {
		MOD_InLumGate = false;
		MOD_CurrentLumGate = -1;
		for (int i = 1; i <= 1400; i++) {
			// Only update lumlike values!
			if (isLumLike(i)) {
				AI_fn_vSetBooleanInArray(pGlobal, 42, i, getBitSet(&MOD_RealCollected, i));
			}
		}
	}
}

/** Sets lum values to the lum gates of the given id. */
void setLumGateOverride(int lumGateId) {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");

	if (pGlobal && MOD_CurrentLumGate != lumGateId) {
		if (MOD_InLumGate) {
			// If we were already in a gate, clear overrides first before we re-apply!
			MOD_ClearLumGateOverrides();
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
			if (isLumLike(i)) {
				// Copy out the data into the real collection
				setBitSet(&MOD_RealCollected, i, AI_fn_bGetBooleanInArray(pGlobal, 42, i));

				// Update the data to set the correct lum count we want
				AI_fn_vSetBooleanInArray(pGlobal, 42, i, givenLums++ < finalLums);
			}
		}
	}
}

/** Checks if any lums/cages have been collected since last frame. */
void MOD_CheckVariables() {
	// Check at most twice per second!
	MOD_VariableCheckTicks++;
	if (MOD_VariableCheckTicks < 30) return;
	MOD_VariableCheckTicks = 0;

	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal) {
		// If we're in a lum gate, override the lum gate values.
		const char* szLevelName = GAM_fn_p_szGetLevelName();
		if (compareStringCaseInsensitive(szLevelName, "Nego_10") == 0) {
			if (!MOD_InLumGate) {
				HIE_tdstSuperObject* pLums = HIE_fn_p_stFindObjectByName("NIK_DS1_ZyvaEnvoieTesLums");
				if (pLums) {
					GAM_tdstEngineStructure* structure = GAM_g_stEngineStructure;
					int levelId = structure->ucExitIdToQuitPrevLevel;
					int lumGateId = 0;
					if (levelId == 21) {
						lumGateId = 1;
					} else if (levelId == 33) {
						lumGateId = 2;
					} else if (levelId == 40) {
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
		MOD_ClearLumGateOverrides();

		// If you are in the Menhir Hills 2 we have to fake you having the Elixir, otherwise
		// we leave it default so we don't despawn Jano incorrectly.
		if (compareStringCaseInsensitive(szLevelName, "Rodeo_40") == 0 || compareStringCaseInsensitive(szLevelName, "rodeo_40$01") == 0) {
			if (!MOD_InMenhirHills) {
				MOD_HadElixirPreviously = AI_fn_bGetBooleanInArray(pGlobal, 42, 1123);
				AI_fn_vSetBooleanInArray(pGlobal, 42, 1123, MOD_Elixir);
				MOD_InMenhirHills = TRUE;
			}

			// Walking over the cutscene trigger of the Clark cutscene we send out the check
			// for knowing about the cave of bad dreams as there's no good checks available.
			if (!MOD_SentKnowledgeOfCOBD) {
				HIE_tdstSuperObject* pMain = HIE_fn_p_stFindObjectByName("StdCamer");
				if (pMain) {
					MTH3D_tdstVector* pCoords = &pMain->p_stGlobalMatrix->stPos;
					MTH_tdxReal dx = pCoords->x + 1646.558;
					if (dx < 0) dx = -dx;
					MTH_tdxReal dy = pCoords->y + 18306.199;
					if (dy < 0) dy = -dy;
					MTH_tdxReal dz = pCoords->z + 8052.74;
					if (dz < 0) dz = -dz;

					if (dx <= 30 && dy <= 30 && dz <= 30) {
						MOD_SentKnowledgeOfCOBD = TRUE;
						MOD_SendMessage(MESSAGE_TYPE_COLLECTED, "1101");
					}
				}
			}
		}

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
					// When connected in non-lumsanity update the lum counter immediately for any non-super lums gathered!
					if (MOD_Connected && !MOD_Lumsanity && isLumLike(i) && !isSuperLum(i)) {
						MOD_Lums++;
					}

					// Don't allow collecting the Elixir of Life while in Menhir Hills as that's 
					// the remote version!
					if (i == 1123 && MOD_InMenhirHills) {
						continue;
					}

					// Don't send the base game syncs for getting upgrades since we override them
					// and it causes them to incorrectly be marked off!
					if (i == 1095 || i == 1143) continue;

					// If you rescue Ly we change it into the first silver lum check so it fires even if you already have
					// the silver lum!
					if (i == 953) {
						i = 1095;
					}

					// If you finish Canopy #2 while rescuing Globox you get 957 at the same time you would otherwise get
					// the second silver lum, so we can use it as a backup in case you already have a silver lum.
					if (i == 957) {
						i = 1143;
					}

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
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1095, FALSE);
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1143, FALSE);
		} else if (MOD_Upgrades == 1) {
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1095, TRUE);
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1143, FALSE);
		} else {
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1095, TRUE);
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1143, TRUE);
		}

		// Set whether you have knowledge of the cave of bad dreams always
		AI_fn_vSetBooleanInArray(pGlobal, 42, 1101, MOD_Knowledge);

		// Show the final portal if and only if you have enough masks!
		AI_fn_vSetBooleanInArray(pGlobal, 42, FINAL_LEVEL, MOD_Masks >= 4);

		// While in the menhir hills we sync the elixir state! 
		if (MOD_InMenhirHills) {
			AI_fn_vSetBooleanInArray(pGlobal, 42, 1123, MOD_Elixir);
		}

		// If you're in the menu, quit any chains!
		if (MOD_InLevelChain && compareStringCaseInsensitive(szLevelName, "mapmonde") == 0) {
			MOD_ExitChain(TRUE);
		}

		// In accessible portals mode we show all portals for which we have enough lums!
		if (MOD_AccessiblePortals) {
			{
				ACP_tdxBool changed = FALSE;
				for (int i = 0; i < 3; i++) {
					ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, NO_LUM_GATE_LEVELS[i]);
					if (!dsg) {
						AI_fn_vSetBooleanInArray(pGlobal, 42, NO_LUM_GATE_LEVELS[i], TRUE);
						changed = TRUE;
					}
				}
				if (changed) {
					MOD_ShowScreenText("First level set unlocked!");
				}
			}

			if (MOD_Lums >= MOD_LumGates[0]) {
				ACP_tdxBool changed = FALSE;
				for (int i = 0; i < 4; i++) {
					ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, LUM_GATE_ONE_LEVELS[i]);
					if (!dsg) {
						AI_fn_vSetBooleanInArray(pGlobal, 42, LUM_GATE_ONE_LEVELS[i], TRUE);
						changed = TRUE;
					}
				}
				if (changed) {
					MOD_ShowScreenText("Second level set unlocked!");
				}
			}
			if (MOD_Lums >= MOD_LumGates[1]) {
				ACP_tdxBool changed = FALSE;
				for (int i = 0; i < 5; i++) {
					ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, LUM_GATE_TWO_LEVELS[i]);
					if (!dsg) {
						AI_fn_vSetBooleanInArray(pGlobal, 42, LUM_GATE_TWO_LEVELS[i], TRUE);
						changed = TRUE;
					}
				}
				if (changed) {
					MOD_ShowScreenText("Third level set unlocked!");
				}
			}
			if (MOD_Lums >= MOD_LumGates[2]) {
				ACP_tdxBool changed = FALSE;
				for (int i = 0; i < 2; i++) {
					ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, LUM_GATE_THREE_LEVELS[i]);
					if (!dsg) {
						AI_fn_vSetBooleanInArray(pGlobal, 42, LUM_GATE_THREE_LEVELS[i], TRUE);
						changed = TRUE;
					}
				}
				if (changed) {
					MOD_ShowScreenText("Fourth level set unlocked!");
				}
			}
			if (MOD_Lums >= MOD_LumGates[3]) {
				ACP_tdxBool changed = FALSE;
				for (int i = 0; i < 2; i++) {
					ACP_tdxBool dsg = AI_fn_bGetBooleanInArray(pGlobal, 42, LUM_GATE_FOUR_LEVELS[i]);
					if (!dsg) {
						AI_fn_vSetBooleanInArray(pGlobal, 42, LUM_GATE_FOUR_LEVELS[i], TRUE);
						changed = TRUE;
					}
				}
				if (changed) {
					MOD_ShowScreenText("Last level set unlocked!");
				}
			}
		}
	}
}

/** Ticked by the engine every frame, runs all messages received since last tick. */
void MOD_EngineTick() {
	MOD_RunPendingMessages();

	// In dev mode we send console messages whenever checks happen!
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
		if (MOD_GetDeathLink(FALSE) && !MOD_IgnoreDeath) {
			MOD_SendMessage(MESSAGE_TYPE_DEATH, "Rayman died");
		}
		MOD_IgnoreDeath = FALSE;
	}

	GAM_fn_vChooseTheGoodInit();
}

/** Updates the current archipelago settings. */
void MOD_UpdateSettings(BOOL connected, BOOL deathLink, int endGoal, BOOL lumsanity, BOOL roomRandomisation, BOOL accessiblePortals, int* lumGates, char** levelIds, int* chainLengths, int** chainContents) {
	if (MOD_Connected != connected) {
		// Clear the collection cache whenever we reconnect so we resend all the information!
		clearBitSet(&MOD_LastCollected);
	}

	MOD_Connected = connected;
	MOD_DeathLink = deathLink;
	MOD_EndGoal = endGoal;
	MOD_Lumsanity = lumsanity;
	MOD_RoomRandomisation = roomRandomisation;
	MOD_AccessiblePortals = accessiblePortals;
	for (int i = 0; i < 6; i++) {
		MOD_LumGates[i] = lumGates[i];
	}
	for (int i = 0; i < LEVEL_COUNT; i++) {
		strncpy(MOD_LevelIds[i], levelIds[i], MAX_LENGTH - 1);
	}
	for (int i = 0; i < CHAIN_COUNT; i++) {
		MOD_LevelChainsLengths[i] = chainLengths[i];
	}
	for (int i = 0; i < CHAIN_COUNT; i++) {
		if (MOD_InitLevelChains) {
			free(MOD_LevelChainContents[i]);
		}
		MOD_LevelChainContents[i] = chainContents[i];
	}
	MOD_InitLevelChains = TRUE;
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
		FILE* pFile = fopen("ap_log_console.txt", "a");
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

/** Returns whether the pirate ship is unlocked. */
BOOL MOD_HasUnlockedPirateShip() {
	HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
	if (pGlobal) {
		return AI_fn_bGetBooleanInArray(pGlobal, 42, PIRATE_SHIP_PORTAL);
	}
	return false;
}

/** Returns whether death link is currently enabled. */
BOOL MOD_GetDeathLink(BOOL ignoreOverride) {
	if (ignoreOverride) {
		return MOD_DeathLink;
	} else {
		return MOD_DeathLink && !MOD_DeathLinkOverride;
	}
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

/** Crawls through the level chains and appends to the output list. */
void CrawlLevelInfo(int chainId, int currentLevel, LevelInfo** info, int* length, int depth) {
	// Determine the level to place here
	int chainLength = MOD_LevelChainsLengths[chainId];
	if (currentLevel < 0 || currentLevel >= chainLength) return;

	// Extend the length and array with this new level
	*length = *length + 1;
	LevelInfo* tmp = realloc(*info, *length * sizeof(LevelInfo));
	if (!tmp) return;
	*info = tmp;

	// Prepare the level data for this level
	LevelInfo* level = &(*info)[*length - 1];
	memset(level, 0, sizeof(LevelInfo));

	level->depth = depth;

	// Determine the level in this spot
	int levelId = MOD_LevelChainContents[chainId][currentLevel];
	char* levelName = MOD_LevelIds[levelId];

	// Determine the lums/cages of this level
	// Fairy Glade
	if (compareStringCaseInsensitive(levelName, "Learn_30") == 0) {
		strcpy(level->name, "The Fairy Glade 1");
		level->lumsMax = 9;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 6, 7, 8, 12 }, (int[]) { 1 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 842, 843 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "learn_31") == 0) {
		strcpy(level->name, "The Fairy Glade 2");
		level->lumsMax = 1;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 11 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "Learn_32") == 0) {
		strcpy(level->name, "The Fairy Glade - Revisit");
		level->lumsMax = 2;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 9, 10 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 844 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "bast_20") == 0) {
		strcpy(level->name, "The Fairy Glade 3");
		level->lumsMax = 17;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 27, 26, 25, 29, 28, 23, 24 }, (int[]) { 13, 18 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 845, 846 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "bast_22") == 0) {
		strcpy(level->name, "The Fairy Glade 4");
		level->lumsMax = 4;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 31, 30, 32, 33 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "learn_60") == 0) {
		strcpy(level->name, "The Fairy Glade 5");
		level->lumsMax = 17;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 34, 35, 36, 37, 48, 49, 50, 38, 42, 45, 44, 43, 39, 46, 47, 41, 40 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 847, 848 }, level->cagesMax);
	}

	// Marshes
	else if (compareStringCaseInsensitive(levelName, "Ski_10") == 0) {
		strcpy(level->name, "The Marshes of Awakening 1");
		level->lumsMax = 25;
		level->cagesMax = 4;
		level->lums = CountCollectibleLums((int[]) { 76, 77, 78, 79, 80 }, (int[]) { 66, 81, 86, 91 }, 4, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 852, 849, 850, 851 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "Ski_60") == 0) {
		strcpy(level->name, "The Marshes of Awakening 2");
		level->lumsMax = 25;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 59, 60, 56, 58, 57 }, (int[]) { 71, 61, 51, 96 }, 4, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 853 }, level->cagesMax);
	}

	// Cave of Bad Dreams
	else if (compareStringCaseInsensitive(levelName, "vulca_10") == 0) {
		strcpy(level->name, "The Cave of Bad Dreams 1");
		level->lumsMax = 30;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 769, 768, 767, 770, 771, 772, 773, 774, 775 }, (int[]) { 762, 776 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "vulca_20") == 0) {
		strcpy(level->name, "The Cave of Bad Dreams 2");
		level->lumsMax = 20;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 0 }, (int[]) { 781, 786, 791, 796 }, 4, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	}

	// Bayou
	else if (compareStringCaseInsensitive(levelName, "chase_10") == 0) {
		strcpy(level->name, "The Bayou 1");
		level->lumsMax = 35;
		level->cagesMax = 5;
		level->lums = CountCollectibleLums((int[]) { 122, 123, 101, 102, 103, 120, 121, 128, 129, 130, 131, 104, 105, 106, 107, 108, 118, 119, 109, 110, 111, 135, 124, 125, 126, 127, 114, 115, 116, 117, 112, 113, 132, 133, 134 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 854, 856, 855, 858, 857 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "chase_22") == 0) {
		strcpy(level->name, "The Bayou 2");
		level->lumsMax = 15;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 137, 144, 145, 146, 147, 148, 149, 150, 136, 138, 139, 140, 141, 142, 143 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 859, 860 }, level->cagesMax);
	}

	// Ice and Water
	else if (compareStringCaseInsensitive(levelName, "water_10") == 0) {
		strcpy(level->name, "The Sanctuary of Water and Ice 1");
		level->lumsMax = 40;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 156, 157, 158, 166, 167, 168, 169, 170, 171, 178, 177, 151, 152, 153, 154, 155, 159, 160, 180, 182, 183, 181, 184, 186, 187, 185, 188, 189, 179, 190 }, (int[]) { 172, 161 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 862, 861 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "water_20") == 0) {
		strcpy(level->name, "The Sanctuary of Water and Ice 2");
		level->lumsMax = 10;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 197, 199, 193, 194, 195, 198, 196, 191, 192, 200 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	}

	// Menhir Hills
	else if (compareStringCaseInsensitive(levelName, "rodeo_10") == 0) {
		strcpy(level->name, "The Menhir Hills 1");
		level->lumsMax = 15;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 0 }, (int[]) { 206, 201, 211 }, 3, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 864, 865, 863 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "rodeo_40") == 0) {
		strcpy(level->name, "The Menhir Hills 2");
		level->lumsMax = 23;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 226, 224, 223, 225, 231, 228, 222, 233, 232, 230, 235, 234, 229, 216, 217, 218, 227, 219, 220, 221, 236, 237, 238 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 866, 868, 867 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "rodeo_60") == 0) {
		strcpy(level->name, "The Menhir Hills 3");
		level->lumsMax = 12;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 246, 245, 239, 240, 243, 244, 248, 247, 249, 250, 241, 242 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 869, 870 }, level->cagesMax);
	}

	// Canopy
	else if (compareStringCaseInsensitive(levelName, "glob_30") == 0) {
		strcpy(level->name, "The Canopy 1");
		level->lumsMax = 20;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 266, 251, 262, 252, 253, 254, 263, 258, 264, 259, 255, 265, 256, 261, 260, 257, 267, 268, 268, 270 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 871, 872 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "glob_10") == 0) {
		strcpy(level->name, "The Canopy 2");
		level->lumsMax = 15;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 283, 276, 275, 271, 274, 281, 285, 280, 284, 282, 277, 273, 272, 279, 278 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "glob_20") == 0) {
		strcpy(level->name, "The Canopy 3");
		level->lumsMax = 15;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 286, 291, 290, 287, 288, 289, 299, 300, 297, 298 }, (int[]) { 292 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 874, 873 }, level->cagesMax);
	}

	// Whale Bay
	else if (compareStringCaseInsensitive(levelName, "whale_00") == 0) {
		strcpy(level->name, "Whale Bay 1");
		level->lumsMax = 15;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 339, 341, 327, 320, 323, 321, 326, 338, 340, 322 }, (int[]) { 315 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 875 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "whale_05") == 0) {
		strcpy(level->name, "Whale Bay 2");
		level->lumsMax = 12;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 325, 324 }, (int[]) { 333, 328 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 876 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "whale_10") == 0) {
		strcpy(level->name, "Whale Bay 3");
		level->lumsMax = 23;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 303, 302, 348, 307, 306, 345, 308, 309, 350, 344, 346, 349, 347, 342, 343, 304, 301, 305 }, (int[]) { 310 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 877, 878 }, level->cagesMax);
	}

	// Stone and Fire
	else if (compareStringCaseInsensitive(levelName, "plum_00") == 0) {
		strcpy(level->name, "The Sanctuary of Stone and Fire 1");
		level->lumsMax = 23;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 353, 356, 354, 357, 352, 355, 358, 351 }, (int[]) { 364, 359, 369 }, 3, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 885, 883, 884 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "plum_10") == 0) {
		strcpy(level->name, "The Sanctuary of Stone and Fire 2");
		level->lumsMax = 17;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 386, 390, 388, 374, 387, 385, 389 }, (int[]) { 380, 375 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 886 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "plum_20") == 0) {
		strcpy(level->name, "The Sanctuary of Stone and Fire - Side Temple");
		level->lumsMax = 10;
		level->cagesMax = 4;
		level->lums = CountCollectibleLums((int[]) { 391, 392, 393, 394, 395, 396, 397, 398, 399, 400 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 879, 880, 881, 882 }, level->cagesMax);
	}

	// Echoing Caves
	else if (compareStringCaseInsensitive(levelName, "bast_10") == 0) {
		strcpy(level->name, "The Echoing Caves 1");
		level->lumsMax = 20;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 0 }, (int[]) { 406, 401, 416, 411 }, 4, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "cask_10") == 0) {
		strcpy(level->name, "The Echoing Caves 2");
		level->lumsMax = 15;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 421, 422, 423, 430, 431, 432, 428, 429, 424, 425, 426, 427, 433, 434, 435 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 887, 888 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "cask_30") == 0) {
		strcpy(level->name, "The Echoing Caves 3");
		level->lumsMax = 15;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 436, 437, 438, 448, 449, 446, 447, 439, 440, 441, 442, 443, 444, 445, 450 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 889, 890, 891 }, level->cagesMax);
	}

	// Precipice
	else if (compareStringCaseInsensitive(levelName, "nave_10") == 0) {
		strcpy(level->name, "The Precipice 1");
		level->lumsMax = 10;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 451, 452, 454, 455, 460, 456, 457, 453, 458, 459 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 892, 893, 894 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "nave_15") == 0) {
		strcpy(level->name, "The Precipice 2");
		level->lumsMax = 15;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 465, 466, 468, 470, 471, 472, 469, 462, 473, 474, 463, 467, 475, 461, 464 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 896, 895 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "nave_20") == 0) {
		strcpy(level->name, "The Precipice 3");
		level->lumsMax = 25;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 489, 476, 480, 483, 485, 481, 484, 478, 490, 477, 479, 482, 487, 486, 488 }, (int[]) { 491, 496 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 897 }, level->cagesMax);
	}

	// Top of the World
	else if (compareStringCaseInsensitive(levelName, "Seat_10") == 0) {
		strcpy(level->name, "The Top of the World 1");
		level->lumsMax = 33;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 540, 521, 541, 542, 543, 518, 519, 520, 531, 539, 532, 524, 525, 533, 522, 523, 527, 528, 529, 530, 526, 537, 536, 535, 538, 544, 545, 546, 547, 548, 534, 549, 550 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "seat_11") == 0) {
		strcpy(level->name, "The Top of the World 2");
		level->lumsMax = 17;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 501, 502, 505, 504, 503, 511, 512, 513, 506, 507, 508, 517, 509, 510, 514, 515, 516 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 899, 898 }, level->cagesMax);
	}

	// Rock and Lava
	else if (compareStringCaseInsensitive(levelName, "earth_10") == 0) {
		strcpy(level->name, "The Sanctuary of Rock and Lava 1");
		level->lumsMax = 10;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 555, 551, 552, 553, 554 }, (int[]) { 556 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 900, 901 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "earth_20") == 0) {
		strcpy(level->name, "The Sanctuary of Rock and Lava 2");
		level->lumsMax = 15;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 561, 562, 563, 575, 576, 577, 564, 565, 566, 567, 568, 569, 570, 571, 572, 574, 579, 578, 580, 573 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 902, 903 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "earth_30") == 0) {
		strcpy(level->name, "The Sanctuary of Rock and Lava 3");
		level->lumsMax = 15;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 581, 582, 583, 584, 585, 586, 593, 594, 595, 587, 588, 589, 591, 592, 590, 596, 597, 598, 599, 600 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 904, 905, 906 }, level->cagesMax);
	}

	// Beneath
	else if (compareStringCaseInsensitive(levelName, "helic_10") == 0) {
		strcpy(level->name, "Beneath the Sanctuary of Rock and Lava 1");
		level->lumsMax = 20;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 610, 611, 612, 643, 644, 645, 606, 601, 602, 607, 608, 609, 603, 604, 605 }, (int[]) { 646 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 900, 901 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "helic_20") == 0) {
		strcpy(level->name, "Beneath the Sanctuary of Rock and Lava 2");
		level->lumsMax = 30;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 623, 624, 641, 625, 627, 629, 626, 642, 628, 630 }, (int[]) { 618, 613, 631, 636 }, 4, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 910, 909 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "helic_30") == 0) {
		strcpy(level->name, "Beneath the Sanctuary of Rock and Lava 3");
		level->lumsMax = 0;
		level->cagesMax = 0;
	}

	// Tomb of the Ancients
	else if (compareStringCaseInsensitive(levelName, "morb_00") == 0) {
		strcpy(level->name, "Tomb of the Ancients 1");
		level->lumsMax = 11;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 697, 698, 699, 700, 694, 695, 696, 691, 692, 693, 1014 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 911 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "morb_10") == 0) {
		strcpy(level->name, "Tomb of the Ancients 2");
		level->lumsMax = 40;
		level->cagesMax = 3;
		level->lums = CountCollectibleLums((int[]) { 656, 655, 654, 657, 652, 653, 651, 658, 660, 659 }, (int[]) { 686, 681, 661, 666, 671, 676 }, 6, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 910, 909, 913 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "morb_20") == 0) {
		strcpy(level->name, "Tomb of the Ancients 3");
		level->lumsMax = 0;
		level->cagesMax = 1;
		level->cages = CountCollectibleCages((int[]) { 916 }, level->cagesMax);
	}

	// Iron Mountains
	else if (compareStringCaseInsensitive(levelName, "learn_40") == 0) {
		strcpy(level->name, "The Iron Mountains 1");
		level->lumsMax = 19;
		level->cagesMax = 2;
		level->lums = CountCollectibleLums((int[]) { 711, 708, 707, 709, 712, 713, 717, 716, 715, 714, 719, 718, 710, 701, 702, 703, 706, 705, 704 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 917, 918 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "ile_10") == 0) {
		strcpy(level->name, "The Iron Mountains 2");
		level->lumsMax = 20;
		level->cagesMax = 1;
		level->lums = CountCollectibleLums((int[]) { 730, 726, 728, 727, 729 }, (int[]) { 721, 736, 731 }, 3, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 919 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "mine_10") == 0) {
		strcpy(level->name, "The Iron Mountains 3");
		level->lumsMax = 11;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 720 }, (int[]) { 746, 741 }, 2, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	}

	// Prison Ship
	else if (compareStringCaseInsensitive(levelName, "boat01") == 0) {
		strcpy(level->name, "The Prison Ship 1");
		level->lumsMax = 21;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 1370, 1359, 1369, 1373, 1371, 1377, 1372, 1379, 1374, 1375, 1368, 1376, 1360, 1361, 1367, 1362, 1366, 1364, 1365, 1378, 1363 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "boat02") == 0) {
		strcpy(level->name, "The Prison Ship 2");
		level->lumsMax = 43;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 1334, 1333, 1352, 1351, 1353, 1348, 1324, 1317, 1319, 1318, 1321, 1322, 1331, 1316, 1332, 1320, 1323, 1329, 1330, 1336, 1343, 1341, 1328, 1337, 1338, 1339, 1342, 1344, 1327, 1340, 1326, 1325, 1335, 1345, 1347, 1346, 1349, 1350 }, (int[]) { 1354 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "astro_00") == 0) {
		strcpy(level->name, "The Prison Ship 3");
		level->lumsMax = 15;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 1387, 1388, 1385, 1383, 1382, 1381, 18380, 1384, 1386, 1394 }, (int[]) { 1389 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	} else if (compareStringCaseInsensitive(levelName, "astro_10") == 0) {
		strcpy(level->name, "The Prison Ship 3");
		level->lumsMax = 15;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310 }, (int[]) { 1311 }, 1, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	}

	// Walk of Life
	else if (compareStringCaseInsensitive(levelName, "Ly_10") == 0) {
		strcpy(level->name, "The Walk of Life");
		level->lumsMax = 50;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 1201, 1202, 1203, 1236, 1206, 1205, 1208, 1210, 1212, 1209, 1214, 1211, 1215, 1217, 1216, 1218, 1219, 1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1229, 1230, 1231, 1233, 1244, 1246, 1242, 1232, 1243, 1238, 1239, 1240, 1245, 1247, 1237, 1234, 1228, 1207, 1235, 1241, 1249, 1213, 1250, 1248, 1204 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	}

	// Walk of Power
	else if (compareStringCaseInsensitive(levelName, "Ly_20") == 0) {
		strcpy(level->name, "The Walk of Power");
		level->lumsMax = 50;
		level->cagesMax = 0;
		level->lums = CountCollectibleLums((int[]) { 1272, 1251, 1271, 1273, 1274, 1275, 1252, 1253, 1278, 1277, 1254, 1279, 1255, 1280, 1281, 1282, 1256, 1257, 1284, 1258, 1285, 1276, 1259, 1286, 1269, 1289, 1288, 1260, 1261, 1287, 1262, 1263, 1290, 1291, 1295, 1267, 1292, 1270, 1293, 1265, 1266, 1294, 1296, 1299, 1298, 1268, 1300, 1283, 1297, 1264 }, (int[]) { 0 }, 0, level->lumsMax);
		level->cages = CountCollectibleCages((int[]) { 0 }, level->cagesMax);
	}

	// Fallback
	else {
		strcpy(level->name, levelName);
		level->lumsMax = 0;
		level->cagesMax = 0;
	}

	// Crawl for the next level, if this is Sanc of Stone and Fire or Echoing Caves we include the revisit area!
	// The other three have their own portals which show their contents.
	if (compareStringCaseInsensitive(levelName, "cask_10") == 0) {
		CrawlLevelInfo(CHAIN_FAIRY_REVISIT, 0, info, length, depth + 1);
	} else if (compareStringCaseInsensitive(levelName, "plum_00") == 0) {
		CrawlLevelInfo(CHAIN_SIDE_TEMPLE, 0, info, length, depth + 1);
	}
	CrawlLevelInfo(chainId, currentLevel + 1, info, length, depth);
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

	if (MOD_Connected) {
		// Draw the current Archipelago progression to the bottom in the hall of doors or on the pause screen
		const char* szLevelName = GAM_fn_p_szGetLevelName();
		const auto inMapMonde = compareStringCaseInsensitive(szLevelName, "mapmonde") == 0;
		if (inMapMonde || *AI_g_bInGameMenu) {
			pInfo->xSize = 6;
			pInfo->bRightAlign = TRUE;
			long lineHeight = SPTXT_fn_lGetCharHeight(pInfo->xSize);

			pInfo->X = 995;
			pInfo->Y = 990 - 4 * lineHeight;
			SPTXT_vPrintFmtLine("/o200:Archipelago Received");
			SPTXT_vPrintFmtLine("/o400:Lums /o0:%d of 1000/o400:, Cages /o0:%d of 80", MOD_Lums, MOD_Cages);
			SPTXT_vPrintFmtLine("/o400:Masks /o0:%d of 4/o400:, Power /o0:%d of 2", MOD_Masks, MOD_Upgrades);
			SPTXT_vPrintFmtLine("/o400:Elixir %s/o400:, Knowledge %s", MOD_Elixir ? "/o0:Yes" : "/o200:No", MOD_Knowledge ? "/o0:Yes" : "/o200:No");
		}

		// When hovering over a portal in mapmonde we show information on the level chain inside
		if (inMapMonde) {
			HIE_tdstSuperObject* pLums = HIE_fn_p_stFindObjectByName("YAM_Lums_I1");
			HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
			if (pLums && pGlobal) {
				// This variable stores the fade time on the UI box, so 255
				// means that we've been at a portal for enough time to show
				// the UI.
				int* p_stInt;
				AI_fn_bGetDsgVar(pLums, 19, NULL, &p_stInt);
				if (*p_stInt == 255) {
					// Load the currently highlighted portal into a variable
					HIE_tdstSuperObject** pPastille;
					AI_fn_bGetDsgVar(pLums, 5, NULL, &pPastille);

					if (*pPastille) {
						// Dsg var 2 on portals stores their level offset relative to 960
						int* p_stLevelId;
						AI_fn_bGetDsgVar(*pPastille, 2, NULL, &p_stLevelId);

						// Draw information based on the chain
						int chainId = -1;
						int nextLevelValue = 960;
						int portal = 1;

						// Determine which chain this is and what the next level is for checking if this was completed
						if (*p_stLevelId == 1) {
							chainId = CHAIN_FAIRY_GLADE;
							nextLevelValue += 4;
						} else if (*p_stLevelId == 4) {
							chainId = CHAIN_MARSHES;
							nextLevelValue += 7;
							portal = 2;
						} else if (*p_stLevelId == 6) {
							chainId = CHAIN_COBD;
							nextLevelValue = 1123; // We show info when you have the Elixir of Life!
							portal = 3;
						} else if (*p_stLevelId == 7) {
							chainId = CHAIN_BAYOU;
							nextLevelValue += 10;
							portal = 4;
						} else if (*p_stLevelId == 9) {
							chainId = CHAIN_WALK_LIFE;
							nextLevelValue += 0; // Walk is always visible!
							portal = 5;
						} else if (*p_stLevelId == 10) {
							chainId = CHAIN_SANC_WATER;
							nextLevelValue += 12;
							portal = 6;
						} else if (*p_stLevelId == 12) {
							chainId = CHAIN_MENHIR;
							nextLevelValue += 16;
							portal = 7;
						} else if (*p_stLevelId == 16) {
							chainId = CHAIN_CANOPY;
							nextLevelValue += 19;
							portal = 8;
						} else if (*p_stLevelId == 19) {
							chainId = CHAIN_WHALE;
							nextLevelValue += 21;
							portal = 9;
						} else if (*p_stLevelId == 21) {
							chainId = CHAIN_SANC_STONE;
							nextLevelValue += 15;
							portal = 10;
						} else if (*p_stLevelId == 15) {
							chainId = CHAIN_ECHOING;
							nextLevelValue += 25;
							portal = 11;
						} else if (*p_stLevelId == 25) {
							chainId = CHAIN_PRECIPICE;
							nextLevelValue += 28;
							portal = 12;
						} else if (*p_stLevelId == 28) {
							chainId = CHAIN_TOP;
							nextLevelValue += 30;
							portal = 13;
						} else if (*p_stLevelId == 32) {
							chainId = CHAIN_WALK_POWER;
							nextLevelValue += 0; // Walk is always visible!
							portal = 15;
						} else if (*p_stLevelId == 30) {
							chainId = CHAIN_SANC_ROCK;
							nextLevelValue += 33;
							portal = 14;
						} else if (*p_stLevelId == 33) {
							chainId = CHAIN_BENEATH;
							nextLevelValue += 47;
							portal = 16;
						} else if (*p_stLevelId == 47) {
							chainId = CHAIN_TOMB;
							nextLevelValue += 40;
							portal = 17;
						} else if (*p_stLevelId == 40) {
							chainId = CHAIN_IRON_MOUNT;
							nextLevelValue += 42;
							portal = 18;
						} else if (*p_stLevelId == 42) {
							chainId = CHAIN_PRISON;
							nextLevelValue += 0; // Prison Ship is always visible because we have no reliable checks and it's the last level anyway so you can process of elimination it.
							portal = 19;
						}

						if (chainId != -1) {
							pInfo->xSize = 7;
							pInfo->bRightAlign = FALSE;
							pInfo->X = 15;

							long lineHeight = SPTXT_fn_lGetCharHeight(pInfo->xSize);
							ACP_tdxBool isCompleted = AI_fn_bGetBooleanInArray(pGlobal, 42, nextLevelValue);

							if (!isCompleted && !MOD_AccessiblePortals) {
								int lines = 1;
								pInfo->Y = 450 - lines * lineHeight;
								SPTXT_vPrintFmtLine("/o200:Unlock the next level");
								SPTXT_vPrintFmtLine("/o200:to reveal rooms!");
							} else {
								// Determine the level contents!
								LevelInfo* levelInfo = NULL;
								int length = 0;
								CrawlLevelInfo(chainId, 0, &levelInfo, &length, 0);

								// Draw the level info in a list, start with a header showing portal number and room count
								long spacer = lineHeight - 2;
								pInfo->Y = 320;
								SPTXT_vPrintFmtLine("/o200:Portal nr. %d", portal);
								pInfo->X = 22;
								SPTXT_vPrintFmtLine("/o0:%d rooms", length);
								pInfo->Y = pInfo->Y + spacer;

								for (int i = 0; i < length; i++) {
									LevelInfo info = levelInfo[i];
									pInfo->X = 15 + info.depth * 10;
									SPTXT_vPrintFmtLine("/o400:%s", info.name);
									pInfo->X = 22 + info.depth * 10;
									if (info.cagesMax == 0) {
										SPTXT_vPrintFmtLine("/o400:Lums %s%d of %d", info.lums >= info.lumsMax ? "/o200:" : "/o0:", info.lums, info.lumsMax);
									} else {
										SPTXT_vPrintFmtLine("/o400:Lums %s%d of %d/o400:, Cages %s%d of %d", info.lums >= info.lumsMax ? "/o200:" : "/o0:", info.lums, info.lumsMax, info.cages >= info.cagesMax ? "/o200:" : "/o0:", info.cages, info.cagesMax);
									}
									pInfo->Y = pInfo->Y + spacer;
								}
							}
						}
					}
				}
			}
		}
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