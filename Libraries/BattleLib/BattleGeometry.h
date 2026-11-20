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
#ifndef __BATTLE_GEOMETRY_H__
#define __BATTLE_GEOMETRY_H__
#include "BaseLib/Float32Vec2.h"


#define BATTLE_PI		3.14159265358979f
#define BATTLE_TWO_PI	6.28318530717959f


//Geometry in the plane a battle is fought in: x is to the right of the screen and y up it.  A heading is in radians, 0
//facing up the screen (+y) and growing clockwise, so turning right adds to it.
//
//A convex outline is its corners in order, anticlockwise, and what collides with a ship collides with its outline.

void	BattleForward(SFloat32Vec2* psDest, float32 fHeading);
void	BattleRight(SFloat32Vec2* psDest, float32 fHeading);
float32	BattleHeading(const SFloat32Vec2* psDirection);
float32	BattleWrapAngle(float32 fAngle);
float32	BattleAngleBetween(float32 fFrom, float32 fTo);
float32	BattleWrap(float32 fValue, float32 fSize);
float32	BattleWrapDelta(float32 fDelta, float32 fSize);

size	BattleConvexHull(SFloat32Vec2* asDest, SFloat32Vec2* asPoints, size uiNumPoints);
float32	BattleConvexRadius(const SFloat32Vec2* asCorners, size uiNumCorners);
void	BattleConvexPlace(SFloat32Vec2* asDest, const SFloat32Vec2* asCorners, size uiNumCorners, const SFloat32Vec2* psPosition, float32 fHeading);
float32	BattleConvexDistance(const SFloat32Vec2* asCorners, size uiNumCorners, const SFloat32Vec2* psPoint);
bool	BattleConvexSegment(const SFloat32Vec2* asCorners, size uiNumCorners, const SFloat32Vec2* psStart, const SFloat32Vec2* psEnd, float32* pfEnter);
bool	BattleConvexOverlap(const SFloat32Vec2* asA, size uiNumA, const SFloat32Vec2* asB, size uiNumB, SFloat32Vec2* psPush);
bool	BattleCircleSegment(const SFloat32Vec2* psCentre, float32 fRadius, const SFloat32Vec2* psStart, const SFloat32Vec2* psEnd, float32* pfEnter);


#endif // __BATTLE_GEOMETRY_H__

