#pragma once
#include "framework.h"
#include "connector.h"
#include "bitset.h"

typedef struct LevelInfo {
	char name[MAX_LEVEL_NAME_LENGTH];
	char levelName[MAX_LEVEL_NAME_LENGTH];
	int lums;
	int lumsMax;
	int cages;
	int cagesMax;
	int depth;
} LevelInfo;

int compareStringCaseInsensitive(char const* a, char const* b);

LRESULT CALLBACK MOD_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MOD_EngineTick();
void MOD_Init();
void MOD_Reset();

void MOD_ClearLumGateOverrides();

BOOL MOD_InDevMode();
void MOD_SetDevMode(BOOL state);

void MOD_BugReport();

BOOL MOD_SendToCurrentLevel();
BOOL MOD_ProgressLevelChain();
void MOD_EnterLevelChain(int chainId);
BOOL MOD_JumpToLevel(char* levelName);
void MOD_ExitChain();
void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame);

void MOD_Print(char*, ...);
void MOD_ShowScreenText(char*, ...);
void MOD_Main();
void MOD_UpdateSettings(BOOL connected, BOOL deathLink, int endGoal, BOOL lumsanity, BOOL roomRandomisation, BOOL accessiblePortals, int deathLinkAmnesty, BOOL betterLevelPortals, int lumBundleSize, int* lumGates, char** levelIds, int* chainLengths, int** chainContents);
void MOD_UpdateState(int lums, int cages, int masks, int upgrades, BOOL elixir, BOOL knowledge);
void MOD_TriggerDeath(char* data);
void MOD_TestDeathLink();

void MOD_CrawlLevelInfo(int chainId, int currentLevel, LevelInfo** info, int* length, int depth);

BOOL MOD_GetDeathLink(BOOL ignoreOverride);
void MOD_ToggleDeathLink();

void AI_fn_vSetBooleanInArray(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, unsigned int ulIndex, ACP_tdxBool value);

extern void MOD_InitCommands();