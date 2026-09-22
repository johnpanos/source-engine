//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Read-only game-asset lookup port for the editor (RFC 0002,
//			hammer.ports). A narrow contract for resolving a logical asset path
//			(e.g. "materials/concrete/concretefloor001a.vmt") to its bytes, and
//			for enumerating available assets by prefix/extension. The material
//			catalog and any future asset browser depend on this behavior, not on
//			whether the bytes live inside a VPK, on loose disk, or in a fake.
//
//			Path convention (the contract every provider honors): forward slashes,
//			lower-cased, no leading slash, extension included -- exactly the form
//			a Source VPK directory stores. Providers normalize their own storage
//			to this form; callers pass paths already in it (the catalog does the
//			normalizing of VMF-authored material names once, at its boundary).
//
//			This is a genuine polymorphic contract (a VPK archive vs a loose-disk
//			tree vs an ordered search path vs a fake), so a virtual interface is
//			appropriate here per the RFC's guidance.
//
//=============================================================================//

#ifndef HAMMER_PORTS_ASSET_SOURCE_H
#define HAMMER_PORTS_ASSET_SOURCE_H

#include <string>
#include <vector>

namespace hammer::ports
{

class IAssetSource
{
public:
	virtual ~IAssetSource() = default;

	// Whether an asset exists at 'path' (canonical form; see file header) without
	// reading its bytes.
	virtual bool HasAsset( const std::string &path ) const = 0;

	// Reads the whole asset at 'path' into 'out' (assigned, not appended). Returns
	// false (out unchanged) when the asset is absent or cannot be read.
	virtual bool ReadAsset( const std::string &path, std::string &out ) const = 0;

	// Appends to 'out' every asset path that begins with 'prefix' (when non-empty)
	// and ends with 'extensionWithDot' (when non-empty, e.g. ".vmt"). Order is
	// provider-defined; the catalog sorts. An empty prefix and empty extension
	// lists every asset. Duplicates within one provider are not produced.
	virtual void ListAssets( const std::string &prefix, const std::string &extensionWithDot,
	    std::vector<std::string> &out ) const = 0;
};

} // namespace hammer::ports

#endif // HAMMER_PORTS_ASSET_SOURCE_H
