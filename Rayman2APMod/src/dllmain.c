#include "framework.h"
#include "mod.h"
#include "connector.h"

void fn_vAttachHooks( void )
{
	FHK_M_lCreateHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lCreateHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lCreateHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
	FHK_M_lCreateHook(&GAM_fn_vSetLevelName, MOD_SetLevel);
	FHK_M_lCreateHook(&GAM_fn_vSetNextLevelName, MOD_SetNextLevel);
	FHK_M_lCreateHook(&GAM_fn_vSetFirstLevelName, MOD_SetFirstLevel);
}

void fn_vDetachHooks( void )
{
	FHK_M_lDestroyHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lDestroyHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lDestroyHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
	FHK_M_lDestroyHook(&GAM_fn_vSetLevelName, MOD_SetLevel);
	FHK_M_lDestroyHook(&GAM_fn_vSetNextLevelName, MOD_SetNextLevel);
	FHK_M_lDestroyHook(&GAM_fn_vSetFirstLevelName, MOD_SetFirstLevel);
}

/*
	Information on currently collected information is stored into the global AI object.
	4 (int): Total cages in current level
	5 (int): Collected cages in current level
	42 (integer array): All collected objects (1-800 and 1201-1400 are yellow lums, 962-963 is silver lums, 840-481 are cages in Learn10, 849-853 are cages in Ski10, ranges are inclusive)
	43 (unsigned byte): Remaining lums in current level
	44 (unsigned byte): Total lums in current level
	45 (signed byte): Collected lums in current level
*/

__declspec(dllexport)
int ModMain( BOOL bInit )
{
	if ( bInit )
	{
		fn_vAttachHooks();

		// Try to start the AP connector, shut down the program on failure!
		if (MOD_StartConnector()) {
			exit(2);
			return 1;
		}
		MOD_Main();
	}
	else
	{
		MOD_StopConnector();
		fn_vDetachHooks();
	}

	return 0;
}