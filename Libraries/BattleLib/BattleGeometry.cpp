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
#include <stdlib.h>
#include "BattleGeometry.h"


//////////////////////////////////////////////////////////////////////////
//
// The way a heading faces: up the screen for 0, right for a quarter turn.
//
//////////////////////////////////////////////////////////////////////////
void BattleForward(SFloat32Vec2* psDest, float32 fHeading)
{
	psDest->x = sinf(fHeading);
	psDest->y = cosf(fHeading);
}


//////////////////////////////////////////////////////////////////////////
//
// To the right of something facing fHeading.
//
//////////////////////////////////////////////////////////////////////////
void BattleRight(SFloat32Vec2* psDest, float32 fHeading)
{
	psDest->x = cosf(fHeading);
	psDest->y = -sinf(fHeading);
}


//////////////////////////////////////////////////////////////////////////
//
// The heading that faces along psDirection, 0 to two pi; 0 for no
// direction at all.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleHeading(const SFloat32Vec2* psDirection)
{
	if ((psDirection->x == 0.0f) && (psDirection->y == 0.0f))
	{
		return 0.0f;
	}
	return BattleWrapAngle(atan2f(psDirection->x, psDirection->y));
}


//////////////////////////////////////////////////////////////////////////
//
// The same angle from 0 up to, not including, two pi.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleWrapAngle(float32 fAngle)
{
	fAngle = fmodf(fAngle, BATTLE_TWO_PI);
	if (fAngle < 0.0f)
	{
		fAngle += BATTLE_TWO_PI;
	}
	if (fAngle >= BATTLE_TWO_PI)
	{
		fAngle = 0.0f;
	}
	return fAngle;
}


//////////////////////////////////////////////////////////////////////////
//
// The shortest turn from one heading to another, from minus pi up to pi:
// positive is a turn to the right.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleAngleBetween(float32 fFrom, float32 fTo)
{
	float32	fTurn;

	fTurn = BattleWrapAngle(fTo - fFrom);
	if (fTurn >= BATTLE_PI)
	{
		fTurn -= BATTLE_TWO_PI;
	}
	return fTurn;
}


//////////////////////////////////////////////////////////////////////////
//
// Where fValue is in a space that goes from 0 up to fSize and round to 0
// again.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleWrap(float32 fValue, float32 fSize)
{
	fValue = fmodf(fValue, fSize);
	if (fValue < 0.0f)
	{
		fValue += fSize;
	}
	if (fValue >= fSize)
	{
		fValue = 0.0f;
	}
	return fValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The shorter way between two places fDelta apart in a space that goes
// round every fSize: from minus half of it up to half of it.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleWrapDelta(float32 fDelta, float32 fSize)
{
	fDelta = BattleWrap(fDelta, fSize);
	if (fDelta >= fSize * 0.5f)
	{
		fDelta -= fSize;
	}
	return fDelta;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int __cdecl BattleComparePoints(const void* pvLeft, const void* pvRight)
{
	const SFloat32Vec2*	psLeft;
	const SFloat32Vec2*	psRight;

	psLeft = (const SFloat32Vec2*)pvLeft;
	psRight = (const SFloat32Vec2*)pvRight;
	if (psLeft->x != psRight->x)
	{
		return (psLeft->x < psRight->x) ? -1 : 1;
	}
	if (psLeft->y != psRight->y)
	{
		return (psLeft->y < psRight->y) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Twice the area of the triangle, positive when it goes anticlockwise.
//
//////////////////////////////////////////////////////////////////////////
static float32 BattleTurn(const SFloat32Vec2* psA, const SFloat32Vec2* psB, const SFloat32Vec2* psC)
{
	return (psB->x - psA->x) * (psC->y - psA->y) - (psB->y - psA->y) * (psC->x - psA->x);
}


//////////////////////////////////////////////////////////////////////////
//
// The convex outline round asPoints, anticlockwise, with no corner on a
// straight edge.  asPoints is sorted on the way, and asDest must hold a
// corner more than there are points.  Returns how many corners it has:
// fewer than three for points that are all in a line.
//
//////////////////////////////////////////////////////////////////////////
size BattleConvexHull(SFloat32Vec2* asDest, SFloat32Vec2* asPoints, size uiNumPoints)
{
	size	i;
	size	uiCount;
	size	uiLower;

	if (uiNumPoints == 0)
	{
		return 0;
	}
	qsort(asPoints, uiNumPoints, sizeof(SFloat32Vec2), BattleComparePoints);

	uiCount = 0;
	for (i = 0; i < uiNumPoints; i++)
	{
		while ((uiCount >= 2) && (BattleTurn(&asDest[uiCount - 2], &asDest[uiCount - 1], &asPoints[i]) <= 0.0f))
		{
			uiCount--;
		}
		asDest[uiCount] = asPoints[i];
		uiCount++;
	}

	uiLower = uiCount + 1;
	for (i = uiNumPoints - 1; i > 0; i--)
	{
		while ((uiCount >= uiLower) && (BattleTurn(&asDest[uiCount - 2], &asDest[uiCount - 1], &asPoints[i - 1]) <= 0.0f))
		{
			uiCount--;
		}
		asDest[uiCount] = asPoints[i - 1];
		uiCount++;
	}

	//The last is the first again.
	return (uiCount > 1) ? uiCount - 1 : uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// How far the furthest corner is from the origin the corners are about.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleConvexRadius(const SFloat32Vec2* asCorners, size uiNumCorners)
{
	float32	fSquare;
	float32	fLargest;
	size	i;

	fLargest = 0.0f;
	for (i = 0; i < uiNumCorners; i++)
	{
		fSquare = asCorners[i].x * asCorners[i].x + asCorners[i].y * asCorners[i].y;
		if (fSquare > fLargest)
		{
			fLargest = fSquare;
		}
	}
	return sqrtf(fLargest);
}


//////////////////////////////////////////////////////////////////////////
//
// An outline about its own origin, x to its right and y the way it faces,
// put at psPosition facing fHeading.  It stays anticlockwise.
//
//////////////////////////////////////////////////////////////////////////
void BattleConvexPlace(SFloat32Vec2* asDest, const SFloat32Vec2* asCorners, size uiNumCorners, const SFloat32Vec2* psPosition, float32 fHeading)
{
	SFloat32Vec2	sForward;
	SFloat32Vec2	sRight;
	size			i;

	BattleForward(&sForward, fHeading);
	BattleRight(&sRight, fHeading);
	for (i = 0; i < uiNumCorners; i++)
	{
		asDest[i].x = psPosition->x + sRight.x * asCorners[i].x + sForward.x * asCorners[i].y;
		asDest[i].y = psPosition->y + sRight.y * asCorners[i].x + sForward.y * asCorners[i].y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How far psPoint is from the segment.
//
//////////////////////////////////////////////////////////////////////////
static float32 BattleSegmentDistance(const SFloat32Vec2* psStart, const SFloat32Vec2* psEnd, const SFloat32Vec2* psPoint)
{
	float32	fX;
	float32	fY;
	float32	fSquare;
	float32	fAlong;

	fX = psEnd->x - psStart->x;
	fY = psEnd->y - psStart->y;
	fSquare = fX * fX + fY * fY;
	fAlong = 0.0f;
	if (fSquare > 0.0f)
	{
		fAlong = ((psPoint->x - psStart->x) * fX + (psPoint->y - psStart->y) * fY) / fSquare;
		fAlong = (fAlong < 0.0f) ? 0.0f : ((fAlong > 1.0f) ? 1.0f : fAlong);
	}
	fX = psStart->x + fX * fAlong - psPoint->x;
	fY = psStart->y + fY * fAlong - psPoint->y;
	return sqrtf(fX * fX + fY * fY);
}


//////////////////////////////////////////////////////////////////////////
//
// How far psPoint is from the outline: negative inside it, by how far it
// is from the nearest edge.
//
//////////////////////////////////////////////////////////////////////////
float32 BattleConvexDistance(const SFloat32Vec2* asCorners, size uiNumCorners, const SFloat32Vec2* psPoint)
{
	const SFloat32Vec2*	psStart;
	const SFloat32Vec2*	psEnd;
	float32				fDistance;
	float32				fNearest;
	bool				bInside;
	size				i;

	if (uiNumCorners == 0)
	{
		return 3.0e38f;
	}

	bInside = uiNumCorners >= 3;
	fNearest = 3.0e38f;
	for (i = 0; i < uiNumCorners; i++)
	{
		psStart = &asCorners[i];
		psEnd = &asCorners[(i + 1) % uiNumCorners];
		if (BattleTurn(psStart, psEnd, psPoint) < 0.0f)
		{
			bInside = false;
		}
		fDistance = BattleSegmentDistance(psStart, psEnd, psPoint);
		if (fDistance < fNearest)
		{
			fNearest = fDistance;
		}
	}
	return bInside ? -fNearest : fNearest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the segment touches the outline.  pfEnter is then how far along
// it, 0 at its start to 1 at its end, it first does: 0 if it starts
// inside.
//
//////////////////////////////////////////////////////////////////////////
bool BattleConvexSegment(const SFloat32Vec2* asCorners, size uiNumCorners, const SFloat32Vec2* psStart, const SFloat32Vec2* psEnd, float32* pfEnter)
{
	const SFloat32Vec2*	psA;
	const SFloat32Vec2*	psB;
	float32				fEnter;
	float32				fLeave;
	float32				fNormalX;
	float32				fNormalY;
	float32				fStart;
	float32				fAlong;
	float32				fCross;
	size				i;

	if (uiNumCorners < 3)
	{
		return false;
	}

	//Clipped by each edge's line, whose outside is to its right.
	fEnter = 0.0f;
	fLeave = 1.0f;
	for (i = 0; i < uiNumCorners; i++)
	{
		psA = &asCorners[i];
		psB = &asCorners[(i + 1) % uiNumCorners];
		fNormalX = psB->y - psA->y;
		fNormalY = psA->x - psB->x;
		fStart = (psStart->x - psA->x) * fNormalX + (psStart->y - psA->y) * fNormalY;
		fAlong = (psEnd->x - psStart->x) * fNormalX + (psEnd->y - psStart->y) * fNormalY;
		if (fAlong == 0.0f)
		{
			if (fStart > 0.0f)
			{
				return false;
			}
		}
		else
		{
			fCross = -fStart / fAlong;
			if (fAlong < 0.0f)
			{
				if (fCross > fEnter)
				{
					fEnter = fCross;
				}
			}
			else if (fCross < fLeave)
			{
				fLeave = fCross;
			}
			if (fEnter > fLeave)
			{
				return false;
			}
		}
	}
	*pfEnter = fEnter;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How far the outline reaches along the axis, least and most.
//
//////////////////////////////////////////////////////////////////////////
static void BattleConvexProject(const SFloat32Vec2* asCorners, size uiNumCorners, float32 fAxisX, float32 fAxisY, float32* pfLeast, float32* pfMost)
{
	float32	fAlong;
	size	i;

	*pfLeast = 3.0e38f;
	*pfMost = -3.0e38f;
	for (i = 0; i < uiNumCorners; i++)
	{
		fAlong = asCorners[i].x * fAxisX + asCorners[i].y * fAxisY;
		if (fAlong < *pfLeast)
		{
			*pfLeast = fAlong;
		}
		if (fAlong > *pfMost)
		{
			*pfMost = fAlong;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The least overlap of the two outlines along the outward normals of
// asEdges's edges, and that normal; false if one of them keeps the two
// apart.  bFlip says the edges are B's, so their normals point from B.
//
//////////////////////////////////////////////////////////////////////////
static bool BattleConvexLeastOverlap(const SFloat32Vec2* asEdges, size uiNumEdges, const SFloat32Vec2* asA, size uiNumA, const SFloat32Vec2* asB, size uiNumB, bool bFlip, float32* pfOverlap, SFloat32Vec2* psPush)
{
	const SFloat32Vec2*	psStart;
	const SFloat32Vec2*	psEnd;
	float32				fAxisX;
	float32				fAxisY;
	float32				fLength;
	float32				fLeastA;
	float32				fMostA;
	float32				fLeastB;
	float32				fMostB;
	float32				fOverlap;
	size				i;

	for (i = 0; i < uiNumEdges; i++)
	{
		psStart = &asEdges[i];
		psEnd = &asEdges[(i + 1) % uiNumEdges];
		fAxisX = psEnd->y - psStart->y;
		fAxisY = psStart->x - psEnd->x;
		fLength = sqrtf(fAxisX * fAxisX + fAxisY * fAxisY);
		if (fLength > 0.0f)
		{
			fAxisX /= fLength;
			fAxisY /= fLength;
			BattleConvexProject(asA, uiNumA, fAxisX, fAxisY, &fLeastA, &fMostA);
			BattleConvexProject(asB, uiNumB, fAxisX, fAxisY, &fLeastB, &fMostB);

			//Along an edge's own normal, its outline is behind it and the other in front.
			fOverlap = bFlip ? (fMostB - fLeastA) : (fMostA - fLeastB);
			if (fOverlap <= 0.0f)
			{
				return false;
			}
			if (fOverlap < *pfOverlap)
			{
				*pfOverlap = fOverlap;

				//A is pushed back along its own edge's normal, or on along B's.
				psPush->x = bFlip ? fAxisX : -fAxisX;
				psPush->y = bFlip ? fAxisY : -fAxisY;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two outlines overlap.  psPush is then the least A has to move
// to be clear of B.
//
//////////////////////////////////////////////////////////////////////////
bool BattleConvexOverlap(const SFloat32Vec2* asA, size uiNumA, const SFloat32Vec2* asB, size uiNumB, SFloat32Vec2* psPush)
{
	float32			fOverlap;
	SFloat32Vec2	sPush;

	if ((uiNumA < 3) || (uiNumB < 3))
	{
		return false;
	}

	fOverlap = 3.0e38f;
	sPush.Init(0.0f, 0.0f);
	if (!BattleConvexLeastOverlap(asA, uiNumA, asA, uiNumA, asB, uiNumB, false, &fOverlap, &sPush))
	{
		return false;
	}
	if (!BattleConvexLeastOverlap(asB, uiNumB, asA, uiNumA, asB, uiNumB, true, &fOverlap, &sPush))
	{
		return false;
	}
	psPush->x = sPush.x * fOverlap;
	psPush->y = sPush.y * fOverlap;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the segment touches the circle.  pfEnter is then how far along
// it, 0 to 1, it first does: 0 if it starts inside.
//
//////////////////////////////////////////////////////////////////////////
bool BattleCircleSegment(const SFloat32Vec2* psCentre, float32 fRadius, const SFloat32Vec2* psStart, const SFloat32Vec2* psEnd, float32* pfEnter)
{
	float32	fX;
	float32	fY;
	float32	fFromX;
	float32	fFromY;
	float32	fA;
	float32	fB;
	float32	fC;
	float32	fRoot;
	float32	fEnter;

	fX = psEnd->x - psStart->x;
	fY = psEnd->y - psStart->y;
	fFromX = psStart->x - psCentre->x;
	fFromY = psStart->y - psCentre->y;
	fC = fFromX * fFromX + fFromY * fFromY - fRadius * fRadius;
	if (fC <= 0.0f)
	{
		*pfEnter = 0.0f;
		return true;
	}

	fA = fX * fX + fY * fY;
	fB = fFromX * fX + fFromY * fY;
	if ((fA == 0.0f) || (fB >= 0.0f))
	{
		return false;
	}
	fRoot = fB * fB - fA * fC;
	if (fRoot < 0.0f)
	{
		return false;
	}
	fEnter = (-fB - sqrtf(fRoot)) / fA;
	if (fEnter > 1.0f)
	{
		return false;
	}
	*pfEnter = fEnter;
	return true;
}

