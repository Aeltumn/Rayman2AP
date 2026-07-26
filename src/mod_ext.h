#pragma once

#include "framework.h"

#ifdef __cplusplus
extern "C" {	
#endif

void MOD_Print(const char*, ...);
void MOD_UpdateSettings(bool connected, bool deathLink, bool damageLink, int endGoal, bool lumsanity, bool roomRandomisation, bool accessiblePortals, int deathLinkAmnesty, bool betterLevelPortals, int lumBundleSize, int* lumGates, const char** levelIds, int* chainLengths, int** chainContents);
void MOD_UpdateState(int lums, int cages, int masks, int upgrades, bool elixir, bool knowledge, bool fragmented, bool hover, bool ledge, bool swim, bool lavaHover);
void MOD_TriggerDeath(const char* data);
void MOD_Notify(const char* message);
void MOD_Chat(const char* message);
void MOD_Reset();

#ifdef __cplusplus
}
#endif