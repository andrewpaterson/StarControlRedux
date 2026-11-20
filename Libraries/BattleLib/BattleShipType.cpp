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
#include "BaseLib/Logger.h"
#include "BattleGeometry.h"
#include "BattleShipType.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SBattleWeapon::Init(EBattleWeapon eKind)
{
	memset(this, 0, sizeof(SBattleWeapon));
	this->eKind = eKind;
}


//////////////////////////////////////////////////////////////////////////
//
// A ship of no size that can't move and carries nothing.
//
//////////////////////////////////////////////////////////////////////////
void CBattleShipType::Init(const char* szName)
{
	mszName.Init(szName);
	memset(masOutline, 0, sizeof(masOutline));
	muiNumOutline = 0;
	mfRadius = 0.0f;

	mfMass = 1.0f;
	mfHull = 1.0f;
	mfEnergy = 0.0f;
	mfRecharge = 0.0f;
	mfThrust = 0.0f;
	mfMaxSpeed = 0.0f;
	mfTurnSpeed = 0.0f;
	mfTurnAcceleration = 0.0f;

	msPrimary.Init(BW_None);
	msSecondary.Init(BW_None);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleShipType::Kill(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The convex outline round the points, which are sorted on the way.
// False, leaving the outline as it was, if they're all in a line or the
// outline has more corners than a ship's may.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleShipType::SetOutline(SFloat32Vec2* asPoints, size uiNumPoints)
{
	CArrayFloat32Vec2	asHull;
	size				uiCorners;

	asHull.Init();
	asHull.Resize(uiNumPoints + 1);
	uiCorners = BattleConvexHull(asHull.GetData(), asPoints, uiNumPoints);
	if ((uiCorners < 3) || (uiCorners > BATTLE_MAX_OUTLINE_CORNERS))
	{
		gcLogger.Error2(__METHOD__, " The outline of [", mszName.Text(), "] must have from 3 to 32 corners.", NULL);
		asHull.Kill();
		return false;
	}

	memcpy(masOutline, asHull.GetData(), uiCorners * sizeof(SFloat32Vec2));
	muiNumOutline = uiCorners;
	mfRadius = BattleConvexRadius(masOutline, muiNumOutline);
	asHull.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A small, quick ship: it turns and gets going fast and takes little.
// It fires bolts from each wing in turn, and a seeker that follows the
// other ship.  Its outline is a dart's until a mesh gives it its own.
//
//////////////////////////////////////////////////////////////////////////
void BattleShipKestrel(CBattleShipType* pcType)
{
	SFloat32Vec2	asOutline[4];

	pcType->Init("Kestrel");
	asOutline[0].Init(0.0f, 4.0f);
	asOutline[1].Init(-3.0f, -3.0f);
	asOutline[2].Init(0.0f, -2.0f);
	asOutline[3].Init(3.0f, -3.0f);
	pcType->SetOutline(asOutline, 4);

	pcType->mfMass = 1.0f;
	pcType->mfHull = 16.0f;
	pcType->mfEnergy = 20.0f;
	pcType->mfRecharge = 3.0f;
	pcType->mfThrust = 42.0f;
	pcType->mfMaxSpeed = 62.0f;
	pcType->mfTurnSpeed = 4.2f;
	pcType->mfTurnAcceleration = 26.0f;

	pcType->msPrimary.Init(BW_Bolt);
	pcType->msPrimary.fEnergy = 1.0f;
	pcType->msPrimary.fCooldown = 0.2f;
	pcType->msPrimary.fDamage = 1.0f;
	pcType->msPrimary.fSpeed = 120.0f;
	pcType->msPrimary.fLife = 0.75f;
	pcType->msPrimary.fRadius = 0.5f;
	pcType->msPrimary.fMuzzle = 2.5f;
	pcType->msPrimary.fSpread = 2.2f;

	pcType->msSecondary.Init(BW_Seeker);
	pcType->msSecondary.fEnergy = 8.0f;
	pcType->msSecondary.fCooldown = 0.9f;
	pcType->msSecondary.fDamage = 4.0f;
	pcType->msSecondary.fSpeed = 58.0f;
	pcType->msSecondary.fLife = 5.0f;
	pcType->msSecondary.fTurn = 2.2f;
	pcType->msSecondary.fRadius = 0.8f;
	pcType->msSecondary.fMuzzle = 4.5f;
}


//////////////////////////////////////////////////////////////////////////
//
// A big, slow ship that takes a lot.  Its beam reaches a short way in
// front of it for as long as it's held, and its burst goes off all round
// it, which is what it has against seekers.  Its outline is a slab's
// until a mesh gives it its own.
//
//////////////////////////////////////////////////////////////////////////
void BattleShipBulwark(CBattleShipType* pcType)
{
	SFloat32Vec2	asOutline[6];

	pcType->Init("Bulwark");
	asOutline[0].Init(-2.0f, 6.0f);
	asOutline[1].Init(-5.0f, 0.0f);
	asOutline[2].Init(-4.0f, -6.0f);
	asOutline[3].Init(4.0f, -6.0f);
	asOutline[4].Init(5.0f, 0.0f);
	asOutline[5].Init(2.0f, 6.0f);
	pcType->SetOutline(asOutline, 6);

	pcType->mfMass = 4.0f;
	pcType->mfHull = 40.0f;
	pcType->mfEnergy = 36.0f;
	pcType->mfRecharge = 4.0f;
	pcType->mfThrust = 20.0f;
	pcType->mfMaxSpeed = 40.0f;
	pcType->mfTurnSpeed = 2.3f;
	pcType->mfTurnAcceleration = 9.0f;

	pcType->msPrimary.Init(BW_Beam);
	pcType->msPrimary.fEnergy = 9.0f;
	pcType->msPrimary.fDamage = 4.0f;
	pcType->msPrimary.fRange = 34.0f;
	pcType->msPrimary.fMuzzle = 7.0f;

	pcType->msSecondary.Init(BW_Burst);
	pcType->msSecondary.fEnergy = 12.0f;
	pcType->msSecondary.fCooldown = 1.2f;
	pcType->msSecondary.fDamage = 3.0f;
	pcType->msSecondary.fRange = 16.0f;
}

