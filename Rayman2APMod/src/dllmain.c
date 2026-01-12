#include "framework.h"
#include "mod.h"
#include "connector.h"

/*
	Information on currently collected information is stored into the global AI object:
	4 (int): Total cages in current level
	5 (int): Collected cages in current level
	6 (int): Total cages in any level
	42 (integer array): All collected objects (ranges are inclusive)
		1-800 and 1201-1400 are yellow lums
		840-919 are cages
		1188 is nomovies
		1101 is the knowledge of the name of the Cave of Bad Dreams
		1123 is the Elixir of Life
		1146 are given on game completion

		962-963 is golden fists, 962 & 963 means 3, 962 means 2, 963 means 1
		1095 and 1143 appear to be projectile upgrades (silver lums)
		1095 determines if you can grab purple lums.

		Whether portals are present on the main menu uses 960+DsgVar2 on the portal
		object in the hall of doors.

	43 (unsigned byte): Remaining lums in current level
	44 (unsigned byte): Total lums in current level
	45 (unsigned byte): Collected lums in current level
	46 (unsigned byte): Collected cages for health increase

	Usage notes:
	42 should normally be set to contain the current state of the save file so we can generally safely reference against it.
	We should swap it out for a custom one when doing Lum Gate lookup s which happens in Ray_Nego only,
	so we can check if the current level is nego_10 and have it set to the fake values.

	Var 5 on Ray is the level id in this case:
	40 -> Iron Mountains
	33 -> Beneath the Sanctuary
	21 -> Lava Sanctuary
	10 -> Water Sanctuary

	It compares the current total 1's in the lum spots of the 42 array to the hardcoded lum requirements (550, 475, 300, 100) and
	prints text on the screen if you're under, or sets global vars at 1008, 1010, 1011, or 1012, these are then read by the portal code
	later to determine where you get sent.

	There is entirely separate code for the walk of life/power checks in DS1_ZyvaEnvoieTesLums. This checks for 60 or 450 minimum hardcoded
	into variable 20 of object "ZOR_ZyVaLesLums" only on the bayou/sanctuary levels. Its variable 18 is whether you have enough and 19 is how many you have.

	Changing 18 is enough to make the portal spawn at the end of the sequence.

	The actual entrance opens up whenever 992 is set to 1 on the global array.

	Woods of Light lets you through the cutscene if 43 is 0.
*/

/** Attach detours hooks to game events. */
void fn_vAttachHooks( void ) {
	FHK_M_lCreateHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lCreateHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lCreateHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
}

/** Remove detours hooks from game events. */
void fn_vDetachHooks( void ) {
	FHK_M_lDestroyHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lDestroyHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lDestroyHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
}

__declspec(dllexport)
int ModMain(BOOL bInit) {
	if (bInit) {
		fn_vAttachHooks();

		// Try to start the AP connector, shut down the program on failure!
		if (MOD_StartConnector()) {
			exit(2);
			return 1;
		}
		MOD_Main();
	} else {
		MOD_StopConnector();
		fn_vDetachHooks();
	}
	return 0;
}