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
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A heading of 0 faces up the screen and a quarter turn is to the right,
// and the headings and the directions agree with each other.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometryHeadings(void)
{
	SFloat32Vec2	sForward;
	SFloat32Vec2	sRight;

	BattleForward(&sForward, 0.0f);
	AssertFloat(0.0f, sForward.x, 5);
	AssertFloat(1.0f, sForward.y, 5);
	BattleRight(&sRight, 0.0f);
	AssertFloat(1.0f, sRight.x, 5);
	AssertFloat(0.0f, sRight.y, 5);

	BattleForward(&sForward, BATTLE_PI * 0.5f);
	AssertFloat(1.0f, sForward.x, 5);
	AssertFloat(0.0f, sForward.y, 5);
	BattleRight(&sRight, BATTLE_PI * 0.5f);
	AssertFloat(0.0f, sRight.x, 5);
	AssertFloat(-1.0f, sRight.y, 5);

	sForward.Init(3.0f, 0.0f);
	AssertFloat(BATTLE_PI * 0.5f, BattleHeading(&sForward), 5);
	sForward.Init(0.0f, -2.0f);
	AssertFloat(BATTLE_PI, BattleHeading(&sForward), 5);
	sForward.Init(-1.0f, 0.0f);
	AssertFloat(BATTLE_PI * 1.5f, BattleHeading(&sForward), 5);
	sForward.Init(0.0f, 0.0f);
	AssertFloat(0.0f, BattleHeading(&sForward), 5);

	AssertFloat(0.5f, BattleWrapAngle(0.5f + BATTLE_TWO_PI), 4);
	AssertFloat(BATTLE_TWO_PI - 0.5f, BattleWrapAngle(-0.5f), 4);
	AssertFloat(0.3f, BattleAngleBetween(6.2f, 6.2f + 0.3f - BATTLE_TWO_PI), 4);
	AssertFloat(-0.3f, BattleAngleBetween(0.1f, BATTLE_TWO_PI - 0.2f), 4);
	AssertFloat(-BATTLE_PI, BattleAngleBetween(0.0f, BATTLE_PI), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A place past an edge is the same distance in from the other, and the
// way between two places is the shorter one.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometryWrap(void)
{
	AssertFloat(5.0f, BattleWrap(105.0f, 100.0f), 4);
	AssertFloat(95.0f, BattleWrap(-5.0f, 100.0f), 4);
	AssertFloat(0.0f, BattleWrap(100.0f, 100.0f), 4);
	AssertFloat(0.0f, BattleWrap(0.0f, 100.0f), 4);

	AssertFloat(10.0f, BattleWrapDelta(10.0f, 100.0f), 4);
	AssertFloat(-10.0f, BattleWrapDelta(90.0f, 100.0f), 4);
	AssertFloat(10.0f, BattleWrapDelta(-90.0f, 100.0f), 4);
	AssertFloat(-50.0f, BattleWrapDelta(50.0f, 100.0f), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// The outline round points is anticlockwise and leaves out what's inside
// it or along an edge.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometryHull(void)
{
	SFloat32Vec2	asPoints[8];
	SFloat32Vec2	asHull[9];
	size			uiCorners;
	size			i;
	float32			fArea;

	asPoints[0].Init(2.0f, 2.0f);
	asPoints[1].Init(-2.0f, -2.0f);
	asPoints[2].Init(0.0f, 0.5f);  //Inside.
	asPoints[3].Init(2.0f, -2.0f);
	asPoints[4].Init(-2.0f, 2.0f);
	asPoints[5].Init(0.0f, 2.0f);  //Along the top edge.
	asPoints[6].Init(2.0f, 2.0f);  //Twice.
	asPoints[7].Init(1.0f, -1.0f);  //Inside.
	uiCorners = BattleConvexHull(asHull, asPoints, 8);
	AssertSize(4, uiCorners);

	fArea = 0.0f;
	for (i = 0; i < uiCorners; i++)
	{
		fArea += asHull[i].x * asHull[(i + 1) % uiCorners].y - asHull[(i + 1) % uiCorners].x * asHull[i].y;
		AssertFloat(2.0f, fabsf(asHull[i].x), 5);
		AssertFloat(2.0f, fabsf(asHull[i].y), 5);
	}
	AssertFloat(32.0f, fArea, 4);  //Twice its area, and positive as it's anticlockwise.
	AssertFloat(sqrtf(8.0f), BattleConvexRadius(asHull, uiCorners), 4);

	//Points in a line have no outline.
	asPoints[0].Init(0.0f, 0.0f);
	asPoints[1].Init(1.0f, 1.0f);
	asPoints[2].Init(2.0f, 2.0f);
	AssertSize(2, BattleConvexHull(asHull, asPoints, 3));
	AssertSize(1, BattleConvexHull(asHull, asPoints, 1));
	AssertSize(0, BattleConvexHull(asHull, asPoints, 0));
}


//////////////////////////////////////////////////////////////////////////
//
// A square two a side about the origin.
//
//////////////////////////////////////////////////////////////////////////
static void TestBattleSquare(SFloat32Vec2* asDest, float32 x, float32 y, float32 fHeading)
{
	SFloat32Vec2	asSquare[4];
	SFloat32Vec2	sPosition;

	asSquare[0].Init(-1.0f, -1.0f);
	asSquare[1].Init(1.0f, -1.0f);
	asSquare[2].Init(1.0f, 1.0f);
	asSquare[3].Init(-1.0f, 1.0f);
	sPosition.Init(x, y);
	BattleConvexPlace(asDest, asSquare, 4, &sPosition, fHeading);
}


//////////////////////////////////////////////////////////////////////////
//
// An outline placed faces its heading: what was in front of it is where
// the heading points.  How far a point is from it is negative inside.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometryPlaceAndDistance(void)
{
	SFloat32Vec2	asNose[3];
	SFloat32Vec2	asPlaced[4];
	SFloat32Vec2	sPosition;
	SFloat32Vec2	sPoint;

	asNose[0].Init(0.0f, 3.0f);
	asNose[1].Init(-1.0f, 0.0f);
	asNose[2].Init(1.0f, 0.0f);
	sPosition.Init(10.0f, 20.0f);
	BattleConvexPlace(asPlaced, asNose, 3, &sPosition, BATTLE_PI * 0.5f);
	AssertFloat(13.0f, asPlaced[0].x, 4);  //Its nose is to the right.
	AssertFloat(20.0f, asPlaced[0].y, 4);
	AssertFloat(10.0f, asPlaced[1].x, 4);  //Its left is up the screen.
	AssertFloat(21.0f, asPlaced[1].y, 4);
	AssertFloat(10.0f, asPlaced[2].x, 4);
	AssertFloat(19.0f, asPlaced[2].y, 4);

	TestBattleSquare(asPlaced, 5.0f, 5.0f, 0.0f);
	sPoint.Init(5.0f, 5.0f);
	AssertFloat(-1.0f, BattleConvexDistance(asPlaced, 4, &sPoint), 4);
	sPoint.Init(5.5f, 5.0f);
	AssertFloat(-0.5f, BattleConvexDistance(asPlaced, 4, &sPoint), 4);
	sPoint.Init(8.0f, 5.0f);
	AssertFloat(2.0f, BattleConvexDistance(asPlaced, 4, &sPoint), 4);
	sPoint.Init(9.0f, 9.0f);  //Past a corner, it's the corner it's from.
	AssertFloat(sqrtf(18.0f), BattleConvexDistance(asPlaced, 4, &sPoint), 4);
	sPoint.Init(6.0f, 5.0f);
	AssertFloat(0.0f, BattleConvexDistance(asPlaced, 4, &sPoint), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A segment meets an outline, or a circle, where it first touches it.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometrySegments(void)
{
	SFloat32Vec2	asSquare[4];
	SFloat32Vec2	sStart;
	SFloat32Vec2	sEnd;
	SFloat32Vec2	sCentre;
	float32			fEnter;

	TestBattleSquare(asSquare, 0.0f, 0.0f, 0.0f);
	sStart.Init(-5.0f, 0.0f);
	sEnd.Init(5.0f, 0.0f);
	AssertTrue(BattleConvexSegment(asSquare, 4, &sStart, &sEnd, &fEnter));
	AssertFloat(0.4f, fEnter, 4);

	sEnd.Init(-2.0f, 0.0f);  //Stops short.
	AssertFalse(BattleConvexSegment(asSquare, 4, &sStart, &sEnd, &fEnter));
	sStart.Init(-5.0f, 3.0f);  //Passes above.
	sEnd.Init(5.0f, 3.0f);
	AssertFalse(BattleConvexSegment(asSquare, 4, &sStart, &sEnd, &fEnter));
	sStart.Init(0.5f, 0.5f);  //Starts inside.
	sEnd.Init(9.0f, 9.0f);
	AssertTrue(BattleConvexSegment(asSquare, 4, &sStart, &sEnd, &fEnter));
	AssertFloat(0.0f, fEnter, 4);
	sStart.Init(-3.0f, -3.0f);  //Corner to corner.
	sEnd.Init(3.0f, 3.0f);
	AssertTrue(BattleConvexSegment(asSquare, 4, &sStart, &sEnd, &fEnter));
	AssertFloat(1.0f / 3.0f, fEnter, 4);

	sCentre.Init(10.0f, 0.0f);
	sStart.Init(0.0f, 0.0f);
	sEnd.Init(20.0f, 0.0f);
	AssertTrue(BattleCircleSegment(&sCentre, 2.0f, &sStart, &sEnd, &fEnter));
	AssertFloat(0.4f, fEnter, 4);
	sEnd.Init(7.0f, 0.0f);  //Stops short.
	AssertFalse(BattleCircleSegment(&sCentre, 2.0f, &sStart, &sEnd, &fEnter));
	sEnd.Init(-20.0f, 0.0f);  //Goes the other way.
	AssertFalse(BattleCircleSegment(&sCentre, 2.0f, &sStart, &sEnd, &fEnter));
	sStart.Init(0.0f, 3.0f);  //Passes beside it.
	sEnd.Init(20.0f, 3.0f);
	AssertFalse(BattleCircleSegment(&sCentre, 2.0f, &sStart, &sEnd, &fEnter));
	sStart.Init(9.0f, 0.5f);  //Starts inside.
	AssertTrue(BattleCircleSegment(&sCentre, 2.0f, &sStart, &sEnd, &fEnter));
	AssertFloat(0.0f, fEnter, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// Outlines that overlap are told how far the first must move to be clear
// of the second, which is the short way out.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometryOverlap(void)
{
	SFloat32Vec2	asFirst[4];
	SFloat32Vec2	asSecond[4];
	SFloat32Vec2	sPush;

	TestBattleSquare(asFirst, 0.0f, 0.0f, 0.0f);
	TestBattleSquare(asSecond, 3.0f, 0.0f, 0.0f);
	AssertFalse(BattleConvexOverlap(asFirst, 4, asSecond, 4, &sPush));

	TestBattleSquare(asSecond, 1.5f, 0.2f, 0.0f);
	AssertTrue(BattleConvexOverlap(asFirst, 4, asSecond, 4, &sPush));
	AssertFloat(-0.5f, sPush.x, 4);
	AssertFloat(0.0f, sPush.y, 4);
	AssertTrue(BattleConvexOverlap(asSecond, 4, asFirst, 4, &sPush));
	AssertFloat(0.5f, sPush.x, 4);
	AssertFloat(0.0f, sPush.y, 4);

	TestBattleSquare(asSecond, 0.3f, -1.75f, 0.0f);
	AssertTrue(BattleConvexOverlap(asFirst, 4, asSecond, 4, &sPush));
	AssertFloat(0.0f, sPush.x, 4);
	AssertFloat(0.25f, sPush.y, 4);

	//Turned an eighth, its corner reaches root two: it touches a square further off than a side would.
	TestBattleSquare(asSecond, 2.3f, 0.0f, BATTLE_PI * 0.25f);
	AssertTrue(BattleConvexOverlap(asFirst, 4, asSecond, 4, &sPush));
	AssertFloat(1.0f + sqrtf(2.0f) - 2.3f, -sPush.x, 4);
	AssertFloat(0.0f, sPush.y, 4);
	TestBattleSquare(asSecond, 2.5f, 0.0f, BATTLE_PI * 0.25f);
	AssertFalse(BattleConvexOverlap(asFirst, 4, asSecond, 4, &sPush));

	//Corners near each other, but apart along the diagonal.
	TestBattleSquare(asFirst, 0.0f, 0.0f, BATTLE_PI * 0.25f);
	TestBattleSquare(asSecond, 1.6f, 1.6f, BATTLE_PI * 0.25f);
	AssertFalse(BattleConvexOverlap(asFirst, 4, asSecond, 4, &sPush));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBattleGeometry(void)
{
	BeginTests();

	TestBattleGeometryHeadings();
	TestBattleGeometryWrap();
	TestBattleGeometryHull();
	TestBattleGeometryPlaceAndDistance();
	TestBattleGeometrySegments();
	TestBattleGeometryOverlap();

	TestStatistics();
}

