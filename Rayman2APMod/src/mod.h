#pragma once
#include "framework.h"
#include "connector.h"

void MOD_EngineTick();
void MOD_Print(char*, ...);
void MOD_Main();
void MOD_TriggerDeath();
void MOD_ShowScreenText(char*);

BOOL MOD_GetDeathLink();
void MOD_SetDeathLink(BOOL value);

extern void MOD_InitCommands();