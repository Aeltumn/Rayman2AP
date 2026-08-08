#include "mod.h"

/** Attach detours hooks to game events. */
void fn_vAttachHooks() {
	FHK_M_lCreateHook(&GAM_fn_WndProc, MOD_WndProc);
	FHK_M_lCreateHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lCreateHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lCreateHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
}

/** Remove detours hooks from game events. */
void fn_vDetachHooks() {
	FHK_M_lDestroyHook(&GAM_fn_WndProc, MOD_WndProc);
	FHK_M_lDestroyHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lDestroyHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lDestroyHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
}

__declspec(dllexport)
int ModMain(BOOL bInit) {
	if (bInit) {
		fn_vAttachHooks();
		if (AP_StartArchipelagoConnector()) {
			MOD_Print("Failed to start Archipelago connector!");
			exit(2);
			return 1;
		}
		MOD_StartMod();
	} else {
		AP_StopArchipelagoConnector();
		fn_vDetachHooks();
	}
	return 0;
}