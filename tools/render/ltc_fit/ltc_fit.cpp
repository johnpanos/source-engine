//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Fits linearly transformed cosines (Heitz, Dupuy, Hill and Neubelt,
//			"Real-Time Polygonal-Light Shading with Linearly Transformed
//			Cosines", SIGGRAPH 2016) to the RFC 0007 GGX lobe of
//			public/render/pbr_brdf.h: D * V * (n . l), alpha = roughness^2,
//			height-correlated Smith visibility, Fresnel one. For each view
//			angle and perceptual roughness it finds the matrix M whose
//			transformed clamped cosine best matches the lobe (Nelder-Mead,
//			sampled from both distributions with the balance heuristic, as
//			the paper's fitting code does, but on the L1 distance rather than
//			the paper's cubed error: an area light integrates the lobe, so
//			the L1 distance bounds its error, while the cubed error favours
//			the peak and gave 0.74 of the lobe's value at its flank at
//			roughness 0.5 and N.V 0.5), and prints
//			the inverse matrix's four free entries, normalized by its middle
//			entry, row by row: roughness i (rows) and sqrt(1 - N.V) j
//			(columns), both from 0 to 1 including the end points.
//
//			Diagnostics on stderr: LTC_FIT_TRACE=1 prints each fit's steps
//			and error; LTC_FIT_PROBE=<row>,<column> prints that entry's
//			lobe against the BRDF in and across the plane of incidence.
//
//			Deterministic: fixed stratified samples, no random numbers, one
//			thread, double precision. tools/render/ltc_table.py builds it,
//			runs it and writes public/render/pbr_ltc_table.h.
//
//=============================================================================//

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kSize = 64;         // table entries per axis
constexpr int kErrorSamples = 32; // per axis, per distribution
constexpr double kMinAlpha = 1e-4;

struct Vec3
{
	double x = 0, y = 0, z = 0;
};
Vec3 operator+( Vec3 a, Vec3 b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vec3 operator-( Vec3 a, Vec3 b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vec3 operator*( double s, Vec3 a )
{
	return { s * a.x, s * a.y, s * a.z };
}
double Dot( Vec3 a, Vec3 b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
double Length( Vec3 a )
{
	return std::sqrt( Dot( a, a ) );
}
Vec3 Normalize( Vec3 a )
{
	return ( 1.0 / Length( a ) ) * a;
}

// Column-major: c[column] is a column.
struct Mat3
{
	Vec3 c[3];
	Vec3 operator*( Vec3 v ) const { return v.x * c[0] + v.y * c[1] + v.z * c[2]; }
	double Determinant() const
	{
		return c[0].x * ( c[1].y * c[2].z - c[2].y * c[1].z ) -
		       c[1].x * ( c[0].y * c[2].z - c[2].y * c[0].z ) +
		       c[2].x * ( c[0].y * c[1].z - c[1].y * c[0].z );
	}
	Mat3 Inverse() const
	{
		const double det = Determinant();
		const Vec3 &a = c[0], &b = c[1], &d = c[2];
		// Rows of the inverse are the cross products of the columns.
		const Vec3 r0 = { b.y * d.z - b.z * d.y, b.z * d.x - b.x * d.z, b.x * d.y - b.y * d.x };
		const Vec3 r1 = { d.y * a.z - d.z * a.y, d.z * a.x - d.x * a.z, d.x * a.y - d.y * a.x };
		const Vec3 r2 = { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
		Mat3 inverse;
		inverse.c[0] = { r0.x / det, r1.x / det, r2.x / det };
		inverse.c[1] = { r0.y / det, r1.y / det, r2.y / det };
		inverse.c[2] = { r0.z / det, r1.z / det, r2.z / det };
		return inverse;
	}
};

// The GGX lobe times the cosine, and the pdf of sampling it through the
// distribution of normals.
struct Ggx
{
	double alpha;

	double Evaluate( Vec3 v, Vec3 l, double *pdf ) const
	{
		*pdf = 0.0;
		if ( v.z <= 0.0 || l.z <= 0.0 )
			return 0.0;
		const Vec3 h = Normalize( v + l );
		const double a2 = alpha * alpha;
		const double cos2 = h.z * h.z;
		const double denominator = ( 1.0 - cos2 ) + cos2 * a2;
		const double d = a2 / ( kPi * denominator * denominator );
		const double lambdaV = std::sqrt( a2 + ( 1.0 - a2 ) * v.z * v.z );
		const double lambdaL = std::sqrt( a2 + ( 1.0 - a2 ) * l.z * l.z );
		const double visibility = 0.5 / ( v.z * lambdaL + l.z * lambdaV );
		*pdf = d * h.z / ( 4.0 * Dot( v, h ) );
		return d * visibility * l.z;
	}

	Vec3 Sample( Vec3 v, double u1, double u2 ) const
	{
		const double phi = 2.0 * kPi * u1;
		const double a2 = alpha * alpha;
		const double cosTheta = std::sqrt( ( 1.0 - u2 ) / ( 1.0 + ( a2 - 1.0 ) * u2 ) );
		const double sinTheta = std::sqrt( std::max( 0.0, 1.0 - cosTheta * cosTheta ) );
		const Vec3 h = { sinTheta * std::cos( phi ), sinTheta * std::sin( phi ), cosTheta };
		return 2.0 * Dot( v, h ) * h - v;
	}
};

struct Ltc
{
	double m11 = 1, m22 = 1, m13 = 0;
	Vec3 x = { 1, 0, 0 }, y = { 0, 1, 0 }, z = { 0, 0, 1 };
	double amplitude = 1;
	Mat3 m, inverse;
	double det = 1;

	void Update()
	{
		const Mat3 frame = { { x, y, z } };
		const Mat3 shape = { { { m11, 0, 0 }, { 0, m22, 0 }, { m13, 0, 1 } } };
		m.c[0] = frame * shape.c[0];
		m.c[1] = frame * shape.c[1];
		m.c[2] = frame * shape.c[2];
		inverse = m.Inverse();
		det = std::fabs( m.Determinant() );
	}

	double Evaluate( Vec3 l ) const
	{
		const Vec3 original = Normalize( inverse * l );
		const Vec3 transformed = m * original;
		const double length = Length( transformed );
		const double jacobian = det / ( length * length * length );
		const double d = std::max( 0.0, original.z ) / kPi;
		return amplitude * d / jacobian;
	}

	Vec3 Sample( double u1, double u2 ) const
	{
		const double theta = std::acos( std::sqrt( u1 ) );
		const double phi = 2.0 * kPi * u2;
		return Normalize( m * Vec3{ std::sin( theta ) * std::cos( phi ),
		                          std::sin( theta ) * std::sin( phi ), std::cos( theta ) } );
	}
};

// The lobe's integral (its directional albedo at Fresnel one) and its
// average direction.
void Moments( const Ggx &brdf, Vec3 v, double *norm, Vec3 *average )
{
	*norm = 0.0;
	*average = {};
	constexpr int kSamples = 64;
	for ( int j = 0; j < kSamples; ++j )
	{
		for ( int i = 0; i < kSamples; ++i )
		{
			const Vec3 l = brdf.Sample( v, ( i + 0.5 ) / kSamples, ( j + 0.5 ) / kSamples );
			double pdf = 0.0;
			const double value = brdf.Evaluate( v, l, &pdf );
			if ( pdf <= 0.0 )
				continue;
			const double weight = value / pdf;
			*norm += weight;
			*average = *average + weight * l;
		}
	}
	*norm /= double( kSamples * kSamples );
	average->y = 0.0;
	*average = Normalize( *average );
}

double Error( const Ltc &ltc, const Ggx &brdf, Vec3 v )
{
	double error = 0.0;
	for ( int j = 0; j < kErrorSamples; ++j )
	{
		for ( int i = 0; i < kErrorSamples; ++i )
		{
			const double u1 = ( i + 0.5 ) / kErrorSamples;
			const double u2 = ( j + 0.5 ) / kErrorSamples;
			for ( int strategy = 0; strategy < 2; ++strategy )
			{
				const Vec3 l = strategy == 0 ? ltc.Sample( u1, u2 ) : brdf.Sample( v, u1, u2 );
				double pdfBrdf = 0.0;
				const double valueBrdf = brdf.Evaluate( v, l, &pdfBrdf );
				const double valueLtc = ltc.Evaluate( l );
				const double pdfLtc = valueLtc / ltc.amplitude;
				const double difference = std::fabs( valueBrdf - valueLtc );
				const double pdf = pdfLtc + pdfBrdf;
				if ( pdf > 0.0 )
					error += difference / pdf;
			}
		}
	}
	return error / double( kErrorSamples * kErrorSamples );
}

void Apply( Ltc &ltc, const double *p, bool isotropic )
{
	if ( isotropic )
	{
		ltc.m11 = ltc.m22 = std::max( p[0], 1e-7 );
		ltc.m13 = 0.0;
	}
	else
	{
		ltc.m11 = std::max( p[0], 1e-7 );
		ltc.m22 = std::max( p[1], 1e-7 );
		ltc.m13 = p[2];
	}
	ltc.Update();
}

// Nelder-Mead in three dimensions (the paper's fitter: reflection,
// expansion, contraction and shrink). It stops when the simplex's values
// agree within a relative tolerance, or after a fixed number of steps. The
// test is relative because a rough lobe's error is far below any absolute
// tolerance that suits a sharp one.
void Fit( Ltc &ltc, const Ggx &brdf, Vec3 v, bool isotropic )
{
	constexpr int kDim = 3;
	constexpr double kDelta = 0.05;
	constexpr double kTolerance = 1e-5; // relative
	constexpr int kMaxSteps = 400;
	auto cost = [&]( const std::array<double, kDim> &p )
	{
		Ltc trial = ltc;
		Apply( trial, p.data(), isotropic );
		return Error( trial, brdf, v );
	};
	std::array<std::array<double, kDim>, kDim + 1> simplex;
	std::array<double, kDim + 1> values;
	simplex[0] = { ltc.m11, ltc.m22, ltc.m13 };
	for ( int i = 1; i <= kDim; ++i )
	{
		simplex[i] = simplex[0];
		simplex[i][i - 1] += kDelta;
	}
	for ( int i = 0; i <= kDim; ++i )
		values[i] = cost( simplex[i] );
	int steps = 0;
	const double first = values[0];
	for ( int step = 0; step < kMaxSteps; ++step, ++steps )
	{
		std::array<int, kDim + 1> order = { 0, 1, 2, 3 };
		std::sort( order.begin(), order.end(),
		    [&]( int a, int b )
		    {
			    return values[a] < values[b];
		    } );
		const int best = order[0];
		const int worst = order[kDim];
		const int second = order[kDim - 1];
		if ( std::fabs( values[best] - values[worst] ) <= kTolerance * std::fabs( values[best] ) )
			break;
		std::array<double, kDim> centroid = {};
		for ( int i = 0; i <= kDim; ++i )
		{
			if ( i == worst )
				continue;
			for ( int k = 0; k < kDim; ++k )
				centroid[k] += simplex[i][k] / kDim;
		}
		auto along = [&]( double t )
		{
			std::array<double, kDim> p;
			for ( int k = 0; k < kDim; ++k )
				p[k] = centroid[k] + t * ( simplex[worst][k] - centroid[k] );
			return p;
		};
		const auto reflected = along( -1.0 );
		const double reflectedValue = cost( reflected );
		if ( reflectedValue < values[best] )
		{
			const auto expanded = along( -2.0 );
			const double expandedValue = cost( expanded );
			if ( expandedValue < reflectedValue )
			{
				simplex[worst] = expanded;
				values[worst] = expandedValue;
			}
			else
			{
				simplex[worst] = reflected;
				values[worst] = reflectedValue;
			}
			continue;
		}
		if ( reflectedValue < values[second] )
		{
			simplex[worst] = reflected;
			values[worst] = reflectedValue;
			continue;
		}
		const bool outside = reflectedValue < values[worst];
		const auto contracted = along( outside ? -0.5 : 0.5 );
		const double contractedValue = cost( contracted );
		if ( contractedValue < ( outside ? reflectedValue : values[worst] ) )
		{
			simplex[worst] = contracted;
			values[worst] = contractedValue;
			continue;
		}
		for ( int i = 0; i <= kDim; ++i )
		{
			if ( i == best )
				continue;
			for ( int k = 0; k < kDim; ++k )
				simplex[i][k] = simplex[best][k] + 0.5 * ( simplex[i][k] - simplex[best][k] );
			values[i] = cost( simplex[i] );
		}
	}
	int best = 0;
	for ( int i = 1; i <= kDim; ++i )
		best = values[i] < values[best] ? i : best;
	Apply( ltc, simplex[best].data(), isotropic );
	if ( std::getenv( "LTC_FIT_TRACE" ) )
		std::fprintf( stderr, "fit steps %d error %.3g -> %.3g\n", steps, first, values[best] );
}

} // namespace

int main()
{
	// Fit from rough to smooth, each row starting from the one above, and
	// along a row from normal incidence, each entry starting from the last.
	static std::array<Mat3, kSize * kSize> fitted;
	Ltc ltc;
	for ( int a = kSize - 1; a >= 0; --a )
	{
		const double roughness = double( a ) / ( kSize - 1 );
		const Ggx brdf = { std::max( roughness * roughness, kMinAlpha ) };
		for ( int t = 0; t < kSize; ++t )
		{
			const double x = double( t ) / ( kSize - 1 );
			const double cosTheta = std::max( 1.0 - x * x, 1e-4 );
			const Vec3 v = { std::sqrt( 1.0 - cosTheta * cosTheta ), 0.0, cosTheta };
			double norm = 0.0;
			Vec3 average;
			Moments( brdf, v, &norm, &average );
			bool isotropic = false;
			if ( t == 0 )
			{
				ltc.x = { 1, 0, 0 };
				ltc.y = { 0, 1, 0 };
				ltc.z = { 0, 0, 1 };
				if ( a == kSize - 1 )
				{
					ltc.m11 = ltc.m22 = 1.0;
				}
				else
				{
					const Mat3 &above = fitted[( a + 1 ) * kSize];
					ltc.m11 = above.c[0].x;
					ltc.m22 = above.c[1].y;
				}
				ltc.m13 = 0.0;
				isotropic = true;
			}
			else
			{
				ltc.x = { average.z, 0, -average.x };
				ltc.y = { 0, 1, 0 };
				ltc.z = average;
			}
			ltc.amplitude = norm;
			ltc.Update();
			Fit( ltc, brdf, v, isotropic );
			fitted[a * kSize + t] = ltc.m;
			if ( const char *probe = std::getenv( "LTC_FIT_PROBE" ) )
			{
				int pa = 0, pt = 0;
				if ( std::sscanf( probe, "%d,%d", &pa, &pt ) == 2 && pa == a && pt == t )
				{
					std::fprintf( stderr,
					    "probe a %d t %d ndv %.3f norm %.4f m11 %.4f m22 %.4f m13 %.4f avg %.3f "
					    "%.3f error %.3g\n",
					    a, t, cosTheta, norm, ltc.m11, ltc.m22, ltc.m13, average.x, average.z,
					    Error( ltc, brdf, v ) );
					for ( int k = -8; k <= 8; ++k )
					{
						const double angle = double( k ) / 8.0 * kPi / 2.0 * 0.98;
						const Vec3 l = { std::sin( angle ), 0.0, std::cos( angle ) };
						double pdf = 0.0;
						const double b = brdf.Evaluate( v, l, &pdf );
						std::fprintf( stderr, "  angle %+6.1f brdf %.4f ltc %.4f\n",
						    angle * 180.0 / kPi, b, ltc.Evaluate( l ) );
					}
					// Across the plane through the lobe's average direction.
					for ( int k = 0; k <= 8; ++k )
					{
						const double side = double( k ) / 8.0 * 0.6;
						const Vec3 l = Normalize( average + Vec3{ 0.0, side, 0.0 } );
						double pdf = 0.0;
						const double b = brdf.Evaluate( v, l, &pdf );
						std::fprintf( stderr, "  side %.3f brdf %.4f ltc %.4f\n", side, b,
						    ltc.Evaluate( l ) );
					}
				}
			}
		}
		std::fprintf( stderr, "roughness row %d of %d\n", kSize - a, kSize );
	}
	std::printf( "%d\n", kSize );
	for ( int a = 0; a < kSize; ++a )
	{
		for ( int t = 0; t < kSize; ++t )
		{
			const Mat3 inverse = fitted[a * kSize + t].Inverse();
			const double middle = inverse.c[1].y;
			// mat3( vec3( x, 0, y ), vec3( 0, 1, 0 ), vec3( z, 0, w ) ) in GLSL.
			std::printf( "%.9g %.9g %.9g %.9g\n", inverse.c[0].x / middle, inverse.c[0].z / middle,
			    inverse.c[2].x / middle, inverse.c[2].z / middle );
		}
	}
	return 0;
}
