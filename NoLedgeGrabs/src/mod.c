#include "mod.h"

/** Sets the value of a DSG variable. */
BOOL AI_fn_bSetDsgVar(HIE_tdstSuperObject* p_stSuperObj, unsigned char ucDsgVarId, void** p_pValue_In) {
	if (!HIE_M_bSuperObjectIsActor(p_stSuperObj))
		return FALSE;

	AI_tdstMind* hMind = AI_M_hGetMindOfSuperObj(p_stSuperObj);

	if (ucDsgVarId > AI_M_ucGetNbDsgVar(hMind))
		return FALSE;

	AI_tdstDsgVarInfo* p_stDsgInfo = AI_M_p_stGetDsgVarInfo(hMind, ucDsgVarId);

	// Update the value at the pointer
	*(AI_M_p_cGetDsgMemBuffer(hMind) + p_stDsgInfo->ulOffsetInDsgMem) = *p_pValue_In;

	return TRUE;
}

/** Ticked by the engine every frame. */
void MOD_EngineTick() {
	HIE_tdstEngineObject* pRayEngine = HIE_M_hSuperObjectGetActor(HIE_M_hGetMainActor());
	HIE_tdstSuperObject* pRayman = HIE_fn_p_stFindObjectByName("Rayman");
	if (pRayman) {
		// Set can grab to false every frame.
		ACP_tdxBool canGrab = FALSE;
		AI_fn_bSetDsgVar(pRayman, 44, &canGrab);

		// Set the last jump time to 7 constantly as when it's above 80
		// 44 is automatically set to true. Only do this when in the methods
		// that interact with ledge grabbing!
		int activeComport = -1;
		AI_tdstMind* mind = pRayEngine->hBrain->p_stMind;
		AI_tdstAIModel* model = mind->p_stAIModel;
		AI_tdstScriptAI* scriptAI = model->a_stScriptAIIntel;
		AI_tdstIntelligence* intelligence = mind->p_stIntelligence;
		for (int i = 0; i < scriptAI->ulNbComport; i++) {
			if (intelligence->p_stCurrentComport == &scriptAI->a_stComport[i]) {
				activeComport = i;
				break;
			}
		}
		if (activeComport == 4 || // YLT_Helico
			activeComport == 2 // YLT_SautReception
			) {
			int lastJumpTime = 7;
			AI_fn_bSetDsgVar(pRayman, 24, &lastJumpTime);
		}
	}

	GAM_fn_vEngine();
}