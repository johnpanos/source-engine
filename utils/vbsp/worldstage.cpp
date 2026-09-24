//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Author the compiled world geometry as an OpenUSD layer.
//
//=============================================================================//

#include "worldstage.h"

#include "utils/worldstage/sourceEntityAPI.h"
#include "utils/worldstage/sourceLightAPI.h"
#include "utils/worldstage/sourceMeshAPI.h"
#include "utils/worldstage/sourceWorldAPI.h"

#include "pxr/base/gf/vec2f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec4i.h"
#include "pxr/base/vt/array.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
#include "pxr/usd/usdGeom/scope.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/xformable.h"
#include "pxr/usd/usdLux/sphereLight.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bsplib.h"
#include "../common/map_file_io.h"
#include "tier0/valve_minmax_off.h"

namespace
{

constexpr int kAtlasWidth = 1024;
constexpr int kAtlasMaxHeight = 4096;
constexpr int kChartGutter = 2;

struct FaceGeometry
{
	int faceId;
	int chartX;
	int chartY;
	int chartWidth;
	int chartHeight;
	std::string materialPath;
	std::vector<pxr::GfVec3f> points;
	std::vector<pxr::GfVec2f> materialUV;
	// Indices into points that bound the face's surface (see SurfaceCorners);
	// empty when the face encloses no area.
	std::vector<int> corners;
	pxr::GfVec3f normal;
	bool reverseWinding;
};

struct EntityRecord
{
	std::vector<std::string> keys;
	std::vector<std::string> values;
};

bool IsEntityWhitespace( char value )
{
	return value == ' ' || value == '\t' || value == '\r' || value == '\n' || value == '\v' ||
	       value == '\f';
}

bool ParseCompiledEntities( std::vector<EntityRecord> &records, std::string &error )
{
	if ( dentdata.Count() < 1 || dentdata[dentdata.Count() - 1] != '\0' )
	{
		error = "compiled BSP entity lump has no final NUL";
		return false;
	}
	const std::string_view source( dentdata.Base(), dentdata.Count() - 1 );
	std::size_t position = 0;
	const auto skipWhitespace = [&]()
	{
		while ( position < source.size() && IsEntityWhitespace( source[position] ) )
			++position;
	};
	const auto readQuoted = [&]( std::string &value ) -> bool
	{
		if ( position >= source.size() || source[position] != '"' )
			return false;
		const std::size_t begin = ++position;
		while ( position < source.size() && source[position] != '"' )
			++position;
		if ( position == source.size() )
			return false;
		value.assign( source.substr( begin, position - begin ) );
		++position;
		return true;
	};
	skipWhitespace();
	while ( position < source.size() )
	{
		if ( source[position++] != '{' )
		{
			error = "compiled BSP entity lump has an invalid entity opening";
			return false;
		}
		EntityRecord record;
		skipWhitespace();
		while ( position < source.size() && source[position] != '}' )
		{
			std::string key;
			std::string value;
			if ( !readQuoted( key ) )
			{
				error = "compiled BSP entity lump has an invalid key";
				return false;
			}
			skipWhitespace();
			if ( !readQuoted( value ) )
			{
				error = "compiled BSP entity lump has an invalid value";
				return false;
			}
			record.keys.push_back( std::move( key ) );
			record.values.push_back( std::move( value ) );
			skipWhitespace();
		}
		if ( position == source.size() )
		{
			error = "compiled BSP entity lump has an unterminated entity";
			return false;
		}
		++position;
		records.push_back( std::move( record ) );
		skipWhitespace();
	}
	if ( records.empty() )
	{
		error = "compiled BSP entity lump contains no worldspawn";
		return false;
	}
	return true;
}

float Project( const Vector &point, const float vector[4] )
{
	return point.x * vector[0] + point.y * vector[1] + point.z * vector[2] + vector[3];
}

bool MaterialPath( const texinfo_t &info, std::string &name )
{
	if ( info.texdata < 0 || info.texdata >= numtexdata )
		return false;
	const int stringIndex = dtexdata[info.texdata].nameStringTableID;
	if ( stringIndex < 0 || stringIndex >= g_TexDataStringTable.Count() )
		return false;
	const int offset = g_TexDataStringTable[stringIndex];
	if ( offset < 0 || offset >= g_TexDataStringData.Count() )
		return false;
	const char *begin = g_TexDataStringData.Base() + offset;
	const char *end = g_TexDataStringData.Base() + g_TexDataStringData.Count();
	const char *terminator = std::find( begin, end, '\0' );
	if ( terminator == end )
		return false;
	name.assign( begin, terminator );
	return !name.empty();
}

// Vertex welding (POINT_EPSILON in faces.cpp) can fold a face thinner than
// the weld distance back onto itself: vertices a b a c enclose no area. The
// surface is the winding with repeated vertices and spikes (a b a -> a)
// removed, compared by BSP vertex index so no tolerance is involved. Faces
// without folds keep every corner, including T-junction points.
std::vector<int> SurfaceCorners( const std::vector<int> &vertices )
{
	std::vector<int> corners = vertices;
	bool changed = true;
	while ( changed && corners.size() >= 3 )
	{
		changed = false;
		const size_t count = corners.size();
		for ( size_t i = 0; i < count; ++i )
		{
			const size_t next = ( i + 1 ) % count;
			if ( corners[i] == corners[next] )
			{
				corners.erase( corners.begin() + next );
				changed = true;
				break;
			}
			const size_t after = ( i + 2 ) % count;
			if ( corners[i] == corners[after] )
			{
				// Drop the spike tip and its return to corners[i].
				const size_t first = std::min( next, after );
				const size_t second = std::max( next, after );
				corners.erase( corners.begin() + second );
				corners.erase( corners.begin() + first );
				changed = true;
				break;
			}
		}
	}
	if ( corners.size() < 3 )
		corners.clear();
	return corners;
}

bool GatherWorldFaces( std::vector<FaceGeometry> &faces, std::string &error )
{
	if ( nummodels != 1 )
	{
		error = "brush entity models are not yet supported by the World Stage emitter";
		return false;
	}
	const dmodel_t &world = dmodels[0];
	if ( world.firstface < 0 || world.numfaces <= 0 || world.firstface + world.numfaces > numfaces )
	{
		error = "world model has an invalid face range";
		return false;
	}
	faces.reserve( world.numfaces );
	for ( int faceId = world.firstface; faceId < world.firstface + world.numfaces; ++faceId )
	{
		const dface_t &face = dfaces[faceId];
		if ( face.dispinfo != -1 )
		{
			error = "displacement faces require the World Stage tessellation path";
			return false;
		}
		if ( face.numedges < 3 || face.firstedge < 0 ||
		     face.firstedge + face.numedges > numsurfedges || face.texinfo < 0 ||
		     face.texinfo >= texinfo.Count() || face.planenum >= numplanes )
		{
			error = "world face has invalid geometry references";
			return false;
		}
		const texinfo_t &texture = texinfo[face.texinfo];
		FaceGeometry geometry;
		geometry.faceId = faceId;
		geometry.chartWidth = face.m_LightmapTextureSizeInLuxels[0] + 1;
		geometry.chartHeight = face.m_LightmapTextureSizeInLuxels[1] + 1;
		if ( geometry.chartWidth <= 0 || geometry.chartHeight <= 0 ||
		     geometry.chartWidth + 2 * kChartGutter > kAtlasWidth ||
		     geometry.chartHeight + 2 * kChartGutter > kAtlasMaxHeight ||
		     !MaterialPath( texture, geometry.materialPath ) )
		{
			error = "world face has invalid chart or material data";
			return false;
		}
		const dtexdata_t &material = dtexdata[texture.texdata];
		if ( material.width <= 0 || material.height <= 0 )
		{
			error = "world face has invalid material dimensions";
			return false;
		}
		const Vector &planeNormal = dplanes[face.planenum].normal;
		geometry.normal = pxr::GfVec3f( planeNormal.x, planeNormal.y, planeNormal.z );
		const float normalLength = pxr::GfGetLength( geometry.normal );
		if ( !std::isfinite( normalLength ) || std::abs( normalLength - 1.0f ) > 1e-3f )
		{
			error = "world face has an invalid plane normal";
			return false;
		}
		std::vector<int> vertices;
		for ( int edgeIndex = face.firstedge; edgeIndex < face.firstedge + face.numedges;
		    ++edgeIndex )
		{
			const int surfedge = dsurfedges[edgeIndex];
			if ( surfedge == INT32_MIN || std::abs( surfedge ) >= numedges )
			{
				error = "world face has an invalid surfedge";
				return false;
			}
			const int vertex = dedges[std::abs( surfedge )].v[surfedge < 0 ? 1 : 0];
			if ( vertex >= numvertexes )
			{
				error = "world face has an invalid vertex";
				return false;
			}
			const Vector &point = dvertexes[vertex].point;
			vertices.push_back( vertex );
			geometry.points.emplace_back( point.x, point.y, point.z );
			geometry.materialUV.emplace_back(
			    Project( point, texture.textureVecsTexelsPerWorldUnits[0] ) / material.width,
			    Project( point, texture.textureVecsTexelsPerWorldUnits[1] ) / material.height );
		}
		const std::vector<int> surface = SurfaceCorners( vertices );
		// Corners index the first occurrence of each surviving vertex.
		for ( int vertex : surface )
			geometry.corners.push_back( static_cast<int>(
			    std::find( vertices.begin(), vertices.end(), vertex ) - vertices.begin() ) );
		// Newell's polygon normal; FixTjuncs can make the first three points
		// collinear, so a single-corner cross product is not a valid winding test.
		pxr::GfVec3f newell( 0.0f );
		for ( size_t corner = 0; corner < geometry.corners.size(); ++corner )
		{
			const pxr::GfVec3f &current = geometry.points[geometry.corners[corner]];
			const pxr::GfVec3f &next =
			    geometry.points[geometry.corners[( corner + 1 ) % geometry.corners.size()]];
			newell += pxr::GfCross( current, next );
		}
		const float alignment = pxr::GfDot( newell, geometry.normal );
		if ( !std::isfinite( alignment ) )
		{
			error = "world face has a non-finite winding";
			return false;
		}
		// A folded or collinear face has no surface: it keeps its identity,
		// points and chart but authors no triangles.
		if ( std::abs( alignment ) < 1e-5f )
			geometry.corners.clear();
		geometry.reverseWinding = alignment < 0.0f;
		faces.push_back( std::move( geometry ) );
	}
	return true;
}

bool PackCharts( std::vector<FaceGeometry> &faces, int &height, std::string &error )
{
	int x = kChartGutter;
	int y = kChartGutter;
	int rowHeight = 0;
	for ( FaceGeometry &face : faces )
	{
		if ( x + face.chartWidth + kChartGutter > kAtlasWidth )
		{
			x = kChartGutter;
			y += rowHeight + 2 * kChartGutter;
			rowHeight = 0;
		}
		if ( y + face.chartHeight + kChartGutter > kAtlasMaxHeight )
		{
			error = "lightmap charts exceed the experimental single-page atlas";
			return false;
		}
		face.chartX = x;
		face.chartY = y;
		x += face.chartWidth + 2 * kChartGutter;
		rowHeight = std::max( rowHeight, face.chartHeight );
	}
	height = 64;
	while ( height < y + rowHeight + kChartGutter )
		height *= 2;
	return true;
}

bool AuthorFace( const pxr::UsdStageRefPtr &stage, const FaceGeometry &geometry, int atlasHeight )
{
	const dface_t &face = dfaces[geometry.faceId];
	const texinfo_t &texture = texinfo[face.texinfo];
	const pxr::SdfPath path(
	    "/World/Geometry/WorldSpawn/Mesh_" + std::to_string( geometry.faceId ) );
	pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh::Define( stage, path );
	const pxr::SourceMeshAPI source = pxr::SourceMeshAPI::Apply( mesh.GetPrim() );
	if ( !mesh || !source )
		return false;

	pxr::VtArray<pxr::GfVec3f> points( geometry.points.begin(), geometry.points.end() );
	pxr::VtArray<int> counts;
	pxr::VtArray<int> indices;
	pxr::VtArray<pxr::GfVec2f> materialUV;
	pxr::VtArray<pxr::GfVec2f> lightmapUV;
	pxr::VtArray<int> faceIds;
	pxr::VtArray<unsigned int> smoothingGroups;
	pxr::VtArray<int> chartIds;
	pxr::VtArray<pxr::GfVec3f> normals;
	const std::vector<int> &surface = geometry.corners;
	for ( int corner = 1; corner + 1 < static_cast<int>( surface.size() ); ++corner )
	{
		counts.push_back( 3 );
		faceIds.push_back( geometry.faceId );
		smoothingGroups.push_back( face.smoothingGroups );
		chartIds.push_back( geometry.faceId );
		normals.push_back( geometry.normal );
		for ( int fan : { 0, geometry.reverseWinding ? corner + 1 : corner,
		          geometry.reverseWinding ? corner : corner + 1 } )
		{
			const int vertex = surface[fan];
			indices.push_back( vertex );
			materialUV.push_back( geometry.materialUV[vertex] );
			const pxr::GfVec3f &point = geometry.points[vertex];
			const Vector sourcePoint( point[0], point[1], point[2] );
			const float s = Project( sourcePoint, texture.lightmapVecsLuxelsPerWorldUnits[0] ) -
			                face.m_LightmapTextureMinsInLuxels[0];
			const float t = Project( sourcePoint, texture.lightmapVecsLuxelsPerWorldUnits[1] ) -
			                face.m_LightmapTextureMinsInLuxels[1];
			lightmapUV.emplace_back( ( geometry.chartX + s + 0.5f ) / kAtlasWidth,
			    ( geometry.chartY + t + 0.5f ) / atlasHeight );
		}
	}
	const pxr::UsdGeomPrimvarsAPI primvars( mesh.GetPrim() );
	const pxr::UsdGeomPrimvar material = primvars.CreatePrimvar( pxr::TfToken( "st" ),
	    pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying );
	const pxr::UsdGeomPrimvar lightmap = primvars.CreatePrimvar( pxr::TfToken( "lightmap:st" ),
	    pxr::SdfValueTypeNames->TexCoord2fArray, pxr::UsdGeomTokens->faceVarying );
	return mesh.CreatePointsAttr().Set( points ) &&
	       mesh.CreateFaceVertexCountsAttr().Set( counts ) &&
	       mesh.CreateFaceVertexIndicesAttr().Set( indices ) &&
	       mesh.CreateNormalsAttr().Set( normals ) &&
	       mesh.SetNormalsInterpolation( pxr::UsdGeomTokens->uniform ) &&
	       mesh.CreateSubdivisionSchemeAttr().Set( pxr::UsdGeomTokens->none ) &&
	       material.Set( materialUV ) && lightmap.Set( lightmapUV ) &&
	       source.CreateFaceIdAttr().Set( faceIds ) &&
	       source.CreateSmoothingGroupAttr().Set( smoothingGroups ) &&
	       source.CreateLightmapChartIdAttr().Set( chartIds ) &&
	       source.CreateSourceMaterialPathAttr().Set( geometry.materialPath );
}

bool AuthorEntities(
    const pxr::UsdStageRefPtr &stage, const std::vector<EntityRecord> &records, std::string &error )
{
	if ( !pxr::UsdGeomScope::Define( stage, pxr::SdfPath( "/World/Entities" ) ) )
	{
		error = "OpenUSD could not define the entities scope";
		return false;
	}
	for ( std::size_t index = 0; index < records.size(); ++index )
	{
		const pxr::SdfPath path( "/World/Entities/Entity_" + std::to_string( index ) );
		const pxr::UsdPrim prim = stage->DefinePrim( path );
		const pxr::SourceEntityAPI source = pxr::SourceEntityAPI::Apply( prim );
		if ( !prim || !source )
		{
			error = "OpenUSD SourceEntityAPI is not registered";
			return false;
		}
		const EntityRecord &record = records[index];
		const pxr::VtArray<std::string> keys( record.keys.begin(), record.keys.end() );
		const pxr::VtArray<std::string> values( record.values.begin(), record.values.end() );
		if ( !source.CreateSourceEntityIndexAttr().Set( static_cast<int>( index ) ) ||
		     !source.CreateSourceKeyNamesAttr().Set( keys ) ||
		     !source.CreateSourceKeyValuesAttr().Set( values ) )
		{
			error = "OpenUSD could not author compiled entity " + std::to_string( index );
			return false;
		}
	}
	return true;
}

bool FindSingleEntityValue( const EntityRecord &record, const char *key, std::string &value )
{
	bool found = false;
	for ( std::size_t index = 0; index < record.keys.size(); ++index )
	{
		if ( record.keys[index] != key )
			continue;
		if ( found )
			return false;
		value = record.values[index];
		found = true;
	}
	return found;
}

bool ParseLightValues( const std::string &value, float *numbers, int count )
{
	std::istringstream stream( value );
	stream.imbue( std::locale::classic() );
	for ( int index = 0; index < count; ++index )
	{
		if ( !( stream >> numbers[index] ) || !std::isfinite( numbers[index] ) )
			return false;
	}
	std::string extra;
	return !( stream >> extra );
}

bool ParseCompiledLightStyle( const EntityRecord &record, int &style )
{
	style = 0;
	bool found = false;
	for ( std::size_t index = 0; index < record.keys.size(); ++index )
	{
		if ( record.keys[index] != "style" )
			continue;
		if ( found )
			return false;
		found = true;
		const std::string &value = record.values[index];
		const auto parsed = std::from_chars( value.data(), value.data() + value.size(), style );
		if ( parsed.ec != std::errc() || parsed.ptr != value.data() + value.size() || style < 0 ||
		     style >= 255 )
			return false;
	}
	return true;
}

float SrgbChannelToLinear( float channel )
{
	channel /= 255.0f;
	return channel <= 0.04045f ? channel / 12.92f : std::pow( ( channel + 0.055f ) / 1.055f, 2.4f );
}

bool AuthorLights(
    const pxr::UsdStageRefPtr &stage, const std::vector<EntityRecord> &records, std::string &error )
{
	if ( !pxr::UsdGeomScope::Define( stage, pxr::SdfPath( "/World/Lights" ) ) )
	{
		error = "OpenUSD could not define the lights scope";
		return false;
	}
	for ( std::size_t index = 0; index < records.size(); ++index )
	{
		std::string className;
		if ( !FindSingleEntityValue( records[index], "classname", className ) )
		{
			error = "compiled entity has no unique classname";
			return false;
		}
		if ( className == "light_dynamic" || className.rfind( "light", 0 ) != 0 )
			continue;
		if ( className != "light" )
		{
			error = "World Stage preview lighting does not support " + className;
			return false;
		}
		std::string originValue;
		std::string lightValue;
		int style = 0;
		float origin[3];
		float sourceLight[4];
		if ( !FindSingleEntityValue( records[index], "origin", originValue ) ||
		     !FindSingleEntityValue( records[index], "_light", lightValue ) ||
		     !ParseCompiledLightStyle( records[index], style ) ||
		     !ParseLightValues( originValue, origin, 3 ) ||
		     !ParseLightValues( lightValue, sourceLight, 4 ) || sourceLight[0] < 0.0f ||
		     sourceLight[0] > 255.0f || sourceLight[1] < 0.0f || sourceLight[1] > 255.0f ||
		     sourceLight[2] < 0.0f || sourceLight[2] > 255.0f || sourceLight[3] < 0.0f )
		{
			error = "compiled point light has invalid origin or _light data";
			return false;
		}
		const pxr::SdfPath path( "/World/Lights/Light_" + std::to_string( index ) );
		const pxr::UsdLuxSphereLight light = pxr::UsdLuxSphereLight::Define( stage, path );
		const pxr::SourceLightAPI source = pxr::SourceLightAPI::Apply( light.GetPrim() );
		if ( !light || !source ||
		     !pxr::UsdGeomXformable( light ).AddTranslateOp().Set(
		         pxr::GfVec3d( origin[0], origin[1], origin[2] ) ) ||
		     !light.CreateRadiusAttr().Set( 8.0f ) ||
		     !light.CreateIntensityAttr().Set( sourceLight[3] ) ||
		     !light.CreateColorAttr().Set( pxr::GfVec3f( SrgbChannelToLinear( sourceLight[0] ),
		         SrgbChannelToLinear( sourceLight[1] ), SrgbChannelToLinear( sourceLight[2] ) ) ) ||
		     !source.CreateSourceEntityIndexAttr().Set( static_cast<int>( index ) ) ||
		     !source.CreateSourceStyleIdAttr().Set( style ) ||
		     !source.CreateSourceLightingPolicyAttr().Set( pxr::TfToken( "preview-v1" ) ) )
		{
			error = "OpenUSD could not author point light " + std::to_string( index );
			return false;
		}
	}
	return true;
}

bool PublishBsp2( const char *bspPath, std::string &error )
{
	mapcontainer::tooling::FileByteSource source( bspPath );
	if ( !source.IsOpen() )
	{
		error = "World Stage packer could not reopen the compiled BSP";
		return false;
	}
	std::filesystem::path output( bspPath );
	output.replace_extension( ".bsp2" );
	const std::filesystem::path temporary = output.string() + ".tmp";
	mapcontainer::tooling::FileByteSink sink( temporary.string() );
	const mapcontainer::MapContainerStatus status =
	    mapcontainer::ConvertLegacyToBsp2( source, sink );
	if ( !status.Ok() )
	{
		sink.Finish();
		std::filesystem::remove( temporary );
		error = std::string( "BSP2 conversion failed: " ) +
		        mapcontainer::MapContainerErrorName( status.code );
		return false;
	}
	if ( !sink.Finish() )
	{
		std::filesystem::remove( temporary );
		error = "BSP2 conversion could not finish its output";
		return false;
	}
	std::error_code renameError;
	std::filesystem::rename( temporary, output, renameError );
	if ( renameError )
	{
		std::filesystem::remove( temporary );
		error = "BSP2 conversion could not publish its output: " + renameError.message();
		return false;
	}
	return true;
}

bool WriteWorldStageGeometryImpl( const char *bspPath, std::string &error )
{
	std::vector<FaceGeometry> faces;
	if ( !GatherWorldFaces( faces, error ) )
		return false;
	std::vector<EntityRecord> entities;
	if ( !ParseCompiledEntities( entities, error ) )
		return false;
	int atlasHeight = 0;
	if ( !PackCharts( faces, atlasHeight, error ) )
		return false;

	const pxr::UsdStageRefPtr stage = pxr::UsdStage::CreateInMemory();
	if ( !stage )
	{
		error = "OpenUSD could not create an in-memory stage";
		return false;
	}
	pxr::UsdGeomSetStageMetersPerUnit( stage, 0.0254 );
	pxr::UsdGeomSetStageUpAxis( stage, pxr::UsdGeomTokens->z );
	const pxr::UsdGeomXform world = pxr::UsdGeomXform::Define( stage, pxr::SdfPath( "/World" ) );
	const pxr::SourceWorldAPI source = pxr::SourceWorldAPI::Apply( world.GetPrim() );
	if ( !world || !source )
	{
		error = "OpenUSD SourceWorldAPI is not registered";
		return false;
	}
	stage->SetDefaultPrim( world.GetPrim() );
	pxr::UsdGeomScope::Define( stage, pxr::SdfPath( "/World/Geometry" ) );
	pxr::UsdGeomScope::Define( stage, pxr::SdfPath( "/World/Geometry/WorldSpawn" ) );
	pxr::VtArray<int> chartFaceIds;
	pxr::VtArray<pxr::GfVec4i> chartRects;
	for ( const FaceGeometry &face : faces )
	{
		chartFaceIds.push_back( face.faceId );
		chartRects.emplace_back( face.chartX, face.chartY, face.chartWidth, face.chartHeight );
		if ( !AuthorFace( stage, face, atlasHeight ) )
		{
			error = "OpenUSD could not author world face " + std::to_string( face.faceId );
			return false;
		}
	}
	if ( !source.CreateSourceSchemaVersionAttr().Set( 4 ) ||
	     !source.CreateSourceLightmapAtlasWidthAttr().Set( kAtlasWidth ) ||
	     !source.CreateSourceLightmapAtlasHeightAttr().Set( atlasHeight ) ||
	     !source.CreateSourceChartFaceIdsAttr().Set( chartFaceIds ) ||
	     !source.CreateSourceChartRectsAttr().Set( chartRects ) )
	{
		error = "OpenUSD could not author the lightmap chart table";
		return false;
	}
	if ( !AuthorEntities( stage, entities, error ) )
		return false;
	if ( !AuthorLights( stage, entities, error ) )
		return false;
	std::filesystem::path output( bspPath );
	output.replace_extension( ".geometry.usda" );
	const std::filesystem::path temporary = output.string() + ".tmp.usda";
	if ( !stage->Export( temporary.string() ) )
	{
		error = "OpenUSD could not export the geometry layer";
		std::filesystem::remove( temporary );
		return false;
	}
	std::error_code renameError;
	std::filesystem::rename( temporary, output, renameError );
	if ( renameError )
	{
		error = "OpenUSD could not publish the geometry layer: " + renameError.message();
		std::filesystem::remove( temporary );
		return false;
	}
	return PublishBsp2( bspPath, error );
}

} // namespace

bool WriteWorldStageGeometry( const char *bspPath, char *errorBuffer, std::size_t errorBytes )
{
	std::string error;
	const bool success = WriteWorldStageGeometryImpl( bspPath, error );
	if ( !success && errorBuffer != nullptr && errorBytes != 0 )
		std::snprintf( errorBuffer, errorBytes, "%s", error.c_str() );
	return success;
}
