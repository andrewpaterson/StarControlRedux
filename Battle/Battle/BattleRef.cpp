/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Battle

Codaphela Battle is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Battle is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Battle.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "WinRefLib/WinRefInput.h"
#include "BattleApp.h"


//The reference screen: a common developer's.
#define BATTLE_REF_SCREEN_WIDTH		2560
#define BATTLE_REF_SCREEN_HEIGHT	1440
#define BATTLE_REF_WINDOW_WIDTH		1280
#define BATTLE_REF_WINDOW_HEIGHT	800


//////////////////////////////////////////////////////////////////////////
//
// Battle on the reference screen, drawn into memory with nothing on any
// display: for looking at what it draws where there's no screen, with
// BATTLE_SNAPSHOT=file and BATTLE_EXIT (see CBattleApp).  Run from the
// Battle directory, as on a screen.  Nothing outside it can press a key,
// so BATTLE_DEMO has the computer fly both ships.
//
//////////////////////////////////////////////////////////////////////////
int main(void)
{
	bool	bResult;

	InitBattleGlobals();

	{
		Ptr<CWinRefWindowFactory>	pNativeFactory;
		CWinRefInput				cRefInput;
		CInt32Rect					cScreen;

		//Rooted while Battle runs.  Its windows hold it too, so it's freed after the last of them.
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, BATTLE_REF_WINDOW_WIDTH, BATTLE_REF_WINDOW_HEIGHT, (char*)NULL);
		ORoot()->Add(pNativeFactory);
		cScreen.Init(0, 0, BATTLE_REF_SCREEN_WIDTH, BATTLE_REF_SCREEN_HEIGHT);
		pNativeFactory->SetScreenArea(&cScreen);

		cRefInput.Init();
		bResult = RunBattleApp(&pNativeFactory, &cRefInput, "../../../CommonLibraries/Libraries/WinInputLib/xml");
		cRefInput.Kill();

		ORoot()->Remove(pNativeFactory);
		pNativeFactory = NULL;
	}

	KillBattleGlobals();
	return bResult ? 0 : 1;
}

