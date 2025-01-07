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