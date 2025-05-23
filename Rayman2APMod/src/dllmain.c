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
	DsgVar_3 on a lum is its index into DsgVar_42 on the global data which is all collected lums
	DsgVar_45 is the amount of lums collected
	DsgVar_43 is the amount of remaining lums

	Every lum in the game has a globally unique index!
	Every cage has DsgVar_0 for its globally unique index and DsgVar_8 for its lums' globally unique index.
	DsgVar_4 on a cage decides its contents, it always spawns as many 5 lums as possible then 1 lums.
	When it sets a 5 lum the id increments by 5, otherwise by 1.

	YLT_InitLevel in each level sets the lum counter:
	((GlobalActorModel__World)Refs.Perso["global"]).DsgVar_43 = (50-ACT_GetNumberOfBooleanInArray(((GlobalActorModel__World)Refs.Perso["global"]).DsgVar_42, 251, ((251+50)-1)));

	DsgVar_42 needs to store which objects can still be picked up.
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