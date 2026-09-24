//========= Portal 2 port ======================================================//
//
// Purpose: SIMD-aligned 3D point used by the blobulator API.
//
// Clean-room implementation of the blobulator interface the Portal 2 paint
// blob renderer (game/client/portal2/c_paintblob_render.cpp) uses. It is not
// Valve's blobulator library, whose source is unavailable. Member names follow
// the 2010 retail DWARF declarations (common/blobulator/Point3D.h).
//
//=============================================================================//

#ifndef BLOBULATOR_POINT3D_H
#define BLOBULATOR_POINT3D_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "mathlib/ssemath.h"

class ALIGN16 Point3D
{
public:
	union
	{
		fltx4 sse_vec;
		float p[4];
	};

	Point3D() { clear(); }
	Point3D( float x, float y, float z ) { set( x, y, z ); }
	Point3D( const Vector &v ) { set( v.x, v.y, v.z ); }

	Point3D &operator=( const Vector &v )
	{
		set( v.x, v.y, v.z );
		return *this;
	}

	// The xyz components; w is not part of the vector.
	const Vector &AsVector() const { return *reinterpret_cast<const Vector *>( p ); }

	void clear() { p[0] = p[1] = p[2] = p[3] = 0.0f; }
	void set( float x, float y, float z ) { set( x, y, z, 0.0f ); }
	void set( float x, float y, float z, float w )
	{
		p[0] = x;
		p[1] = y;
		p[2] = z;
		p[3] = w;
	}

	float &operator[]( int i ) { return p[i]; }
	float operator[]( int i ) const { return p[i]; }

	float dot( const Point3D &o ) const { return p[0] * o.p[0] + p[1] * o.p[1] + p[2] * o.p[2]; }
	float lengthSq() const { return dot( *this ); }
	float length() const { return sqrtf( lengthSq() ); }
	float lengthSq( const Point3D &o ) const { return ( *this - o ).lengthSq(); }
	float length( const Point3D &o ) const { return sqrtf( lengthSq( o ) ); }

	// Normalizes xyz in place (a zero vector stays zero) and returns *this.
	Point3D &normalize()
	{
		float flLen = length();
		if ( flLen > 0.0f )
			mult( 1.0f / flLen );
		return *this;
	}

	Point3D unit() const
	{
		Point3D r = *this;
		return r.normalize();
	}

	Point3D &negate()
	{
		p[0] = -p[0];
		p[1] = -p[1];
		p[2] = -p[2];
		return *this;
	}
	Point3D operator-() const { return Point3D( -p[0], -p[1], -p[2] ); }

	Point3D &add( const Point3D &o )
	{
		p[0] += o.p[0];
		p[1] += o.p[1];
		p[2] += o.p[2];
		return *this;
	}
	Point3D operator+( const Point3D &o ) const
	{
		return Point3D( p[0] + o.p[0], p[1] + o.p[1], p[2] + o.p[2] );
	}

	Point3D &subtract( const Point3D &o )
	{
		p[0] -= o.p[0];
		p[1] -= o.p[1];
		p[2] -= o.p[2];
		return *this;
	}
	Point3D operator-( const Point3D &o ) const
	{
		return Point3D( p[0] - o.p[0], p[1] - o.p[1], p[2] - o.p[2] );
	}

	Point3D &mult( float s )
	{
		p[0] *= s;
		p[1] *= s;
		p[2] *= s;
		return *this;
	}
	Point3D &mult( const Point3D &o )
	{
		p[0] *= o.p[0];
		p[1] *= o.p[1];
		p[2] *= o.p[2];
		return *this;
	}
	Point3D operator*( float s ) const { return Point3D( p[0] * s, p[1] * s, p[2] * s ); }
	Point3D operator*( const Point3D &o ) const
	{
		return Point3D( p[0] * o.p[0], p[1] * o.p[1], p[2] * o.p[2] );
	}

	Point3D &div( float s ) { return mult( 1.0f / s ); }
	Point3D &div( const Point3D &o )
	{
		p[0] /= o.p[0];
		p[1] /= o.p[1];
		p[2] /= o.p[2];
		return *this;
	}
	Point3D operator/( float s ) const { return *this * ( 1.0f / s ); }
	Point3D operator/( const Point3D &o ) const
	{
		return Point3D( p[0] / o.p[0], p[1] / o.p[1], p[2] / o.p[2] );
	}

	bool operator==( const Point3D &o ) const
	{
		return p[0] == o.p[0] && p[1] == o.p[1] && p[2] == o.p[2];
	}

	Point3D crossProduct( const Point3D &o ) const
	{
		return Point3D( p[1] * o.p[2] - p[2] * o.p[1], p[2] * o.p[0] - p[0] * o.p[2],
		    p[0] * o.p[1] - p[1] * o.p[0] );
	}

	// Truncates xyz towards zero.
	Point3D &toInt()
	{
		p[0] = (float)(int)p[0];
		p[1] = (float)(int)p[1];
		p[2] = (float)(int)p[2];
		return *this;
	}
	Point3D getToInt() const
	{
		Point3D r = *this;
		return r.toInt();
	}
} ALIGN16_POST;

#endif // BLOBULATOR_POINT3D_H
