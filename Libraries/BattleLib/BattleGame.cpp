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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "BattleMeshes.h"
#include "BattleGame.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleGame::Init(void)
{
	Ptr<CMeshEditor>	pFirstMesh;
	Ptr<CMeshEditor>	pSecondMesh;

	PreInit();

	//The ships hit as they look: each type's outline is its mesh's.
	pFirstMesh = BattleMeshKestrel();
	pSecondMesh = BattleMeshBulwark();
	BattleShipKestrel(&mcFirstType);
	BattleShipBulwark(&mcSecondType);
	BattleMeshOutline(pFirstMesh, &mcFirstType);
	BattleMeshOutline(pSecondMesh, &mcSecondType);

	mcBattle.Init(&mcFirstType, &mcSecondType);
	memset(&msPlayers, 0, sizeof(SBattlePlayers));
	msPlayers.asHeld[0].Init();
	msPlayers.asHeld[1].Init();
	msPlayers.abComputer[0] = false;
	msPlayers.abComputer[1] = true;
	mbBegun = false;
	mpScene = OMalloc<CBattleScene>(&mcBattle, pFirstMesh, BattleMeshKestrelFlame(), pSecondMesh, BattleMeshBulwarkFlame());

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleGame::Class(void)
{
	U_Unknown(CBattleShipType, mcFirstType);
	U_Unknown(CBattleShipType, mcSecondType);
	U_Unknown(CBattle, mcBattle);
	U_Unknown(SBattlePlayers, msPlayers);
	U_Bool(mbBegun);
	M_Pointer(mpScene);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleGame::Free(void)
{
	mcBattle.Kill();
	mcSecondType.Kill();
	mcFirstType.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The battle moved on by fSeconds, with each ship flown by its player or
// the computer, and the scene put where the battle then is.  The
// computer decides once for the whole of fSeconds, which is a frame.
// Nothing moves while it's waiting for a player.
//
//////////////////////////////////////////////////////////////////////////
void CBattleGame::Advance(float32 fSeconds)
{
	SBattleControls	sControls;
	int32			i;

	if (IsWaiting())
	{
		mpScene->Update(&mcBattle);
		return;
	}

	for (i = 0; i < BATTLE_SHIPS; i++)
	{
		if (msPlayers.abComputer[i])
		{
			BattlePilot(&mcBattle, i, &sControls);
			mcBattle.SetControls(i, &sControls);
		}
		else
		{
			mcBattle.SetControls(i, &msPlayers.asHeld[i]);
		}
	}
	mcBattle.Advance(fSeconds);
	mpScene->Update(&mcBattle);
}


//////////////////////////////////////////////////////////////////////////
//
// From the beginning again, flown by whoever was flying, and waiting for
// a player if there is one.  What a player is holding is still held.
//
//////////////////////////////////////////////////////////////////////////
void CBattleGame::Restart(void)
{
	mcBattle.Start();
	mbBegun = false;
	mpScene->Update(&mcBattle);
}


//////////////////////////////////////////////////////////////////////////
//
// A player is ready: the battle is no longer waiting.
//
//////////////////////////////////////////////////////////////////////////
void CBattleGame::Begin(void)
{
	mbBegun = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the battle is standing still until a player begins it.  With
// the computer flying both ships it never is.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleGame::IsWaiting(void) const
{
	return !mbBegun && !(msPlayers.abComputer[0] && msPlayers.abComputer[1]);
}


//////////////////////////////////////////////////////////////////////////
//
// What the ship's player is holding, to be changed as keys go down and
// up.  It's what the ship does while the computer isn't flying it.
//
//////////////////////////////////////////////////////////////////////////
SBattleControls* CBattleGame::GetHeld(int32 iShip)
{
	return &msPlayers.asHeld[iShip];
}


void CBattleGame::SetComputer(int32 iShip, bool bComputer) { msPlayers.abComputer[iShip] = bComputer; }
bool CBattleGame::IsComputer(int32 iShip) const { return msPlayers.abComputer[iShip]; }
CBattle* CBattleGame::GetBattle(void) { return &mcBattle; }
Ptr<CBattleScene> CBattleGame::GetScene(void) const { return mpScene; }

