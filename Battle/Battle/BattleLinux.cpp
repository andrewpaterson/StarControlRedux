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
#include "X11Lib/X11WindowFactory.h"
#include "X11Lib/X11Input.h"
#include "BattleApp.h"


//////////////////////////////////////////////////////////////////////////
//
// Battle on Linux.  Run from the Battle directory: the resources and
// WinInputLib's XML are found relative to it.
//
//////////////////////////////////////////////////////////////////////////
int main(void)
{
	bool	bResult;

	InitBattleGlobals();

	{
		Ptr<CX11WindowFactory>	pNativeFactory;
		CX11Input				cX11Input;

		//Rooted while Battle runs.  Its windows hold it too, so it's freed after the last of them.
		pNativeFactory = OMalloc<CX11WindowFactory>(&gcMemoryAllocator, (const char*)NULL);
		ORoot()->Add(pNativeFactory);
		bResult = pNativeFactory->IsOpen();
		if (bResult)
		{
			cX11Input.Init();
			pNativeFactory->SetInput(&cX11Input);

			bResult = RunBattleApp(&pNativeFactory, &cX11Input, "../../../CommonLibraries/Libraries/WinInputLib/xml");

			pNativeFactory->SetInput(NULL);
			cX11Input.Kill();
		}
		ORoot()->Remove(pNativeFactory);
		pNativeFactory = NULL;
	}

	KillBattleGlobals();
	return bResult ? 0 : 1;
}

