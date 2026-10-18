#pragma once

#include "bitset.h"

#ifdef __cplusplus
extern "C" {	
#endif

BOOL AP_HasDeathLink();
int AP_GetDeathLinkAmnesty();
BOOL AP_HasDamageLink();

int AP_GetEndGoal();
BOOL AP_HasLumsanity();
BOOL AP_HasBetterPortals();
BOOL AP_HasRandomizeAbilities();
BOOL AP_HasRoomRandomisation();
BOOL AP_HasAccessiblePortals();
BOOL AP_HasFragmentedSilverLums();
int AP_GetLumBundleSize();
int AP_GetLumGate(int lumGate);
BOOL AP_HasUnlockLevels();
int AP_GetChainLength(int chainId);
int AP_GetLevelInChain(int chainId, int index);
const char* AP_GetLevelName(int id);

int AP_GetLums();
int AP_GetCages();
int AP_GetMasks();
int AP_GetUpgrades();
BOOL AP_HasElixir();
BOOL AP_HasKnowledge();
BOOL AP_HasHover();
BOOL AP_HasLedgeGrab();
BOOL AP_HasSwim();
BOOL AP_HasLavaHover();
int AP_GetUnlockedPortals();
BOOL AP_HasUnlockedLevel(int portal);
BitSet* AP_GetUnlockedChecks();

void AP_PrintReport(FILE* f);

#ifdef __cplusplus
}
#endif