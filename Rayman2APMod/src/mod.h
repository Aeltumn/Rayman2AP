#pragma once
#include "framework.h"
#include "connector.h"
#include "bitset.h"

int compareStringCaseInsensitive(char const* a, char const* b);

void MOD_EngineTick();
void MOD_Init();
void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame);

void MOD_Print(char*, ...);
void MOD_ShowScreenText(char*, ...);
void MOD_Main();
void MOD_UpdateSettings(BOOL connected, BOOL deathLink, int endGoal, BOOL lumsanity, BOOL roomRandomisation, int* lumGates, char** levelSwapKeys, char** levelSwapTargets);
void MOD_UpdateState(int lums, int cages, int masks, int upgrades, BOOL elixir, BOOL knowledge);
void MOD_TriggerDeath();

BOOL MOD_GetDeathLink();
void MOD_ToggleDeathLink();

void AI_fn_bSetBooleanInArray(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, unsigned int ulIndex, ACP_tdxBool value);

extern void MOD_InitCommands();