#include "framework.h"
#include "mod.h"
#include "connector.h"


void fn_vAttachHooks( void )
{
	// FHK_M_lCreateHook();
}

void fn_vDetachHooks( void )
{
	// FML_M_lDestroyHook();
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