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
#ifndef __BATTLE_CANVAS_DRAW_H__
#define __BATTLE_CANVAS_DRAW_H__
#include "SceneViewLib/SceneCanvasDraw.h"


#define BATTLE_VIEW_BACKGROUND		0xFF04050A	//Space, 0xAARRGGBB.
#define BATTLE_VIEW_MIN_DISTANCE	90.0f		//The nearest the view comes, however close the ships are.
#define BATTLE_VIEW_CLOSING			3.0f		//How fast the view moves to where it should be: this share of what's left, a second.


//Draws a battle's scene from straight above its middle (the scene's origin), from as far off as shows what it's told
//has to be seen (SetSpan): a distance either side of the middle across the canvas, and one up and down it.  The view
//moves out and in to that smoothly, by the time SetSpan says has passed, and never comes nearer than
//BATTLE_VIEW_MIN_DISTANCE.
//
//It is a scene canvas draw whose view isn't turned by anything, so it's drawn with Vulkan when there's a device for it
//and by the rasteriser otherwise, and SwitchRenderer goes between them.
class CBattleCanvasDraw : public CSceneCanvasDraw
{
CONSTRUCTABLE(CBattleCanvasDraw);
DESTRUCTABLE(CBattleCanvasDraw);
protected:
	float32		mfHalfAcross;
	float32		mfHalfUp;
	float32		mfDistance;  //Where the view is, 0 before it's first drawn.
	float32		mfUnmoved;  //Seconds that have passed since the view was last moved.

public:
	void	Init(Ptr<CWindowTheme> pTheme);
	void	Class(void) override;

	void	SetSpan(float32 fHalfAcross, float32 fHalfUp, float32 fSeconds);
	float32	GetDistance(void) const;
	bool	IsVulkan(void) const;
	float32	DistanceFor(int32 iWidth, int32 iHeight) const;
	void	MoveView(int32 iWidth, int32 iHeight);
	void	Redraw(void);
	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


#endif // __BATTLE_CANVAS_DRAW_H__

