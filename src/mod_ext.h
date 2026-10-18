#pragma once

#include "framework.h"

#ifdef __cplusplus
extern "C" {	
#endif

void MOD_Print(const char*, ...);
void MOD_TriggerDeath(const char* data);
void MOD_Notify(const char* message);
void MOD_Chat(const char* message);
void MOD_Reset();
BOOL MOD_IsLumLike(int x);
BOOL MOD_IsSuperLum(int x);

#ifdef __cplusplus
}
#endif