#include "framework.h"
#include "mod.h"

/** Attach detours hooks to game events. */
void fn_vAttachHooks( void ) {
	FHK_M_lCreateHook(&GAM_fn_vEngine, MOD_EngineTick);
}

/** Remove detours hooks from game events. */
void fn_vDetachHooks( void ) {
	FHK_M_lDestroyHook(&GAM_fn_vEngine, MOD_EngineTick);
}

__declspec(dllexport)
int ModMain(BOOL bInit) {
	if (bInit) {
		fn_vAttachHooks();
	} else {
		fn_vDetachHooks();
	}
	return 0;
}