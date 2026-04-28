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

// While in the Menhir Hills store if you had the elixir beforehand
BOOL MOD_InMenhirHills = FALSE;
BOOL MOD_HadElixirPreviously = FALSE;
BOOL MOD_SentKnowledgeOfCOBD = FALSE;

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
BOOL MOD_DevMode = TRUE;

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

BOOL MOD_ProgressLevelChainAndIncrement(int increment) {
	// If we're in a level chain, continue it!
	if (MOD_InLevelChain) {
		// Determine the current level chain
		int chainId = MOD_LevelChainCurrent;

		// Get the level we are meant to be in and load into it
		int currentLevel = MOD_LevelChainActive[chainId] + increment;
		int chainLength = MOD_LevelChainsLengths[chainId];

		// Save the new index on this chain
		MOD_LevelChainActive[chainId] = currentLevel;
		MOD_Print("Progressing through chain %d, level is %d out of %d", chainId, currentLevel, chainLength);

		if (currentLevel < 0 || currentLevel >= chainLength) {
			// We can't progress past the end of the chain!
			return;
		} else {
			// Find which level ID this is
			int levelId = MOD_LevelChainContents[chainId][currentLevel];
			char* levelName = MOD_LevelIds[levelId];

			// Learn_32 is the internal ID for the fairy glade revisit!
			if (compareStringCaseInsensitive(levelName, "Learn_32") == 0) {
				levelName = "Learn_31";

				// 70 is cask_10 which makes it use the right entrance!
				GAM_g_stEngineStructure->ucPreviousLevel = 70;
			}
			MOD_Print("Due to level chain we're entering %s", levelName);
			GAM_fn_vAskToChangeLevel(levelName, FALSE);

			// If we enter a walk level or COBD we spawn their portal which makes them
			// accessible from the HOF and remove the lum check.
			HIE_tdstSuperObject* pGlobal = HIE_fn_p_stFindObjectByName("global");
			if (pGlobal) {
				if (chainId == CHAIN_WALK_LIFE) {
					AI_fn_bSetBooleanInArray(pGlobal, 42, 969, TRUE);
				} else if (chainId == CHAIN_WALK_POWER) {
					AI_fn_bSetBooleanInArray(pGlobal, 42, 992, TRUE);
				} else if (chainId == CHAIN_COBD) {
					AI_fn_bSetBooleanInArray(pGlobal, 42, 966, TRUE);
				}
			}
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
		MOD_Print("We were in %d before!", MOD_LevelChainCurrent);
		MOD_LevelChainLast[chainId] = MOD_LevelChainCurrent + 1;
	}

	// Mark down that we've entered a level chain, then determine what the first level is in this chain!
	MOD_InLevelChain = TRUE;
	MOD_LevelChainCurrent = chainId;
	MOD_Print("Entering chain %d", chainId);
	MOD_ProgressLevelChainAndIncrement(0);
}

void MOD_ExitChain(ACP_tdxBool bSaveGame) {
	// Ignore if we are not in a chain!
	if (!MOD_InLevelChain) {
		GAM_fn_vAskToChangeLevel("mapmonde", bSaveGame);
		return;
	}

	// Read data and then immediately void it as we exit
	int chainId = MOD_LevelChainCurrent;
	int lastChain = MOD_LevelChainLast[chainId] - 1;
	int currentLevel = MOD_LevelChainActive[chainId];
	int chainLength = MOD_LevelChainsLengths[chainId];

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

	MOD_Print("Exiting chain %d, last is %d, completed %d (%d out of %d)", chainId, lastChain, completedChain, currentLevel, chainLength);

	// If you didn't complete the chain, didn't have a last chain, or it's the walks you return to the hall of doors instead of
	// your previous level you were in.
	if (!completedChain || lastChain == -1 || chainId == CHAIN_WALK_LIFE || chainId == CHAIN_WALK_POWER) {
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
				structure->ucPreviousLevel = 230;
			} else if (chainId == CHAIN_TOMB) {
				structure->ucPreviousLevel = 118;
			} else if (chainId == CHAIN_IRON_MOUNT) {
				structure->ucPreviousLevel = 125;
			} else if (chainId == CHAIN_PRISON) {
				structure->ucPreviousLevel = 240;
			}
		} else {
			if (chainId == CHAIN_FAIRY_GLADE) {
				structure->ucPreviousLevel = 10;
			} else if (chainId == CHAIN_MARSHES) {
				structure->ucPreviousLevel = 25;
			} else if (chainId == CHAIN_COBD) {
				structure->ucPreviousLevel = 135;
			} else if (chainId == CHAIN_BAYOU) {
				structure->ucPreviousLevel = 15;
			} else if (chainId == CHAIN_WALK_LIFE) {
				structure->ucPreviousLevel = 20;
			} else if (chainId == CHAIN_MENHIR) {
				structure->ucPreviousLevel = 55;
			} else if (chainId == CHAIN_SANC_WATER) {
				structure->ucPreviousLevel = 160;
			} else if (chainId == CHAIN_CANOPY) {
				structure->ucPreviousLevel = 210;
			} else if (chainId == CHAIN_WHALE) {
				structure->ucPreviousLevel = 45;
			} else if (chainId == CHAIN_SANC_STONE) {
				structure->ucPreviousLevel = 195;
			} else if (chainId == CHAIN_ECHOING) {
				structure->ucPreviousLevel = 130;
			} else if (chainId == CHAIN_PRECIPICE) {
				structure->ucPreviousLevel = 80;
			} else if (chainId == CHAIN_TOP) {
				structure->ucPreviousLevel = 40;
			} else if (chainId == CHAIN_SANC_ROCK) {
				structure->ucPreviousLevel = 95;
			} else if (chainId == CHAIN_WALK_POWER) {
				structure->ucPreviousLevel = 115;
			} else if (chainId == CHAIN_BENEATH) {
				structure->ucPreviousLevel = 105;
			} else if (chainId == CHAIN_TOMB) {
				structure->ucPreviousLevel = 118;
			} else if (chainId == CHAIN_IRON_MOUNT) {
				structure->ucPreviousLevel = 12;
			} else if (chainId == CHAIN_PRISON) {
				structure->ucPreviousLevel = 140;
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
			if (pGlobal) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, 1120, TRUE);
			}
		} else if (chainId == CHAIN_FAIRY_REVISIT) {
			structure->ucPreviousLevel = 11;
		} else if (chainId == CHAIN_SIDE_TEMPLE) {
			structure->ucPreviousLevel = 190;
		}

		MOD_Print("Going back to chain %d", lastChain);
		MOD_EnterLevelChain(lastChain);
	}
}

BOOL MOD_ReturnToPreviousChain(int ifChain, int thenChain) {
	// If we're in a chain and it's the if chain then move into the then chain
	// instead of progressing! This is used to cap off revisits.
	if (MOD_InLevelChain && MOD_LevelChainCurrent == ifChain) {
		MOD_ExitChain(FALSE);
		return true;
	}
	return false;
}

void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame) {
	// Fetch the base game strucure as we'll have to modify it
	GAM_tdstEngineStructure* structure = GAM_g_stEngineStructure;

	// Re-check for the COBD knowledge & treasure ending check on level switch
	MOD_SentKnowledgeOfCOBD = FALSE;
	MOD_TreasureComplete = FALSE;

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
			} else if (!hasEnoughCages) {
				MOD_PrintConsolePlusScreen("Game not complete, not enough cages!");
			} else {
				MOD_PrintConsolePlusScreen("Game completed 100%!");
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
				if (MOD_ReturnToPreviousChain(CHAIN_SIDE_TEMPLE, CHAIN_SANC_STONE)) return;
				MOD_EnterLevelChain(CHAIN_SANC_STONE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "plum_20") == 0) {
				// The second level of the Sanctuary is the side temple which is it's
				// own chain, not progress through the chain!
				MOD_EnterLevelChain(CHAIN_SIDE_TEMPLE);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "plum_10") == 0) {
				if (MOD_ProgressLevelChain()) return;
			}

			// Echoing Caves
			if (compareStringCaseInsensitive(szLevelName, "bast_10") == 0) {
				MOD_EnterLevelChain(CHAIN_ECHOING);
				return;
			} else if (compareStringCaseInsensitive(szLevelName, "cask_10") == 0) {
				if (MOD_ReturnToPreviousChain(CHAIN_FAIRY_REVISIT, CHAIN_ECHOING)) return;
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
			} else if (compareStringCaseInsensitive(szLevelName, "mine_10") == 0) {
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
			if (isLumLike(i)) {
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

		// If you are in the Menhir Hills 2 we have to fake you having the Elixir, otherwise
		// we leave it default so we don't despawn Jano incorrectly.
		if (compareStringCaseInsensitive(szLevelName, "Rodeo_40") == 0 || compareStringCaseInsensitive(szLevelName, "rodeo_40$01") == 0) {
			if (!MOD_InMenhirHills) {
				MOD_HadElixirPreviously = AI_fn_bGetBooleanInArray(pGlobal, 42, 1123);
				AI_fn_bSetBooleanInArray(pGlobal, 42, 1123, MOD_Elixir);
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
		} else {
			if (MOD_InMenhirHills) {
				AI_fn_bSetBooleanInArray(pGlobal, 42, 1123, MOD_HadElixirPreviously);
				MOD_InMenhirHills = FALSE;
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
					// When connected in non-lumsanity update the lum counter immediately for any non-super lums gathered!
					if (MOD_Connected && !MOD_Lumsanity && isLumLike(i) && !isSuperLum(i)) {
						MOD_Lums++;
					}

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
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1095, FALSE);
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1143, FALSE);
		} else if (MOD_Upgrades == 1) {
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1095, TRUE);
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1143, FALSE);
		} else {
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1095, TRUE);
			AI_fn_bSetBooleanInArray(pGlobal, 42, 1143, TRUE);
		}

		// Set whether you have knowledge of the cave of bad dreams always
		AI_fn_bSetBooleanInArray(pGlobal, 42, 1101, MOD_Knowledge);

		// Show the final portal if and only if you have enough masks!
		AI_fn_bSetBooleanInArray(pGlobal, 42, FINAL_LEVEL, MOD_Masks >= 4);
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
void MOD_UpdateSettings(BOOL connected, BOOL deathLink, int endGoal, BOOL lumsanity, BOOL roomRandomisation, int* lumGates, char** levelIds, int* chainLengths, int** chainContents) {
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