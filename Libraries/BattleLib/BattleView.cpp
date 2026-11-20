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
#include <string.h>
#include "BaseLib/StringHelper.h"
#include "BaseLib/Timer.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "BattleView.h"


static const SBattleKey	gasBattleKeys[] =
{
	{ "A",				0,	BK_Left },
	{ "D",				0,	BK_Right },
	{ "W",				0,	BK_Thrust },
	{ "Space",			0,	BK_Primary },
	{ "S",				0,	BK_Secondary },
	{ "Cursor Left",	1,	BK_Left },
	{ "Cursor Right",	1,	BK_Right },
	{ "Cursor Up",		1,	BK_Thrust },
	{ "Cursor Down",	1,	BK_Primary },
	{ "/",				1,	BK_Secondary },
	{ "1",				0,	BK_Computer },
	{ "2",				1,	BK_Computer },
	{ "Enter",			0,	BK_Restart },
	{ "V",				0,	BK_Renderer },
};


#define BATTLE_NUM_KEYS	(sizeof(gasBattleKeys) / sizeof(SBattleKey))


//////////////////////////////////////////////////////////////////////////
//
// A battle, ready to be flown, in a canvas that fills what it's given.
// With bVulkan it's drawn with Vulkan if there's a device for it;
// without, by the rasteriser, and no device is looked for.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bVulkan)
{
	Ptr<CBattleScene>	pScene;

	PreInit();

	mpGame = OMalloc<CBattleGame>();
	mpDraw = OMalloc<CBattleCanvasDraw>(pTheme);
	CCanvas::Init(pWindow, mpDraw);
	miLastAdvance = 0;
	mfKeySeconds = BATTLE_VIEW_KEY_SECONDS;
	memset(mabDown, 0, sizeof(mabDown));
	memset(mafKeep, 0, sizeof(mafKeep));

	pScene = mpGame->GetScene();
	mpDraw->UseVulkan(bVulkan);
	mpDraw->SetScene(pScene->GetScene(), "Battle");
	mpDraw->SetSpan(pScene->GetHalfAcross(), pScene->GetHalfUp(), 0.0f);

	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetCanGetFocus(true);
	AddInputAction(IAF_AnyKeyDown, NULL, this, &CBattleView::KeyDownAction);
	AddInputAction(IAF_AnyKeyUp, NULL, this, &CBattleView::KeyUpAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::Class(void)
{
	CCanvas::Class();

	M_Pointer(mpGame);
	M_Pointer(mpDraw);
	U_Int64(miLastAdvance);
	U_Float32(mfKeySeconds);
	U_BoolArray(mabDown, BATTLE_SHIPS * BATTLE_VIEW_CONTROLS);
	U_Float32Array(mafKeep, BATTLE_SHIPS * BATTLE_VIEW_CONTROLS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::Free(void)
{
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// The battle moved on by the time since this was last called, none the
// first time, and none while it's paused.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::Advance(void)
{
	int64	iNow;
	float32	fSeconds;

	iNow = GetDebugTickCount();
	fSeconds = ((miLastAdvance != 0) && !IsPaused()) ? (float32)(iNow - miLastAdvance) / 1000.0f : 0.0f;
	miLastAdvance = iNow;
	AdvanceBy(fSeconds);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the battle is standing still because its window isn't the
// active one: no key reaches it then, so a player couldn't fly.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleView::IsPaused(void)
{
	return !GetWindow()->GetFocus()->IsActive();
}


//////////////////////////////////////////////////////////////////////////
//
// The battle moved on by fSeconds, and the canvas drawn again showing
// both ships.  Controls kept on after their keys came up are let go when
// their time is over.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::AdvanceBy(float32 fSeconds)
{
	Ptr<CBattleScene>	pScene;

	mpGame->Advance(fSeconds);
	ReleaseKept(fSeconds);
	pScene = mpGame->GetScene();
	mpDraw->SetSpan(pScene->GetHalfAcross(), pScene->GetHalfUp(), fSeconds);
	mpDraw->Redraw();
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// The field of a ship's controls that a control's key holds.
//
//////////////////////////////////////////////////////////////////////////
static bool* BattleViewHeld(SBattleControls* psHeld, EBattleKey eControl)
{
	switch (eControl)
	{
	case BK_Left:
		return &psHeld->bLeft;
	case BK_Right:
		return &psHeld->bRight;
	case BK_Thrust:
		return &psHeld->bThrust;
	case BK_Primary:
		return &psHeld->bPrimary;
	default:
		return &psHeld->bSecondary;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A control's key has gone down or come up.  Down, the control is on, and
// stays on for at least the key seconds.  Up, it goes off unless it's
// still being kept on, when ReleaseKept turns it off in its time.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::Control(int32 iShip, EBattleKey eControl, bool bDown)
{
	size	uiControl;
	bool*	pbHeld;

	uiControl = (size)iShip * BATTLE_VIEW_CONTROLS + (size)eControl;
	pbHeld = BattleViewHeld(mpGame->GetHeld(iShip), eControl);
	mabDown[uiControl] = bDown;
	if (bDown)
	{
		*pbHeld = true;
		mafKeep[uiControl] = mfKeySeconds;
	}
	else if (mafKeep[uiControl] <= 0.0f)
	{
		*pbHeld = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// fSeconds of the battle have gone by with the controls as they were: a
// control that was only being kept on, its key up, goes off once it has
// been on for long enough.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::ReleaseKept(float32 fSeconds)
{
	int32	iShip;
	int32	iControl;
	size	uiControl;

	for (iShip = 0; iShip < BATTLE_SHIPS; iShip++)
	{
		for (iControl = 0; iControl < BATTLE_VIEW_CONTROLS; iControl++)
		{
			uiControl = (size)iShip * BATTLE_VIEW_CONTROLS + (size)iControl;
			if (mafKeep[uiControl] > 0.0f)
			{
				mafKeep[uiControl] -= fSeconds;
				if ((mafKeep[uiControl] <= 0.0f) && !mabDown[uiControl])
				{
					*BattleViewHeld(mpGame->GetHeld(iShip), (EBattleKey)iControl) = false;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How long a control stays on for once its key is pressed, however soon
// the key comes up: none for a control that's on only while its key is
// down.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::SetKeySeconds(float32 fSeconds)
{
	mfKeySeconds = fSeconds;
}


//////////////////////////////////////////////////////////////////////////
//
// Once the view is in its window, so its keys work without clicking it
// first.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::FocusView(void)
{
	GetWindow()->GetFocus()->SetFocus(this);
}


//////////////////////////////////////////////////////////////////////////
//
// A key that isn't one of the battle's goes on to whatever else wants it.
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::KeyDownAction(SComponentInputEvent* psEvent)
{
	psEvent->bPass = !Key(psEvent->szName, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleView::KeyUpAction(SComponentInputEvent* psEvent)
{
	psEvent->bPass = !Key(psEvent->szName, false);
}


//////////////////////////////////////////////////////////////////////////
//
// What the key named does, as it goes down or comes up: a control is
// held while its key is down, and the others act as they go down.  A
// control pressed for a ship a player flies begins a battle that's
// waiting, and so does Enter, which otherwise starts the battle again.
// False if it isn't one of the battle's keys.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleView::Key(const char* szName, bool bDown)
{
	const SBattleKey*	psKey;
	size				i;

	if (szName == NULL)
	{
		return false;
	}

	for (i = 0; i < BATTLE_NUM_KEYS; i++)
	{
		psKey = &gasBattleKeys[i];
		if (StringCompare(psKey->szName, szName) == 0)
		{
			if (psKey->eKey <= BK_Secondary)
			{
				if (bDown && !mpGame->IsComputer(psKey->iShip))
				{
					mpGame->Begin();
				}
				Control(psKey->iShip, psKey->eKey, bDown);
				return true;
			}

			switch (psKey->eKey)
			{
			case BK_Computer:
				if (bDown)
				{
					mpGame->SetComputer(psKey->iShip, !mpGame->IsComputer(psKey->iShip));
				}
				break;
			case BK_Restart:
				if (bDown && mpGame->IsWaiting())
				{
					mpGame->Begin();
				}
				else if (bDown)
				{
					mpGame->Restart();
				}
				break;
			case BK_Renderer:
				if (bDown)
				{
					mpDraw->SwitchRenderer();
				}
				break;
			default:
				break;
			}
			return true;
		}
	}
	return false;
}


Ptr<CBattleGame> CBattleView::GetGame(void) const { return mpGame; }
Ptr<CBattleCanvasDraw> CBattleView::GetDraw(void) const { return mpDraw; }

