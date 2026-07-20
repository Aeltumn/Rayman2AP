#pragma once

#include "mod_ext.h"
#include "framework.h"
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

LRESULT CALLBACK MOD_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void MOD_EngineTick();
void MOD_Init();

void MOD_StartMod();

BOOL MOD_InDevMode();
void MOD_SetDevMode(BOOL state);

BOOL MOD_SendToCurrentLevel();
BOOL MOD_ProgressLevelChain();
void MOD_EnterLevelChain(int chainId);
BOOL MOD_JumpToLevel(char* levelName);
void MOD_ExitChain();
void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame);

void MOD_SetScreenTextShown(int type, BOOL value);
void MOD_ShowScreenText(int type, char*, ...);
void MOD_ClearLumGateOverrides();
void MOD_CrawlLevelInfo(int chainId, int currentLevel, LevelInfo** info, int* length, int depth);
void MOD_BugReport();

void MOD_TestDeathLink();
BOOL MOD_GetDeathLink(BOOL ignoreOverride);
void MOD_ToggleDeathLink();

void AI_fn_vSetBooleanInArray(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, unsigned int ulIndex, ACP_tdxBool value);

extern void MOD_InitCommands();