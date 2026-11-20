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
#ifndef __BATTLE_MESHES_H__
#define __BATTLE_MESHES_H__
#include "SupportLib/MeshEditor.h"
#include "BattleShipType.h"


#define BATTLE_STAR_TILE	400.0f	//A tile of stars is this far across and up.
#define BATTLE_STAR_COUNT	420


//The meshes a battle is drawn with, each made here rather than read from a file.  A scene is right handed with y up, and a
//battle is seen from above: a mesh faces along -z, its right is +x, and a battle's x and y are a scene's x and -z.
//
//A ship's mesh is about the ship's middle.  BattleMeshOutline gives a ship type the outline of a mesh seen from above,
//which is then what things hit, so a ship collides as it looks whatever its mesh is.

Ptr<CMeshEditor>	BattleMeshKestrel(void);
Ptr<CMeshEditor>	BattleMeshKestrelFlame(void);
Ptr<CMeshEditor>	BattleMeshBulwark(void);
Ptr<CMeshEditor>	BattleMeshBulwarkFlame(void);
Ptr<CMeshEditor>	BattleMeshBolt(void);
Ptr<CMeshEditor>	BattleMeshSeeker(void);
Ptr<CMeshEditor>	BattleMeshBeam(void);
Ptr<CMeshEditor>	BattleMeshPlanet(float32 fRadius);
Ptr<CMeshEditor>	BattleMeshHit(void);
Ptr<CMeshEditor>	BattleMeshBurst(void);
Ptr<CMeshEditor>	BattleMeshExplosion(void);
Ptr<CMeshEditor>	BattleMeshStars(void);

bool				BattleMeshOutline(Ptr<CMeshEditor> pEditor, CBattleShipType* pcType);


#endif // __BATTLE_MESHES_H__

