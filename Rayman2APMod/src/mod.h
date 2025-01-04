#pragma once
#include "framework.h"
#include "connector.h"

void CALLBACK MOD_EngineTick();
void CALLBACK MOD_DesInit();

void MOD_Print(char*, ...);
void MOD_Main();
void MOD_TriggerDeath();
void MOD_ShowScreenText(char*);

BOOL MOD_GetDeathLink();
void MOD_SetDeathLink(BOOL value);

extern void MOD_InitCommands();