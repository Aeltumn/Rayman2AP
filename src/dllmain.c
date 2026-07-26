#include "framework.h"
#include "ap_connect.h"
#include "mod.h"

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

		1123 is the Elixir of Life, you only get this within the cutscene
		in Marshes, so we also send the check when 1120 is sent which is
		set when you are kicked out of the Cave of Bad Dreams after the ending cutscene.

		962-963 is golden fists, 962 & 963 means 3, 962 means 2, 963 means 1
		1095 and 1143 appear to be projectile upgrades (silver lums)
		1095 determines if you can grab purple lums (blue)
		1143 determines if you can charge shots (gold)


		1089 is for completing the intro cutscene.
		1091 for watching woods of light intro.
		1097 for watching the Bayou cutscene.
		1102 for completing Menhir Hills #2 cutscene.
		1114 for watching the 2nd Polocus cutscene - changes ending of Beneath 2
		1125 for talking to Ly in Beneath
		1131 for watching the Precipice cutscene.
		1132 for watching the Tomb cutscene.
		1133 for watching woods of light teensie cutscene and whether we go to mapmonde or jail.
		1159 for deating Foutch 
		1176 for talking to Murfy in the Woods of Light
		1175 for talking to Murfy in the Woods of Light again

		1171 for Clark destroying the first wall in Menhir Hills #2
		1172 for Clark destroying the second wall in Menhir Hills #2

		953 for saving Ly
		954 for saving Carmen
		957 for opening gate in Canopy #2
		958 for saving Globox
		959 for opening gate in Canopy #3

		Whether portals are present on the main menu uses 960+DsgVar2 on the portal
		object in the hall of doors.

	43 (unsigned byte): Remaining lums in current level
	44 (unsigned byte): Total lums in current level
	45 (unsigned byte): Collected lums in current level
	46 (unsigned byte): Collected cages for health increase
	67 (int): Last level id in HOF

	Usage notes:
	42 should normally be set to contain the current state of the save file so we can generally safely reference against it.
	We should swap it out for a custom one when doing Lum Gate lookup s which happens in Ray_Nego only,
	so we can check if the current level is nego_10 and have it set to the fake values.

	Game state stores level id which are for lum gates:
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

	Woods of Light lets you through the cutscene if 43 is 0, aka all lums collected that are available.

	Silver Lum progression is determined as either:
	2x Silver Lum
	OR
	1 Fairy Glade Revisit Swing
	2 Cave of Bad Dreams 1 Swings
	4 Cave of Bad Dreams 2 Swings
	8 Stone and Fire Side Temple Swing
	16 Fairy Glade 4 Swing
	32 Fairy Glade 5 Swing
	64 Bayou 1 Swings
	128 Bayou 2 Swing
	256 Water and Ice 2 Swings
	512 Menhir Hills 2 Swings
	1024 Menhir Hills 3 Swing
	2048 Canopy 3 Swing
	4096 Whale Bay 1 Swing
	8192 Stone and Fire 1 Swings
	16384 Stone and Fire 2 Swings
	32768 Precipice 1 Swings
	65536 Rock and Lava 1 Swing
	131072 Beneath Rock and Lava 3 Swing
	262144 Tomb of the Ancients 2 Swings
	524288 Iron Mountains 1 Swings
	1048576 Iron Mountains 3 Swings
	2097152 Powered Shots
*/

/** Attach detours hooks to game events. */
void fn_vAttachHooks( void ) {
	FHK_M_lCreateHook(&GAM_fn_WndProc, MOD_WndProc);
	FHK_M_lCreateHook(&GAM_fn_vEngine, MOD_EngineTick);
	FHK_M_lCreateHook(&GAM_fn_vChooseTheGoodInit, MOD_Init);
	FHK_M_lCreateHook(&GAM_fn_vAskToChangeLevel, MOD_ChangeLevel);
}

/** Remove detours hooks from game events. */
void fn_vDetachHooks( void ) {
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