//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Corpus conformance for content.studio-model.v1
//			(unittests/mdltest/contracts/content.studio-model.v1.md): every
//			model of the game VPKs STUDIO_MODEL_CORPUS_VPKS names (a comma-
//			separated list of _dir.vpk paths; Portal's portal_pak_dir.vpk and
//			Portal 2's pak01_dir.vpk) that has its .vvd parses, with relations
//			an independent look at the data must satisfy: triangles face the
//			way their vertex normals do (the winding swap), every model with
//			triangles has a non-empty box, and most materials resolve in the
//			same VPK. Each game's metal_box.mdl is checked in detail. The VPK
//			reader is the editor's (hammer.formats); the content is not in
//			the repository, so the row is optional.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/formats/vpk_archive.h"
#include "mdl/studio_model.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{
int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const std::string &what )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
	}
	std::printf( "%s: %s\n", condition ? "ok" : "FAIL", what.c_str() );
}

class VpkModelFiles : public mdl::IModelFiles
{
public:
	explicit VpkModelFiles( const hammer::formats::VpkArchive &vpk ) : m_vpk( vpk ) {}
	bool Exists( const std::string &path ) const override { return m_vpk.HasAsset( path ); }
	bool Read( const std::string &path, std::string &out ) const override
	{
		return m_vpk.ReadAsset( path, out );
	}

private:
	const hammer::formats::VpkArchive &m_vpk;
};

struct PoseTally
{
	int sequences = 0;         // sequences with a stored box, posed at their first frame
	int layers = 0;            // partial-weight sequences, not judged (see PoseChecks)
	int includedSequences = 0; // of 'sequences', from $includemodel models (bone boxes)
	int inside = 0;            // posed bounds inside the stored box (with tolerance)
	int bindOutside = 0;       // the bind (unposed) bounds are not inside it
	int referenceModels = 0;
	int referenceExact = 0; // posing at the reference pose gives the bind vertices
	int staticProps = 0;
	int staticUnchanged = 0; // a $staticprop's first frame gives the bind vertices
	int caughtSwap = 0;      // seeded defects the box oracle rejects (among 'inside')
	int caughtParent = 0;
	int caughtPosScale = 0;
	int affectedSwap = 0; // sequences a defect changes at all
	int affectedParent = 0;
	int affectedPosScale = 0;
	std::vector<std::string> outside; // the first few misses, for the log
};

struct Tally
{
	int models = 0;
	int parsed = 0;
	int bodyVariantsParsed = 0;
	int defaultBodyMatches = 0;
	int modelsWithAlternatives = 0;
	int alternativesChecked = 0;
	int alternativesMatched = 0;
	int withTriangles = 0;
	int emptyBoxes = 0;
	std::size_t triangles = 0;
	std::size_t facingNormals = 0;
	std::size_t textures = 0;
	std::size_t resolved = 0;
	PoseTally pose;
};

mdl::Float3 Extent( const mdl::Model &m )
{
	return { m.maxs.x - m.mins.x, m.maxs.y - m.mins.y, m.maxs.z - m.mins.z };
}

// Raw geometry import promises the stored attributes, including unused VVD
// vertices with nonfinite fields. Numeric equality would reject even identical
// NaN payloads. Compare every field's bits without omitting those vertices.
bool SameImportedMesh( const mdl::Mesh &a, const mdl::Mesh &b )
{
	if ( a.textureRef != b.textureRef || a.bodyPart != b.bodyPart || a.bodyModel != b.bodyModel ||
	     a.indices != b.indices || a.weights.size() != b.weights.size() ||
	     a.vertices.size() != b.vertices.size() )
		return false;
	const auto same = []( float x, float y )
	{
		return std::bit_cast<std::uint32_t>( x ) == std::bit_cast<std::uint32_t>( y );
	};
	const auto same3 = [&]( mdl::Float3 x, mdl::Float3 y )
	{
		return same( x.x, y.x ) && same( x.y, y.y ) && same( x.z, y.z );
	};
	for ( std::size_t i = 0; i < a.vertices.size(); ++i )
	{
		const mdl::Vertex &x = a.vertices[i];
		const mdl::Vertex &y = b.vertices[i];
		if ( !same3( x.position, y.position ) || !same3( x.normal, y.normal ) ||
		     !same3( x.tangent, y.tangent ) || !same( x.tangentSign, y.tangentSign ) ||
		     !same( x.u, y.u ) || !same( x.v, y.v ) )
			return false;
	}
	for ( std::size_t i = 0; i < a.weights.size(); ++i )
	{
		if ( a.weights[i].count != b.weights[i].count || a.weights[i].bones != b.weights[i].bones )
			return false;
		for ( int w = 0; w < 3; ++w )
		{
			if ( !same( a.weights[i].weights[w], b.weights[i].weights[w] ) )
				return false;
		}
	}
	return true;
}

void ImportedMeshComparatorControls()
{
	mdl::Mesh original;
	original.vertices.resize( 1 );
	original.vertices[0].normal.x = std::bit_cast<float>( 0x7fc00001u );
	original.weights.resize( 1 );
	original.indices = { 0 };
	Check( SameImportedMesh( original, original ),
	    "geometry comparator preserves identical NaN bits" );
	mdl::Mesh changed = original;
	changed.vertices[0].normal.x = std::bit_cast<float>( 0x7fc00002u );
	Check(
	    !SameImportedMesh( original, changed ), "geometry comparator catches changed NaN payload" );
	changed = original;
	changed.vertices[0].tangentSign = -1.0f;
	Check(
	    !SameImportedMesh( original, changed ), "geometry comparator catches tangent handedness" );
	changed = original;
	changed.vertices[0].u = 0.5f;
	Check(
	    !SameImportedMesh( original, changed ), "geometry comparator catches texture coordinates" );
	changed = original;
	changed.weights[0].bones[1] = 3;
	Check( !SameImportedMesh( original, changed ), "geometry comparator catches bone identity" );
	changed = original;
	changed.weights[0].weights[2] = 0.5f;
	Check(
	    !SameImportedMesh( original, changed ), "geometry comparator catches every bone weight" );
	changed = original;
	changed.indices[0] = 1;
	Check( !SameImportedMesh( original, changed ), "geometry comparator catches triangle indices" );
	changed = original;
	changed.bodyModel = 1;
	Check( !SameImportedMesh( original, changed ), "geometry comparator catches body identity" );
}

// 'm''s bounds inside the compiler's box [lo, hi] (over all of the
// sequence's frames), grown by 1 unit, 2% of the box's size and 0.2% of its
// distance from the origin (float16 positions: 4 units apart at 4096 to
// 8192). The compiler clamps a box to +-16384; a clamped side bounds nothing.
bool InsideBox(
    const mdl::Float3 &mins, const mdl::Float3 &maxs, const mdl::Float3 &lo, const mdl::Float3 &hi )
{
	const float size = std::max( { hi.x - lo.x, hi.y - lo.y, hi.z - lo.z, 0.0f } );
	const float far = std::max( { std::fabs( lo.x ), std::fabs( lo.y ), std::fabs( lo.z ),
	    std::fabs( hi.x ), std::fabs( hi.y ), std::fabs( hi.z ) } );
	const float tol = 1.0f + 0.02f * size + 0.002f * far;
	const auto above = [&]( float v, float bound )
	{
		return bound <= -16383.0f || v >= bound - tol;
	};
	const auto below = [&]( float v, float bound )
	{
		return bound >= 16383.0f || v <= bound + tol;
	};
	return above( mins.x, lo.x ) && above( mins.y, lo.y ) && above( mins.z, lo.z ) &&
	       below( maxs.x, hi.x ) && below( maxs.y, hi.y ) && below( maxs.z, hi.z );
}

bool Inside( const mdl::Model &m, const mdl::Float3 &lo, const mdl::Float3 &hi )
{
	return InsideBox( m.mins, m.maxs, lo, hi );
}

// The box of the bones' origins in a pose: what the compiler bounds an
// animation-only model's sequences by (it has no vertices).
bool BonesInside(
    const mdl::Model &m, const std::vector<mdl::BoneTransform> &pose, const mdl::Sequence &seq )
{
	const std::vector<mdl::Matrix3x4> bones = mdl::BoneToModel( m, pose );
	if ( bones.empty() )
	{
		return false;
	}
	mdl::Float3 lo{ bones[0].m[0][3], bones[0].m[1][3], bones[0].m[2][3] };
	mdl::Float3 hi = lo;
	for ( const mdl::Matrix3x4 &b : bones )
	{
		const mdl::Float3 p{ b.m[0][3], b.m[1][3], b.m[2][3] };
		lo = { std::min( lo.x, p.x ), std::min( lo.y, p.y ), std::min( lo.z, p.z ) };
		hi = { std::max( hi.x, p.x ), std::max( hi.y, p.y ), std::max( hi.z, p.z ) };
	}
	return InsideBox( lo, hi, seq.boundsMin, seq.boundsMax );
}

// The largest distance between corresponding vertices of two models.
float MaxVertexDistance( const mdl::Model &a, const mdl::Model &b )
{
	float worst = 0.0f;
	for ( std::size_t m = 0; m < a.meshes.size() && m < b.meshes.size(); ++m )
	{
		const auto &va = a.meshes[m].vertices;
		const auto &vb = b.meshes[m].vertices;
		for ( std::size_t v = 0; v < va.size() && v < vb.size(); ++v )
		{
			const float dx = va[v].position.x - vb[v].position.x;
			const float dy = va[v].position.y - vb[v].position.y;
			const float dz = va[v].position.z - vb[v].position.z;
			worst = std::max( worst, std::sqrt( dx * dx + dy * dy + dz * dz ) );
		}
	}
	return worst;
}

bool SameBounds( const mdl::Model &a, const mdl::Model &b )
{
	const auto near = []( const mdl::Float3 &p, const mdl::Float3 &q )
	{
		return std::fabs( p.x - q.x ) < 0.01f && std::fabs( p.y - q.y ) < 0.01f &&
		       std::fabs( p.z - q.z ) < 0.01f;
	};
	return near( a.mins, b.mins ) && near( a.maxs, b.maxs );
}

// Seeded defects, applied to a decoded first frame (the reader is not
// changed): quaternion x and y read swapped; ignoring posscale (an animated
// position's offset from the reference divided back by the bone's scale); and
// a dropped parent concatenation (every bone's local transform taken as
// model space), which needs its own skinning.
std::vector<mdl::BoneTransform> SwappedQuaternions( std::vector<mdl::BoneTransform> pose )
{
	for ( mdl::BoneTransform &b : pose )
	{
		std::swap( b.rotation.x, b.rotation.y );
	}
	return pose;
}

std::vector<mdl::BoneTransform> IgnoredPosScale(
    const mdl::Model &m, std::vector<mdl::BoneTransform> pose )
{
	for ( std::size_t i = 0; i < pose.size(); ++i )
	{
		const mdl::Bone &bone = m.bones[i];
		float *p[3] = { &pose[i].position.x, &pose[i].position.y, &pose[i].position.z };
		const float base[3] = { bone.position.x, bone.position.y, bone.position.z };
		const float scale[3] = { bone.positionScale.x, bone.positionScale.y, bone.positionScale.z };
		for ( int k = 0; k < 3; ++k )
		{
			if ( scale[k] != 0.0f )
			{
				*p[k] = base[k] + ( *p[k] - base[k] ) / scale[k];
			}
		}
	}
	return pose;
}

mdl::Model DroppedParentConcat( mdl::Model m, const std::vector<mdl::BoneTransform> &pose )
{
	for ( mdl::Bone &bone : m.bones )
	{
		bone.parent = -1;
	}
	return mdl::PoseModel( m, pose );
}

void PoseChecks( const std::string &name, const mdl::Model &m, PoseTally &tally )
{
	if ( m.TriangleCount() == 0 )
	{
		return;
	}
	++tally.referenceModels;
	const mdl::Model reference = mdl::PoseModel( m, mdl::ReferencePose( m ) );
	const mdl::Float3 e = Extent( m );
	const float scale = std::max( { e.x, e.y, e.z, 1.0f } );
	if ( MaxVertexDistance( reference, m ) <= 1.0e-3f * scale )
	{
		++tally.referenceExact;
	}
	else if ( tally.outside.size() < 12 )
	{
		tally.outside.push_back( name + ": reference pose moves vertices" );
	}
	if ( ( m.flags & mdl::kStaticPropFlag ) && !m.sequences.empty() )
	{
		++tally.staticProps;
		const mdl::Model posed = mdl::PoseModel( m, m.sequences[0].firstFrame );
		if ( MaxVertexDistance( posed, m ) <= 1.0e-3f * scale )
		{
			++tally.staticUnchanged;
		}
	}
	for ( const mdl::Sequence &seq : m.sequences )
	{
		if ( seq.flags & mdl::kDeltaSequence )
		{
			continue; // a delta's box is that of the reference it adds to
		}
		if ( std::any_of( seq.boneWeights.begin(), seq.boneWeights.end(),
		         []( float w )
		         {
			         return w < 1.0f;
		         } ) )
		{
			// A layer: bones it does not weigh keep the reference when it is
			// drawn alone (bone_setup.cpp), while the compiler's box is its
			// animation at full weight.
			++tally.layers;
			continue;
		}
		if ( !( seq.boundsMin.x < seq.boundsMax.x && seq.boundsMin.y < seq.boundsMax.y &&
		         seq.boundsMin.z < seq.boundsMax.z ) )
		{
			continue;
		}
		++tally.sequences;
		// A sequence from an included (animation-only) model is bounded by its
		// bones' origins; the model's own by its vertices.
		const bool byBones = seq.group > 0;
		tally.includedSequences += byBones ? 1 : 0;
		const std::vector<mdl::BoneTransform> reference = mdl::ReferencePose( m );
		const auto judge =
		    [&]( const mdl::Model &model, const std::vector<mdl::BoneTransform> &pose )
		{
			return byBones ? BonesInside( model, pose, seq )
			               : Inside( mdl::PoseModel( model, pose ), seq.boundsMin, seq.boundsMax );
		};
		const mdl::Model posed = byBones ? mdl::Model{} : mdl::PoseModel( m, seq.firstFrame );
		const bool inside = judge( m, seq.firstFrame );
		tally.inside += inside ? 1 : 0;
		tally.bindOutside += judge( m, reference ) ? 0 : 1;
		if ( !inside )
		{
			if ( tally.outside.size() < 12 )
			{
				char line[256];
				std::snprintf( line, sizeof( line ),
				    "%s:%s posed%s (%.1f %.1f %.1f)-(%.1f %.1f %.1f) box (%.1f %.1f %.1f)-(%.1f "
				    "%.1f %.1f)",
				    name.c_str(), seq.label.c_str(), byBones ? " bones" : "", posed.mins.x,
				    posed.mins.y, posed.mins.z, posed.maxs.x, posed.maxs.y, posed.maxs.z,
				    seq.boundsMin.x, seq.boundsMin.y, seq.boundsMin.z, seq.boundsMax.x,
				    seq.boundsMax.y, seq.boundsMax.z );
				tally.outside.push_back( line );
			}
			continue;
		}
		if ( byBones )
		{
			continue; // the seeded defects are judged on vertex boxes
		}
		const std::vector<mdl::BoneTransform> frame = seq.firstFrame;
		const mdl::Model swapped = mdl::PoseModel( m, SwappedQuaternions( frame ) );
		const mdl::Model dropped = DroppedParentConcat( m, frame );
		const mdl::Model unscaled = mdl::PoseModel( m, IgnoredPosScale( m, frame ) );
		const auto tallyDefect = [&]( const mdl::Model &defect, int &affected, int &caught )
		{
			if ( !SameBounds( defect, posed ) )
			{
				++affected;
				caught += Inside( defect, seq.boundsMin, seq.boundsMax ) ? 0 : 1;
			}
		};
		tallyDefect( swapped, tally.affectedSwap, tally.caughtSwap );
		tallyDefect( dropped, tally.affectedParent, tally.caughtParent );
		tallyDefect( unscaled, tally.affectedPosScale, tally.caughtPosScale );
	}
}

// A triangle faces its normals when its geometric normal points along the
// sum of its vertex normals.
bool FacesNormals( const mdl::Mesh &mesh, std::size_t i )
{
	const mdl::Vertex &a = mesh.vertices[mesh.indices[i]];
	const mdl::Vertex &b = mesh.vertices[mesh.indices[i + 1]];
	const mdl::Vertex &c = mesh.vertices[mesh.indices[i + 2]];
	const double e1[3] = {
	    b.position.x - a.position.x, b.position.y - a.position.y, b.position.z - a.position.z };
	const double e2[3] = {
	    c.position.x - a.position.x, c.position.y - a.position.y, c.position.z - a.position.z };
	const double n[3] = { e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2],
	    e1[0] * e2[1] - e1[1] * e2[0] };
	const double s[3] = { double( a.normal.x ) + b.normal.x + c.normal.x,
	    double( a.normal.y ) + b.normal.y + c.normal.y,
	    double( a.normal.z ) + b.normal.z + c.normal.z };
	return n[0] * s[0] + n[1] * s[1] + n[2] * s[2] > 0.0;
}

bool NearQuaternion( const mdl::Quaternion &a, const mdl::Quaternion &b )
{
	// q and -q are one rotation.
	const float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	return std::fabs( std::fabs( dot ) - 1.0f ) < 2.0e-3f;
}

// A named model posed at 'label': the root's first-frame rotation is 'root'
// (figures from an independent Python walk of the records), the posed bounds
// lie inside the sequence's stored box and, when 'bindOutside', the bind
// vertices do not (the reported defect: the Y-up reference drawn unposed).
void NamedModel( const mdl::IModelFiles &files, const std::string &path, const std::string &label,
    const mdl::Quaternion &root, bool bindOutside )
{
	auto loaded = mdl::LoadModel( files, path );
	Check( loaded.HasValue(), path + " parses" );
	if ( !loaded.HasValue() )
	{
		return;
	}
	const mdl::Model &m = loaded.Value();
	const std::int32_t index = mdl::FindSequence( m, label );
	Check( index >= 0, path + ": has sequence " + label );
	if ( index < 0 )
	{
		return;
	}
	const mdl::Sequence &seq = m.sequences[std::size_t( index )];
	Check( !seq.firstFrame.empty() && NearQuaternion( seq.firstFrame[0].rotation, root ),
	    path + ":" + label + ": root rotation at frame 0 is the recorded one" );
	const mdl::Model posed = mdl::PoseModel( m, index );
	char line[160];
	std::snprintf( line, sizeof( line ), " (posed %.1f %.1f %.1f - %.1f %.1f %.1f)", posed.mins.x,
	    posed.mins.y, posed.mins.z, posed.maxs.x, posed.maxs.y, posed.maxs.z );
	Check( Inside( posed, seq.boundsMin, seq.boundsMax ),
	    path + ":" + label + ": posed bounds inside the stored box" + line );
	if ( bindOutside )
	{
		Check( !Inside( m, seq.boundsMin, seq.boundsMax ),
		    path + ":" + label +
		        ": the unposed bind vertices are not (the oracle sees the "
		        "reported defect)" );
	}
}

void RunVpk( const std::string &path )
{
	hammer::adapters::platform::DiskByteStore store;
	std::string error;
	std::unique_ptr<hammer::formats::VpkArchive> vpk =
	    hammer::formats::VpkArchive::Open( store, path, error );
	Check( vpk != nullptr, "opens " + path + ( error.empty() ? "" : " (" + error + ")" ) );
	if ( !vpk )
	{
		return;
	}
	VpkModelFiles files( *vpk );
	std::vector<std::string> models;
	vpk->ListAssets( "models/", ".mdl", models );
	Tally tally;
	int firstFailures = 0;
	for ( const std::string &model : models )
	{
		const std::string stem = model.substr( 0, model.size() - 4 );
		if ( !vpk->HasAsset( stem + ".vvd" ) )
		{
			continue; // an animation-only or $includemodel file
		}
		++tally.models;
		std::string mdlBytes, vvdBytes, vtxBytes;
		(void)files.Read( model, mdlBytes );
		(void)files.Read( stem + ".vvd", vvdBytes );
		for ( const char *suffix : { ".dx90.vtx", ".vtx", ".dx80.vtx", ".sw.vtx" } )
		{
			if ( files.Read( stem + suffix, vtxBytes ) )
				break;
		}
		const auto variants =
		    mdl::ParseModelGeometryVariants( { mdlBytes, vvdBytes, vtxBytes, {} } );
		if ( variants )
		{
			++tally.bodyVariantsParsed;
			if ( std::any_of( variants.Value().bodyParts.begin(), variants.Value().bodyParts.end(),
			         []( const mdl::BodyPart &part )
			         {
				         return part.modelCount > 1;
			         } ) )
				++tally.modelsWithAlternatives;
			for ( const mdl::BodyPart &part : variants.Value().bodyParts )
			{
				for ( std::uint32_t alternative = 1; alternative < part.modelCount; ++alternative )
				{
					const std::int32_t body = std::int32_t( part.base * alternative );
					++tally.alternativesChecked;
					const auto selected = mdl::LoadModel( files, model, body );
					std::vector<mdl::Mesh> meshes;
					for ( const mdl::Mesh &mesh : variants.Value().meshes )
					{
						if ( mesh.lod == 0 &&
						     variants.Value().bodyParts[mesh.bodyPart].SelectedModel( body ) ==
						         mesh.bodyModel )
							meshes.push_back( mesh );
					}
					if ( selected && meshes.size() == selected.Value().meshes.size() &&
					     std::equal( meshes.begin(), meshes.end(), selected.Value().meshes.begin(),
					         SameImportedMesh ) )
						++tally.alternativesMatched;
					else
						std::printf( "  alternative mismatch %s body %d\n", model.c_str(), body );
				}
			}
		}
		else if ( firstFailures++ < 10 )
		{
			std::printf( "  body variants %s: %s\n", model.c_str(),
			    mdl::Describe( variants.Error() ).c_str() );
		}
		auto result = mdl::LoadModel( files, model );
		if ( !result.HasValue() )
		{
			if ( ++firstFailures <= 10 )
			{
				std::printf( "  %s: %s\n", model.c_str(), mdl::Describe( result.Error() ).c_str() );
			}
			continue;
		}
		++tally.parsed;
		const mdl::Model &m = result.Value();
		if ( variants )
		{
			std::vector<mdl::Mesh> bodyZero;
			for ( const mdl::Mesh &mesh : variants.Value().meshes )
			{
				if ( mesh.lod == 0 && variants.Value().bodyParts[mesh.bodyPart].SelectedModel(
				                          0 ) == mesh.bodyModel )
					bodyZero.push_back( mesh );
			}
			if ( bodyZero.size() == m.meshes.size() && std::equal( bodyZero.begin(), bodyZero.end(),
			                                               m.meshes.begin(), SameImportedMesh ) )
				++tally.defaultBodyMatches;
			else
			{
				std::printf( "  body-zero mismatch %s: %zu selected meshes, %zu original meshes\n",
				    model.c_str(), bodyZero.size(), m.meshes.size() );
				for ( std::size_t i = 0; i < std::min( bodyZero.size(), m.meshes.size() ); ++i )
				{
					if ( bodyZero[i] != m.meshes[i] )
						std::printf(
						    "    mesh %zu: vertices %zu/%zu, weights %zu/%zu, "
						    "indices %zu/%zu, vertex equality %d, weights %d, indices %d\n",
						    i, bodyZero[i].vertices.size(), m.meshes[i].vertices.size(),
						    bodyZero[i].weights.size(), m.meshes[i].weights.size(),
						    bodyZero[i].indices.size(), m.meshes[i].indices.size(),
						    bodyZero[i].vertices == m.meshes[i].vertices,
						    bodyZero[i].weights == m.meshes[i].weights,
						    bodyZero[i].indices == m.meshes[i].indices );
				}
			}
		}
		PoseChecks( model, m, tally.pose );
		if ( m.TriangleCount() > 0 )
		{
			++tally.withTriangles;
			if ( !( m.mins.x < m.maxs.x || m.mins.y < m.maxs.y || m.mins.z < m.maxs.z ) )
			{
				++tally.emptyBoxes;
			}
		}
		for ( const mdl::Mesh &mesh : m.meshes )
		{
			for ( std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3 )
			{
				++tally.triangles;
				tally.facingNormals += FacesNormals( mesh, i ) ? 1 : 0;
			}
		}
		for ( const mdl::ResolvedMaterial &material : mdl::ResolveMaterials( m, files ) )
		{
			++tally.textures;
			tally.resolved += material.found ? 1 : 0;
		}
	}
	std::printf( "  %s: %d models, %d parsed, %zu triangles (%zu facing their normals), "
	             "%zu of %zu materials resolved in this VPK\n",
	    path.c_str(), tally.models, tally.parsed, tally.triangles, tally.facingNormals,
	    tally.resolved, tally.textures );
	Check( tally.models >= 100,
	    "the VPK holds a model corpus (" + std::to_string( tally.models ) + ")" );
	Check( tally.parsed == tally.models, "every model with a .vvd parses (" +
	                                         std::to_string( tally.parsed ) + " of " +
	                                         std::to_string( tally.models ) + ")" );
	Check( tally.bodyVariantsParsed == tally.models,
	    "every runtime model imports all body variants without ANI data (" +
	        std::to_string( tally.bodyVariantsParsed ) + " of " + std::to_string( tally.models ) +
	        ")" );
	Check( tally.defaultBodyMatches == tally.models,
	    "body-zero selection matches the existing consumer's geometry for every model" );
	std::printf(
	    "  %d real models have nondefault body alternatives\n", tally.modelsWithAlternatives );
	Check( tally.alternativesChecked == tally.alternativesMatched &&
	           tally.alternativesChecked >= tally.modelsWithAlternatives,
	    "nondefault body selections preserve every imported attribute (" +
	        std::to_string( tally.alternativesMatched ) + " of " +
	        std::to_string( tally.alternativesChecked ) + ")" );
	Check( tally.emptyBoxes == 0, "every model with triangles has a box" );
	Check( tally.triangles > 0 && double( tally.facingNormals ) >= 0.97 * double( tally.triangles ),
	    "at least 97% of triangles wind counter-clockwise about their normals" );

	const PoseTally &pose = tally.pose;
	std::printf( "  first frames: %d of %d sequences posed inside their stored box (%d from "
	             "included models, by bone origins; the bind pose is outside it for %d; %d "
	             "layer sequences not judged); reference pose exact for %d of %d models; %d of "
	             "%d static props unchanged\n"
	             "  seeded defects caught by the box: swapped quaternion %d of %d affected, "
	             "dropped parent concat %d of %d, ignored posscale %d of %d\n",
	    pose.inside, pose.sequences, pose.includedSequences, pose.bindOutside, pose.layers,
	    pose.referenceExact, pose.referenceModels, pose.staticUnchanged, pose.staticProps,
	    pose.caughtSwap, pose.affectedSwap, pose.caughtParent, pose.affectedParent,
	    pose.caughtPosScale, pose.affectedPosScale );
	for ( const std::string &line : pose.outside )
	{
		std::printf( "    %s\n", line.c_str() );
	}
	Check( pose.sequences > 0 && pose.inside * 1000 >= pose.sequences * 995,
	    "at least 99.5% of full-weight sequences pose inside their stored box (" +
	        std::to_string( pose.inside ) + " of " + std::to_string( pose.sequences ) + ")" );
	Check( pose.bindOutside * 10 >= pose.sequences,
	    "the box oracle is sensitive: the bind vertices miss it for at least 10% (" +
	        std::to_string( pose.bindOutside ) + ")" );
	Check( pose.referenceModels > 0 && pose.referenceExact == pose.referenceModels,
	    "posing at the reference pose gives the bind vertices for every model" );
	Check( pose.staticProps > 0 && pose.staticUnchanged == pose.staticProps,
	    "every $staticprop's first frame gives its bind vertices (" +
	        std::to_string( pose.staticProps ) + ")" );
	const auto caught = [&]( int got, int affected, const char *what )
	{
		Check( affected > 0 && got * 10 >= affected * 6,
		    std::string( "seeded defect '" ) + what + "' is caught for at least 60% of the " +
		        "sequences it changes (" + std::to_string( got ) + " of " +
		        std::to_string( affected ) + ")" );
	};
	caught( pose.caughtSwap, pose.affectedSwap, "quaternion x and y swapped" );
	caught( pose.caughtParent, pose.affectedParent, "parent concatenation dropped" );
	caught( pose.caughtPosScale, pose.affectedPosScale, "posscale ignored" );

	// The models of sp_a2_trust_fling the defect was reported on: the cube
	// dropper, the panel arms (sequences from their $includemodel animation
	// model, in its .ani blocks) and the turbine elevator; the faith plate
	// and the turret.
	if ( vpk->HasAsset( "models/props_backstage/item_dropper_wrecked.mdl" ) )
	{
		NamedModel( files, "models/props_backstage/item_dropper_wrecked.mdl",
		    "item_dropper_idle_closed", { 0.707f, 0.0f, 0.707f, 0.001f }, true );
		const std::string arm = "models/anim_wp/room_transform/arm64x64_interior_rusty.mdl";
		NamedModel( files, arm, "trustflings_platfor_b01_idleend",
		    { -0.707f, 0.707f, 0.0f, 0.001f }, true );
		auto loaded = mdl::LoadModel( files, arm );
		if ( loaded.HasValue() )
		{
			const mdl::Model &m = loaded.Value();
			int resolved = 0;
			int inside = 0;
			std::vector<mdl::Model> posed;
			for ( const char *set : { "a", "b" } )
			{
				for ( int panel = 1; panel <= 4; ++panel )
				{
					const std::string label = std::string( "trustflings_platfor_" ) + set + "0" +
					                          std::to_string( panel ) + "_idleend";
					const std::int32_t index = mdl::FindSequence( m, label );
					if ( index < 0 || m.sequences[std::size_t( index )].group < 1 )
					{
						continue;
					}
					++resolved;
					const mdl::Sequence &seq = m.sequences[std::size_t( index )];
					posed.push_back( mdl::PoseModel( m, index ) );
					inside += Inside( posed.back(), seq.boundsMin, seq.boundsMax ) ? 1 : 0;
				}
			}
			Check( m.includeModels.size() == 1 && resolved == 8,
			    "arm64x64_interior_rusty: the eight trust_fling panel sequences come from its "
			    "included animation model (" +
			        std::to_string( resolved ) + ")" );
			Check( inside == 8, "arm64x64_interior_rusty: each posed inside its stored box (" +
			                        std::to_string( inside ) + " of 8)" );
			int distinct = 0;
			for ( std::size_t a = 0; a < posed.size(); ++a )
			{
				for ( std::size_t b = a + 1; b < posed.size(); ++b )
				{
					distinct += SameBounds( posed[a], posed[b] ) ? 0 : 1;
				}
			}
			Check( posed.size() == 8 && distinct == 28,
			    "arm64x64_interior_rusty: the eight arms reach eight different places (bind pose: "
			    "all overlap)" );
		}
	}
	if ( vpk->HasAsset( "models/elevator/elevator_tube_opener.mdl" ) )
	{
		const mdl::Quaternion half{ 0.5f, 0.5f, 0.5f, 0.5f };
		NamedModel( files, "models/elevator/elevator_tube_opener.mdl", "close_idle", half, true );
		NamedModel( files, "models/elevator/elevator_blades.mdl", "spin",
		    { 0.242f, -0.664f, -0.664f, 0.242f }, true );
		NamedModel( files, "models/elevator/elevator_b.mdl", "BindPose", half, true );
		NamedModel( files, "models/props/faith_plate.mdl", "idle", half, true );
		NamedModel( files, "models/npcs/turret/turret.mdl", "idle", half, true );
	}

	// The cube, checked in detail against figures an independent reader
	// (a Python walk of the same records) gave: Portal's version 44 box
	// and Portal 2's version 49 box.
	if ( vpk->HasAsset( "models/props/metal_box.mdl" ) &&
	     vpk->HasAsset( "materials/models/props/metal_box.vmt" ) )
	{
		auto box = mdl::LoadModel( files, "models/props/metal_box.mdl" );
		Check( box.HasValue(), "metal_box.mdl parses" );
		if ( box.HasValue() )
		{
			const mdl::Model &m = box.Value();
			const bool portal2 = m.version == 49;
			const std::size_t triangles = portal2 ? 5352 : 4664;
			const std::size_t textures = portal2 ? 12 : 2;
			Check( ( m.version == 44 || portal2 ) && m.meshes.size() == 1 &&
			           m.TriangleCount() == triangles,
			    "metal_box: version " + std::to_string( m.version ) + ", one mesh, " +
			        std::to_string( triangles ) + " triangles" );
			Check( m.textures.size() == textures && m.textures[0] == "metal_box" &&
			           m.cdMaterials == std::vector<std::string>{ "models/props/" },
			    "metal_box: " + std::to_string( textures ) +
			        " textures, $cdmaterials models/props/" );
			const auto materials = mdl::ResolveMaterials( m, files );
			Check( !materials.empty() &&
			           materials[0] == mdl::ResolvedMaterial{ "models/props/metal_box", true },
			    "metal_box: skin 0 material resolves" );
			Check( m.mins.x > -40 && m.maxs.x < 40 && m.maxs.z - m.mins.z > 20,
			    "metal_box: a cube about 36 units across" );
		}
	}
}

} // namespace

int main()
{
	ImportedMeshComparatorControls();
	const char *list = std::getenv( "STUDIO_MODEL_CORPUS_VPKS" );
	Check( list && *list, "STUDIO_MODEL_CORPUS_VPKS names the corpus" );
	if ( list )
	{
		std::stringstream stream( list );
		std::string path;
		while ( std::getline( stream, path, ',' ) )
		{
			if ( !path.empty() )
			{
				RunVpk( path );
			}
		}
	}
	return testing::ReportConformance( g_checks, g_failures );
}
