#pragma once
#include "framework.h"
#include "connector.h"
#include "bitset.h"

void MOD_EngineTick();
void MOD_Init();
void MOD_ChangeLevel(const char* szLevelName, ACP_tdxBool bSaveGame);
void MOD_SetLevel(const char* szName);
void MOD_SetNextLevel(const char* szName);
void MOD_SetFirstLevel(const char* szName);

void MOD_Print(char*, ...);
void MOD_Main();
void MOD_TriggerDeath();
void MOD_ShowScreenText(char*);

BOOL MOD_GetDeathLink();
void MOD_SetDeathLink(BOOL value);

extern void MOD_InitCommands();