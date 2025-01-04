#include "framework.h"
#include "mod.h"
#include "connector.h"


void fn_vAttachHooks( void )
{
	FHK_M_lCreateHook(&GAM_fn_vEngine, MOD_EngineTick);
}

void fn_vDetachHooks( void )
{
	FML_M_lDestroyHook(&GAM_fn_vEngine, MOD_EngineTick);
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