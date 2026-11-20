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
#include "BattleGeometry.h"
#include "BattlePilot.h"


//////////////////////////////////////////////////////////////////////////
//
// How far the weapon reaches and how fast what it fires gets there; no
// speed for a beam or a burst, which are there at once.
//
//////////////////////////////////////////////////////////////////////////
static void BattlePilotReach(SBattleWeapon* psWeapon, float32* pfReach, float32* pfSpeed)
{
	*pfReach = 0.0f;
	*pfSpeed = 0.0f;
	switch (psWeapon->eKind)
	{
	case BW_Bolt:
	case BW_Seeker:
		*pfReach = psWeapon->fMuzzle + psWeapon->fSpeed * psWeapon->fLife;
		*pfSpeed = psWeapon->fSpeed;
		break;
	case BW_Beam:
		*pfReach = psWeapon->fMuzzle + psWeapon->fRange;
		break;
	case BW_Burst:
		*pfReach = psWeapon->fRange;
		break;
	case BW_None:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a burst from the ship would catch anything: the other ship's
// middle, or one of its shots.
//
//////////////////////////////////////////////////////////////////////////
static bool BattlePilotBurstWorthIt(CBattle* pcBattle, int32 iShip, SBattleWeapon* psWeapon)
{
	SBattleShip*	psShip;
	SBattleShip*	psOther;
	SBattleShot*	psShot;
	SFloat32Vec2	sDelta;
	size			i;

	psShip = pcBattle->GetShip(iShip);
	psOther = pcBattle->GetShip(1 - iShip);
	if (psOther->bAlive)
	{
		pcBattle->Delta(&sDelta, &psShip->sPosition, &psOther->sPosition);
		if (sDelta.Magnitude() <= psWeapon->fRange)
		{
			return true;
		}
	}
	for (i = 0; i < pcBattle->NumShots(); i++)
	{
		psShot = pcBattle->GetShot(i);
		if (psShot->iOwner != iShip)
		{
			pcBattle->Delta(&sDelta, &psShip->sPosition, &psShot->sPosition);
			if (sDelta.Magnitude() <= psWeapon->fRange * 0.8f)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether going from the ship along psWay passes nearer the planet's
// middle than fClear before it's gone as far as psWay is long and fPast
// further.  psAside is then the way to the side that passes it by the
// shorter way round, a unit long.
//
//////////////////////////////////////////////////////////////////////////
static bool BattlePilotBlocked(const SFloat32Vec2* psToPlanet, const SFloat32Vec2* psWay, float32 fClear, float32 fPast, SFloat32Vec2* psAside)
{
	float32	fLength;
	float32	fAlong;
	float32	fOffX;
	float32	fOffY;
	float32	fOff;

	fLength = sqrtf(psWay->x * psWay->x + psWay->y * psWay->y);
	if (fLength <= 0.0f)
	{
		return false;
	}

	//How far along the way the planet is, and how far off to the side of it.
	fAlong = (psToPlanet->x * psWay->x + psToPlanet->y * psWay->y) / fLength;
	if ((fAlong <= 0.0f) || (fAlong >= fLength + fPast))
	{
		return false;
	}
	fOffX = psToPlanet->x - psWay->x / fLength * fAlong;
	fOffY = psToPlanet->y - psWay->y / fLength * fAlong;
	fOff = sqrtf(fOffX * fOffX + fOffY * fOffY);
	if (fOff >= fClear)
	{
		return false;
	}

	if (fOff > 0.001f)
	{
		psAside->x = -fOffX / fOff;
		psAside->y = -fOffY / fOff;
	}
	else
	{
		//Straight at its middle: to the right of the way.
		psAside->x = psWay->y / fLength;
		psAside->y = -psWay->x / fLength;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BattlePilot(CBattle* pcBattle, int32 iShip, SBattleControls* psDest)
{
	SBattleShip*		psShip;
	SBattleShip*		psOther;
	CBattleShipType*	pcType;
	SFloat32Vec2		sToOther;
	SFloat32Vec2		sToPlanet;
	SFloat32Vec2		sAim;
	SFloat32Vec2		sAside;
	SFloat32Vec2		sAhead;
	SFloat32Vec2		sWanted;
	SFloat32Vec2		sPlanet;
	float32				fDistance;
	float32				fPlanetDistance;
	float32				fClear;
	float32				fReach;
	float32				fSpeed;
	float32				fSecondaryReach;
	float32				fSecondarySpeed;
	float32				fSeconds;
	float32				fTurn;
	float32				fThrustTurn;
	float32				fClosing;
	float32				fFurther;
	float32				fWanted;
	float32				fLength;
	bool				bRound;
	bool				bBehind;

	psDest->Init();
	psShip = pcBattle->GetShip(iShip);
	psOther = pcBattle->GetShip(1 - iShip);
	if (!psShip->bAlive)
	{
		return;
	}
	pcType = psShip->pcType;

	sPlanet = pcBattle->GetPlanet();
	pcBattle->Delta(&sToPlanet, &psShip->sPosition, &sPlanet);
	fPlanetDistance = sToPlanet.Magnitude();
	fClear = pcBattle->GetPlanetRadius() + pcType->mfRadius + BATTLE_PILOT_PLANET_MARGIN;
	if (pcType->msSecondary.eKind == BW_Burst)
	{
		psDest->bSecondary = BattlePilotBurstWorthIt(pcBattle, iShip, &pcType->msSecondary);
	}

	//Where it's drifting comes first: if that's into the planet within a few seconds it thrusts aside, and straight
	//out if it's already too near for that.
	sAhead.x = psShip->sVelocity.x * BATTLE_PILOT_LOOK_AHEAD;
	sAhead.y = psShip->sVelocity.y * BATTLE_PILOT_LOOK_AHEAD;
	bRound = BattlePilotBlocked(&sToPlanet, &sAhead, fClear, fClear, &sAside);
	if (fPlanetDistance < fClear)
	{
		sAside.Init(0.0f, 1.0f);
		if (fPlanetDistance > 0.0f)
		{
			sAside.x = -sToPlanet.x / fPlanetDistance;
			sAside.y = -sToPlanet.y / fPlanetDistance;
		}
		bRound = true;
	}
	if (bRound)
	{
		fTurn = BattleAngleBetween(psShip->fHeading, BattleHeading(&sAside));
		psDest->bLeft = fTurn < -BATTLE_PILOT_TURN;
		psDest->bRight = fTurn > BATTLE_PILOT_TURN;
		psDest->bThrust = fabsf(fTurn) < 1.0f;
		return;
	}
	if (!psOther->bAlive)
	{
		return;
	}

	//Where the other ship will be when a shot fired now gets as far as it is.
	BattlePilotReach(&pcType->msPrimary, &fReach, &fSpeed);
	BattlePilotReach(&pcType->msSecondary, &fSecondaryReach, &fSecondarySpeed);
	pcBattle->Delta(&sToOther, &psShip->sPosition, &psOther->sPosition);
	fDistance = sToOther.Magnitude();
	sAim = sToOther;
	if (fSpeed > 0.0f)
	{
		fSeconds = fDistance / fSpeed;
		sAim.x += (psOther->sVelocity.x - psShip->sVelocity.x) * fSeconds;
		sAim.y += (psOther->sVelocity.y - psShip->sVelocity.y) * fSeconds;
	}
	fTurn = BattleAngleBetween(psShip->fHeading, BattleHeading(&sAim));
	fFurther = fDistance - fReach * BATTLE_PILOT_REACH_SHARE;
	fClosing = 0.0f;
	if (fDistance > 0.0f)
	{
		fClosing = ((psShip->sVelocity.x - psOther->sVelocity.x) * sToOther.x + (psShip->sVelocity.y - psOther->sVelocity.y) * sToOther.y) / fDistance;
	}

	//A ship this side of the planet, even one sitting on it, isn't behind it.
	bBehind = BattlePilotBlocked(&sToPlanet, &sToOther, pcBattle->GetPlanetRadius() + BATTLE_PILOT_SHOT_MARGIN, -psOther->pcType->mfRadius, &sAside);
	if (!bBehind && (fDistance <= fReach + psOther->pcType->mfRadius))
	{
		//In reach: it faces where to shoot and fires when it's facing there.  It only closes while it's further
		//than it likes to be, and slowly.
		psDest->bLeft = fTurn < -BATTLE_PILOT_TURN;
		psDest->bRight = fTurn > BATTLE_PILOT_TURN;
		psDest->bPrimary = fabsf(fTurn) < BATTLE_PILOT_AIM;
		psDest->bThrust = (fFurther > 0.0f) && (fabsf(fTurn) < 0.6f) && (fClosing < BATTLE_PILOT_ARRIVE_SPEED);
	}
	else
	{
		//Out of reach it flies to where it would be in reach, round the planet if that's in the way: it thrusts
		//towards the speed it wants, which is how it slows down too, so that it arrives rather than flies past.
		sWanted = sToOther;
		fWanted = BATTLE_PILOT_ARRIVE_SPEED + ((fFurther > 0.0f) ? fFurther * BATTLE_PILOT_ARRIVE_RATE : 0.0f);
		if (bBehind)
		{
			sWanted.x = sToPlanet.x + sAside.x * fClear * 1.5f;
			sWanted.y = sToPlanet.y + sAside.y * fClear * 1.5f;
		}
		if (fWanted > pcType->mfMaxSpeed)
		{
			fWanted = pcType->mfMaxSpeed;
		}
		fLength = sWanted.Magnitude();
		if (fLength > 0.0f)
		{
			sWanted.x = sWanted.x / fLength * fWanted + psOther->sVelocity.x - psShip->sVelocity.x;
			sWanted.y = sWanted.y / fLength * fWanted + psOther->sVelocity.y - psShip->sVelocity.y;
		}
		fThrustTurn = BattleAngleBetween(psShip->fHeading, BattleHeading(&sWanted));
		psDest->bLeft = fThrustTurn < -BATTLE_PILOT_TURN;
		psDest->bRight = fThrustTurn > BATTLE_PILOT_TURN;
		psDest->bThrust = (fabsf(fThrustTurn) < 0.5f) && (sWanted.Magnitude() > BATTLE_PILOT_SPEED_SLACK);
	}

	switch (pcType->msSecondary.eKind)
	{
	case BW_Seeker:
		psDest->bSecondary = !bBehind && (fDistance <= fSecondaryReach) && (fabsf(fTurn) < 1.0f) && (psShip->fEnergy >= pcType->mfEnergy * 0.6f);
		break;
	case BW_Bolt:
	case BW_Beam:
		psDest->bSecondary = !bBehind && (fabsf(fTurn) < BATTLE_PILOT_AIM) && (fDistance <= fSecondaryReach + psOther->pcType->mfRadius);
		break;
	case BW_Burst:
	case BW_None:
		break;
	}
}

