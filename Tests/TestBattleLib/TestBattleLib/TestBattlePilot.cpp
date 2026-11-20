/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela TestBattleLib

Codaphela TestBattleLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela TestBattleLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela TestBattleLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BattleLib/BattleGeometry.h"
#include "BattleLib/BattlePilot.h"
#include "TestLib/Assert.h"


#define TEST_PILOT_STEP	(1.0f / 120.0f)


//////////////////////////////////////////////////////////////////////////
//
// The battle flown by pilots for up to fSeconds, or until it's over.
// A pilot for a ship that hasn't one leaves it doing nothing.
//
//////////////////////////////////////////////////////////////////////////
static void TestBattlePilotFly(CBattle* pcBattle, bool bFirst, bool bSecond, float32 fSeconds)
{
	SBattleControls	sControls;
	int32			iSteps;
	int32			i;

	iSteps = (int32)(fSeconds / TEST_PILOT_STEP);
	for (i = 0; (i < iSteps) && !pcBattle->IsOver(); i++)
	{
		if (bFirst)
		{
			BattlePilot(pcBattle, 0, &sControls);
			pcBattle->SetControls(0, &sControls);
		}
		if (bSecond)
		{
			BattlePilot(pcBattle, 1, &sControls);
			pcBattle->SetControls(1, &sControls);
		}
		pcBattle->Step(TEST_PILOT_STEP);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pilot turns the shorter way to face the other ship, thrusts when
// it's far off and facing it, and fires when it's in reach and in front.
// The planet comes before the other ship.
//
//////////////////////////////////////////////////////////////////////////
void TestBattlePilotDecides(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleShip*	psOther;
	SBattleControls	sControls;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);
	psShip = cBattle.GetShip(0);
	psOther = cBattle.GetShip(1);

	//The other ship far off to the right, along the bottom away from the planet.
	psShip->sPosition.Init(20.0f, 10.0f);
	psShip->fHeading = 0.0f;
	psOther->sPosition.Init(180.0f, 10.0f);
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bRight);
	AssertFalse(sControls.bLeft);
	AssertFalse(sControls.bThrust);  //Not while it's facing a quarter turn away.
	AssertFalse(sControls.bPrimary);

	psShip->fHeading = BATTLE_PI;
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bLeft);
	AssertFalse(sControls.bRight);

	psShip->fHeading = BATTLE_PI * 0.5f;
	BattlePilot(&cBattle, 0, &sControls);
	AssertFalse(sControls.bLeft);
	AssertFalse(sControls.bRight);
	AssertTrue(sControls.bThrust);
	AssertFalse(sControls.bPrimary);  //Out of its bolts' reach,
	AssertTrue(sControls.bSecondary);  //though not its seeker's.

	//In reach and in front: bolts, and a seeker while there's energy to spare.
	psOther->sPosition.Init(70.0f, 10.0f);
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bPrimary);
	AssertTrue(sControls.bSecondary);
	AssertFalse(sControls.bThrust);
	psShip->fEnergy = cKestrel.mfEnergy * 0.3f;
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bPrimary);
	AssertFalse(sControls.bSecondary);

	//The Bulwark's pilot sets its burst off when a shot is close, or the other ship is.
	BattlePilot(&cBattle, 1, &sControls);
	AssertFalse(sControls.bSecondary);
	psShip->sPosition.Init(180.0f, 10.0f);
	psOther->sPosition.Init(170.0f, 10.0f);
	BattlePilot(&cBattle, 1, &sControls);
	AssertTrue(sControls.bSecondary);

	//Drifting at the planet, a pilot turns to the side to pass it, and thrusts once it's nearly facing that way,
	//whatever the other ship is doing.
	psShip->sPosition.Init(200.0f, 150.0f);
	psShip->sVelocity.Init(1.0f, 30.0f);
	psShip->fHeading = 0.0f;
	psOther->sPosition.Init(300.0f, 300.0f);
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bRight);  //The planet is a little to the left of where it's going, so it passes on the right.
	AssertFalse(sControls.bLeft);
	AssertFalse(sControls.bThrust);
	AssertFalse(sControls.bPrimary);
	psShip->fHeading = BATTLE_PI * 0.5f + 0.1f;
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bThrust);
	AssertFalse(sControls.bRight);

	//Too near it, it thrusts straight out.
	psShip->sPosition.Init(200.0f, 180.0f);
	psShip->sVelocity.Init(0.0f, 0.0f);
	psShip->fHeading = BATTLE_PI;
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bThrust);
	AssertFalse(sControls.bLeft);
	AssertFalse(sControls.bRight);

	//With the planet between the ships it flies round, not at the other ship, and holds its fire.
	psShip->sPosition.Init(140.0f, 199.0f);
	psShip->fHeading = BATTLE_PI * 0.5f;
	psOther->sPosition.Init(260.0f, 200.0f);
	BattlePilot(&cBattle, 0, &sControls);
	AssertTrue(sControls.bRight);  //The planet is a little above the way across, so round underneath it.
	AssertFalse(sControls.bLeft);
	AssertFalse(sControls.bPrimary);
	AssertFalse(sControls.bSecondary);

	//A destroyed ship's pilot does nothing.
	psShip->bAlive = false;
	BattlePilot(&cBattle, 0, &sControls);
	AssertFalse(sControls.bThrust);
	AssertFalse(sControls.bLeft);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pilot beats a ship that does nothing, with either ship, and isn't
// hurt doing it: it never flies into the planet.
//
//////////////////////////////////////////////////////////////////////////
void TestBattlePilotBeatsNobody(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);

	cBattle.Init(&cKestrel, &cBulwark);
	TestBattlePilotFly(&cBattle, true, false, 180.0f);
	AssertTrue(cBattle.IsOver());
	AssertInt(0, cBattle.GetWinner());
	AssertFloat(cKestrel.mfHull, cBattle.GetShip(0)->fHull, 4);
	cBattle.Kill();

	cBattle.Init(&cKestrel, &cBulwark);
	TestBattlePilotFly(&cBattle, false, true, 180.0f);
	AssertTrue(cBattle.IsOver());
	AssertInt(1, cBattle.GetWinner());
	AssertFloat(cBulwark.mfHull, cBattle.GetShip(1)->fHull, 4);
	cBattle.Kill();

	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two pilots finish a battle, and the same battle goes the same way
// every time.
//
//////////////////////////////////////////////////////////////////////////
void TestBattlePilotAgainstPilot(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	int32			iWinner;
	float32			fSeconds;
	float32			fHull;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);

	cBattle.Init(&cKestrel, &cBulwark);
	TestBattlePilotFly(&cBattle, true, true, 600.0f);
	AssertTrue(cBattle.IsOver());
	iWinner = cBattle.GetWinner();
	fSeconds = cBattle.GetSeconds();
	fHull = cBattle.GetShip(0)->fHull + cBattle.GetShip(1)->fHull;
	AssertTrue(cBattle.GetShip(0)->fHull < cKestrel.mfHull);  //Both were hit on the way.
	AssertTrue(cBattle.GetShip(1)->fHull < cBulwark.mfHull);
	cBattle.Kill();

	cBattle.Init(&cKestrel, &cBulwark);
	TestBattlePilotFly(&cBattle, true, true, 600.0f);
	AssertInt(iWinner, cBattle.GetWinner());
	AssertFloat(fSeconds, cBattle.GetSeconds(), 5);
	AssertFloat(fHull, cBattle.GetShip(0)->fHull + cBattle.GetShip(1)->fHull, 5);
	cBattle.Kill();

	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBattlePilot(void)
{
	BeginTests();

	TestBattlePilotDecides();
	TestBattlePilotBeatsNobody();
	TestBattlePilotAgainstPilot();

	TestStatistics();
}

