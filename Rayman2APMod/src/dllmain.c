#include "framework.h"
#include "mod.h"
#include "connector.h"


void fn_vAttachHooks( void )
{
}

void fn_vDetachHooks( void )
{
}

__declspec(dllexport)
int ModMain( BOOL bInit )
{
	if ( bInit )
	{
		fn_vAttachHooks();

		// Try to start the AP connector, quit out if it fails!
		if (MOD_StartConnector()) {
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