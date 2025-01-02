#include "framework.h"
#include "mod.h"


void fn_vAttachHooks( void )
{
	FHK_M_lCreateHook(&GAM_fn_WndProc, MOD_WndProc);
}

void fn_vDetachHooks( void )
{
	FHK_M_lDestroyHook(&GAM_fn_WndProc, MOD_WndProc);
}


__declspec(dllexport)
int ModMain( BOOL bInit )
{
	if ( bInit )
	{
		fn_vAttachHooks();
		MOD_Main();
	}
	else
	{
		fn_vDetachHooks();
	}

	return 0;
}
