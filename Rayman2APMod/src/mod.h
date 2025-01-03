#pragma once
#include "framework.h"

void MOD_Print(char*, ...);
void MOD_Main();
void MOD_TriggerDeath();

BOOL MOD_GetDeathLink();
void MOD_SetDeathLink(BOOL value);

extern void MOD_InitCommands();