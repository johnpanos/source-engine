//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Material metadata port (RFC 0002, hammer.ports; "Assets and
//			asynchronous jobs"). Texture alignment needs a material's mapping
//			size (fit and justify work in texels), the material browser needs
//			the list of names, and the map check needs to know whether a name
//			exists. Decoded images and GPU bindings are separate concerns.
//
//			Contract: names are compared case-insensitively with '/' and '\'
//			equivalent; Size is the mapping size (VMT $basetexture's width and
//			height, or the material's declared mapping size), not a mip size.
//			A missing material is reported as missing, never as a default size.
//
//=============================================================================//

#ifndef HAMMER_PORTS_MATERIAL_INFO_H
#define HAMMER_PORTS_MATERIAL_INFO_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::ports
{

struct MaterialSize
{
	int width = 0;
	int height = 0;
};

class IMaterialInfo
{
public:
	virtual ~IMaterialInfo() = default;

	virtual bool Exists( std::string_view material ) const = 0;
	// Mapping size in texels; nothing when the material is missing or its size
	// is unknown.
	virtual std::optional<MaterialSize> Size( std::string_view material ) const = 0;
	// Every known material name, sorted.
	virtual std::vector<std::string> Names() const = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_MATERIAL_INFO_H
