//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: mathlib microbenchmarks (mathlibbench).
//
// Each kernel applies one mathlib operation to a fixed array of 1024
// generated inputs and reports nanoseconds per operation, the median of
// several timed samples. Inputs come from a fixed seed, so two builds (other
// flags, another revision of mathlib, another device) time the same work.
// Every kernel folds its outputs into a checksum that is printed, which keeps
// the work alive and lets two builds be compared for identical results.
//
// The kernels are the operations the Portal profile spends time in (vector
// normalization, bone-setup quaternions and matrices, culling, animation
// decode) and the SIMD helpers particles and tools use. Correctness is the
// conformance suite's job (mathlibconformance); run it with the same build.
//
// Usage: mathlibbench [--full] [--samples N] [--min-ms M] [--filter s] [--json file]
//   default: smoke run, one short sample per kernel (checks it runs)
//   --full:  timed samples (default 15, each at least --min-ms, default 4)
//
//=============================================================================//

#include "mathlib/mathlib.h"
#include "mathlib/vector.h"
#include "mathlib/vmatrix.h"
#include "mathlib/ssemath.h"
#include "mathlib/compressed_vector.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace
{

const int N = 1024;

struct Rng
{
	uint64_t s;
	uint64_t Next()
	{
		uint64_t z = ( s += 0x9E3779B97F4A7C15ull );
		z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ull;
		z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBull;
		return z ^ ( z >> 31 );
	}
	float F( float lo, float hi )
	{
		return lo + ( hi - lo ) * (float)( ( Next() >> 40 ) * ( 1.0 / 16777216.0 ) );
	}
};

// Inputs, generated once.
struct Inputs
{
	Vector vec[N], vec2[N], mins[N], maxs[N];
	QAngle ang[N];
	Quaternion q[N], q2[N];
	matrix3x4_t mat[N], mat2[N];
	VMatrix vm[64], vm2[64];
	cplane_t plane[N];
	Frustum_t frustum;
	float f[N], t[N];
	Vector48 v48[N];
	Quaternion48 q48[N];
	Quaternion64 q64[N];
	FourVectors four[N / 4];
	fltx4 x4[N / 4], y4[N / 4];
};

Inputs *g_in;

// Outputs. Each kernel writes here and folds into the checksum.
struct Outputs
{
	Vector vec[N];
	QAngle ang[N];
	Quaternion q[N];
	matrix3x4_t mat[N];
	VMatrix vm[64];
	int i[N];
	float f[N];
	FourVectors four[N / 4];
	fltx4 x4[N / 4];
};

Outputs *g_out;

void Generate()
{
	Rng r{ 0x0BE7C4ull };
	Inputs &in = *g_in;
	for ( int i = 0; i < N; ++i )
	{
		in.vec[i] = Vector( r.F( -1000, 1000 ), r.F( -1000, 1000 ), r.F( -1000, 1000 ) );
		in.vec2[i] = Vector( r.F( -1000, 1000 ), r.F( -1000, 1000 ), r.F( -1000, 1000 ) );
		Vector a = in.vec[i], b = a + Vector( r.F( 1, 200 ), r.F( 1, 200 ), r.F( 1, 200 ) );
		in.mins[i] = a;
		in.maxs[i] = b;
		in.ang[i] = QAngle( r.F( -89, 89 ), r.F( -180, 180 ), r.F( -180, 180 ) );
		AngleQuaternion( in.ang[i], in.q[i] );
		AngleQuaternion( QAngle( r.F( -89, 89 ), r.F( -180, 180 ), r.F( -180, 180 ) ), in.q2[i] );
		AngleMatrix( in.ang[i], in.vec2[i], in.mat[i] );
		AngleMatrix(
		    QAngle( r.F( -89, 89 ), r.F( -180, 180 ), r.F( -180, 180 ) ), in.vec[i], in.mat2[i] );
		Vector n( r.F( -1, 1 ), r.F( -1, 1 ), r.F( -1, 1 ) );
		VectorNormalize( n );
		in.plane[i].normal = n;
		in.plane[i].dist = r.F( -1000, 1000 );
		in.plane[i].type = PLANE_ANYZ;
		in.plane[i].signbits = SignbitsForPlane( &in.plane[i] );
		in.f[i] = r.F( -1000, 1000 );
		in.t[i] = r.F( 0, 1 );
		in.v48[i] = in.vec[i];
		in.q48[i] = in.q[i];
		in.q64[i] = in.q[i];
	}
	for ( int i = 0; i < 64; ++i )
	{
		for ( int a = 0; a < 4; ++a )
			for ( int b = 0; b < 4; ++b )
			{
				in.vm[i].m[a][b] = r.F( -1, 1 ) + ( a == b ? 4.0f : 0.0f );
				in.vm2[i].m[a][b] = r.F( -1, 1 ) + ( a == b ? 4.0f : 0.0f );
			}
	}
	for ( int i = 0; i < N / 4; ++i )
	{
		in.four[i].LoadAndSwizzle(
		    in.vec[4 * i], in.vec[4 * i + 1], in.vec[4 * i + 2], in.vec[4 * i + 3] );
		in.x4[i] = LoadUnalignedSIMD( &in.f[4 * i] );
		in.y4[i] = LoadUnalignedSIMD( &in.t[4 * i] );
	}
	GeneratePerspectiveFrustum(
	    Vector( 0, 0, 0 ), QAngle( 10, 30, 0 ), 4.0f, 2000.0f, 90.0f, 16.0f / 9.0f, in.frustum );
}

// Checksums fold output bits, so identical results give identical sums.
uint64_t Fold( const void *p, size_t bytes )
{
	uint64_t h = 1469598103934665603ull;
	const unsigned char *c = (const unsigned char *)p;
	for ( size_t i = 0; i < bytes; ++i )
		h = ( h ^ c[i] ) * 1099511628211ull;
	return h;
}

struct Kernel
{
	const char *name;
	int opsPerCall; // operations one call performs
	std::function<uint64_t( bool )>
	    run; // one pass over the inputs; with fold, returns the outputs' checksum
};

std::vector<Kernel> MakeKernels()
{
	Inputs &in = *g_in;
	Outputs &out = *g_out;
	std::vector<Kernel> k;

	k.push_back( { "vector.normalize", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
		    {
			    out.vec[i] = in.vec[i];
			    out.f[i] = VectorNormalize( out.vec[i] );
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vec, sizeof( out.vec ) ) ^ Fold( out.f, sizeof( out.f ) );
	    } } );
	k.push_back( { "vector.cross-dot", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
		    {
			    out.vec[i] = CrossProduct( in.vec[i], in.vec2[i] );
			    out.f[i] = DotProduct( out.vec[i], in.vec[i] );
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vec, sizeof( out.vec ) ) ^ Fold( out.f, sizeof( out.f ) );
	    } } );
	k.push_back( { "angles.angle-vectors", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
		    {
			    Vector r, u;
			    AngleVectors( in.ang[i], &out.vec[i], &r, &u );
			    out.f[i] = r.x + u.y;
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vec, sizeof( out.vec ) ) ^ Fold( out.f, sizeof( out.f ) );
	    } } );
	k.push_back( { "angles.angle-matrix", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    AngleMatrix( in.ang[i], in.vec[i], out.mat[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.mat, sizeof( out.mat ) );
	    } } );
	k.push_back( { "angles.angle-quaternion", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    AngleQuaternion( in.ang[i], out.q[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "angles.matrix-angles", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    MatrixAngles( in.mat[i], out.ang[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.ang, sizeof( out.ang ) );
	    } } );
	k.push_back( { "angles.vector-angles", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    VectorAngles( in.vec[i], out.ang[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.ang, sizeof( out.ang ) );
	    } } );
	k.push_back( { "quat.matrix-quaternion", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    MatrixQuaternion( in.mat[i], out.q[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "quat.quaternion-matrix", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    QuaternionMatrix( in.q[i], in.vec[i], out.mat[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.mat, sizeof( out.mat ) );
	    } } );
	k.push_back( { "quat.slerp", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    QuaternionSlerp( in.q[i], in.q2[i], in.t[i], out.q[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "quat.blend", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    QuaternionBlend( in.q[i], in.q2[i], in.t[i], out.q[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "quat.mult", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    QuaternionMult( in.q[i], in.q2[i], out.q[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "quat.scale", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    QuaternionScale( in.q[i], in.t[i], out.q[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "quat.normalize", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
		    {
			    Quaternion q(
			        in.q[i].x * 1.5f, in.q[i].y * 1.5f, in.q[i].z * 1.5f, in.q[i].w * 1.5f );
			    out.f[i] = QuaternionNormalize( q );
			    out.q[i] = q;
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) ) ^ Fold( out.f, sizeof( out.f ) );
	    } } );
	k.push_back( { "matrix.concat-transforms", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    ConcatTransforms( in.mat[i], in.mat2[i], out.mat[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.mat, sizeof( out.mat ) );
	    } } );
	k.push_back( { "matrix.invert", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    MatrixInvert( in.mat[i], out.mat[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.mat, sizeof( out.mat ) );
	    } } );
	k.push_back( { "matrix.vector-transform", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    VectorTransform( in.vec[i], in.mat[i], out.vec[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vec, sizeof( out.vec ) );
	    } } );
	k.push_back( { "matrix.transform-aabb", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
		    {
			    Vector lo;
			    TransformAABB( in.mat[i], in.mins[i], in.maxs[i], lo, out.vec[i] );
			    out.f[i] = lo.x + lo.y + lo.z;
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vec, sizeof( out.vec ) ) ^ Fold( out.f, sizeof( out.f ) );
	    } } );
	k.push_back( { "cull.box-on-plane-side", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.i[i] = BoxOnPlaneSide( in.mins[i], in.maxs[i], &in.plane[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.i, sizeof( out.i ) );
	    } } );
	k.push_back( { "cull.r-cull-box", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.i[i] = R_CullBox( in.mins[i], in.maxs[i], in.frustum );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.i, sizeof( out.i ) );
	    } } );
	k.push_back( { "decode.vector48", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.vec[i] = in.v48[i];
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vec, sizeof( out.vec ) );
	    } } );
	k.push_back( { "decode.quaternion48", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.q[i] = in.q48[i];
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "decode.quaternion64", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.q[i] = in.q64[i];
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.q, sizeof( out.q ) );
	    } } );
	k.push_back( { "convert.round-float-to-int", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.i[i] = RoundFloatToInt( in.f[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.i, sizeof( out.i ) );
	    } } );
	k.push_back( { "convert.round-float-to-unsigned-long", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.i[i] = (int)RoundFloatToUnsignedLong( std::fabs( in.f[i] ) );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.i, sizeof( out.i ) );
	    } } );
	k.push_back( { "convert.floor2int", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.i[i] = Floor2Int( in.f[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.i, sizeof( out.i ) );
	    } } );
	k.push_back( { "scalar.bias", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N; ++i )
			    out.f[i] = Bias( in.t[i], 0.3f );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.f, sizeof( out.f ) );
	    } } );
	k.push_back( { "simd.four-vectors.normalize", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
		    {
			    out.four[i] = in.four[i];
			    out.four[i].VectorNormalize();
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.four, sizeof( out.four ) );
	    } } );
	k.push_back( { "simd.four-vectors.transform-by", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
		    {
			    out.four[i] = in.four[i];
			    out.four[i].TransformBy( in.mat[i] );
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.four, sizeof( out.four ) );
	    } } );
	k.push_back( { "simd.reciprocal-sqrt", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = ReciprocalSqrtSIMD( fabs( in.x4[i] ) );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.reciprocal", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = ReciprocalSIMD( in.x4[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.dot3", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = Dot3SIMD( in.x4[i], in.y4[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.sincos", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
		    {
			    fltx4 s, c;
			    SinCosSIMD( s, c, in.x4[i] );
			    out.x4[i] = AddSIMD( s, c );
		    }
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.arctan2", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = ArcTan2SIMD( in.x4[i], in.y4[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.pow", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = PowSIMD( in.y4[i], 2.75f );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.floor", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = FloorSIMD( in.x4[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "simd.sin-est01", N, [&]( bool fold )
	    {
		    for ( int i = 0; i < N / 4; ++i )
			    out.x4[i] = SinEst01SIMD( in.y4[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.x4, sizeof( out.x4 ) );
	    } } );
	k.push_back( { "vmatrix.multiply", 64, [&]( bool fold )
	    {
		    for ( int i = 0; i < 64; ++i )
			    MatrixMultiply( in.vm[i], in.vm2[i], out.vm[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vm, sizeof( out.vm ) );
	    } } );
	k.push_back( { "vmatrix.inverse-general", 64, [&]( bool fold )
	    {
		    for ( int i = 0; i < 64; ++i )
			    MatrixInverseGeneral( in.vm[i], out.vm[i] );
		    if ( !fold )
			    return uint64_t( 0 );
		    return Fold( out.vm, sizeof( out.vm ) );
	    } } );
	return k;
}

double NowNs()
{
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
	    std::chrono::steady_clock::now().time_since_epoch() )
	    .count();
}

} // namespace

int main( int argc, char **argv )
{
	bool bFull = false;
	int nSamples = 15;
	double minMs = 4.0;
	const char *pszFilter = nullptr, *pszJson = nullptr;
	for ( int i = 1; i < argc; ++i )
	{
		if ( !std::strcmp( argv[i], "--full" ) )
			bFull = true;
		else if ( !std::strcmp( argv[i], "--samples" ) && i + 1 < argc )
			nSamples = std::max( 1, std::atoi( argv[++i] ) );
		else if ( !std::strcmp( argv[i], "--min-ms" ) && i + 1 < argc )
			minMs = std::atof( argv[++i] );
		else if ( !std::strcmp( argv[i], "--filter" ) && i + 1 < argc )
			pszFilter = argv[++i];
		else if ( !std::strcmp( argv[i], "--json" ) && i + 1 < argc )
			pszJson = argv[++i];
		else
		{
			std::fprintf( stderr,
			    "usage: %s [--full] [--samples N] [--min-ms M] [--filter s] [--json file]\n",
			    argv[0] );
			return 2;
		}
	}

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	g_in = new Inputs;
	g_out = new Outputs;
	Generate();
	std::vector<Kernel> kernels = MakeKernels();

	FILE *json = pszJson ? std::fopen( pszJson, "w" ) : nullptr;
	if ( pszJson && !json )
	{
		std::fprintf( stderr, "cannot write %s\n", pszJson );
		return 2;
	}
	if ( json )
		std::fprintf( json,
		    "{\"schema\": \"mathlib-bench/v1\", \"full\": %s, \"samples\": %d, \"kernels\": [\n",
		    bFull ? "true" : "false", nSamples );

	int nRun = 0;
	for ( const Kernel &k : kernels )
	{
		if ( pszFilter && !std::strstr( k.name, pszFilter ) )
			continue;
		uint64_t sum = k.run( false ); // warm-up
		int reps = 1;
		std::vector<double> ns;
		if ( bFull )
		{
			// Calibrate: enough passes that one sample takes at least minMs.
			for ( ;; )
			{
				double t0 = NowNs();
				for ( int r = 0; r < reps; ++r )
					sum ^= k.run( false );
				double dt = NowNs() - t0;
				if ( dt >= minMs * 1e6 || reps >= ( 1 << 24 ) )
					break;
				reps *= 2;
			}
			for ( int s = 0; s < nSamples; ++s )
			{
				double t0 = NowNs();
				for ( int r = 0; r < reps; ++r )
					sum ^= k.run( false );
				ns.push_back( ( NowNs() - t0 ) / ( (double)reps * k.opsPerCall ) );
			}
		}
		else
		{
			double t0 = NowNs();
			sum ^= k.run( false );
			ns.push_back( ( NowNs() - t0 ) / k.opsPerCall );
		}
		std::sort( ns.begin(), ns.end() );
		double med = ns[ns.size() / 2];
		uint64_t check = k.run( true ); // result checksum of one pass, outside the timing
		std::printf( "%-40s %9.3f ns/op  min %9.3f  check %016llx\n", k.name, med, ns.front(),
		    (unsigned long long)check );
		if ( json )
			std::fprintf( json,
			    "%s  {\"name\": \"%s\", \"ns_per_op\": %.4f, \"min_ns_per_op\": %.4f, "
			    "\"max_ns_per_op\": %.4f, \"reps\": %d, \"check\": \"%016llx\"}",
			    nRun ? ",\n" : "", k.name, med, ns.front(), ns.back(), reps,
			    (unsigned long long)check );
		++nRun;
		(void)sum;
	}
	if ( json )
	{
		std::fprintf( json, "\n]}\n" );
		std::fclose( json );
	}
	if ( !nRun )
	{
		std::fprintf( stderr, "no kernel matches the filter\n" );
		return 1;
	}
	return 0;
}
