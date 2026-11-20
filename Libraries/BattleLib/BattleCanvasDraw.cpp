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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Canvas.h"
#include "BattleCanvasDraw.h"


#define BATTLE_VIEW_STRAIGHT_DOWN	1.5707963f


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleCanvasDraw::Init(Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CSceneCanvasDraw::Init(pTheme, NULL);
	muiBackground = BATTLE_VIEW_BACKGROUND;
	mfHalfAcross = 1.0f;
	mfHalfUp = 1.0f;
	mfDistance = 0.0f;
	mfUnmoved = 0.0f;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleCanvasDraw::Class(void)
{
	CSceneCanvasDraw::Class();

	U_Float32(mfHalfAcross);
	U_Float32(mfHalfUp);
	U_Float32(mfDistance);
	U_Float32(mfUnmoved);
}


//////////////////////////////////////////////////////////////////////////
//
// What has to be seen: fHalfAcross to each side of the middle and fHalfUp
// above and below it.  fSeconds have passed since it was last told, which
// is how far the view moves towards showing it when it's next drawn.
//
//////////////////////////////////////////////////////////////////////////
void CBattleCanvasDraw::SetSpan(float32 fHalfAcross, float32 fHalfUp, float32 fSeconds)
{
	mfHalfAcross = fHalfAcross;
	mfHalfUp = fHalfUp;
	mfUnmoved += fSeconds;
}


//////////////////////////////////////////////////////////////////////////
//
// How far above the middle the view has to be for a canvas of this size
// to show the span.
//
//////////////////////////////////////////////////////////////////////////
float32 CBattleCanvasDraw::DistanceFor(int32 iWidth, int32 iHeight) const
{
	float32	fHalfWidth;
	float32	fAcross;
	float32	fUp;
	float32	fDistance;

	if ((iWidth <= 0) || (iHeight <= 0))
	{
		return BATTLE_VIEW_MIN_DISTANCE;
	}

	//A unit away, the view is this far to each side; the field of view is across its width.
	fHalfWidth = tanf(TURNTABLE_FOV * 0.5f);
	fAcross = mfHalfAcross / fHalfWidth;
	fUp = mfHalfUp / (fHalfWidth * (float32)iHeight / (float32)iWidth);
	fDistance = (fAcross > fUp) ? fAcross : fUp;
	return (fDistance > BATTLE_VIEW_MIN_DISTANCE) ? fDistance : BATTLE_VIEW_MIN_DISTANCE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CBattleCanvasDraw::GetDistance(void) const
{
	return mfDistance;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether it's drawn with Vulkan, rather than by the rasteriser.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleCanvasDraw::IsVulkan(void) const
{
	return mbVulkan;
}


//////////////////////////////////////////////////////////////////////////
//
// The scene has moved: the canvas is drawn again at the next paint.
//
//////////////////////////////////////////////////////////////////////////
void CBattleCanvasDraw::Redraw(void)
{
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
// The view is moved towards where a canvas of this size shows the span:
// all the way the first time, and after that the further the more time
// has passed since it was last moved.
//
//////////////////////////////////////////////////////////////////////////
void CBattleCanvasDraw::MoveView(int32 iWidth, int32 iHeight)
{
	float32		fWanted;
	float32		fShare;

	fWanted = DistanceFor(iWidth, iHeight);
	if (mfDistance <= 0.0f)
	{
		mfDistance = fWanted;
	}
	else
	{
		fShare = mfUnmoved * BATTLE_VIEW_CLOSING;
		fShare = (fShare > 1.0f) ? 1.0f : fShare;
		mfDistance += (fWanted - mfDistance) * fShare;
	}
	mfUnmoved = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// The view is moved, then the scene is drawn from straight above.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2	sSize;

	sSize = pCanvas->GetActualSize();
	MoveView(sSize.x, sSize.y);

	msView.sTarget.Init(0.0f, 0.0f, 0.0f);
	msView.fYaw = 0.0f;
	msView.fPitch = BATTLE_VIEW_STRAIGHT_DOWN;
	msView.fDistance = mfDistance;
	msView.fFOV = TURNTABLE_FOV;
	return CSceneCanvasDraw::Draw(pCanvas);
}

