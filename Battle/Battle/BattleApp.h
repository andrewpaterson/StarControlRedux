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
#ifndef __BATTLE_APP_H__
#define __BATTLE_APP_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/FontColours.h"
#include "InputLib/Input.h"
#include "InputLib/NativeInput.h"
#include "WindowLib/ComponentInput.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/Label.h"
#include "WindowLib/NativeWindowFactory.h"
#include "WindowLib/ProgressBar.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowInputSource.h"
#include "WindowLib/WindowTheme.h"
#include "BattleLib/BattleView.h"


#define BATTLE_APP_BAR_HEIGHT		10
#define BATTLE_APP_SNAPSHOT_TICKS	10				//The fewest ticks before BATTLE_SNAPSHOT's image is saved, so everything has been laid out and drawn.
#define BATTLE_APP_SNAPSHOT_STEP	(1.0f / 60.0f)	//How far each tick moves the battle on while a snapshot is waited for.


//A battle between two ships in a window: along the top each ship's name, who flies it, and its hull and energy as bars;
//the battle (BattleLib's CBattleView, which has the keys); and along the bottom what's happening and what the keys are.
//main makes it, adds it to the root, runs it and removes it from the root.
//
//The window is ticked as fast as it's painted, and each tick moves the battle on by the time that's passed (Tick).
//The battle waits for a player's first key before it begins, and stands still while the window isn't the active one;
//the status says which.
//
//From the environment: BATTLE_SNAPSHOT=file saves the window as a PNG there, after BATTLE_SNAPSHOT_SECONDS of the battle
//(none if it isn't set) played a sixtieth of a second a tick rather than by the clock, so it's the same picture every
//time; BATTLE_EXIT then closes the window, which is how it's looked at on the reference screen (BattleRef).  BATTLE_DEMO
//has the computer fly both ships, and BATTLE_NO_VULKAN has it drawn by the rasteriser whatever the machine has.
class CBattleApp : public CObject
{
CONSTRUCTABLE(CBattleApp);
DESTRUCTABLE(CBattleApp);
protected:
	Ptr<CWindow>				mpWindow;
	Ptr<CFontColours>			mpFontColours;
	Ptr<CWindowTheme>			mpTheme;
	Ptr<CGridContainer>			mpMain;  //The ships' bars over the battle over the status.
	Ptr<CGridContainer>			mpBars;
	Ptr<CBattleView>			mpView;
	Ptr<CLabel>					mpFirstName;
	Ptr<CProgressBar>			mpFirstHull;
	Ptr<CProgressBar>			mpFirstEnergy;
	Ptr<CLabel>					mpSecondName;
	Ptr<CProgressBar>			mpSecondHull;
	Ptr<CProgressBar>			mpSecondEnergy;
	Ptr<CLabel>					mpStatus;
	Ptr<CInput>					mpInput;
	Ptr<CWindowInputSource>		mpInputSource;
	CComponentInput				mcComponentInput;
	bool						mbValid;
	bool						mbInputs;  //The component input is set up.
	bool						mbExit;  //The window stops at the next tick.
	bool						mbSnapshot;  //A snapshot is asked for: the battle is moved on a fixed step a tick.
	int32						miSnapshotTicks;  //Ticks until the snapshot is saved, -1 once it has been or if none is asked for.

public:
	void	Init(void);
	bool	Setup(CNativeWindowFactory* pcNativeFactory, CNativeInput* pcNativeInput, const char* szInputDirectory);
	void	Class(void) override;
	void	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	bool	Run(void);
	void	Tick(Ptr<CWindow> pWindow);

protected:
	void	MakeBars(void);
	void	ShowBattle(void);
	void	ShowShip(int32 iShip, Ptr<CLabel> pName, Ptr<CProgressBar> pHull, Ptr<CProgressBar> pEnergy);
	void	SetLabel(Ptr<CLabel> pLabel, const char* szText);
	void	Snapshot(Ptr<CWindow> pWindow);
};


void	InitBattleGlobals(void);
void	KillBattleGlobals(void);
bool	RunBattleApp(CNativeWindowFactory* pcNativeFactory, CNativeInput* pcNativeInput, const char* szInputDirectory);


#endif // __BATTLE_APP_H__

