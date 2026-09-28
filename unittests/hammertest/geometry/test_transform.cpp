//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: world.map-geometry transform conformance (RFC 0002, R08 domain
//			logic): Source angle matrices and their inverse, exact quarter-turn
//			axis rotations, affine composition and inversion, and plane
//			transformation (orientation kept, including under mirrors and
//			non-uniform scale). Negative checks: a singular map is refused.
//
//=============================================================================//

#include "mapgeometry/transform.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>

using namespace mapgeometry;

namespace
{

bool Near( const Vec3d &a, const Vec3d &b, double eps = 1e-9 )
{
	return NearlyEqual( a, b, eps );
}

} // namespace

int main()
{
	testing::Checks checks;

	// AngleMatrix columns are forward, left, up.
	{
		const Mat3 r = AngleMatrix( 0, 90, 0 );
		checks.That(
		    Near( Apply( r, Vec3d( 1, 0, 0 ) ), Vec3d( 0, 1, 0 ) ), "yaw 90 turns +X to +Y" );
		const Mat3 p = AngleMatrix( 90, 0, 0 );
		checks.That( Near( Apply( p, Vec3d( 1, 0, 0 ) ), Vec3d( 0, 0, -1 ) ),
		    "pitch 90 points forward down" );
		const Mat3 roll = AngleMatrix( 0, 0, 90 );
		checks.That(
		    Near( Apply( roll, Vec3d( 0, 1, 0 ) ), Vec3d( 0, 0, 1 ) ), "roll 90 lifts left to up" );
	}

	// MatrixToAngles inverts AngleMatrix away from gimbal lock.
	{
		const double cases[][3] = {
		    { 10, 20, 30 }, { -45, 135, 0 }, { 0, -90, 15 }, { 80, 0, -60 } };
		for ( const auto &c : cases )
		{
			const EulerAngles a = MatrixToAngles( AngleMatrix( c[0], c[1], c[2] ) );
			const Mat3 back = AngleMatrix( a.pitch, a.yaw, a.roll );
			const Mat3 want = AngleMatrix( c[0], c[1], c[2] );
			bool same = true;
			for ( int i = 0; i < 3; ++i )
				for ( int j = 0; j < 3; ++j )
					same = same && std::fabs( back.m[i][j] - want.m[i][j] ) < 1e-9;
			checks.That( same, "angles round-trip through the matrix" );
		}
		const EulerAngles lock = MatrixToAngles( AngleMatrix( 90, 30, 0 ) );
		checks.Near( lock.roll, 0.0, 1e-9, "gimbal lock resolves roll to zero" );
		checks.Near( lock.pitch, 90.0, 1e-6, "gimbal lock keeps the pitch" );
	}

	// Quarter turns are exact (grid points stay on the grid).
	{
		const Mat3 z90 = Mat3::AxisRotation( 2, 90 );
		checks.That( Apply( z90, Vec3d( 64, 0, 0 ) ) == Vec3d( 0, 64, 0 ), "Z 90 is exact" );
		checks.That( Apply( Mat3::AxisRotation( 2, -90 ), Vec3d( 64, 0, 0 ) ) == Vec3d( 0, -64, 0 ),
		    "Z -90 is exact" );
		checks.That( Apply( Mat3::AxisRotation( 0, 180 ), Vec3d( 0, 8, 4 ) ) == Vec3d( 0, -8, -4 ),
		    "X 180 is exact" );
		checks.That( Apply( Mat3::AxisRotation( 1, 90 ), Vec3d( 0, 0, 1 ) ) == Vec3d( 1, 0, 0 ),
		    "Y 90 is right-handed (Z to X)" );
		checks.That( Near( Apply( Mat3::AxisRotation( 2, 45 ), Vec3d( 1, 0, 0 ) ),
		                 Vec3d( std::sqrt( 0.5 ), std::sqrt( 0.5 ), 0 ) ),
		    "non-quarter turns use sin/cos" );
	}

	// Affine: pivot, composition, inverse, mirror detection.
	{
		const Affine about = Affine::About( Mat3::AxisRotation( 2, 90 ), Vec3d( 10, 10, 0 ) );
		checks.That( about.Point( Vec3d( 10, 10, 5 ) ) == Vec3d( 10, 10, 5 ), "pivot is fixed" );
		checks.That(
		    about.Point( Vec3d( 20, 10, 0 ) ) == Vec3d( 10, 20, 0 ), "rotation about the pivot" );
		const Affine move = Affine::Translation( Vec3d( 1, 2, 3 ) );
		checks.That( move.IsTranslation() && !about.IsTranslation(), "translation detection" );
		const Affine both = Compose( move, about );
		checks.That(
		    both.Point( Vec3d( 20, 10, 0 ) ) == Vec3d( 11, 22, 3 ), "compose applies right first" );
		const std::optional<Affine> inv = Inverse( both );
		checks.That(
		    inv.has_value() && Near( inv->Point( Vec3d( 11, 22, 3 ) ), Vec3d( 20, 10, 0 ) ),
		    "inverse undoes the map" );
		checks.That(
		    Affine::About( Mat3::Mirror( 0 ), Vec3d() ).Mirrors(), "a mirror flips handedness" );
		checks.That( !about.Mirrors(), "a rotation does not" );
		// Negative: singular maps have no inverse.
		checks.That(
		    !Inverse( Affine::About( Mat3::Scale( Vec3d( 1, 0, 1 ) ), Vec3d() ) ).has_value(),
		    "a singular map has no inverse" );
	}

	// Planes keep their outside under rotation, mirror and non-uniform scale.
	{
		Plane top;
		top.normal = Vec3d( 0, 0, 1 );
		top.dist = 64;
		const std::optional<Plane> moved =
		    TransformPlane( Affine::Translation( Vec3d( 0, 0, 16 ) ), top );
		checks.That( moved && Near( moved->normal, Vec3d( 0, 0, 1 ) ), "translated normal" );
		checks.That( moved && std::fabs( moved->dist - 80 ) < 1e-9, "translated distance" );

		const std::optional<Plane> flipped =
		    TransformPlane( Affine::About( Mat3::Mirror( 2 ), Vec3d() ), top );
		checks.That( flipped && Near( flipped->normal, Vec3d( 0, 0, -1 ) ) &&
		                 std::fabs( flipped->dist - 64 ) < 1e-9,
		    "mirroring Z makes the top a bottom, outside kept" );

		Plane slope;
		slope.normal = Normalize( Vec3d( 1, 0, 1 ) );
		slope.dist = 0;
		const std::optional<Plane> scaled =
		    TransformPlane( Affine::About( Mat3::Scale( Vec3d( 2, 1, 1 ) ), Vec3d() ), slope );
		checks.That( scaled && Near( scaled->normal, Normalize( Vec3d( 1, 0, 2 ) ) ),
		    "non-uniform scale transforms the normal by the inverse transpose" );
		checks.That(
		    !TransformPlane( Affine::About( Mat3::Scale( Vec3d( 0, 1, 1 ) ), Vec3d() ), top ),
		    "a singular map refuses planes" );
	}

	return checks.Report();
}
