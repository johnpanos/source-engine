//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_MATERIAL_VMT_MATRIX_H
#define RENDER_MATERIAL_VMT_MATRIX_H

#include <cstdio>
#include <string>

namespace RenderMaterialVmt
{
// VMT matrices are row-major. CMaterialVar::GetStringValue instead prints
// columns and only three decimal places; it is not a VMT serialization.
inline std::string MatrixValue( const float *rows )
{
	std::string value = "[ ";
	for ( unsigned i = 0; i < 16; ++i )
	{
		char number[64];
		std::snprintf( number, sizeof( number ), "%.9g ", rows[i] );
		value += number;
	}
	value += ']';
	return value;
}
} // namespace RenderMaterialVmt
#endif
