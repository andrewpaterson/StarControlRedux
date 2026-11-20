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
#include <math.h>
#include <string.h>
#include "BattleGeometry.h"
#include "Battle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SBattleControls::Init(void)
{
	bLeft = false;
	bRight = false;
	bThrust = false;
	bPrimary = false;
	bSecondary = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A battle between a ship of each type, started.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Init(CBattleShipType* pcFirst, CBattleShipType* pcSecond)
{
	memset(masShips, 0, sizeof(masShips));
	masShips[0].pcType = pcFirst;
	masShips[1].pcType = pcSecond;
	masShots.Init();
	masEffects.Init();
	mfSize = BATTLE_ARENA_SIZE;
	msPlanet.Init(mfSize * 0.5f, mfSize * 0.5f);
	mfPlanetRadius = BATTLE_PLANET_RADIUS;
	Start();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Kill(void)
{
	masEffects.Kill();
	masShots.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// From the beginning again: the ships whole and still, on either side of
// the planet and facing opposite ways along it, with nothing in flight.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Start(void)
{
	CBattleShipType*	pcType;
	SBattleShip*		psShip;
	int32				i;

	for (i = 0; i < BATTLE_SHIPS; i++)
	{
		psShip = &masShips[i];
		pcType = psShip->pcType;
		memset(psShip, 0, sizeof(SBattleShip));
		psShip->pcType = pcType;
		psShip->sControls.Init();
		psShip->sPosition.Init(mfSize * ((i == 0) ? BATTLE_START_FIRST : BATTLE_START_SECOND), mfSize * 0.5f);
		psShip->sVelocity.Init(0.0f, 0.0f);
		psShip->fHeading = (i == 0) ? 0.0f : BATTLE_PI;
		psShip->fHull = pcType->mfHull;
		psShip->fEnergy = pcType->mfEnergy;
		psShip->bAlive = true;
	}
	masShots.ReInit();
	masEffects.ReInit();
	mfSeconds = 0.0f;
	mfUnstepped = 0.0f;
	miWinner = BATTLE_NO_SHIP;
}


//////////////////////////////////////////////////////////////////////////
//
// What the ship's pilot is doing, until it's set again.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::SetControls(int32 iShip, SBattleControls* psControls)
{
	masShips[iShip].sControls = *psControls;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the battle on by the time that's passed, in steps of BATTLE_STEP
// so that it goes the same way however the time arrives.  What's left
// over is kept for the next call, and a long wait is cut short rather
// than caught up on.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Advance(float32 fSeconds)
{
	mfUnstepped += fSeconds;
	if (mfUnstepped > 0.25f)
	{
		mfUnstepped = 0.25f;
	}

	//A step is taken when all but a rounding's worth of it has arrived, so that the same time in pieces takes as many.
	while (mfUnstepped >= BATTLE_STEP * 0.999f)
	{
		Step(BATTLE_STEP);
		mfUnstepped = (mfUnstepped > BATTLE_STEP) ? mfUnstepped - BATTLE_STEP : 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Everything moved on by fSeconds, which should be short: nothing is
// looked for between where a thing was and where it ends up.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Step(float32 fSeconds)
{
	int32	i;

	for (i = 0; i < BATTLE_SHIPS; i++)
	{
		if (masShips[i].bAlive)
		{
			StepShip(i, fSeconds);
		}
	}
	for (i = 0; i < BATTLE_SHIPS; i++)
	{
		if (masShips[i].bAlive)
		{
			CollidePlanet(i);
		}
	}
	CollideShips();
	StepShots(fSeconds);
	StepEffects(fSeconds);
	mfSeconds += fSeconds;
	FindWinner();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattle::StepShip(int32 iShip, float32 fSeconds)
{
	SBattleShip*		psShip;
	CBattleShipType*	pcType;
	SFloat32Vec2		sForward;
	SFloat32Vec2		sToPlanet;
	float32				fWanted;
	float32				fChange;
	float32				fBefore;
	float32				fAfter;
	float32				fLimit;
	float32				fDistance;
	float32				fPull;

	psShip = &masShips[iShip];
	pcType = psShip->pcType;

	psShip->fEnergy += pcType->mfRecharge * fSeconds;
	if (psShip->fEnergy > pcType->mfEnergy)
	{
		psShip->fEnergy = pcType->mfEnergy;
	}
	psShip->fPrimaryWait = (psShip->fPrimaryWait > fSeconds) ? psShip->fPrimaryWait - fSeconds : 0.0f;
	psShip->fSecondaryWait = (psShip->fSecondaryWait > fSeconds) ? psShip->fSecondaryWait - fSeconds : 0.0f;
	psShip->fPlanetWait = (psShip->fPlanetWait > fSeconds) ? psShip->fPlanetWait - fSeconds : 0.0f;

	//Its turning speeds up towards what's asked for, and slows to nothing when nothing is.
	fWanted = 0.0f;
	if (psShip->sControls.bLeft)
	{
		fWanted -= pcType->mfTurnSpeed;
	}
	if (psShip->sControls.bRight)
	{
		fWanted += pcType->mfTurnSpeed;
	}
	fChange = pcType->mfTurnAcceleration * fSeconds;
	if (psShip->fTurning < fWanted)
	{
		psShip->fTurning = (psShip->fTurning + fChange < fWanted) ? psShip->fTurning + fChange : fWanted;
	}
	else if (psShip->fTurning > fWanted)
	{
		psShip->fTurning = (psShip->fTurning - fChange > fWanted) ? psShip->fTurning - fChange : fWanted;
	}
	psShip->fHeading = BattleWrapAngle(psShip->fHeading + psShip->fTurning * fSeconds);
	BattleForward(&sForward, psShip->fHeading);

	//Thrust takes it no faster than its top speed, and never slows what's already faster.
	psShip->bThrusting = psShip->sControls.bThrust;
	if (psShip->bThrusting)
	{
		fBefore = psShip->sVelocity.Magnitude();
		psShip->sVelocity.x += sForward.x * pcType->mfThrust * fSeconds;
		psShip->sVelocity.y += sForward.y * pcType->mfThrust * fSeconds;
		fAfter = psShip->sVelocity.Magnitude();
		fLimit = (fBefore > pcType->mfMaxSpeed) ? fBefore : pcType->mfMaxSpeed;
		if (fAfter > fLimit)
		{
			psShip->sVelocity.x *= fLimit / fAfter;
			psShip->sVelocity.y *= fLimit / fAfter;
		}
	}

	Delta(&sToPlanet, &psShip->sPosition, &msPlanet);
	fDistance = sToPlanet.Magnitude();
	if (fDistance > 0.0f)
	{
		fPull = BATTLE_PLANET_GRAVITY / (fDistance * fDistance);
		if (fPull > BATTLE_PLANET_MAX_GRAVITY)
		{
			fPull = BATTLE_PLANET_MAX_GRAVITY;
		}
		psShip->sVelocity.x += sToPlanet.x / fDistance * fPull * fSeconds;
		psShip->sVelocity.y += sToPlanet.y / fDistance * fPull * fSeconds;
	}

	psShip->sPosition.x += psShip->sVelocity.x * fSeconds;
	psShip->sPosition.y += psShip->sVelocity.y * fSeconds;
	Wrap(&psShip->sPosition);

	psShip->bBeam = false;
	psShip->fBeamLength = 0.0f;
	Fire(iShip, &pcType->msPrimary, &psShip->fPrimaryWait, psShip->sControls.bPrimary, fSeconds);
	Fire(iShip, &pcType->msSecondary, &psShip->fSecondaryWait, psShip->sControls.bSecondary, fSeconds);
}


//////////////////////////////////////////////////////////////////////////
//
// The weapon fires if its control is held, it's ready and the ship has
// the energy for it.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Fire(int32 iShip, SBattleWeapon* psWeapon, float32* pfWait, bool bHeld, float32 fSeconds)
{
	SBattleShip*	psShip;
	float32			fCost;

	psShip = &masShips[iShip];
	if (!bHeld || (psWeapon->eKind == BW_None))
	{
		return;
	}

	if (psWeapon->eKind == BW_Beam)
	{
		fCost = psWeapon->fEnergy * fSeconds;
		if (psShip->fEnergy >= fCost)
		{
			psShip->fEnergy -= fCost;
			FireBeam(iShip, psWeapon, fSeconds);
		}
		return;
	}

	if ((*pfWait > 0.0f) || (psShip->fEnergy < psWeapon->fEnergy))
	{
		return;
	}
	psShip->fEnergy -= psWeapon->fEnergy;
	*pfWait = psWeapon->fCooldown;
	if (psWeapon->eKind == BW_Burst)
	{
		FireBurst(iShip, psWeapon);
	}
	else
	{
		FireShot(iShip, psWeapon);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where the weapon's shot or beam starts: in front of the ship's middle,
// and for a weapon with a spread to the side the next bolt comes from.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::GetMuzzle(int32 iShip, SBattleWeapon* psWeapon, SFloat32Vec2* psDest)
{
	SBattleShip*	psShip;
	SFloat32Vec2	sForward;
	SFloat32Vec2	sRight;
	float32			fSide;

	psShip = &masShips[iShip];
	BattleForward(&sForward, psShip->fHeading);
	BattleRight(&sRight, psShip->fHeading);
	fSide = psShip->bLeftBolt ? -psWeapon->fSpread : psWeapon->fSpread;
	psDest->x = psShip->sPosition.x + sForward.x * psWeapon->fMuzzle + sRight.x * fSide;
	psDest->y = psShip->sPosition.y + sForward.y * psWeapon->fMuzzle + sRight.y * fSide;
}


//////////////////////////////////////////////////////////////////////////
//
// A bolt leaves with the ship's own speed added to its own.  A seeker
// flies at its own speed whatever the ship was doing.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::FireShot(int32 iShip, SBattleWeapon* psWeapon)
{
	SBattleShip*	psShip;
	SBattleShot*	psShot;
	SFloat32Vec2	sForward;

	psShip = &masShips[iShip];
	BattleForward(&sForward, psShip->fHeading);

	psShot = AddShot();
	psShot->eKind = psWeapon->eKind;
	psShot->iOwner = iShip;
	GetMuzzle(iShip, psWeapon, &psShot->sPosition);
	Wrap(&psShot->sPosition);
	psShot->sVelocity.x = sForward.x * psWeapon->fSpeed;
	psShot->sVelocity.y = sForward.y * psWeapon->fSpeed;
	if (psWeapon->eKind == BW_Bolt)
	{
		psShot->sVelocity.x += psShip->sVelocity.x;
		psShot->sVelocity.y += psShip->sVelocity.y;
	}
	psShot->fHeading = psShip->fHeading;
	psShot->fSpeed = psWeapon->fSpeed;
	psShot->fTurn = psWeapon->fTurn;
	psShot->fLife = psWeapon->fLife;
	psShot->fDamage = psWeapon->fDamage;
	psShot->fRadius = psWeapon->fRadius;

	psShip->bLeftBolt = !psShip->bLeftBolt;
}


//////////////////////////////////////////////////////////////////////////
//
// The beam reaches from the muzzle to the first thing in its way: the
// planet, which stops it; the other ship, which it hurts for as long as
// it's on it; or one of the other ship's seekers, which it destroys.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::FireBeam(int32 iShip, SBattleWeapon* psWeapon, float32 fSeconds)
{
	SBattleShip*	psShip;
	SBattleShip*	psOther;
	SBattleShot*	psShot;
	SBattleShot*	psHitShot;
	SFloat32Vec2	asOutline[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	sForward;
	SFloat32Vec2	sStart;
	SFloat32Vec2	sEnd;
	SFloat32Vec2	sDelta;
	SFloat32Vec2	sCentre;
	float32			fNearest;
	float32			fEnter;
	int32			iOther;
	bool			bHitShip;
	size			i;

	psShip = &masShips[iShip];
	iOther = 1 - iShip;
	psOther = &masShips[iOther];
	BattleForward(&sForward, psShip->fHeading);
	GetMuzzle(iShip, psWeapon, &sStart);
	sEnd.x = sStart.x + sForward.x * psWeapon->fRange;
	sEnd.y = sStart.y + sForward.y * psWeapon->fRange;

	fNearest = 1.0f;
	bHitShip = false;
	psHitShot = NULL;

	Delta(&sDelta, &sStart, &msPlanet);
	sCentre.x = sStart.x + sDelta.x;
	sCentre.y = sStart.y + sDelta.y;
	if (BattleCircleSegment(&sCentre, mfPlanetRadius, &sStart, &sEnd, &fEnter) && (fEnter < fNearest))
	{
		fNearest = fEnter;
	}

	if (psOther->bAlive)
	{
		GetOutline(iOther, &sStart, asOutline);
		if (BattleConvexSegment(asOutline, psOther->pcType->muiNumOutline, &sStart, &sEnd, &fEnter) && (fEnter < fNearest))
		{
			fNearest = fEnter;
			bHitShip = true;
		}
	}

	for (i = 0; i < masShots.NumElements(); i++)
	{
		psShot = masShots.Get(i);
		if ((psShot->eKind == BW_Seeker) && (psShot->iOwner != iShip) && (psShot->fLife > 0.0f))
		{
			Delta(&sDelta, &sStart, &psShot->sPosition);
			sCentre.x = sStart.x + sDelta.x;
			sCentre.y = sStart.y + sDelta.y;
			if (BattleCircleSegment(&sCentre, psShot->fRadius, &sStart, &sEnd, &fEnter) && (fEnter < fNearest))
			{
				fNearest = fEnter;
				bHitShip = false;
				psHitShot = psShot;
			}
		}
	}

	if (psHitShot != NULL)
	{
		//Gone when the shots are next stepped.
		psHitShot->fLife = 0.0f;
		AddEffect(BE_Hit, &psHitShot->sPosition, psHitShot->fRadius * 3.0f, BATTLE_HIT_SECONDS);
	}
	else if (bHitShip)
	{
		Damage(iOther, psWeapon->fDamage * fSeconds);
	}
	psShip->bBeam = true;
	psShip->fBeamLength = psWeapon->fRange * fNearest;
}


//////////////////////////////////////////////////////////////////////////
//
// Everything of the other ship's within the burst's range of this ship's
// middle: its shots are destroyed, and it's hurt if any of it is there.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::FireBurst(int32 iShip, SBattleWeapon* psWeapon)
{
	SBattleShip*	psShip;
	SBattleShip*	psOther;
	SBattleShot*	psShot;
	SFloat32Vec2	asOutline[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	sDelta;
	int32			iOther;
	size			i;

	psShip = &masShips[iShip];
	iOther = 1 - iShip;
	psOther = &masShips[iOther];
	AddEffect(BE_Burst, &psShip->sPosition, psWeapon->fRange, BATTLE_BURST_SECONDS);

	for (i = 0; i < masShots.NumElements(); i++)
	{
		psShot = masShots.Get(i);
		if ((psShot->iOwner != iShip) && (psShot->fLife > 0.0f))
		{
			Delta(&sDelta, &psShip->sPosition, &psShot->sPosition);
			if (sDelta.Magnitude() <= psWeapon->fRange + psShot->fRadius)
			{
				//Gone when the shots are next stepped.
				psShot->fLife = 0.0f;
				AddEffect(BE_Hit, &psShot->sPosition, psShot->fRadius * 3.0f, BATTLE_HIT_SECONDS);
			}
		}
	}

	if (psOther->bAlive)
	{
		GetOutline(iOther, &psShip->sPosition, asOutline);
		if (BattleConvexDistance(asOutline, psOther->pcType->muiNumOutline, &psShip->sPosition) <= psWeapon->fRange)
		{
			Damage(iOther, psWeapon->fDamage);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A seeker turns towards the other ship, as fast as it can, while that
// ship is there.  Every shot then moves and is gone if it hit something
// or has flown as long as it does.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::StepShots(float32 fSeconds)
{
	SBattleShot*	psShot;
	SBattleShip*	psTarget;
	SFloat32Vec2	sDelta;
	SFloat32Vec2	sForward;
	float32			fTurn;
	float32			fMost;
	size			i;

	i = masShots.NumElements();
	while (i > 0)
	{
		i--;
		psShot = masShots.Get(i);
		psShot->fLife -= fSeconds;
		if (psShot->fLife <= 0.0f)
		{
			masShots.RemoveAt(i);
			continue;
		}

		if (psShot->eKind == BW_Seeker)
		{
			psTarget = &masShips[1 - psShot->iOwner];
			if (psTarget->bAlive)
			{
				Delta(&sDelta, &psShot->sPosition, &psTarget->sPosition);
				fTurn = BattleAngleBetween(psShot->fHeading, BattleHeading(&sDelta));
				fMost = psShot->fTurn * fSeconds;
				fTurn = (fTurn > fMost) ? fMost : ((fTurn < -fMost) ? -fMost : fTurn);
				psShot->fHeading = BattleWrapAngle(psShot->fHeading + fTurn);
			}
			BattleForward(&sForward, psShot->fHeading);
			psShot->sVelocity.x = sForward.x * psShot->fSpeed;
			psShot->sVelocity.y = sForward.y * psShot->fSpeed;
		}

		psShot->sPosition.x += psShot->sVelocity.x * fSeconds;
		psShot->sPosition.y += psShot->sVelocity.y * fSeconds;
		Wrap(&psShot->sPosition);
		if (ShotHits(psShot))
		{
			masShots.RemoveAt(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the shot has hit something, which is done to it here: the
// planet, the ship that didn't fire it, which it hurts, or, for a bolt,
// one of that ship's seekers, which goes with it.
//
//////////////////////////////////////////////////////////////////////////
bool CBattle::ShotHits(SBattleShot* psShot)
{
	SBattleShip*	psShip;
	SBattleShot*	psOther;
	SFloat32Vec2	asOutline[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	sDelta;
	int32			iShip;
	size			i;

	Delta(&sDelta, &psShot->sPosition, &msPlanet);
	if (sDelta.Magnitude() <= mfPlanetRadius + psShot->fRadius)
	{
		AddEffect(BE_Hit, &psShot->sPosition, psShot->fRadius * 3.0f, BATTLE_HIT_SECONDS);
		return true;
	}

	iShip = 1 - psShot->iOwner;
	psShip = &masShips[iShip];
	if (psShip->bAlive)
	{
		Delta(&sDelta, &psShot->sPosition, &psShip->sPosition);
		if (sDelta.Magnitude() <= psShip->pcType->mfRadius + psShot->fRadius)
		{
			GetOutline(iShip, &psShot->sPosition, asOutline);
			if (BattleConvexDistance(asOutline, psShip->pcType->muiNumOutline, &psShot->sPosition) <= psShot->fRadius)
			{
				AddEffect(BE_Hit, &psShot->sPosition, psShot->fRadius * 3.0f, BATTLE_HIT_SECONDS);
				Damage(iShip, psShot->fDamage);
				return true;
			}
		}
	}

	if (psShot->eKind == BW_Bolt)
	{
		for (i = 0; i < masShots.NumElements(); i++)
		{
			psOther = masShots.Get(i);
			if ((psOther->eKind == BW_Seeker) && (psOther->iOwner != psShot->iOwner) && (psOther->fLife > 0.0f))
			{
				Delta(&sDelta, &psShot->sPosition, &psOther->sPosition);
				if (sDelta.Magnitude() <= psShot->fRadius + psOther->fRadius)
				{
					//The seeker is gone when it's next stepped.
					psOther->fLife = 0.0f;
					AddEffect(BE_Hit, &psOther->sPosition, psOther->fRadius * 3.0f, BATTLE_HIT_SECONDS);
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattle::StepEffects(float32 fSeconds)
{
	SBattleEffect*	psEffect;
	size			i;

	i = masEffects.NumElements();
	while (i > 0)
	{
		i--;
		psEffect = masEffects.Get(i);
		psEffect->fAge += fSeconds;
		if (psEffect->fAge >= psEffect->fLife)
		{
			masEffects.RemoveAt(i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A ship whose outline is in the planet is put back outside it and
// leaves with some of the speed it arrived with.  If that was fast enough
// it's hurt, though not again for a moment.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::CollidePlanet(int32 iShip)
{
	SBattleShip*	psShip;
	SFloat32Vec2	asOutline[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	sDelta;
	SFloat32Vec2	sPlanet;
	SFloat32Vec2	sAway;
	float32			fDistance;
	float32			fLength;
	float32			fInto;

	psShip = &masShips[iShip];
	Delta(&sDelta, &psShip->sPosition, &msPlanet);
	fLength = sDelta.Magnitude();
	if (fLength > mfPlanetRadius + psShip->pcType->mfRadius)
	{
		return;
	}

	sPlanet.x = psShip->sPosition.x + sDelta.x;
	sPlanet.y = psShip->sPosition.y + sDelta.y;
	GetOutline(iShip, NULL, asOutline);
	fDistance = BattleConvexDistance(asOutline, psShip->pcType->muiNumOutline, &sPlanet);
	if (fDistance >= mfPlanetRadius)
	{
		return;
	}

	sAway.Init(0.0f, 1.0f);
	if (fLength > 0.0f)
	{
		sAway.x = -sDelta.x / fLength;
		sAway.y = -sDelta.y / fLength;
	}
	psShip->sPosition.x += sAway.x * (mfPlanetRadius - fDistance);
	psShip->sPosition.y += sAway.y * (mfPlanetRadius - fDistance);
	Wrap(&psShip->sPosition);

	fInto = psShip->sVelocity.x * sAway.x + psShip->sVelocity.y * sAway.y;
	if (fInto < 0.0f)
	{
		psShip->sVelocity.x -= sAway.x * fInto * (1.0f + BATTLE_BOUNCE);
		psShip->sVelocity.y -= sAway.y * fInto * (1.0f + BATTLE_BOUNCE);
	}
	if ((fInto <= -BATTLE_PLANET_HURT_SPEED) && (psShip->fPlanetWait <= 0.0f))
	{
		psShip->fPlanetWait = BATTLE_PLANET_HURT_SECONDS;
		Damage(iShip, BATTLE_PLANET_DAMAGE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ships whose outlines overlap are moved apart, the lighter further, and
// if they were closing they bounce off each other.  Neither is hurt.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::CollideShips(void)
{
	SBattleShip*	psFirst;
	SBattleShip*	psSecond;
	SFloat32Vec2	asFirst[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	asSecond[BATTLE_MAX_OUTLINE_CORNERS];
	SFloat32Vec2	sDelta;
	SFloat32Vec2	sPush;
	float32			fLength;
	float32			fFirstShare;
	float32			fClosing;
	float32			fImpulse;

	psFirst = &masShips[0];
	psSecond = &masShips[1];
	if (!psFirst->bAlive || !psSecond->bAlive)
	{
		return;
	}
	Delta(&sDelta, &psFirst->sPosition, &psSecond->sPosition);
	if (sDelta.Magnitude() > psFirst->pcType->mfRadius + psSecond->pcType->mfRadius)
	{
		return;
	}

	GetOutline(0, NULL, asFirst);
	GetOutline(1, &psFirst->sPosition, asSecond);
	if (!BattleConvexOverlap(asFirst, psFirst->pcType->muiNumOutline, asSecond, psSecond->pcType->muiNumOutline, &sPush))
	{
		return;
	}
	fLength = sPush.Magnitude();
	if (fLength <= 0.0f)
	{
		return;
	}

	fFirstShare = psSecond->pcType->mfMass / (psFirst->pcType->mfMass + psSecond->pcType->mfMass);
	psFirst->sPosition.x += sPush.x * fFirstShare;
	psFirst->sPosition.y += sPush.y * fFirstShare;
	psSecond->sPosition.x -= sPush.x * (1.0f - fFirstShare);
	psSecond->sPosition.y -= sPush.y * (1.0f - fFirstShare);
	Wrap(&psFirst->sPosition);
	Wrap(&psSecond->sPosition);

	//Along the push, which is the way the first goes from the second.
	sPush.x /= fLength;
	sPush.y /= fLength;
	fClosing = (psFirst->sVelocity.x - psSecond->sVelocity.x) * sPush.x + (psFirst->sVelocity.y - psSecond->sVelocity.y) * sPush.y;
	if (fClosing < 0.0f)
	{
		fImpulse = -(1.0f + BATTLE_BOUNCE) * fClosing / (1.0f / psFirst->pcType->mfMass + 1.0f / psSecond->pcType->mfMass);
		psFirst->sVelocity.x += sPush.x * fImpulse / psFirst->pcType->mfMass;
		psFirst->sVelocity.y += sPush.y * fImpulse / psFirst->pcType->mfMass;
		psSecond->sVelocity.x -= sPush.x * fImpulse / psSecond->pcType->mfMass;
		psSecond->sVelocity.y -= sPush.y * fImpulse / psSecond->pcType->mfMass;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A ship whose hull is gone is destroyed.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Damage(int32 iShip, float32 fDamage)
{
	SBattleShip*	psShip;

	psShip = &masShips[iShip];
	if (!psShip->bAlive)
	{
		return;
	}
	psShip->fHull -= fDamage;
	if (psShip->fHull <= 0.0f)
	{
		psShip->fHull = 0.0f;
		psShip->bAlive = false;
		psShip->bThrusting = false;
		psShip->bBeam = false;
		AddEffect(BE_Explosion, &psShip->sPosition, psShip->pcType->mfRadius * 3.0f, BATTLE_EXPLOSION_SECONDS);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Once a ship is gone the battle is the other's, or nobody's if both
// are.  It stays that way.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::FindWinner(void)
{
	if (miWinner != BATTLE_NO_SHIP)
	{
		return;
	}
	if (!masShips[0].bAlive && !masShips[1].bAlive)
	{
		miWinner = BATTLE_DRAWN;
	}
	else if (!masShips[0].bAlive)
	{
		miWinner = 1;
	}
	else if (!masShips[1].bAlive)
	{
		miWinner = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattle::AddEffect(EBattleEffect eKind, const SFloat32Vec2* psPosition, float32 fRadius, float32 fLife)
{
	SBattleEffect*	psEffect;

	psEffect = masEffects.Add();
	psEffect->eKind = eKind;
	psEffect->sPosition = *psPosition;
	psEffect->fRadius = fRadius;
	psEffect->fAge = 0.0f;
	psEffect->fLife = fLife;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Wrap(SFloat32Vec2* psPosition) const
{
	psPosition->x = BattleWrap(psPosition->x, mfSize);
	psPosition->y = BattleWrap(psPosition->y, mfSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The shorter way from one place to another, which may be across an edge.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::Delta(SFloat32Vec2* psDest, const SFloat32Vec2* psFrom, const SFloat32Vec2* psTo) const
{
	psDest->x = BattleWrapDelta(psTo->x - psFrom->x, mfSize);
	psDest->y = BattleWrapDelta(psTo->y - psFrom->y, mfSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The ship's outline where it is, or, with psNear, where it is by the
// shorter way from there, so that something near an edge meets a ship
// just across it.  asDest holds BATTLE_MAX_OUTLINE_CORNERS.
//
//////////////////////////////////////////////////////////////////////////
void CBattle::GetOutline(int32 iShip, const SFloat32Vec2* psNear, SFloat32Vec2* asDest)
{
	SBattleShip*	psShip;
	SFloat32Vec2	sPosition;
	SFloat32Vec2	sDelta;

	psShip = &masShips[iShip];
	sPosition = psShip->sPosition;
	if (psNear != NULL)
	{
		Delta(&sDelta, psNear, &psShip->sPosition);
		sPosition.x = psNear->x + sDelta.x;
		sPosition.y = psNear->y + sDelta.y;
	}
	BattleConvexPlace(asDest, psShip->pcType->masOutline, psShip->pcType->muiNumOutline, &sPosition, psShip->fHeading);
}


//////////////////////////////////////////////////////////////////////////
//
// A shot with nothing set, for whoever is putting one in flight.
//
//////////////////////////////////////////////////////////////////////////
SBattleShot* CBattle::AddShot(void)
{
	SBattleShot*	psShot;

	psShot = masShots.Add();
	memset(psShot, 0, sizeof(SBattleShot));
	return psShot;
}


SBattleShip* CBattle::GetShip(int32 iShip) { return &masShips[iShip]; }
size CBattle::NumShots(void) const { return masShots.NumElements(); }
SBattleShot* CBattle::GetShot(size uiIndex) { return masShots.Get(uiIndex); }
size CBattle::NumEffects(void) const { return masEffects.NumElements(); }
SBattleEffect* CBattle::GetEffect(size uiIndex) { return masEffects.Get(uiIndex); }
float32 CBattle::GetSize(void) const { return mfSize; }
SFloat32Vec2 CBattle::GetPlanet(void) const { return msPlanet; }
float32 CBattle::GetPlanetRadius(void) const { return mfPlanetRadius; }
float32 CBattle::GetSeconds(void) const { return mfSeconds; }
int32 CBattle::GetWinner(void) const { return miWinner; }
bool CBattle::IsOver(void) const { return miWinner != BATTLE_NO_SHIP; }

