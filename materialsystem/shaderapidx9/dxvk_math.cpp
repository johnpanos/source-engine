//========= Copyright Valve Corporation, All rights reserved. ============//
// Private D3DX math compatibility for the native Vulkan provider. These values
// never cross the engine's preserved public interfaces.
#include "dxvk_compat.h"
#include <cmath>
#include <vector>
#include <cstring>

D3DXMATRIX *WINAPI D3DXMatrixMultiply( D3DXMATRIX *out, const D3DXMATRIX *a, const D3DXMATRIX *b )
{
	D3DXMATRIX result;
	for ( int row = 0; row != 4; ++row )
		for ( int col = 0; col != 4; ++col )
		{
			result.m[row][col] = 0;
			for ( int k = 0; k != 4; ++k )
				result.m[row][col] += a->m[row][k] * b->m[k][col];
		}
	*out = result;
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixTranspose( D3DXMATRIX *out, const D3DXMATRIX *in )
{
	D3DXMATRIX result;
	for ( int row = 0; row != 4; ++row )
		for ( int col = 0; col != 4; ++col )
			result.m[row][col] = in->m[col][row];
	*out = result;
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixInverse( D3DXMATRIX *out, FLOAT *determinant, const D3DXMATRIX *in )
{
	double work[4][8];
	double det = 1;
	for ( int row = 0; row != 4; ++row )
		for ( int col = 0; col != 8; ++col )
			work[row][col] = col < 4 ? in->m[row][col] : ( col - 4 == row ? 1 : 0 );
	for ( int col = 0; col != 4; ++col )
	{
		int pivot = col;
		for ( int row = col + 1; row != 4; ++row )
			if ( std::fabs( work[row][col] ) > std::fabs( work[pivot][col] ) )
				pivot = row;
		if ( work[pivot][col] == 0 )
		{
			if ( determinant )
				*determinant = 0;
			return NULL;
		}
		if ( pivot != col )
		{
			for ( int k = 0; k != 8; ++k )
				std::swap( work[pivot][k], work[col][k] );
			det = -det;
		}
		const double scale = work[col][col];
		det *= scale;
		for ( int k = 0; k != 8; ++k )
			work[col][k] /= scale;
		for ( int row = 0; row != 4; ++row )
		{
			if ( row == col )
				continue;
			const double factor = work[row][col];
			for ( int k = 0; k != 8; ++k )
				work[row][k] -= factor * work[col][k];
		}
	}
	for ( int row = 0; row != 4; ++row )
		for ( int col = 0; col != 4; ++col )
			out->m[row][col] = static_cast<float>( work[row][col + 4] );
	if ( determinant )
		*determinant = static_cast<float>( det );
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixTranslation( D3DXMATRIX *out, FLOAT x, FLOAT y, FLOAT z )
{
	D3DXMatrixIdentity( out );
	out->_41 = x;
	out->_42 = y;
	out->_43 = z;
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixScaling( D3DXMATRIX *out, FLOAT x, FLOAT y, FLOAT z )
{
	D3DXMatrixIdentity( out );
	out->_11 = x;
	out->_22 = y;
	out->_33 = z;
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixRotationAxis( D3DXMATRIX *out, const D3DXVECTOR3 *axis, FLOAT angle )
{
	const float length = std::sqrt( axis->x * axis->x + axis->y * axis->y + axis->z * axis->z );
	if ( length == 0 )
		return D3DXMatrixIdentity( out );
	const float x = axis->x / length, y = axis->y / length, z = axis->z / length;
	const float c = std::cos( angle ), s = std::sin( angle ), t = 1 - c;
	D3DXMatrixIdentity( out );
	out->_11 = t * x * x + c;
	out->_12 = t * x * y + s * z;
	out->_13 = t * x * z - s * y;
	out->_21 = t * x * y - s * z;
	out->_22 = t * y * y + c;
	out->_23 = t * y * z + s * x;
	out->_31 = t * x * z + s * y;
	out->_32 = t * y * z - s * x;
	out->_33 = t * z * z + c;
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixRotationYawPitchRoll(
    D3DXMATRIX *out, FLOAT yaw, FLOAT pitch, FLOAT roll )
{
	D3DXMATRIX x, y, z;
	D3DXVECTOR3 ax( 1, 0, 0 ), ay( 0, 1, 0 ), az( 0, 0, 1 );
	D3DXMatrixRotationAxis( &x, &ax, pitch );
	D3DXMatrixRotationAxis( &y, &ay, yaw );
	D3DXMatrixRotationAxis( &z, &az, roll );
	D3DXMatrixMultiply( out, &z, &x );
	return D3DXMatrixMultiply( out, out, &y );
}

D3DXVECTOR4 *WINAPI D3DXVec4Transform(
    D3DXVECTOR4 *out, const D3DXVECTOR4 *in, const D3DXMATRIX *matrix )
{
	D3DXVECTOR4 result;
	for ( int col = 0; col != 4; ++col )
		( &result.x )[col] = in->x * matrix->m[0][col] + in->y * matrix->m[1][col] +
		                     in->z * matrix->m[2][col] + in->w * matrix->m[3][col];
	*out = result;
	return out;
}

D3DXVECTOR3 *WINAPI D3DXVec3TransformCoord(
    D3DXVECTOR3 *out, const D3DXVECTOR3 *in, const D3DXMATRIX *matrix )
{
	D3DXVECTOR4 value( in->x, in->y, in->z, 1 );
	D3DXVec4Transform( &value, &value, matrix );
	const float reciprocal = value.w != 0 ? 1 / value.w : 0;
	*out = D3DXVECTOR3( value.x * reciprocal, value.y * reciprocal, value.z * reciprocal );
	return out;
}

D3DXVECTOR4 *WINAPI D3DXVec4Normalize( D3DXVECTOR4 *out, const D3DXVECTOR4 *in )
{
	const float length = std::sqrt( in->x * in->x + in->y * in->y + in->z * in->z + in->w * in->w );
	*out = length == 0 ? D3DXVECTOR4( 0, 0, 0, 0 ) : *in / length;
	return out;
}

D3DXPLANE *WINAPI D3DXPlaneNormalize( D3DXPLANE *out, const D3DXPLANE *in )
{
	const float length = std::sqrt( in->a * in->a + in->b * in->b + in->c * in->c );
	const float scale = length == 0 ? 0 : 1 / length;
	*out = D3DXPLANE( in->a * scale, in->b * scale, in->c * scale, in->d * scale );
	return out;
}

D3DXPLANE *WINAPI D3DXPlaneTransform(
    D3DXPLANE *out, const D3DXPLANE *in, const D3DXMATRIX *matrix )
{
	D3DXVECTOR4 value( in->a, in->b, in->c, in->d );
	D3DXVec4Transform( &value, &value, matrix );
	*out = D3DXPLANE( value.x, value.y, value.z, value.w );
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixOrthoOffCenterRH(
    D3DXMATRIX *out, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf )
{
	D3DXMatrixIdentity( out );
	out->_11 = 2 / ( r - l );
	out->_22 = 2 / ( t - b );
	out->_33 = 1 / ( zn - zf );
	out->_41 = ( l + r ) / ( l - r );
	out->_42 = ( t + b ) / ( b - t );
	out->_43 = zn / ( zn - zf );
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixPerspectiveOffCenterRH(
    D3DXMATRIX *out, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf )
{
	std::memset( out, 0, sizeof( *out ) );
	out->_11 = 2 * zn / ( r - l );
	out->_22 = 2 * zn / ( t - b );
	out->_31 = ( l + r ) / ( r - l );
	out->_32 = ( t + b ) / ( t - b );
	out->_33 = zf / ( zn - zf );
	out->_34 = -1;
	out->_43 = zn * zf / ( zn - zf );
	return out;
}

D3DXMATRIX *WINAPI D3DXMatrixPerspectiveRH(
    D3DXMATRIX *out, FLOAT width, FLOAT height, FLOAT zn, FLOAT zf )
{
	return D3DXMatrixPerspectiveOffCenterRH(
	    out, -width / 2, width / 2, -height / 2, height / 2, zn, zf );
}

namespace
{
class MatrixStack : public ID3DXMatrixStack
{
public:
	MatrixStack() : m_References( 1 ), m_Stack( 1 ) { D3DXMatrixIdentity( &m_Stack.back() ); }
	HRESULT STDMETHODCALLTYPE QueryInterface( REFIID id, void **out )
	{
		if ( !out )
			return E_POINTER;
		*out = NULL;
		const GUID unknown = { 0, 0, 0, { 0xc0, 0, 0, 0, 0, 0, 0, 0x46 } };
		const GUID stack = {
		    0xc7885ba7, 0xf990, 0x4fe7, { 0x92, 0x2d, 0x85, 0x15, 0xe4, 0x77, 0xdd, 0x85 } };
		if ( std::memcmp( &id, &unknown, sizeof( id ) ) &&
		     std::memcmp( &id, &stack, sizeof( id ) ) )
			return E_NOINTERFACE;
		*out = this;
		AddRef();
		return S_OK;
	}
	DXVK_ULONG STDMETHODCALLTYPE AddRef() { return ++m_References; }
	DXVK_ULONG STDMETHODCALLTYPE Release()
	{
		const DXVK_ULONG remaining = --m_References;
		if ( !remaining )
			delete this;
		return remaining;
	}
	HRESULT STDMETHODCALLTYPE Pop()
	{
		if ( m_Stack.size() == 1 )
			return D3DERR_INVALIDCALL;
		m_Stack.pop_back();
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE Push()
	{
		m_Stack.push_back( m_Stack.back() );
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE LoadIdentity()
	{
		D3DXMatrixIdentity( GetTop() );
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE LoadMatrix( const D3DXMATRIX *in )
	{
		*GetTop() = *in;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE MultMatrix( const D3DXMATRIX *in )
	{
		D3DXMatrixMultiply( GetTop(), GetTop(), in );
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE MultMatrixLocal( const D3DXMATRIX *in )
	{
		D3DXMatrixMultiply( GetTop(), in, GetTop() );
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE RotateAxis( const D3DXVECTOR3 *axis, FLOAT angle )
	{
		D3DXMATRIX value;
		D3DXMatrixRotationAxis( &value, axis, angle );
		return MultMatrix( &value );
	}
	HRESULT STDMETHODCALLTYPE RotateAxisLocal( const D3DXVECTOR3 *axis, FLOAT angle )
	{
		D3DXMATRIX value;
		D3DXMatrixRotationAxis( &value, axis, angle );
		return MultMatrixLocal( &value );
	}
	HRESULT STDMETHODCALLTYPE RotateYawPitchRoll( FLOAT yaw, FLOAT pitch, FLOAT roll )
	{
		D3DXMATRIX value;
		D3DXMatrixRotationYawPitchRoll( &value, yaw, pitch, roll );
		return MultMatrix( &value );
	}
	HRESULT STDMETHODCALLTYPE RotateYawPitchRollLocal( FLOAT yaw, FLOAT pitch, FLOAT roll )
	{
		D3DXMATRIX value;
		D3DXMatrixRotationYawPitchRoll( &value, yaw, pitch, roll );
		return MultMatrixLocal( &value );
	}
	HRESULT STDMETHODCALLTYPE Scale( FLOAT x, FLOAT y, FLOAT z )
	{
		D3DXMATRIX value;
		D3DXMatrixScaling( &value, x, y, z );
		return MultMatrix( &value );
	}
	HRESULT STDMETHODCALLTYPE ScaleLocal( FLOAT x, FLOAT y, FLOAT z )
	{
		D3DXMATRIX value;
		D3DXMatrixScaling( &value, x, y, z );
		return MultMatrixLocal( &value );
	}
	HRESULT STDMETHODCALLTYPE Translate( FLOAT x, FLOAT y, FLOAT z )
	{
		D3DXMATRIX value;
		D3DXMatrixTranslation( &value, x, y, z );
		return MultMatrix( &value );
	}
	HRESULT STDMETHODCALLTYPE TranslateLocal( FLOAT x, FLOAT y, FLOAT z )
	{
		D3DXMATRIX value;
		D3DXMatrixTranslation( &value, x, y, z );
		return MultMatrixLocal( &value );
	}
	D3DXMATRIX *STDMETHODCALLTYPE GetTop() { return &m_Stack.back(); }

private:
	DXVK_ULONG m_References;
	std::vector<D3DXMATRIX> m_Stack;
};
}

HRESULT WINAPI D3DXCreateMatrixStack( DWORD, ID3DXMatrixStack **out )
{
	if ( !out )
		return E_POINTER;
	*out = new MatrixStack;
	return S_OK;
}

DWORD WINAPI D3DXGetShaderVersion( const DWORD *bytecode )
{
	return bytecode ? *bytecode : 0;
}
const char *WINAPI D3DXGetPixelShaderProfile( IDirect3DDevice9 *device )
{
	D3DCAPS9 caps;
	if ( !device || FAILED( device->GetDeviceCaps( &caps ) ) )
		return NULL;
	return caps.PixelShaderVersion >= D3DPS_VERSION( 3, 0 ) ? "ps_3_0" : "ps_2_0";
}

HRESULT WINAPI D3DXCompileShader( const char *, UINT, const D3DXMACRO *, ID3DXInclude *,
    const char *, const char *, DWORD, ID3DXBuffer **shader, ID3DXBuffer **errors,
    ID3DXConstantTable **constants )
{
	// This profile consumes packaged shader bytecode. Runtime source compilation
	// is an explicit unsupported capability, never a successful empty shader.
	if ( shader )
		*shader = NULL;
	if ( errors )
		*errors = NULL;
	if ( constants )
		*constants = NULL;
	return E_NOTIMPL;
}

HRESULT WINAPI D3DXDisassembleShader( const DWORD *, WINBOOL, const char *, ID3DXBuffer **out )
{
	if ( out )
		*out = NULL;
	return E_NOTIMPL;
}
