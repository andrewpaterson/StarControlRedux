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
#ifndef __BATTLE_SHIP_TYPE_H__
#define __BATTLE_SHIP_TYPE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Float32Vec2.h"


#define BATTLE_MAX_OUTLINE_CORNERS	32


//What a weapon is.
enum EBattleWeapon
{
	BW_None,
	BW_Bolt,	//A shot that flies straight until it hits something or its life runs out.
	BW_Seeker,	//A shot that turns towards the other ship as it flies, and that a bolt, a beam or a burst destroys.
	BW_Beam,	//Reaches out from the ship while its control is held, hurting the first thing it meets.
	BW_Burst,	//Goes off round the ship: it destroys the other ship's shots in its range and hurts the other ship if it's there.
};


//A weapon's numbers.  Which of them a kind of weapon uses is said beside each.
struct SBattleWeapon
{
	EBattleWeapon	eKind;
	float32			fEnergy;	//What firing it once costs; a beam's is for each second it's on.
	float32			fCooldown;	//Seconds before it fires again.  Not a beam's.
	float32			fDamage;	//To the hull it hits; a beam's is for each second it's on something.
	float32			fSpeed;		//A bolt's and a seeker's, on top of the ship's own for a bolt.
	float32			fLife;		//Seconds a bolt or a seeker flies for.
	float32			fRange;		//How far a beam reaches from its muzzle, or a burst from the ship's middle.
	float32			fTurn;		//Radians a second a seeker turns.
	float32			fRadius;	//Of a bolt or a seeker.
	float32			fMuzzle;	//How far in front of the ship's middle a bolt, a seeker or a beam starts.
	float32			fSpread;	//How far to each side of the ship's middle bolts start, one side then the other.

	void Init(EBattleWeapon eKind);
};


//A kind of ship: what it's called, how it flies, what it carries and its outline, which is what things hit.  The outline is
//about the ship's middle, x to its right and y the way it faces, and SetOutline takes the convex outline round the points
//it's given, so it can be handed every point of a mesh seen from above.
class CBattleShipType
{
public:
	CChars			mszName;
	SFloat32Vec2	masOutline[BATTLE_MAX_OUTLINE_CORNERS];  //Anticlockwise.
	size			muiNumOutline;
	float32			mfRadius;  //How far its outline goes from its middle.

	float32			mfMass;
	float32			mfHull;  //How much it takes before it's destroyed.
	float32			mfEnergy;  //What its weapons draw on, when full.
	float32			mfRecharge;  //Energy a second.
	float32			mfThrust;  //Speed gained a second.
	float32			mfMaxSpeed;  //What its own thrust takes it to: the planet can throw it faster.
	float32			mfTurnSpeed;  //Radians a second, turning as hard as it can.
	float32			mfTurnAcceleration;  //Radians a second gained or lost each second: how long it takes to start and stop turning.

	SBattleWeapon	msPrimary;
	SBattleWeapon	msSecondary;

	void	Init(const char* szName);
	void	Kill(void);

	bool	SetOutline(SFloat32Vec2* asPoints, size uiNumPoints);
};


void	BattleShipKestrel(CBattleShipType* pcType);
void	BattleShipBulwark(CBattleShipType* pcType);


#endif // __BATTLE_SHIP_TYPE_H__

