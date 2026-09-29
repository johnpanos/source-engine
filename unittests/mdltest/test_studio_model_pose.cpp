//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance suite for the skeleton, first frames and posing of
//			content.studio-model.v1 (unittests/mdltest/contracts/
//			content.studio-model.v1.md): synthetic models written by
//			synthetic_model.h, whose expected poses are worked out by hand
//			here (not with the reader's math), for every encoding a first
//			frame can use: raw Quaternion64 (the turbine elevator's root),
//			raw Quaternion48 and Vector48, run-length values with their
//			scales and euler base, frame x bone data (Quaternion48S, float
//			vectors, constants), sections, .ani blocks and zero-frame data;
//			hierarchy, blended weights, sequence bone weights, delta
//			sequences and the static-prop rule. Seeded defects (swapped
//			quaternion components, a dropped parent concatenation, an
//			ignored posscale) must be rejected by the same checks. Malformed
//			skeletons, weights, sequences and animation data fail with their
//			named status and file; random damage never breaks posing.
//
//=============================================================================//

#include "synthetic_model.h"

#include "mdl/studio_model.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
#include <random>
#include <string>

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
		std::printf( "FAIL: %s\n", what.c_str() );
	}
}

using mdl::Float3;
using mdl::Quaternion;
using mdltest::Record;
using mdltest::SyntheticAnimation;
using mdltest::SyntheticBone;
using mdltest::SyntheticFiles;
using mdltest::SyntheticModel;
using mdltest::SyntheticSequence;

constexpr float kHalfSqrt2 = 0.70710678f;

foundation::Expected<mdl::Model, mdl::ModelError> Parse( const SyntheticFiles &files )
{
	return mdl::ParseModel( mdl::ModelBytes{ files.mdl, files.vvd, files.vtx, files.ani } );
}

bool Near( const Float3 &a, const Float3 &b, float eps = 1.0e-3f )
{
	return std::fabs( a.x - b.x ) <= eps && std::fabs( a.y - b.y ) <= eps &&
	       std::fabs( a.z - b.z ) <= eps;
}

bool NearQ( const Quaternion &a, const Quaternion &b, float eps = 1.0e-3f )
{
	const float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	return std::fabs( std::fabs( dot ) - 1.0f ) <= eps;
}

std::string Text( const Float3 &p )
{
	char buffer[96];
	std::snprintf( buffer, sizeof( buffer ), "(%.3f %.3f %.3f)", p.x, p.y, p.z );
	return buffer;
}

bool Fails( const SyntheticFiles &files, mdl::ModelStatus status, mdl::ModelFile file )
{
	auto result = Parse( files );
	if ( result.HasValue() )
	{
		std::printf( "  parsed\n" );
		return false;
	}
	if ( result.Error().status != status || result.Error().file != file )
	{
		std::printf( "  got %s\n", mdl::Describe( result.Error() ).c_str() );
		return false;
	}
	return true;
}

// A translation-only matrix (the poseToBone of an unrotated bone at 't' is
// Translation(-t)).
mdl::Matrix3x4 Translation( Float3 t )
{
	mdl::Matrix3x4 m;
	m.m[0][3] = t.x;
	m.m[1][3] = t.y;
	m.m[2][3] = t.z;
	return m;
}

std::size_t Get32( const std::string &bytes, std::size_t at )
{
	std::uint32_t u = 0;
	for ( int i = 0; i < 4; ++i )
	{
		u |= std::uint32_t( static_cast<std::uint8_t>( bytes[at + i] ) ) << ( 8 * i );
	}
	return static_cast<std::size_t>( static_cast<std::int32_t>( u ) );
}

// A one-mesh model: a box of half-extents 'half' at 'center', one texture.
SyntheticModel BoxAt( Float3 center, Float3 half )
{
	SyntheticModel model = mdltest::BoxModel( half, "a/", "t" );
	model.bodyParts[0][0][0] = mdltest::BoxMesh( center, half );
	return model;
}

// One animation (its records) and one sequence drawing it.
void Animate( SyntheticModel &model, const std::string &label, std::string data,
    std::uint32_t animFlags = 0, std::uint32_t seqFlags = 0 )
{
	SyntheticAnimation anim;
	anim.flags = animFlags;
	anim.data = std::move( data );
	model.animations.push_back( anim );
	SyntheticSequence seq;
	seq.label = label;
	seq.flags = seqFlags;
	seq.blends = { static_cast<std::int16_t>( model.animations.size() - 1 ) };
	model.sequences.push_back( seq );
}

std::string RawRotation64( Quaternion q )
{
	return mdltest::Quaternion64( q );
}

// --- Skeleton and the reference pose ------------------------------------------------------

void SkeletonCases()
{
	// A box with the default skeleton: one identity root, no sequences.
	auto plain = Parse( mdltest::WriteModel( BoxAt( { 0, 0, 0 }, { 2, 3, 4 } ) ) );
	Check( plain.HasValue(), "a box with the default skeleton parses" );
	if ( plain.HasValue() )
	{
		const mdl::Model &m = plain.Value();
		Check( m.bones.size() == 1 && m.bones[0].name == "root" && m.bones[0].parent == -1 &&
		           m.sequences.empty(),
		    "one root bone, no sequences" );
		Check( m.meshes[0].weights.size() == m.meshes[0].vertices.size() &&
		           m.meshes[0].weights[0] == mdl::BoneWeights{},
		    "each vertex has its weights (bone 0, weight 1)" );
		Check( mdl::PoseModel( m, 0 ) == m && mdl::PoseModel( m, -1 ) == m,
		    "posing by a sequence the model lacks leaves it unchanged" );
		Check( mdl::FindSequence( m, "idle" ) == -1, "FindSequence: none" );
	}

	// Two bones: a root at (0, 0, 10) turned 90 degrees about z, an arm 5 units
	// along the root's x. Its bind-space vertices, some on each bone, one
	// shared.
	SyntheticModel two = BoxAt( { 0, 5, 10 }, { 1, 1, 1 } );
	SyntheticBone root;
	root.name = "Root";
	root.position = { 0, 0, 10 };
	root.rotation = { 0, 0, kHalfSqrt2, kHalfSqrt2 };
	// boneToModel(root) = Rz90, t (0 0 10); its inverse: Rz-90, t (0 0 -10).
	root.poseToBone.m = { { { 0, 1, 0, 0 }, { -1, 0, 0, 0 }, { 0, 0, 1, -10 } } };
	SyntheticBone arm;
	arm.name = "Arm";
	arm.parent = 0;
	arm.position = { 5, 0, 0 };
	// boneToModel(arm) = Rz90, t (0 5 10).
	arm.poseToBone.m = { { { 0, 1, 0, -5 }, { -1, 0, 0, 0 }, { 0, 0, 1, -10 } } };
	arm.flags = 0x100;
	two.bones = { root, arm };
	auto &mesh = two.bodyParts[0][0][0];
	mesh.weights.assign( mesh.vertices.size(), mdl::BoneWeights{} );
	for ( std::size_t v = 0; v < mesh.weights.size(); ++v )
	{
		if ( v % 3 == 1 )
		{
			mesh.weights[v] = { 1, { 1, 0, 0 }, { 1, 0, 0 } };
		}
		else if ( v % 3 == 2 )
		{
			mesh.weights[v] = { 2, { 0, 1, 0 }, { 0.25f, 0.75f, 0 } };
		}
	}
	auto parsed = Parse( mdltest::WriteModel( two ) );
	Check( parsed.HasValue(), "a two-bone model parses" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	const mdl::Model &m = parsed.Value();
	Check( m.bones.size() == 2 && m.bones[0].name == "Root" && m.bones[1].name == "Arm" &&
	           m.bones[1].parent == 0 && m.bones[1].flags == 0x100,
	    "bone names (case kept), parents and flags" );
	Check( m.bones[0].position == Float3{ 0, 0, 10 } &&
	           m.bones[0].rotation == Quaternion{ 0, 0, kHalfSqrt2, kHalfSqrt2 } &&
	           m.bones[1].poseToBone == arm.poseToBone,
	    "reference position, rotation and poseToBone as stored" );
	Check( m.meshes[0].weights[2] == mdl::BoneWeights{ 2, { 0, 1, 0 }, { 0.25f, 0.75f, 0 } },
	    "blended vertex weights as stored" );
	const std::vector<mdl::Matrix3x4> b2m = mdl::BoneToModel( m, mdl::ReferencePose( m ) );
	Check(
	    b2m.size() == 2 && Near( { b2m[1].m[0][3], b2m[1].m[1][3], b2m[1].m[2][3] }, { 0, 5, 10 } ),
	    "bone-to-model concatenates the parent: the arm is at (0 5 10)" );
	const mdl::Model reference = mdl::PoseModel( m, mdl::ReferencePose( m ) );
	bool same = reference.meshes.size() == 1;
	for ( std::size_t v = 0; same && v < m.meshes[0].vertices.size(); ++v )
	{
		same =
		    Near( reference.meshes[0].vertices[v].position, m.meshes[0].vertices[v].position,
		        1.0e-4f ) &&
		    Near( reference.meshes[0].vertices[v].normal, m.meshes[0].vertices[v].normal, 1.0e-4f );
	}
	Check(
	    same && Near( reference.mins, m.mins, 1.0e-4f ) && Near( reference.maxs, m.maxs, 1.0e-4f ),
	    "posing at the reference pose gives the bind vertices, normals and bounds" );
	Check( mdl::PoseModel( m, std::vector<mdl::BoneTransform>( 1 ) ) == m,
	    "a pose with the wrong bone count leaves the model unchanged" );
}

// --- First frames -------------------------------------------------------------------------

// The turbine elevator's case: a Y-up reference drawn Z-up by the root's raw
// Quaternion64 (0.5 0.5 0.5 0.5), which maps (x y z) to (z x y).
void RawQuaternion64Cases()
{
	SyntheticModel model = BoxAt( { 0, 10, 0 }, { 1, 5, 2 } );
	Animate( model, "BindPose",
	    mdltest::Records( { { 0, 0x20, RawRotation64( { 0.5f, 0.5f, 0.5f, 0.5f } ) } } ) );
	Animate( model, "close_idle",
	    mdltest::Records( { { 0, 0x20, RawRotation64( { 0.5f, 0.5f, 0.5f, 0.5f } ) } } ) );
	auto parsed = Parse( mdltest::WriteModel( model ) );
	Check( parsed.HasValue(), "raw Quaternion64: parses" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	const mdl::Model &m = parsed.Value();
	Check( m.sequences.size() == 2 && m.sequences[1].label == "close_idle" &&
	           mdl::FindSequence( m, "CLOSE_IDLE" ) == 1,
	    "sequence labels; FindSequence ignores case" );
	Check( NearQ( m.sequences[1].firstFrame[0].rotation, { 0.5f, 0.5f, 0.5f, 0.5f }, 1.0e-5f ),
	    "raw Quaternion64 decodes" );
	const mdl::Model posed = mdl::PoseModel( m, 1 );
	// Bind box (-1 5 -2)-(1 15 2) -> (z x y): (-2 -1 5)-(2 1 15).
	Check( Near( posed.mins, { -2, -1, 5 } ) && Near( posed.maxs, { 2, 1, 15 } ),
	    "posed: the Y-up box stands along z " + Text( posed.mins ) + Text( posed.maxs ) );
	bool normals = true;
	const auto &bind = m.meshes[0].vertices;
	const auto &moved = posed.meshes[0].vertices;
	for ( std::size_t v = 0; v < bind.size(); ++v )
	{
		normals = normals &&
		          Near( moved[v].normal, { bind[v].normal.z, bind[v].normal.x, bind[v].normal.y } );
	}
	Check( normals, "normals turn with the vertices" );
	Check( posed.meshes[0].indices == m.meshes[0].indices && moved.size() == bind.size() &&
	           moved[0].u == bind[0].u,
	    "posing keeps the triangles and uv" );
}

// Raw Quaternion48 (90 degrees about x) and a raw Vector48 position.
void RawQuaternion48Cases()
{
	SyntheticModel model = BoxAt( { 0, 10, 0 }, { 1, 5, 2 } );
	const Quaternion rx90{ kHalfSqrt2, 0, 0, kHalfSqrt2 };
	Animate( model, "idle",
	    mdltest::Records( { { 0, 0x02 | 0x01,
	        mdltest::Quaternion48( rx90 ) + mdltest::Vector48( { 0, 0, 32 } ) } } ) );
	auto parsed = Parse( mdltest::WriteModel( model ) );
	Check( parsed.HasValue(), "raw Quaternion48 and Vector48: parses" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	const mdl::Model &m = parsed.Value();
	const mdl::BoneTransform &frame = m.sequences[0].firstFrame[0];
	Check( NearQ( frame.rotation, rx90, 1.0e-4f ) && Near( frame.position, { 0, 0, 32 } ),
	    "raw Quaternion48 and Vector48 decode" );
	// Rx90: (x y z) -> (x -z y); bind (-1 5 -2)-(1 15 2) -> (-1 -2 5)-(1 2 15), + (0 0 32).
	const Float3 lo{ -1, -2, 37 };
	const Float3 hi{ 1, 2, 47 };
	const mdl::Model posed = mdl::PoseModel( m, 0 );
	Check( Near( posed.mins, lo, 0.01f ) && Near( posed.maxs, hi, 0.01f ),
	    "posed by the raw rotation and position " + Text( posed.mins ) + Text( posed.maxs ) );
	// Seeded defect: x and y read swapped (a rotation about y instead).
	std::vector<mdl::BoneTransform> swapped = m.sequences[0].firstFrame;
	std::swap( swapped[0].rotation.x, swapped[0].rotation.y );
	const mdl::Model defect = mdl::PoseModel( m, swapped );
	Check( !( Near( defect.mins, lo, 0.01f ) && Near( defect.maxs, hi, 0.01f ) ),
	    "seeded defect: swapped quaternion components are rejected" );
}

// Run-length values: rotation = euler base + value x rotscale, position =
// reference + value x posscale.
void StreamCases()
{
	// The root's reference is at (1 2 3), unrotated; the euler base the
	// rotation values add to is 0.5 radians about z.
	SyntheticModel model = BoxAt( { 1, 2, 3 }, { 2, 1, 1 } );
	SyntheticBone root;
	root.position = { 1, 2, 3 };
	root.euler = { 0, 0, 0.5f };
	root.poseToBone = Translation( { -1, -2, -3 } );
	root.rotationScale = { 0, 0, 1.0e-4f };
	root.positionScale = { 0.5f, 0.5f, 0.5f };
	model.bones = { root };
	const std::int16_t rotation[3] = { 0, 0, 10708 }; // + 0.5 base = 1.5708: 90 degrees
	const std::int16_t position[3] = { 0, 0, 20 };    // 20 x 0.5 = +10
	Animate( model, "idle",
	    mdltest::Records(
	        { { 0, 0x08 | 0x04, mdltest::RotationPositionStreams( rotation, position ) } } ) );
	auto parsed = Parse( mdltest::WriteModel( model ) );
	Check( parsed.HasValue(), "run-length streams: parses" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	const mdl::Model &m = parsed.Value();
	const mdl::BoneTransform &frame = m.sequences[0].firstFrame[0];
	const Quaternion rz90{ 0, 0, kHalfSqrt2, kHalfSqrt2 };
	Check( NearQ( frame.rotation, rz90, 1.0e-6f ),
	    "rotation: euler base plus value x rotscale (90 degrees about z)" );
	Check( Near( frame.position, { 1, 2, 13 }, 1.0e-5f ),
	    "position: reference plus value x posscale " + Text( frame.position ) );
	// In bone space the box is (-2 -1 -1)-(2 1 1); Rz90 (x y z -> -y x z)
	// gives (-1 -2 -1)-(1 2 1), placed at (1 2 13).
	const Float3 lo{ 0, 0, 12 };
	const Float3 hi{ 2, 4, 14 };
	const mdl::Model posed = mdl::PoseModel( m, 0 );
	Check( Near( posed.mins, lo ) && Near( posed.maxs, hi ),
	    "posed by the streams " + Text( posed.mins ) + Text( posed.maxs ) );
	// Seeded defect: posscale ignored (the stored value, 20, used as is).
	std::vector<mdl::BoneTransform> unscaled = m.sequences[0].firstFrame;
	unscaled[0].position.z = 3.0f + 20.0f;
	const mdl::Model defect = mdl::PoseModel( m, unscaled );
	Check( !( Near( defect.mins, lo ) && Near( defect.maxs, hi ) ),
	    "seeded defect: an ignored posscale is rejected" );

	// A second stream layout: a run of no frames the engine steps over, and a
	// run with no valid value, which reads its own header (bone_setup.cpp).
	SyntheticModel odd = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	odd.bones.resize( 1 );
	odd.bones[0].positionScale = { 1, 1, 1 };
	std::string streams( 6, '\0' );
	mdltest::detail::Put16( streams, 0, 6 );  // x: an empty run, then (1, 1, 7)
	mdltest::detail::Put16( streams, 2, 12 ); // y: (0 valid, 2 total): the header, 0x0200
	streams += mdltest::Bytes16( { 0x0000, 0x0101, 7 } );
	streams += mdltest::Bytes16( { 0x0200 } );
	Animate( odd, "idle", mdltest::Records( { { 0, 0x04, streams } } ) );
	auto oddParsed = Parse( mdltest::WriteModel( odd ) );
	Check( oddParsed.HasValue() &&
	           Near( oddParsed.Value().sequences[0].firstFrame[0].position, { 7, 512, 0 } ),
	    "run-length edge cases read as the engine reads them" );
}

// Parent concatenation and blended weights.
void HierarchyCases()
{
	// Root at the origin; a child 10 units along x. A vertex at (10 0 0) on
	// the child, one at (20 0 0) half on each.
	SyntheticModel model = BoxAt( { 10, 0, 0 }, { 1, 1, 1 } );
	SyntheticBone child;
	child.name = "child";
	child.parent = 0;
	child.position = { 10, 0, 0 };
	child.poseToBone = Translation( { -10, 0, 0 } );
	model.bones = { SyntheticBone{}, child };
	auto &mesh = model.bodyParts[0][0][0];
	mesh.weights.assign( mesh.vertices.size(), { 1, { 1, 0, 0 }, { 1, 0, 0 } } );
	const Quaternion rz90{ 0, 0, kHalfSqrt2, kHalfSqrt2 };
	// Root turned 90 degrees about z; the child keeps its reference.
	Animate( model, "turn", mdltest::Records( { { 0, 0x20, RawRotation64( rz90 ) } } ) );
	// The child turned 90 degrees about z; the root keeps its reference.
	Animate( model, "bend", mdltest::Records( { { 1, 0x20, RawRotation64( rz90 ) } } ) );
	auto parsed = Parse( mdltest::WriteModel( model ) );
	Check( parsed.HasValue(), "hierarchy: parses" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	mdl::Model m = parsed.Value();
	Check( m.sequences[0].firstFrame[1].position == Float3{ 10, 0, 0 } &&
	           m.sequences[0].firstFrame[1].rotation == Quaternion{},
	    "a bone without a record keeps its reference" );
	// turn: the child's box (9 -1 -1)-(11 1 1) about the root: (-1 9 -1)-(1 11 1).
	const Float3 lo{ -1, 9, -1 };
	const Float3 hi{ 1, 11, 1 };
	const mdl::Model turned = mdl::PoseModel( m, 0 );
	Check( Near( turned.mins, lo ) && Near( turned.maxs, hi ),
	    "a child follows its parent " + Text( turned.mins ) + Text( turned.maxs ) );
	// Seeded defect: the parent's transform not concatenated.
	mdl::Model orphan = m;
	orphan.bones[1].parent = -1;
	const mdl::Model defect = mdl::PoseModel( orphan, m.sequences[0].firstFrame );
	Check( !( Near( defect.mins, lo ) && Near( defect.maxs, hi ) ),
	    "seeded defect: a dropped parent concatenation is rejected" );
	// bend: vertex (11 0 0) on the child -> (10 1 0); half on the root (11 0 0):
	// the blend is (10.5 0.5 0).
	m.meshes[0].weights[0] = { 2, { 0, 1, 0 }, { 0.5f, 0.5f, 0 } };
	const Float3 bind = m.meshes[0].vertices[0].position;
	const mdl::Model bent = mdl::PoseModel( m, 1 );
	const Float3 onChild{ 10.0f - bind.y, bind.x - 10.0f, bind.z };
	const Float3 expected{ ( bind.x + onChild.x ) * 0.5f, ( bind.y + onChild.y ) * 0.5f,
	    ( bind.z + onChild.z ) * 0.5f };
	Check( Near( bent.meshes[0].vertices[0].position, expected ),
	    "two weights blend their bones' skinned positions " +
	        Text( bent.meshes[0].vertices[0].position ) + " expected " + Text( expected ) );
}

// Frame x bone data: Quaternion48S constants, float and Vector48 positions,
// Quaternion48 frames.
void FrameAnimCases()
{
	SyntheticModel model = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	SyntheticBone child;
	child.parent = 0;
	child.position = { 3, 0, 0 };
	child.poseToBone = Translation( { -3, 0, 0 } );
	model.bones = { SyntheticBone{}, child, SyntheticBone{} };
	model.bones[2].parent = 1;
	model.bones[2].position = { 0, 4, 0 };
	const Quaternion rz90{ 0, 0, kHalfSqrt2, kHalfSqrt2 };
	const Quaternion rx90{ kHalfSqrt2, 0, 0, kHalfSqrt2 };
	// bone 0: constant rotation (48S, stored from component 3) and a float
	// position in the frame; bone 1: a Quaternion48 rotation in the frame and
	// a constant Vector48 position; bone 2: no data.
	const std::string constants =
	    mdltest::Quaternion48S( rz90, 3 ) + mdltest::Vector48( { 6, 0, 0 } );
	std::string frame;
	const float fp[3] = { 1.5f, -2.25f, 8.0f };
	frame.append( reinterpret_cast<const char *>( fp ), 12 );
	frame += mdltest::Quaternion48( rx90 );
	Animate( model, "idle",
	    mdltest::FrameAnim( { 0x40 | 0x10, 0x08 | 0x01, 0x00 }, constants, frame ), 0x40 );
	auto parsed = Parse( mdltest::WriteModel( model ) );
	Check( parsed.HasValue(), "frame x bone: parses" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	const std::vector<mdl::BoneTransform> &f = parsed.Value().sequences[0].firstFrame;
	Check( NearQ( f[0].rotation, rz90, 1.0e-4f ) && Near( f[0].position, { 1.5f, -2.25f, 8 } ),
	    "bone 0: constant Quaternion48S and a float frame position" );
	Check( NearQ( f[1].rotation, rx90, 1.0e-4f ) && Near( f[1].position, { 6, 0, 0 } ),
	    "bone 1: frame Quaternion48 and a constant Vector48" );
	Check( f[2].position == Float3{ 0, 4, 0 } && f[2].rotation == Quaternion{},
	    "bone 2 without data keeps its reference" );
	// Every Quaternion48S layout (which component is rebuilt).
	bool all = true;
	for ( int first = 0; first < 4; ++first )
	{
		// The rebuilt component is the largest (the stored ones fit +-0.707).
		float c[4] = {};
		c[first] = 0.1f;
		c[( first + 1 ) % 4] = -0.3f;
		c[( first + 2 ) % 4] = 0.5f;
		c[( first + 3 ) % 4] = first % 2 ? -0.8062258f : 0.8062258f;
		const Quaternion q{ c[0], c[1], c[2], c[3] };
		SyntheticModel one = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
		Animate( one, "idle",
		    mdltest::FrameAnim( { 0x40 }, mdltest::Quaternion48S( q, first ), "" ), 0x40 );
		auto p = Parse( mdltest::WriteModel( one ) );
		all = all && p.HasValue() &&
		      NearQ( p.Value().sequences[0].firstFrame[0].rotation, q, 1.0e-4f );
	}
	Check( all, "Quaternion48S rebuilds whichever component it leaves out" );
}

// Sections, .ani blocks and zero-frame data.
void BlockCases()
{
	const Quaternion rz90{ 0, 0, kHalfSqrt2, kHalfSqrt2 };
	const Quaternion rx90{ kHalfSqrt2, 0, 0, kHalfSqrt2 };
	const std::string records = mdltest::Records( { { 0, 0x20, RawRotation64( rz90 ) } } );

	SyntheticModel sectioned = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	Animate( sectioned, "idle", records );
	sectioned.animations[0].sectioned = true;
	auto s = Parse( mdltest::WriteModel( sectioned ) );
	Check( s.HasValue() && NearQ( s.Value().sequences[0].firstFrame[0].rotation, rz90, 1.0e-5f ),
	    "a sectioned animation's frame 0 is in section 0" );

	SyntheticModel blocked = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	blocked.bones = { SyntheticBone{} };
	blocked.bones[0].flags = 0x00400000; // saves a zero-frame rotation
	Animate( blocked, "idle", records );
	blocked.animations[0].block = 1;
	blocked.animations[0].sectioned = true;
	const SyntheticFiles files = mdltest::WriteModel( blocked );
	auto b = Parse( files );
	Check( b.HasValue() && NearQ( b.Value().sequences[0].firstFrame[0].rotation, rz90, 1.0e-5f ),
	    "an animation in an .ani block decodes from the .ani" );
	SyntheticFiles noAni = files;
	noAni.ani.clear();
	Check( Fails( noAni, mdl::ModelStatus::MissingFile, mdl::ModelFile::Ani ),
	    "without the .ani and without zero-frame data: MissingFile in ani" );
	blocked.animations[0].zeroFrame = RawRotation64( rx90 );
	SyntheticFiles zero = mdltest::WriteModel( blocked );
	zero.ani.clear();
	auto z = Parse( zero );
	Check( z.HasValue() && NearQ( z.Value().sequences[0].firstFrame[0].rotation, rx90, 1.0e-5f ),
	    "without the .ani: the zero-frame data stands in, as the engine draws it" );

	blocked.bones[0].flags = 0x00800000; // Quaternion32 zero-frame rotations
	SyntheticFiles rot32 = mdltest::WriteModel( blocked );
	rot32.ani.clear();
	Check( Fails( rot32, mdl::ModelStatus::UnsupportedVersion, mdl::ModelFile::Mdl ),
	    "Quaternion32 zero-frame data is refused, not misread" );

	// LoadModel reads the .ani the header names.
	class Files : public mdl::IModelFiles
	{
	public:
		std::map<std::string, std::string> files;
		bool Exists( const std::string &path ) const override { return files.count( path ) != 0; }
		bool Read( const std::string &path, std::string &out ) const override
		{
			auto it = files.find( path );
			if ( it == files.end() )
			{
				return false;
			}
			out = it->second;
			return true;
		}
	} store;
	store.files["models/m.mdl"] = files.mdl;
	store.files["models/m.vvd"] = files.vvd;
	store.files["models/m.dx90.vtx"] = files.vtx;
	store.files["synthetic/box.ani"] = files.ani;
	auto loaded = mdl::LoadModel( store, "models/m.mdl" );
	Check( loaded.HasValue() &&
	           NearQ( loaded.Value().sequences[0].firstFrame[0].rotation, rz90, 1.0e-5f ),
	    "LoadModel reads the .ani its header names" );
	store.files.erase( "synthetic/box.ani" );
	auto missing = mdl::LoadModel( store, "models/m.mdl" );
	Check( !missing.HasValue() && missing.Error().file == mdl::ModelFile::Ani,
	    "LoadModel without that .ani: MissingFile in ani" );
}

// Sequence bone weights, delta sequences, blends and the static-prop rule.
void SequenceCases()
{
	const Quaternion rz90{ 0, 0, kHalfSqrt2, kHalfSqrt2 };
	SyntheticModel model = BoxAt( { 5, 0, 0 }, { 1, 1, 1 } );
	model.bones = { SyntheticBone{}, SyntheticBone{} };
	model.bones[1].parent = 0;
	const std::string both = mdltest::Records(
	    { { 0, 0x20, RawRotation64( rz90 ) }, { 1, 0x20, RawRotation64( rz90 ) } } );
	Animate( model, "half", both );
	model.sequences[0].weights = { 0.5f, 0.0f };
	// A delta: +90 degrees about z on the root, +(0 0 5) on the child.
	model.bones[1].positionScale = { 1, 1, 1 };
	const std::int16_t position[3] = { 0, 0, 5 };
	Animate( model, "delta",
	    mdltest::Records( { { 0, 0x20 | 0x10, RawRotation64( rz90 ) },
	        { 1, 0x04 | 0x10, mdltest::ValueStreams( position ) } } ),
	    0x04, mdl::kDeltaSequence );
	// A two-animation blend grid draws its first animation.
	Animate( model, "blend", both );
	model.sequences[2].blends = { 0, 1 };
	auto parsed = Parse( mdltest::WriteModel( model ) );
	Check( parsed.HasValue(), "sequence weights, deltas and blends: parse" );
	if ( !parsed.HasValue() )
	{
		return;
	}
	const mdl::Model &m = parsed.Value();
	const Quaternion rz45{ 0, 0, std::sin( 0.3926991f ), std::cos( 0.3926991f ) };
	Check( m.sequences[0].boneWeights == std::vector<float>{ 0.5f, 0.0f },
	    "the sequence's bone weights as stored" );
	Check( NearQ( m.sequences[0].firstFrame[0].rotation, rz45, 1.0e-4f ) &&
	           m.sequences[0].firstFrame[1].rotation == Quaternion{},
	    "weight 0.5 goes halfway from the reference; weight 0 keeps it" );
	Check( NearQ( m.sequences[1].firstFrame[0].rotation, rz90, 1.0e-5f ) &&
	           Near( m.sequences[1].firstFrame[1].position, { 0, 0, 5 } ),
	    "a delta sequence adds its frame to the reference" );
	Check( m.sequences[2].blendCount == 2 && m.sequences[2].animation == 0,
	    "a blend grid draws its first animation" );

	// A $staticprop is drawn unposed by sequence, as the engine draws it.
	SyntheticModel prop = BoxAt( { 0, 10, 0 }, { 1, 5, 2 } );
	prop.flags = mdl::kStaticPropFlag;
	Animate( prop, "idle", mdltest::Records( { { 0, 0x20, RawRotation64( rz90 ) } } ) );
	auto p = Parse( mdltest::WriteModel( prop ) );
	Check( p.HasValue() && mdl::PoseModel( p.Value(), 0 ) == p.Value() &&
	           !( mdl::PoseModel( p.Value(), p.Value().sequences[0].firstFrame ) == p.Value() ),
	    "a $staticprop posed by sequence is unchanged; by its frame explicitly it is not" );
}

// $includemodel: the engine's virtual model.
void IncludeCases()
{
	const Quaternion rz90{ 0, 0, kHalfSqrt2, kHalfSqrt2 };
	// The model: root, Arm (at x 10) and Tail; a 6 x 2 x 2 box on the arm.
	SyntheticModel base = BoxAt( { 10, 0, 0 }, { 3, 1, 1 } );
	SyntheticBone arm;
	arm.name = "Arm";
	arm.parent = 0;
	arm.position = { 10, 0, 0 };
	arm.poseToBone = Translation( { -10, 0, 0 } );
	SyntheticBone tail;
	tail.name = "Tail";
	tail.parent = 0;
	tail.position = { -5, 0, 0 };
	tail.poseToBone = Translation( { 5, 0, 0 } );
	base.bones = { SyntheticBone{}, arm, tail };
	auto &mesh = base.bodyParts[0][0][0];
	mesh.weights.assign( mesh.vertices.size(), { 1, { 1, 0, 0 }, { 1, 0, 0 } } );
	Animate( base, "idle", mdltest::Records( { { 0, 0x20, RawRotation64( {} ) } } ) );
	Animate( base, "forward", mdltest::Records( { { 0, 0x20, RawRotation64( {} ) } } ) );
	base.sequences[1].flags = mdl::kOverrideSequence;
	base.includes = { "Models/Base_Anims.mdl" };

	// The animation model: its own bone order and case, a bone the model
	// lacks, no Tail.
	SyntheticModel anims = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	anims.checksum = 0x1234;
	SyntheticBone extra;
	extra.name = "Extra";
	extra.position = { 0, 0, 99 };
	SyntheticBone root;
	root.name = "ROOT";
	SyntheticBone arm2 = arm;
	arm2.name = "arm";
	arm2.parent = 1;
	anims.bones = { extra, root, arm2 };
	Animate( anims, "IDLE", mdltest::Records( { { 1, 0x20, RawRotation64( rz90 ) } } ) );
	Animate( anims, "wave",
	    mdltest::Records(
	        { { 0, 0x20, RawRotation64( rz90 ) }, { 2, 0x20, RawRotation64( rz90 ) } } ) );
	Animate( anims, "forward", mdltest::Records( { { 2, 0x20, RawRotation64( rz90 ) } } ) );
	const SyntheticFiles baseFiles = mdltest::WriteModel( base );
	const SyntheticFiles animFiles = mdltest::WriteModel( anims );

	mdl::ModelBytes bytes{ baseFiles.mdl, baseFiles.vvd, baseFiles.vtx, {} };
	bytes.includes.push_back( { animFiles.mdl, {} } );
	auto merged = mdl::ParseModel( bytes );
	Check( merged.HasValue(), "a model with an included animation model parses" );
	if ( !merged.HasValue() )
	{
		std::printf( "  %s\n", mdl::Describe( merged.Error() ).c_str() );
		return;
	}
	const mdl::Model &m = merged.Value();
	Check( m.includeModels == std::vector<std::string>{ "models/base_anims.mdl" },
	    "the $includemodel list, as stored (lower case)" );
	Check( m.sequences.size() == 3 && m.sequences[0].label == "idle" && m.sequences[0].group == 0 &&
	           m.sequences[1].label == "forward" && m.sequences[1].group == 1 &&
	           m.sequences[2].label == "wave" && m.sequences[2].group == 1,
	    "own sequences first; a duplicate label keeps the first; a forward declaration is "
	    "replaced in place; new labels follow" );
	if ( m.sequences.size() != 3 )
	{
		return;
	}
	const mdl::Sequence &wave = m.sequences[2];
	Check( NearQ( wave.firstFrame[1].rotation, rz90, 1.0e-5f ) &&
	           wave.firstFrame[1].position == Float3{ 10, 0, 0 },
	    "an included bone drives this model's bone of the same name (any case)" );
	Check( wave.firstFrame[0].rotation == Quaternion{} &&
	           wave.firstFrame[2].position == Float3{ -5, 0, 0 } && wave.boneWeights[2] == 0.0f,
	    "a bone the included model lacks keeps this model's reference; its bone the model lacks "
	    "is ignored" );
	// The arm's box (7 -1 -1)-(13 1 1) turned 90 degrees about its bone at x 10.
	const mdl::Model posed = mdl::PoseModel( m, mdl::FindSequence( m, "Wave" ) );
	Check( Near( posed.mins, { 9, -3, -1 } ) && Near( posed.maxs, { 11, 3, 1 } ),
	    "posed by an included sequence " + Text( posed.mins ) + Text( posed.maxs ) );

	// LoadModel finds the include through the same files; an absent one is
	// skipped, as the engine skips it.
	class Files : public mdl::IModelFiles
	{
	public:
		std::map<std::string, std::string> files;
		bool Exists( const std::string &path ) const override { return files.count( path ) != 0; }
		bool Read( const std::string &path, std::string &out ) const override
		{
			auto it = files.find( path );
			if ( it == files.end() )
			{
				return false;
			}
			out = it->second;
			return true;
		}
	} store;
	store.files["models/base.mdl"] = baseFiles.mdl;
	store.files["models/base.vvd"] = baseFiles.vvd;
	store.files["models/base.dx90.vtx"] = baseFiles.vtx;
	store.files["models/base_anims.mdl"] = animFiles.mdl;
	auto loaded = mdl::LoadModel( store, "models/base.mdl" );
	Check( loaded.HasValue() && loaded.Value().sequences.size() == 3 &&
	           mdl::FindSequence( loaded.Value(), "wave" ) == 2,
	    "LoadModel merges the included model's sequences" );
	store.files.erase( "models/base_anims.mdl" );
	auto alone = mdl::LoadModel( store, "models/base.mdl" );
	Check( alone.HasValue() && alone.Value().sequences.size() == 2,
	    "an absent included model is skipped" );

	// A malformed included model is an error in that file.
	std::string broken = animFiles.mdl;
	broken[0] = 'X';
	mdl::ModelBytes bad{ baseFiles.mdl, baseFiles.vvd, baseFiles.vtx, {} };
	bad.includes.push_back( { broken, {} } );
	auto rejected = mdl::ParseModel( bad );
	Check( !rejected.HasValue() && rejected.Error().status == mdl::ModelStatus::BadMagic &&
	           rejected.Error().file == mdl::ModelFile::IncludeMdl,
	    "a malformed included model: BadMagic in included mdl" );
	std::string cut = animFiles.mdl.substr( 0, animFiles.mdl.size() - 3 );
	mdl::ModelBytes truncated{ baseFiles.mdl, baseFiles.vvd, baseFiles.vtx, {} };
	truncated.includes.push_back( { cut, {} } );
	auto t = mdl::ParseModel( truncated );
	Check( !t.HasValue() && t.Error().file == mdl::ModelFile::IncludeMdl,
	    "a truncated included model fails in included mdl" );
}

// --- Malformed input ----------------------------------------------------------------------

void MalformedCases()
{
	SyntheticModel model = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	model.bones = { SyntheticBone{}, SyntheticBone{} };
	model.bones[1].parent = 0;
	Animate( model, "idle",
	    mdltest::Records(
	        { { 0, 0x20, RawRotation64( {} ) }, { 1, 0x02, mdltest::Quaternion48( {} ) } } ) );
	const SyntheticFiles good = mdltest::WriteModel( model );
	Check( Parse( good ).HasValue(), "the malformed cases' base model parses" );
	const std::size_t bones = Get32( good.mdl, 160 );
	const std::size_t descs = Get32( good.mdl, 184 );
	const std::size_t seqs = Get32( good.mdl, 192 );
	const std::size_t records = descs + Get32( good.mdl, descs + 56 );
	const std::size_t vertex0 = Get32( good.vvd, 56 );
	const auto mutate = [&]( const std::function<void( SyntheticFiles & )> &edit )
	{
		SyntheticFiles f = good;
		edit( f );
		return f;
	};
	using mdl::ModelFile;
	using mdl::ModelStatus;
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, 156, 0 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Mdl ),
	    "no bones" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, 156, 100000 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Mdl ),
	    "more bones than a vertex can name" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, bones + 216 + 4, 1 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "a bone its own parent" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, bones + 4, -2 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "a parent index below -1" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  f.vvd[vertex0 + 12] = 2;
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Vvd ),
	    "a vertex weight naming a bone that does not exist" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  f.vvd[vertex0 + 15] = 0;
	                  } ),
	           ModelStatus::BadCount, ModelFile::Vvd ),
	    "a vertex with no weights" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  f.vvd[vertex0 + 15] = 4;
	                  } ),
	           ModelStatus::BadCount, ModelFile::Vvd ),
	    "a vertex with four weights" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  const std::size_t blends = seqs + Get32( f.mdl, seqs + 60 );
		                  mdltest::detail::Put16( f.mdl, blends, 7 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "a sequence naming an animation that does not exist" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, seqs + 56, 0 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Mdl ),
	    "a sequence of no blends" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, seqs + 156, 1 << 30 );
	                  } ),
	           ModelStatus::BadOffset, ModelFile::Mdl ),
	    "a weight list outside the file" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  f.mdl[records] = 9;
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "an animation record for a bone that does not exist" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put16( f.mdl, records + 2, 0xfff0 );
	                  } ),
	           ModelStatus::BadOffset, ModelFile::Mdl ),
	    "an animation record chaining backwards" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put16( f.mdl, records + 2, 0x7ff0 );
	                  } ),
	           ModelStatus::Truncated, ModelFile::Mdl ),
	    "an animation record chaining past the end" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, descs + 52, 3 );
	                  } ),
	           ModelStatus::BadIndex, ModelFile::Mdl ),
	    "an animation block the model does not have" );
	Check( Fails( mutate(
	                  [&]( SyntheticFiles &f )
	                  {
		                  mdltest::detail::Put32( f.mdl, descs + 84, -1 );
	                  } ),
	           ModelStatus::BadCount, ModelFile::Mdl ),
	    "negative section frames" );

	// An animation in an .ani block.
	SyntheticModel blocked = model;
	blocked.animations[0].block = 1;
	const SyntheticFiles withAni = mdltest::WriteModel( blocked );
	Check( Parse( withAni ).HasValue(), "the block cases' base model parses" );
	const std::size_t table = Get32( withAni.mdl, 356 );
	SyntheticFiles shortAni = withAni;
	shortAni.ani.resize( shortAni.ani.size() - 4 );
	Check( Fails( shortAni, ModelStatus::BadOffset, ModelFile::Mdl ),
	    "a block extending past the .ani" );
	SyntheticFiles narrow = withAni;
	mdltest::detail::Put32(
	    narrow.mdl, table + 12, static_cast<std::int32_t>( Get32( narrow.mdl, table + 8 ) + 6 ) );
	Check( Fails( narrow, ModelStatus::Truncated, ModelFile::Ani ),
	    "a record reading past its block's end: Truncated in ani" );
	SyntheticFiles backwards = withAni;
	mdltest::detail::Put32( backwards.mdl, table + 12, 2 );
	Check( Fails( backwards, ModelStatus::BadOffset, ModelFile::Mdl ),
	    "a block ending before it starts" );

	// Frame x bone data whose flags run past the file.
	SyntheticModel frames = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	Animate(
	    frames, "idle", mdltest::FrameAnim( { 0x08 }, "", mdltest::Quaternion48( {} ) ), 0x40 );
	SyntheticFiles f = mdltest::WriteModel( frames );
	Check( Parse( f ).HasValue(), "the frame x bone base model parses" );
	const std::size_t frameData = Get32( f.mdl, 184 ) + Get32( f.mdl, Get32( f.mdl, 184 ) + 56 );
	mdltest::detail::Put32( f.mdl, frameData + 4, 1 << 30 );
	Check( Fails( f, ModelStatus::BadOffset, ModelFile::Mdl ), "a frame block outside the file" );
}

// Random damage: whatever still parses poses without leaving its data.
void RobustnessCases()
{
	SyntheticModel model = BoxAt( { 0, 0, 0 }, { 1, 1, 1 } );
	model.bones = { SyntheticBone{}, SyntheticBone{} };
	model.bones[1].parent = 0;
	const std::int16_t position[3] = { 3, 0, 9 };
	Animate( model, "idle",
	    mdltest::Records( { { 0, 0x20, RawRotation64( {} ) },
	        { 1, 0x04, mdltest::ValueStreams( position ) } } ) );
	Animate( model, "frames",
	    mdltest::FrameAnim( { 0x02 | 0x01, 0x08 },
	        mdltest::Quaternion48( {} ) + mdltest::Vector48( {} ), mdltest::Quaternion48( {} ) ),
	    0x40 );
	const SyntheticFiles good = mdltest::WriteModel( model );
	std::mt19937 random( 20260928u );
	int parsed = 0;
	bool finite = true;
	for ( int trial = 0; trial < 3000; ++trial )
	{
		SyntheticFiles broken = good;
		std::string &target = trial % 2 == 0 ? broken.mdl : broken.vvd;
		for ( int flips = 1 + static_cast<int>( random() % 3 ); flips > 0; --flips )
		{
			target[random() % target.size()] = static_cast<char>( random() & 0xff );
		}
		auto result = Parse( broken );
		if ( !result.HasValue() )
		{
			continue;
		}
		++parsed;
		for ( std::size_t s = 0; s < result.Value().sequences.size(); ++s )
		{
			const mdl::Model posed = mdl::PoseModel( result.Value(), std::int32_t( s ) );
			finite = finite && posed.meshes.size() == result.Value().meshes.size();
		}
	}
	Check(
	    parsed > 0 && finite, "damaged files that still parse pose without leaving their data (" +
	                              std::to_string( parsed ) + " of 3000 parsed)" );
	int prefixes = 0;
	int failures = 0;
	for ( std::size_t length = 0; length < good.mdl.size(); ++length )
	{
		SyntheticFiles cut = good;
		cut.mdl.resize( length );
		++prefixes;
		failures += Parse( cut ).HasValue() ? 0 : 1;
	}
	Check( failures == prefixes, "every strict prefix of an animated MDL fails" );
}

} // namespace

int main()
{
	SkeletonCases();
	RawQuaternion64Cases();
	RawQuaternion48Cases();
	StreamCases();
	HierarchyCases();
	FrameAnimCases();
	BlockCases();
	SequenceCases();
	IncludeCases();
	MalformedCases();
	RobustnessCases();
	std::printf( "content.studio-model.pose: %d checks, %d failures\n", g_checks, g_failures );
	return testing::ReportConformance( g_checks, g_failures );
}
