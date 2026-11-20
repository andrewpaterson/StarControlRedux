/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BattleLib

Codaphela BattleLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BattleLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BattleLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __BATTLE_VIEW_H__
#define __BATTLE_VIEW_H__
#include "WindowLib/Canvas.h"
#include "BattleCanvasDraw.h"
#include "BattleGame.h"


#define BATTLE_VIEW_CONTROLS		5		//A ship's controls: the first of EBattleKey.
#define BATTLE_VIEW_KEY_SECONDS	0.13f	//The least a control stays on for once its key is pressed.


//What a key does in a battle.
enum EBattleKey
{
	BK_Left,		//A ship's controls, held while the key is: these five come first.
	BK_Right,
	BK_Thrust,
	BK_Primary,
	BK_Secondary,
	BK_Computer,	//The computer flies the ship, or stops flying it.
	BK_Restart,		//The battle starts again, or begins if it's waiting to.
	BK_Renderer,	//Drawn by the other renderer.
};


//A key, by the name its keyboard gives it, and what it does to which ship.
struct SBattleKey
{
	const char*	szName;
	int32		iShip;
	EBattleKey	eKey;
};


//A canvas showing a battle, with the keys that fly it.  The first ship is flown with A and D to turn, W to thrust,
//Space for its primary weapon and S for its secondary; the second with the cursor keys left, right and up, and the
//cursor's down for its primary and / for its secondary.  1 and 2 hand the first and the second ship to the computer
//or take it back, Enter starts the battle again, and V goes between Vulkan and the rasteriser.  None of them is a
//shift, control or alt, as a key pressed with one of those held is a chord and not that key.
//
//Advance moves the battle on by the time since it was last called, and has the canvas drawn again: the window's tick
//calls it.  It's the view's keys that are held, so the view must have the focus (FocusView).
//
//A battle with a player in it doesn't begin until a player presses one of a ship's controls, or Enter (see
//CBattleGame), and it stands still while its window isn't the active one, when no key would reach it (IsPaused).
//
//A control stays on for at least BATTLE_VIEW_KEY_SECONDS of the battle's time from when its key is pressed, however soon
//the key comes up (SetKeySeconds changes how long).  A remote desktop, TeamViewer for one, sends a key's press and its
//release in the same instant, and sends both again every 70 to 90 milliseconds while the key is held at the other end:
//without this such a key is never down when the battle moves on, and with it a key held there is a control held here.
//On a keyboard it only makes a very quick tap a little longer.
class CBattleView : public CCanvas
{
CONSTRUCTABLE(CBattleView);
DESTRUCTABLE(CBattleView);
protected:
	Ptr<CBattleGame>		mpGame;
	Ptr<CBattleCanvasDraw>	mpDraw;
	int64					miLastAdvance;  //When Advance was last called, 0 if it hasn't been.
	float32					mfKeySeconds;
	bool					mabDown[BATTLE_SHIPS * BATTLE_VIEW_CONTROLS];  //Each ship's controls whose keys are down.
	float32					mafKeep[BATTLE_SHIPS * BATTLE_VIEW_CONTROLS];  //Seconds each stays on for yet, whatever its key does.

public:
	void					Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bVulkan);
	void					Class(void) override;
	void					Free(void) override;

	void					Advance(void);
	void					AdvanceBy(float32 fSeconds);
	void					FocusView(void);
	bool					IsPaused(void);
	void					SetKeySeconds(float32 fSeconds);

	Ptr<CBattleGame>		GetGame(void) const;
	Ptr<CBattleCanvasDraw>	GetDraw(void) const;

	void					KeyDownAction(SComponentInputEvent* psEvent);
	void					KeyUpAction(SComponentInputEvent* psEvent);

protected:
	bool					Key(const char* szName, bool bDown);
	void					Control(int32 iShip, EBattleKey eControl, bool bDown);
	void					ReleaseKept(float32 fSeconds);
};


#endif // __BATTLE_VIEW_H__

