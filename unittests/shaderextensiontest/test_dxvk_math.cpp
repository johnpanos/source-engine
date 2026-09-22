//========= Copyright Valve Corporation, All rights reserved. ============//
#include "../../materialsystem/shaderapidx9/dxvk_compat.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{
int g_Checks;
void Check( bool condition )
{
	++g_Checks;
	if ( !condition )
	{
		std::fprintf( stderr, "DXVK math check %d failed\n", g_Checks );
		std::exit( 1 );
	}
}
void Near( float actual, float expected )
{
	Check( std::fabs( actual - expected ) < 0.00001f );
}
void Identity( const D3DXMATRIX &matrix )
{
	for ( int row = 0; row != 4; ++row )
		for ( int col = 0; col != 4; ++col )
			Near( matrix.m[row][col], row == col ? 1 : 0 );
}
}

int main()
{
	D3DXMATRIX translate, scale, combined, inverse, product;
	D3DXMatrixTranslation( &translate, 3, 4, 5 );
	D3DXMatrixScaling( &scale, 2, 3, 4 );
	D3DXMatrixMultiply( &combined, &scale, &translate );
	D3DXVECTOR3 point( 1, 2, 3 );
	D3DXVec3TransformCoord( &point, &point, &combined );
	Near( point.x, 5 );
	Near( point.y, 10 );
	Near( point.z, 17 );
	float determinant;
	Check( D3DXMatrixInverse( &inverse, &determinant, &combined ) == &inverse );
	Near( determinant, 24 );
	D3DXMatrixMultiply( &product, &combined, &inverse );
	Identity( product );
	Check( D3DXMatrixInverse( &combined, NULL, &combined ) == &combined );
	for ( int row = 0; row != 4; ++row )
		for ( int col = 0; col != 4; ++col )
			Near( combined.m[row][col], inverse.m[row][col] );
	D3DXMatrixScaling( &scale, 1, 0, 1 );
	Check( D3DXMatrixInverse( &inverse, &determinant, &scale ) == NULL );
	Near( determinant, 0 );

	D3DXMATRIX projection;
	D3DXMatrixPerspectiveOffCenterRH( &projection, -1, 3, -2, 2, 2, 20 );
	point = D3DXVECTOR3( -1, -2, -2 );
	D3DXVec3TransformCoord( &point, &point, &projection );
	Near( point.x, -1 );
	Near( point.y, -1 );
	Near( point.z, 0 );
	point = D3DXVECTOR3( 30, 20, -20 );
	D3DXVec3TransformCoord( &point, &point, &projection );
	Near( point.x, 1 );
	Near( point.y, 1 );
	Near( point.z, 1 );
	D3DXMatrixOrthoOffCenterRH( &projection, -1, 3, -2, 2, 2, 20 );
	point = D3DXVECTOR3( 3, 2, -20 );
	D3DXVec3TransformCoord( &point, &point, &projection );
	Near( point.x, 1 );
	Near( point.y, 1 );
	Near( point.z, 1 );

	D3DXVECTOR3 axis( 0, 0, 3 );
	D3DXMatrixRotationAxis( &product, &axis, D3DX_PI / 2 );
	point = D3DXVECTOR3( 1, 0, 0 );
	D3DXVec3TransformCoord( &point, &point, &product );
	Near( point.x, 0 );
	Near( point.y, 1 );
	Near( point.z, 0 );
	D3DXPLANE plane( 2, 0, 0, 4 );
	D3DXPlaneNormalize( &plane, &plane );
	Near( plane.a, 1 );
	Near( plane.d, 2 );
	D3DXMATRIX transposed;
	D3DXMatrixTranspose( &transposed, &translate );
	D3DXPlaneTransform( &plane, &plane, &transposed );
	Near( plane.a, 1 );
	Near( plane.b, 0 );
	Near( plane.c, 0 );
	Near( plane.d, 5 );

	ID3DXMatrixStack *stack = NULL;
	Check( D3DXCreateMatrixStack( 0, NULL ) == E_POINTER );
	Check( D3DXCreateMatrixStack( 0, &stack ) == S_OK && stack );
	Identity( *stack->GetTop() );
	Check( stack->Pop() == D3DERR_INVALIDCALL );
	Check( stack->Translate( 3, 4, 5 ) == S_OK );
	Check( stack->Push() == S_OK );
	Check( stack->ScaleLocal( 2, 3, 4 ) == S_OK );
	point = D3DXVECTOR3( 1, 2, 3 );
	D3DXVec3TransformCoord( &point, &point, stack->GetTop() );
	Near( point.x, 5 );
	Near( point.y, 10 );
	Near( point.z, 17 );
	Check( stack->Pop() == S_OK );
	Near( stack->GetTop()->_41, 3 );
	Check( stack->Scale( 2, 3, 4 ) == S_OK );
	Near( stack->GetTop()->_41, 6 );
	Check( stack->Release() == 0 );
	std::printf( "CHECKS %d\n", g_Checks );
	return 0;
}
