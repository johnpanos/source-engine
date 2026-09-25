//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Typed in-memory map input for USD-authored maps (RFC 0009 U1).
//
// Reads a `source-authored-brushset/v1` document (see authoredmap.h and
// quality/usd_authoring/source_compile_v1.json) and fills the same CMapFile
// records the VMF loader fills: entities with their key/value pairs, and one
// mapbrush_t per authored convex solid with one side_t per authored face.
// Everything after loading (CSG, BSP, portals, leak detection, faces, static
// props, the BSP writer) is vbsp's normal path.
//
//=============================================================================//

#include "vbsp.h"
#include "map_shared.h"
#include "authoredmap.h"
#include "materialpatch.h"
#include "KeyValues.h"
#include "tier1/keyvaluesjson.h"
#include "tier1/strtools.h"

#include <math.h>
#include <stdio.h>

bool g_bAuthoredInput = false;

extern CMapError g_MapError;
bool IsAreaPortal( char const *pClassName );

static const char *const s_pszBrushSetSchema = "source-authored-brushset/v1";

// A reconstructed side must reproduce its authored polygon to this distance.
static const float s_flAuthoredPointTolerance = 0.05f;
// The plane vbsp snaps from three authored corners must match the authored one.
static const float s_flAuthoredNormalDot = 0.9999f;
static const float s_flAuthoredDistTolerance = 0.01f;
static const int s_nMaxBrushSetBytes = 64 * 1024 * 1024;

// Classes that the VMF loader rewrites while loading (moving brushes to the
// world, collecting cubemaps, overlays, instances, ...). The typed input does
// not reproduce those rewrites, so it rejects them instead of emitting them
// unchanged.
static const char *const s_pszLoaderRewrittenClasses[] = {
    "func_detail",
    "func_viscluster",
    "func_ladder",
    "env_cubemap",
    "test_sidelist",
    "info_overlay",
    "info_overlay_transition",
    "info_no_dynamic_shadow",
    "func_instance",
    "func_instance_parms",
};

//-----------------------------------------------------------------------------
// Typed field access. Every failure is fatal and names the authored object.
//-----------------------------------------------------------------------------
static void AuthoredError( const char *pszContext, const char *pszFormat, ... )
{
	char message[1024];
	va_list args;
	va_start( args, pszFormat );
	V_vsnprintf( message, sizeof( message ), pszFormat, args );
	va_end( args );
	Error( "authored map: %s: %s\n", pszContext, message );
}

static bool ReadNumber( KeyValues *pValue, double &value )
{
	switch ( pValue->GetDataType() )
	{
	case KeyValues::TYPE_INT:
		value = pValue->GetInt();
		return true;
	case KeyValues::TYPE_FLOAT:
		value = pValue->GetFloat();
		return true;
	default:
		return false;
	}
}

static const char *RequireString( KeyValues *pObject, const char *pszKey, const char *pszContext )
{
	KeyValues *pValue = pObject->FindKey( pszKey );
	if ( !pValue || pValue->GetDataType() != KeyValues::TYPE_STRING || !pValue->GetString()[0] )
		AuthoredError( pszContext, "\"%s\" must be a non-empty string", pszKey );
	return pValue->GetString();
}

static int RequirePositiveInt( KeyValues *pObject, const char *pszKey, const char *pszContext )
{
	KeyValues *pValue = pObject->FindKey( pszKey );
	if ( !pValue || pValue->GetDataType() != KeyValues::TYPE_INT || pValue->GetInt() <= 0 )
		AuthoredError( pszContext, "\"%s\" must be a positive integer", pszKey );
	return pValue->GetInt();
}

// Reads an array of exactly nCount finite numbers.
static void ReadNumbers(
    KeyValues *pArray, int nCount, double *pOut, const char *pszWhat, const char *pszContext )
{
	int nRead = 0;
	for ( KeyValues *pValue = pArray ? pArray->GetFirstSubKey() : NULL; pValue;
	    pValue = pValue->GetNextKey() )
	{
		if ( nRead == nCount || !ReadNumber( pValue, pOut[nRead] ) || !isfinite( pOut[nRead] ) )
			AuthoredError( pszContext, "\"%s\" must be %d finite numbers", pszWhat, nCount );
		++nRead;
	}
	if ( nRead != nCount )
		AuthoredError( pszContext, "\"%s\" must be %d finite numbers", pszWhat, nCount );
}

static void RequireNumbers(
    KeyValues *pObject, const char *pszKey, int nCount, double *pOut, const char *pszContext )
{
	KeyValues *pArray = pObject->FindKey( pszKey );
	if ( !pArray || pArray->GetDataType() != KeyValues::TYPE_NONE )
		AuthoredError( pszContext, "\"%s\" must be an array", pszKey );
	ReadNumbers( pArray, nCount, pOut, pszKey, pszContext );
}

static void SetVectorKey( entity_t *pEntity, const char *pszKey, const double *pValues )
{
	char value[128];
	V_snprintf( value, sizeof( value ), "%.9g %.9g %.9g", pValues[0], pValues[1], pValues[2] );
	SetKeyValue( pEntity, pszKey, value );
}

//-----------------------------------------------------------------------------
// One authored face becomes one brush side.
//-----------------------------------------------------------------------------
struct AuthoredSide_t
{
	CUtlVector<Vector> m_Points;
	Vector m_Normal;
	double m_flDist;
};

// Texture axes from the authored affine st, in texture repeats per Source
// unit: s = S.p + s0 and t = T.p + t0. st follows USD's convention (t up), so
// the texel row is (1 - t) * height; see source_compile_v1.json "texture".
static void AuthoredTexture(
    KeyValues *pSide, const char *pszMaterial, brush_texture_t &td, const char *pszContext )
{
	bool bFound = false;
	MaterialSystemMaterial_t material = FindOriginalMaterial( pszMaterial, &bFound, false );
	if ( material == MATERIAL_NOT_FOUND || !bFound )
		AuthoredError( pszContext, "material \"%s\" is not in the game content", pszMaterial );
	int nWidth = 0;
	int nHeight = 0;
	GetMaterialDimensions( material, &nWidth, &nHeight );
	if ( nWidth <= 0 || nHeight <= 0 )
		AuthoredError( pszContext, "material \"%s\" has no texture size", pszMaterial );

	double s[4];
	double t[4];
	RequireNumbers( pSide, "s", 4, s, pszContext );
	RequireNumbers( pSide, "t", 4, t, pszContext );
	const Vector uTexels( s[0] * nWidth, s[1] * nWidth, s[2] * nWidth );
	const Vector vTexels( -t[0] * nHeight, -t[1] * nHeight, -t[2] * nHeight );
	const float flU = uTexels.Length();
	const float flV = vTexels.Length();
	if ( flU < 1e-6f || flV < 1e-6f )
		AuthoredError( pszContext, "st is degenerate" );

	td.UAxis = uTexels / flU;
	td.VAxis = vTexels / flV;
	td.textureWorldUnitsPerTexel[0] = 1.0f / flU;
	td.textureWorldUnitsPerTexel[1] = 1.0f / flV;
	td.shift[0] = s[3] * nWidth;
	td.shift[1] = ( 1.0 - t[3] ) * nHeight;
	td.rotate = 0;
}

static void ReadAuthoredPolygon( KeyValues *pSide, AuthoredSide_t &side, const char *pszContext )
{
	double plane[4];
	RequireNumbers( pSide, "plane", 4, plane, pszContext );
	side.m_Normal.Init( plane[0], plane[1], plane[2] );
	side.m_flDist = plane[3];
	if ( fabs( side.m_Normal.Length() - 1.0f ) > 1e-4f )
		AuthoredError( pszContext, "the plane normal is not unit length" );

	KeyValues *pPoints = pSide->FindKey( "points" );
	if ( !pPoints || pPoints->GetDataType() != KeyValues::TYPE_NONE )
		AuthoredError( pszContext, "\"points\" must be an array" );
	for ( KeyValues *pPoint = pPoints->GetFirstSubKey(); pPoint; pPoint = pPoint->GetNextKey() )
	{
		double xyz[3];
		ReadNumbers( pPoint, 3, xyz, "points", pszContext );
		side.m_Points.AddToTail( Vector( xyz[0], xyz[1], xyz[2] ) );
	}
	if ( side.m_Points.Count() < 3 )
		AuthoredError( pszContext, "a face needs at least three points" );
}

// vbsp derives a side's plane from three points, as it does for VMF sides:
// PlaneFromPoints snaps near-axial planes. Take the authored polygon's widest
// triangle, in the reverse order (the authored polygon winds outward
// counter-clockwise; PlaneFromPoints expects clockwise).
static int AuthoredPlane( CMapFile *pMap, const AuthoredSide_t &side, const char *pszContext )
{
	const CUtlVector<Vector> &p = side.m_Points;
	int nBest1 = 1;
	int nBest2 = 2;
	float flBestArea = -1.0f;
	for ( int i = 1; i < p.Count(); ++i )
	{
		for ( int j = i + 1; j < p.Count(); ++j )
		{
			const float flArea = CrossProduct( p[i] - p[0], p[j] - p[0] ).Length();
			if ( flArea > flBestArea )
			{
				flBestArea = flArea;
				nBest1 = i;
				nBest2 = j;
			}
		}
	}
	const int nPlane = pMap->PlaneFromPoints( p[nBest2], p[nBest1], p[0] );
	if ( nPlane < 0 )
		AuthoredError( pszContext, "the face has no plane" );
	const plane_t &plane = pMap->mapplanes[nPlane];
	if ( DotProduct( plane.normal, side.m_Normal ) < s_flAuthoredNormalDot ||
	     fabs( plane.dist - side.m_flDist ) > s_flAuthoredDistTolerance )
	{
		AuthoredError( pszContext,
		    "the plane from the face corners (%g %g %g, %g) differs from the authored plane "
		    "(%g %g %g, %g)",
		    plane.normal.x, plane.normal.y, plane.normal.z, plane.dist, side.m_Normal.x,
		    side.m_Normal.y, side.m_Normal.z, side.m_flDist );
	}
	return nPlane;
}

// The brush that vbsp rebuilds from the side planes must be the authored
// solid: every side keeps a winding with exactly the authored corners.
static void CheckAuthoredWindings(
    const mapbrush_t *b, const CUtlVector<AuthoredSide_t> &sides, const char *pszObject )
{
	for ( int i = 0; i < b->numsides; ++i )
	{
		const winding_t *w = b->original_sides[i].winding;
		const CUtlVector<Vector> &authored = sides[i].m_Points;
		char context[256];
		V_snprintf( context, sizeof( context ), "%s side %d", pszObject, i );
		if ( !w || w->numpoints != authored.Count() )
		{
			AuthoredError( context, "the rebuilt side has %d corners; the authored face has %d",
			    w ? w->numpoints : 0, authored.Count() );
		}
		for ( int k = 0; k < authored.Count(); ++k )
		{
			bool bMatched = false;
			for ( int m = 0; m < w->numpoints && !bMatched; ++m )
				bMatched = ( w->p[m] - authored[k] ).Length() <= s_flAuthoredPointTolerance;
			if ( !bMatched )
			{
				AuthoredError( context, "authored corner (%g %g %g) is not on the rebuilt side",
				    authored[k].x, authored[k].y, authored[k].z );
			}
		}
	}
}

static void LoadAuthoredSolid( CMapFile *pMap, entity_t *pEntity, KeyValues *pSolid )
{
	const char *pszObject = RequireString( pSolid, "id", "solid" );
	if ( pMap->nummapbrushes == MAX_MAP_BRUSHES )
		AuthoredError( pszObject, "more than %d brushes", MAX_MAP_BRUSHES );

	mapbrush_t *b = &pMap->mapbrushes[pMap->nummapbrushes];
	b->original_sides = &pMap->brushsides[pMap->nummapbrushsides];
	b->entitynum = pMap->num_entities - 1;
	b->brushnum = pMap->nummapbrushes - pEntity->firstbrush;
	b->id = RequirePositiveInt( pSolid, "brush_key", pszObject );
	b->numsides = 0;
	g_MapError.BrushState( b->id );

	KeyValues *pSides = pSolid->FindKey( "sides" );
	if ( !pSides || pSides->GetDataType() != KeyValues::TYPE_NONE || !pSides->GetFirstSubKey() )
		AuthoredError( pszObject, "\"sides\" must be a non-empty array" );

	CUtlVector<AuthoredSide_t> authored;
	for ( KeyValues *pSide = pSides->GetFirstSubKey(); pSide; pSide = pSide->GetNextKey() )
	{
		char context[256];
		V_snprintf( context, sizeof( context ), "%s side %d", pszObject, b->numsides );
		if ( pMap->nummapbrushsides == MAX_MAP_BRUSHSIDES || b->numsides == MAX_BRUSH_SIDES )
			AuthoredError( context, "too many brush sides" );
		g_MapError.BrushSide( b->numsides );

		AuthoredSide_t &geometry = authored[authored.AddToTail()];
		ReadAuthoredPolygon( pSide, geometry, context );

		side_t *side = &pMap->brushsides[pMap->nummapbrushsides];
		side->pMapDisp = NULL;
		side->id = RequirePositiveInt( pSide, "side_key", context );

		brush_texture_t td;
		const char *pszMaterial = RequireString( pSide, "material", context );
		if ( V_strlen( pszMaterial ) >= (int)sizeof( td.name ) )
			AuthoredError( context, "material name is too long" );
		V_strncpy( td.name, pszMaterial, sizeof( td.name ) );
		g_MapError.TextureState( td.name );
		AuthoredTexture( pSide, td.name, td, context );

		// Default flags and contents from the material, as for a VMF side.
		const int nMiptex = FindMiptex( td.name );
		td.flags = textureref[nMiptex].flags;
		td.lightmapWorldUnitsPerLuxel = ScaleLightmapWorldUnitsPerLuxel(
		    RequirePositiveInt( pSide, "lightmap_scale", context ) );
		side->contents = textureref[nMiptex].contents;
		side->surf = td.flags;
		FinishSideContents( side );

		const int nPlane = AuthoredPlane( pMap, geometry, context );
		for ( int k = 0; k < b->numsides; ++k )
		{
			const int nOther = b->original_sides[k].planenum;
			if ( nOther == nPlane || nOther == ( nPlane ^ 1 ) )
				AuthoredError( context, "the face repeats the plane of side %d", k );
		}
		side->planenum = nPlane;
		side->texinfo = TexinfoForBrushTexture( &pMap->mapplanes[nPlane], &td, vec3_origin );
		pMap->side_brushtextures[pMap->nummapbrushsides] = td;
		pMap->nummapbrushsides++;
		b->numsides++;
	}

	b->contents = BrushContents( b );
	if ( b->entitynum == 0 && ( b->contents & ( CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP ) ) )
		AuthoredError( pszObject, "clip contents are not part of the authored profile" );
	if ( b->contents & CONTENTS_ORIGIN )
		AuthoredError( pszObject, "origin brushes are not part of the authored profile" );

	pMap->MakeBrushWindings( b );
	CheckAuthoredWindings( b, authored, pszObject );
	pMap->AddBrushBevels( b );

	pMap->nummapbrushes++;
	pEntity->numbrushes++;
}

//-----------------------------------------------------------------------------
// Entities: typed origin and angles, string keys, and optional solids.
//-----------------------------------------------------------------------------
static void LoadAuthoredEntity( CMapFile *pMap, KeyValues *pRecord )
{
	const int nIndex = pMap->num_entities;
	const char *pszClass = RequireString( pRecord, "classname", "entity" );
	KeyValues *pId = pRecord->FindKey( "id" );
	const char *pszObject = pId ? RequireString( pRecord, "id", pszClass ) : pszClass;

	if ( ( nIndex == 0 ) != !V_strcmp( pszClass, "worldspawn" ) )
		AuthoredError( pszObject, "worldspawn must be the first entity, and only the first" );
	if ( ( nIndex == 0 ) == ( pId != NULL ) )
		AuthoredError( pszObject, "every entity but worldspawn carries an authored id" );
	if ( IsAreaPortal( pszClass ) )
		AuthoredError( pszObject, "class %s is not accepted by the typed input", pszClass );
	for ( int i = 0; i < ARRAYSIZE( s_pszLoaderRewrittenClasses ); ++i )
	{
		if ( !V_stricmp( pszClass, s_pszLoaderRewrittenClasses[i] ) )
			AuthoredError( pszObject, "class %s is not accepted by the typed input", pszClass );
	}
	if ( nIndex == MAX_MAP_ENTITIES )
		AuthoredError( pszObject, "more than %d entities", MAX_MAP_ENTITIES );

	entity_t *pEntity = &pMap->entities[pMap->num_entities++];
	memset( pEntity, 0, sizeof( *pEntity ) );
	pEntity->firstbrush = pMap->nummapbrushes;
	pEntity->numbrushes = 0;
	SetKeyValue( pEntity, "classname", pszClass );
	if ( pId )
		SetKeyValue( pEntity, "sourcemap_id", pId->GetString() );

	double vector[3];
	if ( pRecord->FindKey( "origin" ) )
	{
		RequireNumbers( pRecord, "origin", 3, vector, pszObject );
		SetVectorKey( pEntity, "origin", vector );
	}
	if ( pRecord->FindKey( "angles" ) )
	{
		RequireNumbers( pRecord, "angles", 3, vector, pszObject );
		SetVectorKey( pEntity, "angles", vector );
	}

	KeyValues *pSolids = pRecord->FindKey( "solids" );
	KeyValues *pKeys = pRecord->FindKey( "keys" );
	if ( pKeys && pKeys->GetDataType() != KeyValues::TYPE_NONE )
		AuthoredError( pszObject, "\"keys\" must be an object" );
	for ( KeyValues *pKey = pKeys ? pKeys->GetFirstSubKey() : NULL; pKey;
	    pKey = pKey->GetNextKey() )
	{
		const char *pszKey = pKey->GetName();
		if ( pKey->GetDataType() != KeyValues::TYPE_STRING )
			AuthoredError( pszObject, "key \"%s\" must have a string value", pszKey );
		if ( !V_stricmp( pszKey, "classname" ) || !V_stricmp( pszKey, "origin" ) ||
		     !V_stricmp( pszKey, "angles" ) || !V_stricmp( pszKey, "sourcemap_id" ) ||
		     ( pSolids && !V_stricmp( pszKey, "model" ) ) )
		{
			AuthoredError( pszObject, "key \"%s\" is owned by a typed field or by vbsp", pszKey );
		}
		SetKeyValue( pEntity, pszKey, pKey->GetString() );
	}

	if ( pSolids )
	{
		if ( pSolids->GetDataType() != KeyValues::TYPE_NONE )
			AuthoredError( pszObject, "\"solids\" must be an array" );
		for ( KeyValues *pSolid = pSolids->GetFirstSubKey(); pSolid; pSolid = pSolid->GetNextKey() )
		{
			LoadAuthoredSolid( pMap, pEntity, pSolid );
		}
	}

	GetVectorForKey( pEntity, "origin", pEntity->origin );
	if ( pEntity->numbrushes && nIndex != 0 && pEntity->origin != vec3_origin )
		AuthoredError( pszObject, "a brush entity with an origin is not supported" );
	if ( nIndex != 0 )
		pMap->RemoveContentsDetailFromEntity( pEntity );
}

void LoadAuthoredMapFile( const char *pszFileName )
{
	if ( g_MainMap )
		Error( "authored map: a map is already loaded\n" );

	FILE *pFile = fopen( pszFileName, "rb" );
	if ( !pFile )
		Error( "authored map: cannot open %s\n", pszFileName );
	fseek( pFile, 0, SEEK_END );
	const long nSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	if ( nSize <= 0 || nSize > s_nMaxBrushSetBytes )
	{
		fclose( pFile );
		Error( "authored map: %s is empty or too large\n", pszFileName );
	}
	CUtlVector<char> text;
	text.SetCount( nSize + 1 );
	const size_t nRead = fread( text.Base(), 1, nSize, pFile );
	fclose( pFile );
	if ( nRead != (size_t)nSize )
		Error( "authored map: cannot read %s\n", pszFileName );
	text[nSize] = '\0';

	KeyValuesJSONParser parser( text.Base(), nSize );
	KeyValues *pRoot = parser.ParseFile();
	if ( !pRoot )
		Error( "authored map: %s:%d: %s\n", pszFileName, parser.m_nLine, parser.m_szErrMsg );
	const char *pszSchema = RequireString( pRoot, "schema", pszFileName );
	if ( V_strcmp( pszSchema, s_pszBrushSetSchema ) )
		Error( "authored map: %s is %s, not %s\n", pszFileName, pszSchema, s_pszBrushSetSchema );

	Msg( "Loading authored brush set %s\n", pszFileName );
	// Texture axes are explicit, as in version 220+ map files.
	g_nMapFileVersion = 400;

	CMapFile *pMap = new CMapFile();
	g_Maps.AddToTail( pMap );
	g_LoadingMap = pMap;
	g_MainMap = pMap;

	KeyValues *pEntities = pRoot->FindKey( "entities" );
	if ( !pEntities || pEntities->GetDataType() != KeyValues::TYPE_NONE ||
	     !pEntities->GetFirstSubKey() )
	{
		Error( "authored map: %s has no entities\n", pszFileName );
	}
	for ( KeyValues *pRecord = pEntities->GetFirstSubKey(); pRecord;
	    pRecord = pRecord->GetNextKey() )
	{
		LoadAuthoredEntity( pMap, pRecord );
	}
	pRoot->deleteThis();

	if ( !pMap->entities[0].numbrushes )
		Error( "authored map: %s has no world solids\n", pszFileName );
	ReportLoadedMapBounds();
	PublishLoadedMap();
}
