//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The FGD-backed entity catalog (RFC 0002, hammer.formats): the
//			hammer::ports::IEntityCatalog that the composition root gives the
//			application. It loads an FGD entry file and its @include files
//			through a caller-supplied loader, parses them with ParseFgd, merges
//			them (a later class of the same name, compared case-insensitively,
//			replaces an earlier one), and resolves every non-base class once
//			with ResolveClass into an immutable EntityClassInfo table.
//
//			Includes are processed depth-first before the including file's own
//			classes, each file once (names compare case-insensitively with '/'
//			and '\' equivalent), so include cycles terminate. A missing entry
//			or include, a parse error and an unknown base class fail the load
//			with the file and line.
//
//			Display hints come from the class header helpers: size(mins, maxs)
//			or size(x y z) (a box of that size centered on the origin),
//			color(r g b), studio("path") / studioprop("path") or, without a
//			path, the default of the class's "model" key, and iconsprite("path").
//			A malformed size or color gives no hint.
//
//			The catalog is immutable after Load and safe to share across
//			threads; pointers from Find stay valid for its lifetime, including
//			across a move.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_FGD_ENTITY_CATALOG_H
#define HAMMER_FORMATS_FGD_ENTITY_CATALOG_H

#include "foundation/expected.h"
#include "hammer/ports/entity_catalog.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace hammer::formats
{

struct FgdCatalogError
{
	std::string message;
	std::string file;     // the file the error is in (the includer for a missing include)
	std::size_t line = 0; // 1-based; 0 when not tied to a line
};

// Returns the text of the FGD named 'name' (as written in @include, or the entry
// name), or nothing when it cannot be read.
using FgdLoader = std::function<std::optional<std::string>( const std::string &name )>;

class FgdEntityCatalog final : public hammer::ports::IEntityCatalog
{
public:
	// Loads 'entryName' and everything it includes through 'loader'.
	static foundation::Expected<FgdEntityCatalog, FgdCatalogError> Load(
	    const std::string &entryName, const FgdLoader &loader );

	// Loads in-memory files in order, as if each were an entry file; their
	// @include directives name other files of the list. Each file is processed
	// once, and later files override classes of earlier ones.
	static foundation::Expected<FgdEntityCatalog, FgdCatalogError> FromTexts(
	    const std::vector<std::pair<std::string, std::string>> &namedTexts );

	FgdEntityCatalog( FgdEntityCatalog &&other ) noexcept = default;
	FgdEntityCatalog &operator=( FgdEntityCatalog &&other ) noexcept = default;
	FgdEntityCatalog( const FgdEntityCatalog & ) = delete;
	FgdEntityCatalog &operator=( const FgdEntityCatalog & ) = delete;
	~FgdEntityCatalog() override = default;

	const hammer::ports::EntityClassInfo *Find( std::string_view name ) const override;
	std::vector<std::string> ClassNames() const override;

	// The files read, in processing order (includes before their includer).
	const std::vector<std::string> &Files() const { return m_files; }

private:
	FgdEntityCatalog() = default;

	static foundation::Expected<FgdEntityCatalog, FgdCatalogError> FromFileSet(
	    const std::vector<std::string> &entries, const FgdLoader &loader );

	std::vector<hammer::ports::EntityClassInfo> m_classes; // sorted case-insensitively
	std::vector<std::string> m_files;
};

} // namespace hammer::formats

#endif // HAMMER_FORMATS_FGD_ENTITY_CATALOG_H
