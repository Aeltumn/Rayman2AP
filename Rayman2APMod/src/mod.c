#include "mod.h"

int g_enabled = 0;

int g_lSize = 6;
int g_lPosX = 10;
int g_lPosY = 10;

/*void MOD_vDrawDebug(SPTXT_tdstTextInfo* pInfo)
{
	// Based on R2DbgScr while I'm figuring out how this all works.

	pInfo->xSize = g_lSize;
	pInfo->X = g_lPosX;
	pInfo->Y = g_lPosY;
	pInfo->bFrame = TRUE;

	// Engine
	SPTXT_vPrintLine(TXT_Red("ENGINE"));
	SPTXT_vPrintFmtLine("state=:" TXT_Yellow("%d"), GAM_fn_ucGetEngineMode());
	SPTXT_vPrintFmtLine("level=:" TXT_Yellow("%s"), GAM_fn_p_szGetLevelName());
	SPTXT_vNewLine();
}

void MOD_vDrawPrompt( SPTXT_tdstTextInfo *pInfo )
{
	pInfo->xSize = g_lSize;
	pInfo->X = 1000-5;
	pInfo-> Y = g_lPosY;
	pInfo->bRightAlign = TRUE;
	SPTXT_vPrintLine("R2Archipelago F9 - " TXT_Yellow("toggle debugging"));
}

void CALLBACK MOD_vTextCallback( SPTXT_tdstTextInfo *p_stInfo )
{
	MOD_vDrawPrompt(p_stInfo);
	SPTXT_vResetTextInfo(p_stInfo);

	if (g_enabled) {
		MOD_vDrawDebug(p_stInfo);
	}
}

LRESULT CALLBACK MOD_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_KEYDOWN && IPT_g_stInputStructure->ulNumberOfEntryElement )
	{
		if ( wParam == VK_F9 )
		{
			g_enabled = ++g_enabled % 2;
			return 0;
		}
	}

	return GAM_fn_WndProc(hWnd, uMsg, wParam, lParam);
}*/

void MOD_Main( void )
{
	//SPTXT_vInit();
	//SPTXT_vAddTextCallback(MOD_vTextCallback);
}
