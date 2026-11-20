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
#include "StandardLib/Objects.h"
#include "SupportLib/Material.h"
#include "SupportLib/Mesh.h"
#include "BattleGeometry.h"
#include "BattleMeshes.h"


#define BATTLE_MESH_MAX_OUTLINE		16


//The plans of the Kestrel's parts: x, then z, with its nose along -z.
static const float32	gaafKestrelBody[5][2] = { { 0.0f, -4.6f }, { 0.95f, -1.4f }, { 1.15f, 2.6f }, { -1.15f, 2.6f }, { -0.95f, -1.4f } };
static const float32	gaafKestrelRightWing[4][2] = { { 0.9f, -1.0f }, { 3.5f, 2.0f }, { 3.5f, 3.0f }, { 0.9f, 2.4f } };
static const float32	gaafKestrelLeftWing[4][2] = { { -0.9f, -1.0f }, { -0.9f, 2.4f }, { -3.5f, 3.0f }, { -3.5f, 2.0f } };
static const float32	gaafKestrelCanopy[4][2] = { { 0.0f, -2.6f }, { 0.5f, -1.3f }, { 0.0f, -0.4f }, { -0.5f, -1.3f } };

//The plans of the Bulwark's parts.
static const float32	gaafBulwarkHull[6][2] = { { -2.0f, -6.0f }, { 2.0f, -6.0f }, { 4.4f, -1.0f }, { 4.4f, 5.0f }, { -4.4f, 5.0f }, { -4.4f, -1.0f } };
static const float32	gaafBulwarkRightPod[4][2] = { { 4.2f, -0.5f }, { 5.8f, 0.5f }, { 5.8f, 5.6f }, { 4.2f, 5.6f } };
static const float32	gaafBulwarkLeftPod[4][2] = { { -4.2f, -0.5f }, { -4.2f, 5.6f }, { -5.8f, 5.6f }, { -5.8f, 0.5f } };
static const float32	gaafBulwarkTower[6][2] = { { -1.2f, 0.4f }, { 1.2f, 0.4f }, { 2.0f, 2.0f }, { 1.2f, 3.8f }, { -1.2f, 3.8f }, { -2.0f, 2.0f } };

//The plan of the seeker's fins that lie flat.
static const float32	gaafSeekerFins[4][2] = { { -1.1f, 0.6f }, { 1.1f, 0.6f }, { 1.1f, 1.3f }, { -1.1f, 1.3f } };

//The planet's bands, from its top: alpha, red, green, blue.
static const float32	gaafPlanetBands[4][4] = { { 1.0f, 0.78f, 0.56f, 0.36f }, { 1.0f, 0.62f, 0.40f, 0.28f }, { 1.0f, 0.86f, 0.70f, 0.50f }, { 1.0f, 0.55f, 0.36f, 0.30f } };

//What colours stars are: red, green, blue.
static const float32	gaafStarColours[3][3] = { { 1.0f, 1.0f, 1.0f }, { 0.7f, 0.8f, 1.0f }, { 1.0f, 0.9f, 0.7f } };


//A mesh being made: its editor, and the material slot the next faces are given.
struct SBattleMeshMaker
{
	CMeshEditor*	pcEditor;
	int				iSlot;
};


//////////////////////////////////////////////////////////////////////////
//
// An empty mesh with flat normals and materials for its faces.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> BattleMeshBegin(SBattleMeshMaker* psMaker)
{
	Ptr<CMeshEditor>	pEditor;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->mpMesh->mcNormals.mbInUse = true;
	pEditor->mpMesh->mcMaterials.mbInUse = true;
	psMaker->pcEditor = &pEditor;
	psMaker->iSlot = -1;
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// The faces added from here on are this colour, and see-through if its
// alpha is less than one.
//
//////////////////////////////////////////////////////////////////////////
static void BattleMeshColour(SBattleMeshMaker* psMaker, float32 fAlpha, float32 fRed, float32 fGreen, float32 fBlue)
{
	Ptr<CMaterial>	pMaterial;
	CMesh*			pcMesh;

	pcMesh = &psMaker->pcEditor->mpMesh;
	pMaterial = OMalloc<CMaterial>();
	pMaterial->msProperties.mcDiffuseColour.Init(fAlpha, fRed, fGreen, fBlue);
	pcMesh->mcMaterials.mapMaterials.Add(pMaterial);
	psMaker->iSlot = (int)pcMesh->mcMaterials.mapMaterials.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle with corners of its own, wound so that it faces away from
// psInside whatever order they're given in.
//
//////////////////////////////////////////////////////////////////////////
static void BattleMeshTriangle(SBattleMeshMaker* psMaker, SFloat32Vec3* psA, SFloat32Vec3* psB, SFloat32Vec3* psC, SFloat32Vec3* psInside)
{
	CMesh*			pcMesh;
	SFloat32Vec3	sAB;
	SFloat32Vec3	sAC;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sOut;
	int				iFirst;

	sAB.Init(psB->x - psA->x, psB->y - psA->y, psB->z - psA->z);
	sAC.Init(psC->x - psA->x, psC->y - psA->y, psC->z - psA->z);
	sNormal.Init(sAB.y * sAC.z - sAB.z * sAC.y, sAB.z * sAC.x - sAB.x * sAC.z, sAB.x * sAC.y - sAB.y * sAC.x);
	if ((sNormal.x == 0.0f) && (sNormal.y == 0.0f) && (sNormal.z == 0.0f))
	{
		//No area: corners that are the same place, as at a cone's point.
		return;
	}
	sOut.Init((psA->x + psB->x + psC->x) / 3.0f - psInside->x, (psA->y + psB->y + psC->y) / 3.0f - psInside->y, (psA->z + psB->z + psC->z) / 3.0f - psInside->z);

	pcMesh = &psMaker->pcEditor->mpMesh;
	iFirst = psMaker->pcEditor->NumCorners();
	psMaker->pcEditor->AddCorner(psA);
	if (sNormal.x * sOut.x + sNormal.y * sOut.y + sNormal.z * sOut.z >= 0.0f)
	{
		psMaker->pcEditor->AddCorner(psB);
		psMaker->pcEditor->AddCorner(psC);
	}
	else
	{
		psMaker->pcEditor->AddCorner(psC);
		psMaker->pcEditor->AddCorner(psB);
	}
	psMaker->pcEditor->AddFace(iFirst, iFirst + 1, iFirst + 2);
	pcMesh->mcMaterials.mcFaces.SetValue((size)pcMesh->NumFaces() - 1, psMaker->iSlot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void BattleMeshQuad(SBattleMeshMaker* psMaker, SFloat32Vec3* psA, SFloat32Vec3* psB, SFloat32Vec3* psC, SFloat32Vec3* psD, SFloat32Vec3* psInside)
{
	BattleMeshTriangle(psMaker, psA, psB, psC, psInside);
	BattleMeshTriangle(psMaker, psA, psC, psD, psInside);
}


//////////////////////////////////////////////////////////////////////////
//
// A slab whose plan is the convex outline asPlan (x, then z) from fBottom
// up to fTop, where it's fTaper the size about the plan's middle: 1 for
// straight sides, less for sides that lean in.
//
//////////////////////////////////////////////////////////////////////////
static void BattleMeshSlab(SBattleMeshMaker* psMaker, const float32 (*asPlan)[2], int iNumPlan, float32 fBottom, float32 fTop, float32 fTaper)
{
	SFloat32Vec3	asLower[BATTLE_MESH_MAX_OUTLINE];
	SFloat32Vec3	asUpper[BATTLE_MESH_MAX_OUTLINE];
	SFloat32Vec3	sInside;
	float32			fMiddleX;
	float32			fMiddleZ;
	int				i;
	int				iNext;

	fMiddleX = 0.0f;
	fMiddleZ = 0.0f;
	for (i = 0; i < iNumPlan; i++)
	{
		fMiddleX += asPlan[i][0] / (float32)iNumPlan;
		fMiddleZ += asPlan[i][1] / (float32)iNumPlan;
	}
	for (i = 0; i < iNumPlan; i++)
	{
		asLower[i].Init(asPlan[i][0], fBottom, asPlan[i][1]);
		asUpper[i].Init(fMiddleX + (asPlan[i][0] - fMiddleX) * fTaper, fTop, fMiddleZ + (asPlan[i][1] - fMiddleZ) * fTaper);
	}
	sInside.Init(fMiddleX, (fBottom + fTop) * 0.5f, fMiddleZ);

	for (i = 0; i < iNumPlan; i++)
	{
		iNext = (i + 1) % iNumPlan;
		BattleMeshQuad(psMaker, &asLower[i], &asLower[iNext], &asUpper[iNext], &asUpper[i], &sInside);
		if ((i > 0) && (iNext != 0))
		{
			BattleMeshTriangle(psMaker, &asLower[0], &asLower[i], &asLower[iNext], &sInside);
			BattleMeshTriangle(psMaker, &asUpper[0], &asUpper[i], &asUpper[iNext], &sInside);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Round the line through fX, fY that runs along z: from fStartZ, where
// its radius is fStartRadius, to fEndZ and fEndRadius, with iSides flat
// sides and both ends closed.  A radius of nothing makes a point.
//
//////////////////////////////////////////////////////////////////////////
static void BattleMeshTube(SBattleMeshMaker* psMaker, float32 fX, float32 fY, float32 fStartZ, float32 fStartRadius, float32 fEndZ, float32 fEndRadius, int iSides)
{
	SFloat32Vec3	sStart;
	SFloat32Vec3	sEnd;
	SFloat32Vec3	sInside;
	SFloat32Vec3	asRing[4];
	float32			fAngle;
	float32			fNext;
	int				i;

	sStart.Init(fX, fY, fStartZ);
	sEnd.Init(fX, fY, fEndZ);
	sInside.Init(fX, fY, (fStartZ + fEndZ) * 0.5f);
	for (i = 0; i < iSides; i++)
	{
		fAngle = BATTLE_TWO_PI * (float32)i / (float32)iSides;
		fNext = BATTLE_TWO_PI * (float32)(i + 1) / (float32)iSides;
		asRing[0].Init(fX + cosf(fAngle) * fStartRadius, fY + sinf(fAngle) * fStartRadius, fStartZ);
		asRing[1].Init(fX + cosf(fNext) * fStartRadius, fY + sinf(fNext) * fStartRadius, fStartZ);
		asRing[2].Init(fX + cosf(fNext) * fEndRadius, fY + sinf(fNext) * fEndRadius, fEndZ);
		asRing[3].Init(fX + cosf(fAngle) * fEndRadius, fY + sinf(fAngle) * fEndRadius, fEndZ);
		BattleMeshQuad(psMaker, &asRing[0], &asRing[1], &asRing[2], &asRing[3], &sInside);
		BattleMeshTriangle(psMaker, &sStart, &asRing[0], &asRing[1], &sInside);
		BattleMeshTriangle(psMaker, &sEnd, &asRing[3], &asRing[2], &sInside);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A ball of iSides sides round and half as many rings from top to bottom.
// With afBands, each ring's faces are a colour of their own: iNumBands
// alpha, red, green, blue fours, gone through from the top and round
// again.
//
//////////////////////////////////////////////////////////////////////////
static void BattleMeshBall(SBattleMeshMaker* psMaker, float32 fX, float32 fY, float32 fZ, float32 fRadius, int iSides, const float32 (*afBands)[4], int iNumBands)
{
	SFloat32Vec3	sInside;
	SFloat32Vec3	asPatch[4];
	float32			afAcross[2];
	float32			afUp[2];
	float32			fAngle;
	float32			fNext;
	int				aiBandSlots[8];
	int				iRings;
	int				iRing;
	int				i;
	int				j;

	for (i = 0; (i < iNumBands) && (i < 8); i++)
	{
		BattleMeshColour(psMaker, afBands[i][0], afBands[i][1], afBands[i][2], afBands[i][3]);
		aiBandSlots[i] = psMaker->iSlot;
	}

	iRings = iSides / 2;
	sInside.Init(fX, fY, fZ);
	for (iRing = 0; iRing < iRings; iRing++)
	{
		if (iNumBands > 0)
		{
			psMaker->iSlot = aiBandSlots[iRing % iNumBands];
		}
		for (j = 0; j < 2; j++)
		{
			fAngle = BATTLE_PI * (float32)(iRing + j) / (float32)iRings;
			afAcross[j] = sinf(fAngle) * fRadius;
			afUp[j] = cosf(fAngle) * fRadius;
		}
		for (i = 0; i < iSides; i++)
		{
			fAngle = BATTLE_TWO_PI * (float32)i / (float32)iSides;
			fNext = BATTLE_TWO_PI * (float32)(i + 1) / (float32)iSides;
			asPatch[0].Init(fX + cosf(fAngle) * afAcross[0], fY + afUp[0], fZ + sinf(fAngle) * afAcross[0]);
			asPatch[1].Init(fX + cosf(fNext) * afAcross[0], fY + afUp[0], fZ + sinf(fNext) * afAcross[0]);
			asPatch[2].Init(fX + cosf(fNext) * afAcross[1], fY + afUp[1], fZ + sinf(fNext) * afAcross[1]);
			asPatch[3].Init(fX + cosf(fAngle) * afAcross[1], fY + afUp[1], fZ + sinf(fAngle) * afAcross[1]);
			if (iRing == 0)
			{
				BattleMeshTriangle(psMaker, &asPatch[0], &asPatch[2], &asPatch[3], &sInside);
			}
			else if (iRing == iRings - 1)
			{
				BattleMeshTriangle(psMaker, &asPatch[0], &asPatch[1], &asPatch[2], &sInside);
			}
			else
			{
				BattleMeshQuad(psMaker, &asPatch[0], &asPatch[1], &asPatch[2], &asPatch[3], &sInside);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A dart: a slim body, swept wings with a gun under each, where its bolts
// come from, and a canopy.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshKestrel(void)
{
	SBattleMeshMaker		sMaker;
	Ptr<CMeshEditor>		pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Kestrel");

	BattleMeshColour(&sMaker, 1.0f, 0.70f, 0.76f, 0.84f);
	BattleMeshSlab(&sMaker, gaafKestrelBody, 5, -0.45f, 0.55f, 0.55f);

	BattleMeshColour(&sMaker, 1.0f, 0.12f, 0.55f, 0.62f);
	BattleMeshSlab(&sMaker, gaafKestrelRightWing, 4, -0.18f, 0.18f, 0.9f);
	BattleMeshSlab(&sMaker, gaafKestrelLeftWing, 4, -0.18f, 0.18f, 0.9f);

	BattleMeshColour(&sMaker, 1.0f, 0.22f, 0.24f, 0.28f);
	BattleMeshTube(&sMaker, 2.2f, -0.25f, 1.6f, 0.24f, -2.5f, 0.2f, 6);
	BattleMeshTube(&sMaker, -2.2f, -0.25f, 1.6f, 0.24f, -2.5f, 0.2f, 6);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, 2.6f, 0.62f, 3.2f, 0.5f, 8);

	BattleMeshColour(&sMaker, 1.0f, 1.0f, 0.72f, 0.2f);
	BattleMeshSlab(&sMaker, gaafKestrelCanopy, 4, 0.5f, 0.85f, 0.4f);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// What comes out of the Kestrel's engine while it thrusts.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshKestrelFlame(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Kestrel Flame");
	BattleMeshColour(&sMaker, 0.8f, 0.45f, 0.85f, 1.0f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, 3.2f, 0.45f, 6.4f, 0.0f, 8);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A slab of a ship: a broad hull with a pod down each side, a tower
// towards the back, and at its nose the emitter its beam comes from.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshBulwark(void)
{
	SBattleMeshMaker		sMaker;
	Ptr<CMeshEditor>		pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Bulwark");

	BattleMeshColour(&sMaker, 1.0f, 0.74f, 0.47f, 0.24f);
	BattleMeshSlab(&sMaker, gaafBulwarkHull, 6, -1.0f, 1.1f, 0.78f);

	BattleMeshColour(&sMaker, 1.0f, 0.30f, 0.31f, 0.36f);
	BattleMeshSlab(&sMaker, gaafBulwarkRightPod, 4, -0.7f, 0.7f, 0.8f);
	BattleMeshSlab(&sMaker, gaafBulwarkLeftPod, 4, -0.7f, 0.7f, 0.8f);
	BattleMeshTube(&sMaker, 2.4f, 0.0f, 5.0f, 0.9f, 5.9f, 0.7f, 8);
	BattleMeshTube(&sMaker, -2.4f, 0.0f, 5.0f, 0.9f, 5.9f, 0.7f, 8);

	BattleMeshColour(&sMaker, 1.0f, 0.86f, 0.80f, 0.62f);
	BattleMeshSlab(&sMaker, gaafBulwarkTower, 6, 1.1f, 2.1f, 0.6f);

	BattleMeshColour(&sMaker, 1.0f, 0.25f, 0.9f, 1.0f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, -6.0f, 0.7f, -7.0f, 0.45f, 8);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshBulwarkFlame(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Bulwark Flame");
	BattleMeshColour(&sMaker, 0.8f, 1.0f, 0.62f, 0.2f);
	BattleMeshTube(&sMaker, 2.4f, 0.0f, 5.9f, 0.65f, 9.5f, 0.0f, 8);
	BattleMeshTube(&sMaker, -2.4f, 0.0f, 5.9f, 0.65f, 9.5f, 0.0f, 8);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A bright sliver, pointed at both ends, about its middle.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshBolt(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Bolt");
	BattleMeshColour(&sMaker, 1.0f, 0.85f, 1.0f, 0.35f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, -1.3f, 0.0f, 0.0f, 0.38f, 6);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, 0.0f, 0.38f, 1.3f, 0.0f, 6);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A stubby rocket with a bright nose and four fins.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshSeeker(void)
{
	SBattleMeshMaker		sMaker;
	Ptr<CMeshEditor>		pEditor;
	SFloat32Vec3			asFin[4];
	SFloat32Vec3			sInside;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Seeker");
	BattleMeshColour(&sMaker, 1.0f, 0.82f, 0.82f, 0.86f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, -0.6f, 0.42f, 1.3f, 0.42f, 8);
	BattleMeshSlab(&sMaker, gaafSeekerFins, 4, -0.07f, 0.07f, 1.0f);

	//The other two fins, up and down.
	asFin[0].Init(0.0f, -1.1f, 0.6f);
	asFin[1].Init(0.0f, 1.1f, 0.6f);
	asFin[2].Init(0.0f, 1.1f, 1.3f);
	asFin[3].Init(0.0f, -1.1f, 1.3f);
	sInside.Init(-1.0f, 0.0f, 0.95f);
	BattleMeshQuad(&sMaker, &asFin[0], &asFin[1], &asFin[2], &asFin[3], &sInside);
	sInside.Init(1.0f, 0.0f, 0.95f);
	BattleMeshQuad(&sMaker, &asFin[0], &asFin[1], &asFin[2], &asFin[3], &sInside);

	BattleMeshColour(&sMaker, 1.0f, 1.0f, 0.3f, 0.15f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, -1.7f, 0.0f, -0.6f, 0.42f, 8);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A unit of beam: from the origin a unit along -z.  It's drawn stretched
// to the beam's length.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshBeam(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Beam");
	BattleMeshColour(&sMaker, 1.0f, 0.85f, 1.0f, 1.0f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, 0.0f, 0.16f, -1.0f, 0.16f, 6);
	BattleMeshColour(&sMaker, 0.45f, 0.2f, 0.8f, 1.0f);
	BattleMeshTube(&sMaker, 0.0f, 0.0f, 0.0f, 0.5f, -1.0f, 0.5f, 6);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A banded ball, its bands running the way the ships fly round it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshPlanet(float32 fRadius)
{
	SBattleMeshMaker		sMaker;
	Ptr<CMeshEditor>		pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Planet");
	BattleMeshBall(&sMaker, 0.0f, 0.0f, 0.0f, fRadius, 32, gaafPlanetBands, 4);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// The flash where something was struck: a ball a unit in radius, drawn
// the size the hit is.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshHit(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Hit");
	BattleMeshColour(&sMaker, 0.75f, 1.0f, 0.95f, 0.6f);
	BattleMeshBall(&sMaker, 0.0f, 0.0f, 0.0f, 1.0f, 8, NULL, 0);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A burst's ring, a unit in radius and flat, drawn growing out to the
// burst's range.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshBurst(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;
	SFloat32Vec3		asPatch[4];
	SFloat32Vec3		sInside;
	float32				fAngle;
	float32				fNext;
	int					i;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Burst");
	BattleMeshColour(&sMaker, 0.6f, 0.75f, 0.9f, 1.0f);

	//Seen from above, so its faces face up.
	sInside.Init(0.0f, -1.0f, 0.0f);
	for (i = 0; i < 32; i++)
	{
		fAngle = BATTLE_TWO_PI * (float32)i / 32.0f;
		fNext = BATTLE_TWO_PI * (float32)(i + 1) / 32.0f;
		asPatch[0].Init(cosf(fAngle) * 0.86f, 0.0f, sinf(fAngle) * 0.86f);
		asPatch[1].Init(cosf(fNext) * 0.86f, 0.0f, sinf(fNext) * 0.86f);
		asPatch[2].Init(cosf(fNext), 0.0f, sinf(fNext));
		asPatch[3].Init(cosf(fAngle), 0.0f, sinf(fAngle));
		BattleMeshQuad(&sMaker, &asPatch[0], &asPatch[1], &asPatch[2], &asPatch[3], &sInside);
	}
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a ship went: a ball in a ball, a unit in radius.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshExplosion(void)
{
	SBattleMeshMaker	sMaker;
	Ptr<CMeshEditor>	pEditor;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Explosion");
	BattleMeshColour(&sMaker, 0.9f, 1.0f, 0.95f, 0.7f);
	BattleMeshBall(&sMaker, 0.0f, 0.0f, 0.0f, 0.5f, 12, NULL, 0);
	BattleMeshColour(&sMaker, 0.5f, 1.0f, 0.45f, 0.1f);
	BattleMeshBall(&sMaker, 0.0f, 0.0f, 0.0f, 1.0f, 12, NULL, 0);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// A tile of stars BATTLE_STAR_TILE across, from its corner at the origin
// along +x and +z, at depths below where the ships fly so that the nearer
// ones pass faster.  They're the same stars every time.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMeshEditor> BattleMeshStars(void)
{
	SBattleMeshMaker		sMaker;
	Ptr<CMeshEditor>		pEditor;
	SFloat32Vec3			asPatch[4];
	SFloat32Vec3			sInside;
	uint32					uiSeed;
	float32					afRandom[4];
	float32					fX;
	float32					fY;
	float32					fZ;
	float32					fSize;
	int						aiSlots[3];
	int						i;
	int						j;

	pEditor = BattleMeshBegin(&sMaker);
	pEditor->mszName.Set("Stars");
	for (i = 0; i < 3; i++)
	{
		BattleMeshColour(&sMaker, 1.0f, gaafStarColours[i][0], gaafStarColours[i][1], gaafStarColours[i][2]);
		aiSlots[i] = sMaker.iSlot;
	}

	uiSeed = 20261120;
	for (i = 0; i < BATTLE_STAR_COUNT; i++)
	{
		for (j = 0; j < 4; j++)
		{
			uiSeed = uiSeed * 1664525 + 1013904223;
			afRandom[j] = (float32)(uiSeed >> 8) / 16777216.0f;
		}
		fX = afRandom[0] * BATTLE_STAR_TILE;
		fZ = afRandom[1] * BATTLE_STAR_TILE;
		fY = -30.0f - afRandom[2] * 150.0f;
		fSize = 0.14f + afRandom[3] * afRandom[3] * 0.4f;
		sMaker.iSlot = aiSlots[i % 3];

		//Flat, facing up.
		asPatch[0].Init(fX - fSize, fY, fZ - fSize);
		asPatch[1].Init(fX + fSize, fY, fZ - fSize);
		asPatch[2].Init(fX + fSize, fY, fZ + fSize);
		asPatch[3].Init(fX - fSize, fY, fZ + fSize);
		sInside.Init(fX, fY - 1.0f, fZ);
		BattleMeshQuad(&sMaker, &asPatch[0], &asPatch[1], &asPatch[2], &asPatch[3], &sInside);
	}
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the ship type the outline of the mesh seen from above: what's
// round all of its corners.  False, leaving the type's outline as it was,
// for a mesh that has none or whose outline has too many corners.
//
//////////////////////////////////////////////////////////////////////////
bool BattleMeshOutline(Ptr<CMeshEditor> pEditor, CBattleShipType* pcType)
{
	Ptr<CMesh>			pMesh;
	CArrayFloat32Vec2	asPoints;
	SFloat32Vec3*		psPosition;
	SFloat32Vec2*		psPoint;
	bool				bResult;
	int					i;

	pMesh = pEditor->GetOutput();
	asPoints.Init();
	for (i = 0; i < pMesh->NumCorners(); i++)
	{
		psPosition = pMesh->GetPosition(i);
		psPoint = asPoints.Add();
		psPoint->x = psPosition->x;
		psPoint->y = -psPosition->z;
	}
	bResult = pcType->SetOutline(asPoints.GetData(), asPoints.NumElements());
	asPoints.Kill();
	return bResult;
}

