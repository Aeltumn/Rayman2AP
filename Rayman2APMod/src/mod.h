#pragma once
#include "framework.h"
#include "connector.h"
#include "bitset.h"

void MOD_EngineTick();
void MOD_Init();
void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame);

void MOD_Print(char*, ...);
void MOD_Main();
void MOD_UpdateState(BOOL connected, int lums, int cages, int masks, int upgrades, BOOL deathLink, int endGoal, BOOL elixir, int* lumGates, char** levelSwapKeys, char** levelSwapTargets);
void MOD_TriggerDeath();
void MOD_vShowScreenText(char*, ...);
void MOD_ShowScreenText(char*);

BOOL MOD_GetDeathLink();
void MOD_ToggleDeathLink();

extern void MOD_InitCommands();