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
#ifndef __BATTLE_H__
#define __BATTLE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BattleShipType.h"


#define BATTLE_SHIPS				2
#define BATTLE_NO_SHIP				(-1)
#define BATTLE_DRAWN				BATTLE_SHIPS	//Who won, when both went at once.

#define BATTLE_ARENA_SIZE			400.0f	//Across and up, before it comes round again.
#define BATTLE_START_FIRST			0.35f	//How far across the first ship starts, as a share of the size,
#define BATTLE_START_SECOND			0.65f	//and the second: the planet is half way between them.
#define BATTLE_PLANET_RADIUS		14.0f
#define BATTLE_PLANET_GRAVITY		2500.0f	//Over the square of the distance to the planet's middle, the speed gained a second.
#define BATTLE_PLANET_MAX_GRAVITY	16.0f	//The most the planet pulls, however near.
#define BATTLE_PLANET_DAMAGE		2.0f	//To a ship that hits it
#define BATTLE_PLANET_HURT_SPEED		6.0f	//at least this fast: slower, it only bounces,
#define BATTLE_PLANET_HURT_SECONDS	0.5f	//and how long before it's hurt by it again.
#define BATTLE_BOUNCE				0.6f	//Of the speed a ship hit something at, what it leaves with.
#define BATTLE_STEP					(1.0f / 120.0f)	//The longest the battle is moved on in one go.
#define BATTLE_HIT_SECONDS			0.25f	//How long a hit's flash lasts,
#define BATTLE_BURST_SECONDS		0.35f	//a burst's ring,
#define BATTLE_EXPLOSION_SECONDS	1.2f	//and a ship's explosion.


//What a pilot is doing with a ship.
struct SBattleControls
{
	bool	bLeft;
	bool	bRight;
	bool	bThrust;
	bool	bPrimary;
	bool	bSecondary;

	void Init(void);
};


//A ship in a battle.
struct SBattleShip
{
	CBattleShipType*	pcType;
	SBattleControls		sControls;
	SFloat32Vec2		sPosition;
	SFloat32Vec2		sVelocity;
	float32				fHeading;
	float32				fTurning;  //Radians a second, to the right.
	float32				fHull;
	float32				fEnergy;
	float32				fPrimaryWait;  //Seconds until the weapon can fire again.
	float32				fSecondaryWait;
	float32				fPlanetWait;  //Seconds until the planet can hurt it again.
	bool				bAlive;
	bool				bThrusting;
	bool				bBeam;  //Its beam is on,
	float32				fBeamLength;  //and reaches this far from its muzzle.
	bool				bLeftBolt;  //The next bolt comes from its left.
};


//A bolt or a seeker in flight.
struct SBattleShot
{
	EBattleWeapon	eKind;
	int32			iOwner;
	SFloat32Vec2	sPosition;
	SFloat32Vec2	sVelocity;
	float32			fHeading;
	float32			fSpeed;
	float32			fTurn;
	float32			fLife;
	float32			fDamage;
	float32			fRadius;
};


//Something only seen, which hurts nothing.
enum EBattleEffect
{
	BE_Hit,			//Where a shot or a beam struck.
	BE_Burst,		//A burst's ring, which grows to its range.
	BE_Explosion,	//Where a ship was destroyed.
};


struct SBattleEffect
{
	EBattleEffect	eKind;
	SFloat32Vec2	sPosition;
	float32			fRadius;  //What it grows to.
	float32			fAge;
	float32			fLife;
};


typedef CArrayTemplate<SBattleShot>		CArrayBattleShot;
typedef CArrayTemplate<SBattleEffect>	CArrayBattleEffect;


//Two ships fighting round a planet, in a square of space that comes round again at its edges: what leaves one side arrives at
//the other, and everything here acts along the shorter way between two things (Delta).
//
//A ship keeps going the way it was going.  Its thrust speeds it up the way it faces, as far as its type's top speed; the
//planet pulls it, and can throw it faster than that.  Turning takes a moment to start and to stop.  Hitting the planet
//throws it back, and hurts it if it hit hard, and the two ships push each other apart.
//
//Step moves everything on, and is the same battle every time from the same start and controls.  The battle is over when a
//ship's hull is gone: GetWinner is the other, or BATTLE_DRAWN if both went together.  What's left flies on.
//
//It holds no ship types: they must outlast it.
class CBattle
{
protected:
	SBattleShip			masShips[BATTLE_SHIPS];
	CArrayBattleShot	masShots;
	CArrayBattleEffect	masEffects;
	float32				mfSize;
	SFloat32Vec2		msPlanet;
	float32				mfPlanetRadius;
	float32				mfSeconds;  //Since the start.
	float32				mfUnstepped;  //Time Advance has been given and Step hasn't yet used.
	int32				miWinner;

public:
	void			Init(CBattleShipType* pcFirst, CBattleShipType* pcSecond);
	void			Kill(void);
	void			Start(void);

	void			SetControls(int32 iShip, SBattleControls* psControls);
	void			Advance(float32 fSeconds);
	void			Step(float32 fSeconds);

	SBattleShip*	GetShip(int32 iShip);
	size			NumShots(void) const;
	SBattleShot*	GetShot(size uiIndex);
	SBattleShot*	AddShot(void);
	size			NumEffects(void) const;
	SBattleEffect*	GetEffect(size uiIndex);
	float32			GetSize(void) const;
	SFloat32Vec2	GetPlanet(void) const;
	float32			GetPlanetRadius(void) const;
	float32			GetSeconds(void) const;
	int32			GetWinner(void) const;
	bool			IsOver(void) const;

	void			Delta(SFloat32Vec2* psDest, const SFloat32Vec2* psFrom, const SFloat32Vec2* psTo) const;
	void			GetMuzzle(int32 iShip, SBattleWeapon* psWeapon, SFloat32Vec2* psDest);
	void			GetOutline(int32 iShip, const SFloat32Vec2* psNear, SFloat32Vec2* asDest);

protected:
	void			StepShip(int32 iShip, float32 fSeconds);
	void			Fire(int32 iShip, SBattleWeapon* psWeapon, float32* pfWait, bool bHeld, float32 fSeconds);
	void			FireShot(int32 iShip, SBattleWeapon* psWeapon);
	void			FireBeam(int32 iShip, SBattleWeapon* psWeapon, float32 fSeconds);
	void			FireBurst(int32 iShip, SBattleWeapon* psWeapon);
	void			StepShots(float32 fSeconds);
	bool			ShotHits(SBattleShot* psShot);
	void			StepEffects(float32 fSeconds);
	void			CollidePlanet(int32 iShip);
	void			CollideShips(void);
	void			Damage(int32 iShip, float32 fDamage);
	void			FindWinner(void);
	void			AddEffect(EBattleEffect eKind, const SFloat32Vec2* psPosition, float32 fRadius, float32 fLife);
	void			Wrap(SFloat32Vec2* psPosition) const;
};


#endif // __BATTLE_H__

