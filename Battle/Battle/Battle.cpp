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
#include <windows.h>
#include "BaseLib/LeakDetector.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WinGdiLib/WinGDIWindowFactory.h"
#include "WinInputLib/WinInput.h"
#include "BattleApp.h"


//////////////////////////////////////////////////////////////////////////
//
// Battle on Windows.  Run from the Battle directory: the resources and
// WinInputLib's XML are found relative to it.
//
//////////////////////////////////////////////////////////////////////////
int PASCAL WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	bool	bResult;

	BreakOnAllocation(0);
	InitBattleGlobals();

	{
		Ptr<CWinGDIWindowFactory>	pNativeFactory;
		CWinInput					cWinInput;

		//Rooted while Battle runs.  Its windows hold it too, so it's freed after the last of them.
		pNativeFactory = OMalloc<CWinGDIWindowFactory>(&gcMemoryAllocator, hInstance, hPrevInstance, nCmdShow, "Battle");
		ORoot()->Add(pNativeFactory);

		//The app's input reads the devices from the XML, then adds these.
		cWinInput.Init(NULL, false);
		pNativeFactory->SetRawInput(WinRawInputCallback, cWinInput.GetRaw());

		bResult = RunBattleApp(&pNativeFactory, &cWinInput, "../../../CommonLibraries/Libraries/WinInputLib/xml");

		pNativeFactory->SetRawInput(NULL, NULL);
		cWinInput.Kill();
		ORoot()->Remove(pNativeFactory);
		pNativeFactory = NULL;
	}

	KillBattleGlobals();
	LeakDetection();
	return bResult ? 0 : 1;
}

