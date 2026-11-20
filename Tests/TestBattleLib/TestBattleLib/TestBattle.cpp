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
#include <string.h>
#include "BattleLib/BattleGeometry.h"
#include "BattleLib/Battle.h"
#include "TestLib/Assert.h"


#define TEST_BATTLE_STEP	(1.0f / 120.0f)


//////////////////////////////////////////////////////////////////////////
//
// Steps the battle on for fSeconds.
//
//////////////////////////////////////////////////////////////////////////
static void TestBattleRun(CBattle* pcBattle, float32 fSeconds)
{
	int32	iSteps;
	int32	i;

	iSteps = (int32)(fSeconds / TEST_BATTLE_STEP + 0.5f);
	for (i = 0; i < iSteps; i++)
	{
		pcBattle->Step(TEST_BATTLE_STEP);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The ship put somewhere, still, far from the planet unless it's put
// near it.
//
//////////////////////////////////////////////////////////////////////////
static SBattleShip* TestBattlePut(CBattle* pcBattle, int32 iShip, float32 x, float32 y, float32 fHeading)
{
	SBattleShip*	psShip;

	psShip = pcBattle->GetShip(iShip);
	psShip->sPosition.Init(x, y);
	psShip->sVelocity.Init(0.0f, 0.0f);
	psShip->fHeading = fHeading;
	psShip->fTurning = 0.0f;
	return psShip;
}


//////////////////////////////////////////////////////////////////////////
//
// A battle starts with both ships whole, either side of the planet, and
// nobody has won.  Start puts it back.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleStart(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleControls	sControls;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	AssertString("Kestrel", cKestrel.mszName.Text());
	AssertSize(3, cKestrel.muiNumOutline);  //Its tail's notch isn't in its outline.
	AssertFloat(sqrtf(18.0f), cKestrel.mfRadius, 4);
	AssertSize(6, cBulwark.muiNumOutline);

	cBattle.Init(&cKestrel, &cBulwark);
	AssertFloat(BATTLE_ARENA_SIZE, cBattle.GetSize(), 3);
	AssertInt(BATTLE_NO_SHIP, cBattle.GetWinner());
	AssertFalse(cBattle.IsOver());
	AssertSize(0, cBattle.NumShots());
	AssertSize(0, cBattle.NumEffects());

	psShip = cBattle.GetShip(0);
	AssertPointer(&cKestrel, psShip->pcType);
	AssertTrue(psShip->bAlive);
	AssertFloat(140.0f, psShip->sPosition.x, 3);
	AssertFloat(200.0f, psShip->sPosition.y, 3);
	AssertFloat(0.0f, psShip->fHeading, 4);
	AssertFloat(cKestrel.mfHull, psShip->fHull, 3);
	AssertFloat(cKestrel.mfEnergy, psShip->fEnergy, 3);
	psShip = cBattle.GetShip(1);
	AssertPointer(&cBulwark, psShip->pcType);
	AssertFloat(260.0f, psShip->sPosition.x, 3);
	AssertFloat(BATTLE_PI, psShip->fHeading, 4);

	sControls.Init();
	sControls.bThrust = true;
	sControls.bPrimary = true;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 1.0f);
	AssertTrue(cBattle.NumShots() > 0);
	AssertFloat(1.0f, cBattle.GetSeconds(), 3);

	cBattle.Start();
	psShip = cBattle.GetShip(0);
	AssertFloat(140.0f, psShip->sPosition.x, 3);
	AssertFloat(0.0f, psShip->sVelocity.Magnitude(), 4);
	AssertFalse(psShip->sControls.bThrust);
	AssertSize(0, cBattle.NumShots());
	AssertFloat(0.0f, cBattle.GetSeconds(), 4);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A ship keeps the speed it has when its thrust stops, goes no faster
// than its top speed by its own thrust, and takes a moment to start
// turning and to stop.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleFlight(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleControls	sControls;
	float32			fSpeed;
	float32			fHeading;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);

	//Along the bottom, as far from the planet as there is, facing right.
	psShip = TestBattlePut(&cBattle, 0, 10.0f, 5.0f, BATTLE_PI * 0.5f);
	sControls.Init();
	sControls.bThrust = true;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 0.5f);
	AssertTrue(psShip->bThrusting);
	fSpeed = psShip->sVelocity.Magnitude();
	AssertTrue(fSpeed > cKestrel.mfThrust * 0.5f * 0.9f);
	AssertTrue(fSpeed < cKestrel.mfThrust * 0.5f * 1.1f);
	AssertTrue(psShip->sVelocity.x > fSpeed * 0.99f);  //The way it faces.
	AssertTrue(psShip->sPosition.x > 15.0f);

	TestBattleRun(&cBattle, 3.0f);
	fSpeed = psShip->sVelocity.Magnitude();
	AssertTrue(fSpeed <= cKestrel.mfMaxSpeed * 1.02f);
	AssertTrue(fSpeed > cKestrel.mfMaxSpeed * 0.95f);

	//Coasting, it keeps going: only the planet, far off, changes its speed a little.
	sControls.bThrust = false;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 1.0f);
	AssertFalse(psShip->bThrusting);
	AssertTrue(psShip->sVelocity.Magnitude() > cKestrel.mfMaxSpeed * 0.9f);

	//Turning right builds up, then holds at its turning speed.
	psShip = TestBattlePut(&cBattle, 0, 10.0f, 5.0f, 0.0f);
	sControls.bRight = true;
	cBattle.SetControls(0, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	AssertTrue(psShip->fTurning > 0.0f);
	AssertTrue(psShip->fTurning < cKestrel.mfTurnSpeed);
	TestBattleRun(&cBattle, 0.5f);
	AssertFloat(cKestrel.mfTurnSpeed, psShip->fTurning, 4);
	AssertTrue(psShip->fHeading > 1.0f);

	//Let go, it turns on a little before it stops.
	fHeading = psShip->fHeading;
	sControls.bRight = false;
	cBattle.SetControls(0, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	AssertTrue(psShip->fTurning > 0.0f);
	TestBattleRun(&cBattle, 0.5f);
	AssertFloat(0.0f, psShip->fTurning, 5);
	AssertTrue(psShip->fHeading > fHeading);
	fHeading = psShip->fHeading;
	TestBattleRun(&cBattle, 0.25f);
	AssertFloat(fHeading, psShip->fHeading, 5);

	//Left is the other way, and both together is neither.
	sControls.bLeft = true;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 0.5f);
	AssertFloat(-cKestrel.mfTurnSpeed, psShip->fTurning, 4);
	sControls.bRight = true;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 0.5f);
	AssertFloat(0.0f, psShip->fTurning, 5);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What leaves one side of the battle arrives at the other, and the way
// between two places goes across an edge when that's shorter.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleComesRound(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SFloat32Vec2	sFrom;
	SFloat32Vec2	sTo;
	SFloat32Vec2	sDelta;
	SFloat32Vec2	asOutline[BATTLE_MAX_OUTLINE_CORNERS];
	size			i;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);

	psShip = TestBattlePut(&cBattle, 0, 398.0f, 2.0f, 0.0f);
	psShip->sVelocity.Init(40.0f, -40.0f);
	TestBattleRun(&cBattle, 0.25f);
	AssertTrue(psShip->sPosition.x > 7.0f);
	AssertTrue(psShip->sPosition.x < 9.0f);
	AssertTrue(psShip->sPosition.y > 391.0f);
	AssertTrue(psShip->sPosition.y < 393.0f);

	sFrom.Init(390.0f, 10.0f);
	sTo.Init(5.0f, 395.0f);
	cBattle.Delta(&sDelta, &sFrom, &sTo);
	AssertFloat(15.0f, sDelta.x, 4);
	AssertFloat(-15.0f, sDelta.y, 4);
	sTo.Init(300.0f, 100.0f);
	cBattle.Delta(&sDelta, &sFrom, &sTo);
	AssertFloat(-90.0f, sDelta.x, 4);
	AssertFloat(90.0f, sDelta.y, 4);

	//A ship just across an edge is met where it is by the shorter way.
	psShip = TestBattlePut(&cBattle, 1, 2.0f, 200.0f, 0.0f);
	sFrom.Init(396.0f, 200.0f);
	cBattle.GetOutline(1, &sFrom, asOutline);
	for (i = 0; i < cBulwark.muiNumOutline; i++)
	{
		AssertTrue(asOutline[i].x > 396.0f);
		AssertTrue(asOutline[i].x < 408.0f);
	}
	cBattle.GetOutline(1, NULL, asOutline);
	for (i = 0; i < cBulwark.muiNumOutline; i++)
	{
		AssertTrue(asOutline[i].x < 8.0f);
	}

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The planet pulls a ship in, harder the nearer it is.  A ship that
// reaches it is thrown back out, and hurt, once, if it hit hard: set
// down on it gently it isn't.
//
//////////////////////////////////////////////////////////////////////////
void TestBattlePlanet(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SFloat32Vec2	sPlanet;
	SFloat32Vec2	sDelta;
	float32			fNear;
	float32			fFar;
	float32			fHull;
	bool			bLeaving;
	int32			i;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);
	sPlanet = cBattle.GetPlanet();
	AssertFloat(200.0f, sPlanet.x, 3);
	AssertFloat(200.0f, sPlanet.y, 3);

	psShip = TestBattlePut(&cBattle, 0, 200.0f, 140.0f, 0.0f);
	TestBattleRun(&cBattle, 0.1f);
	fFar = psShip->sVelocity.y;
	AssertTrue(fFar > 0.0f);  //Up, towards it.
	AssertFloat(0.0f, psShip->sVelocity.x, 4);

	psShip = TestBattlePut(&cBattle, 0, 200.0f, 170.0f, 0.0f);
	TestBattleRun(&cBattle, 0.1f);
	fNear = psShip->sVelocity.y;
	AssertTrue(fNear > fFar * 3.0f);  //Half as far, four times the pull.

	//Left to fall, it hits, is hurt and goes back out.
	fHull = psShip->fHull;
	bLeaving = false;
	for (i = 0; (i < 1200) && !bLeaving; i++)
	{
		cBattle.Step(TEST_BATTLE_STEP);
		bLeaving = psShip->sVelocity.y < 0.0f;
	}
	AssertTrue(bLeaving);
	AssertFloat(fHull - BATTLE_PLANET_DAMAGE, psShip->fHull, 4);
	cBattle.Delta(&sDelta, &psShip->sPosition, &sPlanet);
	AssertTrue(sDelta.Magnitude() >= cBattle.GetPlanetRadius());

	//It's never inside the planet, however long it's left to bounce.
	for (i = 0; i < 2400; i++)
	{
		cBattle.Step(TEST_BATTLE_STEP);
		cBattle.Delta(&sDelta, &psShip->sPosition, &sPlanet);
		if (sDelta.Magnitude() < cBattle.GetPlanetRadius() - 0.01f)
		{
			AssertTrue(false);
			break;
		}
	}


	//Touching it slowly only bounces.
	psShip = TestBattlePut(&cBattle, 0, 200.0f, 200.0f - 14.0f - 4.5f, 0.0f);
	psShip->fPlanetWait = 0.0f;
	fHull = psShip->fHull;
	TestBattleRun(&cBattle, 2.0f);
	AssertFloat(fHull, psShip->fHull, 4);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Bolts cost energy, wait between shots, leave from each side in turn
// with the ship's speed added, hurt the other ship and are gone when
// they hit or when their life is up.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleBolts(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleShip*	psTarget;
	SBattleShot*	psShot;
	SBattleControls	sControls;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cKestrel.mfRecharge = 0.0f;
	cBattle.Init(&cKestrel, &cBulwark);

	psShip = TestBattlePut(&cBattle, 0, 20.0f, 10.0f, BATTLE_PI * 0.5f);
	psTarget = TestBattlePut(&cBattle, 1, 60.0f, 10.0f, 0.0f);
	psShip->sVelocity.Init(10.0f, 0.0f);
	sControls.Init();
	sControls.bPrimary = true;
	cBattle.SetControls(0, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);

	AssertSize(1, cBattle.NumShots());
	psShot = cBattle.GetShot(0);
	AssertInt(BW_Bolt, psShot->eKind);
	AssertInt(0, psShot->iOwner);
	AssertFloat(cKestrel.msPrimary.fSpeed + 10.0f, psShot->sVelocity.x, 2);
	AssertTrue(psShot->sPosition.x > 22.0f);
	AssertTrue(psShot->sPosition.y < 10.0f - 2.0f);  //From its right, which is down the screen as it faces right.
	AssertFloat(cKestrel.mfEnergy - cKestrel.msPrimary.fEnergy, psShip->fEnergy, 4);

	//Held, it waits out its cooldown before the next, which comes from the other side.
	cBattle.Step(TEST_BATTLE_STEP);
	AssertSize(1, cBattle.NumShots());
	TestBattleRun(&cBattle, cKestrel.msPrimary.fCooldown);
	AssertSize(2, cBattle.NumShots());
	AssertTrue(cBattle.GetShot(1)->sPosition.y > 10.0f + 2.0f);

	//They reach the other ship and hurt it.
	sControls.bPrimary = false;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 0.5f);
	AssertSize(0, cBattle.NumShots());
	AssertFloat(cBulwark.mfHull - 2.0f * cKestrel.msPrimary.fDamage, psTarget->fHull, 4);
	AssertTrue(cBattle.NumEffects() <= 2);
	TestBattleRun(&cBattle, BATTLE_HIT_SECONDS);
	AssertSize(0, cBattle.NumEffects());

	//With nothing in the way a bolt is gone when its life is up.
	TestBattlePut(&cBattle, 1, 60.0f, 300.0f, 0.0f);
	psShip->sVelocity.Init(0.0f, 0.0f);
	sControls.bPrimary = true;
	cBattle.SetControls(0, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	sControls.bPrimary = false;
	cBattle.SetControls(0, &sControls);
	AssertSize(1, cBattle.NumShots());
	TestBattleRun(&cBattle, cKestrel.msPrimary.fLife - 0.1f);
	AssertSize(1, cBattle.NumShots());
	AssertTrue(cBattle.GetShot(0)->sPosition.x > 20.0f + cKestrel.msPrimary.fSpeed * (cKestrel.msPrimary.fLife - 0.15f));
	TestBattleRun(&cBattle, 0.2f);
	AssertSize(0, cBattle.NumShots());

	//With no energy for it, nothing is fired.
	psShip->fEnergy = cKestrel.msPrimary.fEnergy * 0.5f;
	sControls.bPrimary = true;
	cBattle.SetControls(0, &sControls);
	TestBattleRun(&cBattle, 0.5f);
	AssertSize(0, cBattle.NumShots());

	//A bolt stops at the planet.
	psShip->fEnergy = cKestrel.mfEnergy;
	TestBattlePut(&cBattle, 0, 150.0f, 200.0f, BATTLE_PI * 0.5f);
	cBattle.Step(TEST_BATTLE_STEP);
	sControls.bPrimary = false;
	cBattle.SetControls(0, &sControls);
	AssertSize(1, cBattle.NumShots());
	TestBattleRun(&cBattle, 0.4f);
	AssertSize(0, cBattle.NumShots());

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A seeker turns towards the other ship and hits it though it wasn't
// fired at it; a bolt that meets one destroys it.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleSeekers(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cOther;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleShip*	psTarget;
	SBattleShot*	psShot;
	SBattleControls	sControls;
	float32			fHeading;

	//Two Kestrels, so the second has bolts to shoot a seeker down with.
	BattleShipKestrel(&cKestrel);
	BattleShipKestrel(&cOther);
	cBattle.Init(&cKestrel, &cOther);

	//Fired up the screen, with the other ship off to the right.
	psShip = TestBattlePut(&cBattle, 0, 20.0f, 10.0f, 0.0f);
	psTarget = TestBattlePut(&cBattle, 1, 80.0f, 40.0f, 0.0f);
	sControls.Init();
	sControls.bSecondary = true;
	cBattle.SetControls(0, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	sControls.bSecondary = false;
	cBattle.SetControls(0, &sControls);

	AssertSize(1, cBattle.NumShots());
	psShot = cBattle.GetShot(0);
	AssertInt(BW_Seeker, psShot->eKind);
	AssertFloat(cKestrel.mfEnergy - cKestrel.msSecondary.fEnergy, psShip->fEnergy, 1);
	fHeading = psShot->fHeading;
	TestBattleRun(&cBattle, 0.25f);
	psShot = cBattle.GetShot(0);
	AssertTrue(psShot->fHeading > fHeading + 0.3f);  //Turning right, towards it.
	AssertFloat(cKestrel.msSecondary.fSpeed, psShot->sVelocity.Magnitude(), 2);

	TestBattleRun(&cBattle, 3.0f);
	AssertSize(0, cBattle.NumShots());
	AssertFloat(cOther.mfHull - cKestrel.msSecondary.fDamage, psTarget->fHull, 4);

	//Fired straight at the other ship, which shoots back along the same line: the seeker never arrives.
	TestBattlePut(&cBattle, 0, 20.0f, 10.0f, BATTLE_PI * 0.5f);
	TestBattlePut(&cBattle, 1, 110.0f, 10.0f, BATTLE_PI * 1.5f);
	psShip->fEnergy = cKestrel.mfEnergy;
	psShip->fSecondaryWait = 0.0f;
	cOther.msPrimary.fSpread = 0.0f;
	cOther.msPrimary.fDamage = 0.0f;  //So the bolts that miss the seeker don't count.
	sControls.bSecondary = true;
	cBattle.SetControls(0, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	sControls.bSecondary = false;
	cBattle.SetControls(0, &sControls);
	sControls.bPrimary = true;
	cBattle.SetControls(1, &sControls);
	TestBattleRun(&cBattle, 2.5f);
	AssertFloat(cOther.mfHull - cKestrel.msSecondary.fDamage, psTarget->fHull, 4);  //Only the first one hit.

	cBattle.Kill();
	cOther.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A beam is on while it's held and there's energy for it.  It hurts the
// other ship while it's on it, reaches no further than its range, and
// stops at the planet.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleBeam(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleShip*	psTarget;
	SBattleControls	sControls;
	float32			fReach;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBulwark.mfRecharge = 0.0f;
	cBattle.Init(&cKestrel, &cBulwark);

	//The Bulwark faces right, with the Kestrel in front of it, in reach.
	psShip = TestBattlePut(&cBattle, 1, 20.0f, 10.0f, BATTLE_PI * 0.5f);
	psTarget = TestBattlePut(&cBattle, 0, 50.0f, 10.0f, 0.0f);
	sControls.Init();
	cBattle.SetControls(1, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	AssertFalse(psShip->bBeam);

	sControls.bPrimary = true;
	cBattle.SetControls(1, &sControls);
	TestBattleRun(&cBattle, 0.5f);
	AssertTrue(psShip->bBeam);
	fReach = 50.0f - 3.0f - (20.0f + cBulwark.msPrimary.fMuzzle);  //To the Kestrel's left wing tip, from the muzzle.
	AssertTrue(psShip->fBeamLength < fReach + 1.5f);
	AssertTrue(psShip->fBeamLength > fReach - 1.5f);
	AssertFloat(cKestrel.mfHull - cBulwark.msPrimary.fDamage * 0.5f, psTarget->fHull, 1);
	AssertFloat(cBulwark.mfEnergy - cBulwark.msPrimary.fEnergy * 0.5f, psShip->fEnergy, 1);
	AssertSize(0, cBattle.NumShots());

	//Out of reach it's on at its full length and hurts nothing.
	psTarget = TestBattlePut(&cBattle, 0, 80.0f, 10.0f, 0.0f);
	psTarget->fHull = cKestrel.mfHull;
	TestBattleRun(&cBattle, 0.25f);
	AssertTrue(psShip->bBeam);
	AssertFloat(cBulwark.msPrimary.fRange, psShip->fBeamLength, 3);
	AssertFloat(cKestrel.mfHull, psTarget->fHull, 4);

	//Beside it, not in front, it misses.
	psTarget = TestBattlePut(&cBattle, 0, 40.0f, 25.0f, 0.0f);
	TestBattleRun(&cBattle, 0.25f);
	AssertFloat(cKestrel.mfHull, psTarget->fHull, 4);

	//Let go, it's off.
	sControls.bPrimary = false;
	cBattle.SetControls(1, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	AssertFalse(psShip->bBeam);

	//It's off when the energy is gone.
	psShip->fEnergy = 0.01f;
	sControls.bPrimary = true;
	cBattle.SetControls(1, &sControls);
	TestBattleRun(&cBattle, 0.1f);
	AssertFalse(psShip->bBeam);

	//The planet stops it: the Kestrel behind the planet isn't touched.
	psShip = TestBattlePut(&cBattle, 1, 200.0f - 14.0f - 20.0f, 200.0f, BATTLE_PI * 0.5f);
	psShip->fEnergy = cBulwark.mfEnergy;
	psTarget = TestBattlePut(&cBattle, 0, 200.0f + 14.0f + 4.0f, 200.0f, BATTLE_PI * 0.5f);
	cBulwark.msPrimary.fRange = 80.0f;
	cBattle.Step(TEST_BATTLE_STEP);
	AssertTrue(psShip->bBeam);
	AssertTrue(psShip->fBeamLength < 20.0f - cBulwark.msPrimary.fMuzzle + 0.5f);
	AssertFloat(cKestrel.mfHull, psTarget->fHull, 4);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A burst destroys the other ship's shots within its range and hurts the
// other ship if it reaches it; its own ship's shots, and what's out of
// range, are left alone.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleBurst(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleShip*	psTarget;
	SBattleShot*	psShot;
	SBattleControls	sControls;
	size			i;
	float32			afAcross[4] = { 30.0f, 12.0f, 40.0f, 28.0f };
	int32			aiOwners[4] = { 0, 0, 0, 1 };

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);

	//The Bulwark with the Kestrel well out of reach, and shots round it: two of the Kestrel's in reach, one out, and one of its own.
	psShip = TestBattlePut(&cBattle, 1, 20.0f, 10.0f, 0.0f);
	psTarget = TestBattlePut(&cBattle, 0, 20.0f, 100.0f, 0.0f);
	for (i = 0; i < 4; i++)
	{
		psShot = cBattle.AddShot();
		psShot->eKind = BW_Bolt;
		psShot->iOwner = aiOwners[i];
		psShot->sPosition.Init(afAcross[i], 10.0f);
		psShot->fLife = 10.0f;
		psShot->fRadius = 0.5f;
	}

	sControls.Init();
	sControls.bSecondary = true;
	cBattle.SetControls(1, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);
	sControls.bSecondary = false;
	cBattle.SetControls(1, &sControls);
	cBattle.Step(TEST_BATTLE_STEP);

	AssertSize(2, cBattle.NumShots());
	AssertFloat(40.0f, cBattle.GetShot(0)->sPosition.x, 3);
	AssertInt(0, cBattle.GetShot(0)->iOwner);
	AssertFloat(28.0f, cBattle.GetShot(1)->sPosition.x, 3);
	AssertInt(1, cBattle.GetShot(1)->iOwner);
	AssertFloat(cKestrel.mfHull, psTarget->fHull, 4);
	AssertFloat(cBulwark.mfEnergy - cBulwark.msSecondary.fEnergy, psShip->fEnergy, 1);
	AssertTrue(cBattle.NumEffects() >= 1);
	AssertInt(BE_Burst, cBattle.GetEffect(0)->eKind);
	AssertFloat(cBulwark.msSecondary.fRange, cBattle.GetEffect(0)->fRadius, 3);

	//Held, it doesn't go off again until its cooldown is up.
	sControls.bSecondary = true;
	cBattle.SetControls(1, &sControls);
	psTarget = TestBattlePut(&cBattle, 0, 20.0f, 10.0f + cBulwark.msSecondary.fRange + 2.0f, 0.0f);
	TestBattleRun(&cBattle, cBulwark.msSecondary.fCooldown * 0.5f);
	AssertFloat(cKestrel.mfHull, psTarget->fHull, 4);

	//When it does, the Kestrel's tail is in reach though its middle isn't.
	TestBattleRun(&cBattle, cBulwark.msSecondary.fCooldown * 0.5f + 0.05f);
	AssertFloat(cKestrel.mfHull - cBulwark.msSecondary.fDamage, psTarget->fHull, 4);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Ships that meet are pushed apart, the lighter one further, and bounce:
// neither is hurt, and afterwards their outlines don't overlap.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleShipsCollide(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psLight;
	SBattleShip*	psHeavy;
	SFloat32Vec2	asLight[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	asHeavy[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	sPush;
	float32			fMomentum;
	int32			i;
	bool			bTouched;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);

	//The Kestrel flies right, into the Bulwark's side.
	psLight = TestBattlePut(&cBattle, 0, 20.0f, 10.0f, BATTLE_PI * 0.5f);
	psHeavy = TestBattlePut(&cBattle, 1, 50.0f, 10.0f, 0.0f);
	psLight->sVelocity.Init(40.0f, 0.0f);
	fMomentum = cKestrel.mfMass * 40.0f;

	bTouched = false;
	for (i = 0; i < 240; i++)
	{
		cBattle.Step(TEST_BATTLE_STEP);
		cBattle.GetOutline(0, NULL, asLight);
		cBattle.GetOutline(1, &psLight->sPosition, asHeavy);
		if (BattleConvexOverlap(asLight, cKestrel.muiNumOutline, asHeavy, cBulwark.muiNumOutline, &sPush))
		{
			//Only by what a step's rounding leaves.
			AssertTrue(sPush.Magnitude() < 0.01f);
		}
		bTouched = bTouched || (psHeavy->sVelocity.x > 1.0f);
	}
	AssertTrue(bTouched);
	AssertTrue(psLight->sVelocity.x < 0.0f);  //Thrown back.
	AssertTrue(psHeavy->sVelocity.x > 0.0f);  //Pushed on.
	AssertTrue(psHeavy->sVelocity.x < 40.0f);

	//What they have between them is what the Kestrel arrived with, but for the planet's pull meanwhile.
	AssertTrue(fabsf(cKestrel.mfMass * psLight->sVelocity.x + cBulwark.mfMass * psHeavy->sVelocity.x - fMomentum) < 3.0f);
	AssertFloat(cKestrel.mfHull, psLight->fHull, 4);
	AssertFloat(cBulwark.mfHull, psHeavy->fHull, 4);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A ship whose hull is gone is destroyed, and the other has won: it stays
// won, and what's left flies on.  Both gone together is a draw.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleWinning(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cBattle;
	SBattleShip*	psShip;
	SBattleShip*	psTarget;
	SBattleShot*	psShot;
	SBattleControls	sControls;
	float32			fAcross;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cBattle.Init(&cKestrel, &cBulwark);

	psShip = TestBattlePut(&cBattle, 0, 20.0f, 10.0f, BATTLE_PI * 0.5f);
	psTarget = TestBattlePut(&cBattle, 1, 60.0f, 10.0f, 0.0f);
	psTarget->fHull = 2.5f;
	sControls.Init();
	sControls.bPrimary = true;
	cBattle.SetControls(0, &sControls);
	cBattle.SetControls(1, &sControls);
	TestBattleRun(&cBattle, 1.0f);

	AssertFalse(psTarget->bAlive);
	AssertFloat(0.0f, psTarget->fHull, 4);
	AssertFalse(psTarget->bBeam);
	AssertTrue(cBattle.IsOver());
	AssertInt(0, cBattle.GetWinner());
	AssertTrue(psShip->bAlive);

	//Its bolts now fly through where the other was, and it still flies.
	sControls.bPrimary = false;
	sControls.bThrust = true;
	cBattle.SetControls(0, &sControls);
	fAcross = psShip->sPosition.x;
	TestBattleRun(&cBattle, 1.0f);
	AssertSize(0, cBattle.NumShots());
	AssertTrue(psShip->sPosition.x > fAcross + 10.0f);
	AssertInt(0, cBattle.GetWinner());

	//Destroyed afterwards, by a seeker left over say, the battle is still its.
	psShip->fHull = 0.5f;
	psShot = cBattle.AddShot();
	psShot->eKind = BW_Bolt;
	psShot->iOwner = 1;
	psShot->sPosition = psShip->sPosition;
	psShot->fLife = 1.0f;
	psShot->fDamage = 1.0f;
	psShot->fRadius = 0.5f;
	cBattle.Step(TEST_BATTLE_STEP);
	AssertFalse(psShip->bAlive);
	AssertInt(0, cBattle.GetWinner());

	//Both at once.
	cBattle.Start();
	cBattle.GetShip(0)->fHull = 0.5f;
	cBattle.GetShip(1)->fHull = 0.5f;
	psShot = cBattle.AddShot();
	psShot->eKind = BW_Bolt;
	psShot->iOwner = 1;
	psShot->sPosition = cBattle.GetShip(0)->sPosition;
	psShot->fLife = 1.0f;
	psShot->fDamage = 1.0f;
	psShot->fRadius = 0.5f;
	psShot = cBattle.AddShot();
	psShot->eKind = BW_Bolt;
	psShot->iOwner = 0;
	psShot->sPosition = cBattle.GetShip(1)->sPosition;
	psShot->fLife = 1.0f;
	psShot->fDamage = 1.0f;
	psShot->fRadius = 0.5f;
	cBattle.Step(TEST_BATTLE_STEP);
	AssertInt(BATTLE_DRAWN, cBattle.GetWinner());
	AssertSize(4, cBattle.NumEffects());  //Each has the hit and its explosion.
	AssertInt(BE_Hit, cBattle.GetEffect(0)->eKind);
	AssertInt(BE_Explosion, cBattle.GetEffect(1)->eKind);
	AssertInt(BE_Explosion, cBattle.GetEffect(3)->eKind);

	cBattle.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Advance moves the battle on in steps of its own, so it ends up in the
// same place whether the time comes in one piece or many.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleAdvance(void)
{
	CBattleShipType	cKestrel;
	CBattleShipType	cBulwark;
	CBattle			cOnce;
	CBattle			cMany;
	SBattleControls	sControls;
	int32			i;

	BattleShipKestrel(&cKestrel);
	BattleShipBulwark(&cBulwark);
	cOnce.Init(&cKestrel, &cBulwark);
	cMany.Init(&cKestrel, &cBulwark);

	sControls.Init();
	sControls.bThrust = true;
	sControls.bRight = true;
	sControls.bPrimary = true;
	cOnce.SetControls(0, &sControls);
	cMany.SetControls(0, &sControls);

	for (i = 0; i < 4; i++)
	{
		cOnce.Advance(0.125f);
	}
	for (i = 0; i < 40; i++)
	{
		cMany.Advance(0.0125f);
	}
	AssertFloat(0.5f, cOnce.GetSeconds(), 3);
	AssertFloat(cOnce.GetSeconds(), cMany.GetSeconds(), 5);
	AssertFloat(cOnce.GetShip(0)->sPosition.x, cMany.GetShip(0)->sPosition.x, 5);
	AssertFloat(cOnce.GetShip(0)->sPosition.y, cMany.GetShip(0)->sPosition.y, 5);
	AssertFloat(cOnce.GetShip(0)->fHeading, cMany.GetShip(0)->fHeading, 5);
	AssertSize(cOnce.NumShots(), cMany.NumShots());

	//A long wait isn't caught up on.
	cOnce.Advance(60.0f);
	AssertTrue(cOnce.GetSeconds() < 1.0f);

	cMany.Kill();
	cOnce.Kill();
	cBulwark.Kill();
	cKestrel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBattle(void)
{
	BeginTests();

	TestBattleStart();
	TestBattleFlight();
	TestBattleComesRound();
	TestBattlePlanet();
	TestBattleBolts();
	TestBattleSeekers();
	TestBattleBeam();
	TestBattleBurst();
	TestBattleShipsCollide();
	TestBattleWinning();
	TestBattleAdvance();

	TestStatistics();
}

