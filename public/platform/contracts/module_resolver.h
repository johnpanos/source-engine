//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Module resolution (RFC 0001 rank 5, roadmap R11): turning a module
//			name and a search policy into one native path, separately from
//			opening it (platform.dynamic-library.v1 opens exactly the path it is
//			given). The resolver searches; an IFileProbe answers whether a
//			candidate exists; an optional IModuleVerifier applies validation or
//			signature policy to the chosen candidate before anyone opens it.
//
//			Contract: platform.module-resolver.v1 (shared suite in
//			unittests/platformtest/module_resolver/). File probes have their own
//			suite against native directories (platform.file-probe.v1).
//
//=============================================================================//

#ifndef PLATFORM_CONTRACTS_MODULE_RESOLVER_H
#define PLATFORM_CONTRACTS_MODULE_RESOLVER_H

// Contract header: standard library and foundation vocabulary only.
#include "foundation/expected.h"
#include "platform/contracts/path_types.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace platform
{

enum class FileKind
{
	kMissing = 0,
	kRegularFile,
	kDirectory,
	kOther, // a device, socket or anything else that exists
};

// Reports what a native path names, following symbolic links. Never opens,
// creates or changes anything. A path the platform cannot represent is
// kMissing.
class IFileProbe
{
public:
	virtual ~IFileProbe() = default;
	virtual FileKind Probe( const NativePath &path ) const = 0;
};

enum class VerifyResult
{
	kAccepted = 0,
	kRejected,
};

// Validation or signature policy for a candidate module, before it is opened.
class IModuleVerifier
{
public:
	virtual ~IModuleVerifier() = default;
	virtual VerifyResult VerifyCandidate( const NativePath &candidate ) = 0;
};

// One place to look: an optional directory below the root and a prefix glued to
// the front of the module's file name ("bin/" + "lib" + "engine.so").
struct ModulePattern
{
	std::string directory; // empty, or a VirtualPath ("bin")
	std::string prefix;    // empty or a file-name prefix ("lib"); no '/'
};

struct ModuleSearchPolicy
{
	// Roots in search order; the first existing candidate wins, root by root,
	// pattern by pattern.
	std::vector<NativePath> roots;
	std::vector<ModulePattern> patterns;
	// Appended to the module name (".so", ".dll"). When replaceExtension is set,
	// an extension the name already has is replaced first: the last '.' after
	// the last '/', unless it is the name's first character.
	std::string extension;
	bool replaceExtension = true;
	// By default only regular files are candidates. acceptAnyExisting accepts
	// anything that exists (the legacy stat() rule).
	bool acceptAnyExisting = false;
};

struct ResolvedModule
{
	NativePath path;
	std::size_t root = 0;    // index into policy.roots
	std::size_t pattern = 0; // index into policy.patterns
};

enum class ResolveStatus
{
	kInvalidName = 0, // not a valid VirtualPath, or empty after extension handling
	kInvalidPolicy,   // no roots, no patterns, an empty root, a bad directory or prefix
	kNotFound,        // every candidate was probed and none qualifies
	kRejected,        // the verifier refused the first qualifying candidate
};

struct ResolveFailure
{
	ResolveStatus status = ResolveStatus::kNotFound;
	// Every candidate probed, in order (kNotFound, kRejected); the last entry
	// of a kRejected failure is the refused candidate.
	std::vector<NativePath> attempted;
};

class IModuleResolver
{
public:
	virtual ~IModuleResolver() = default;

	// Probes candidates in policy order and returns the first that qualifies.
	// It never opens a library. A verifier, when the resolver has one, sees
	// only that first candidate; a rejection ends the search (a later copy of
	// a refused module is never substituted for it).
	[[nodiscard]] virtual foundation::Expected<ResolvedModule, ResolveFailure> Resolve(
	    std::string_view moduleName, const ModuleSearchPolicy &policy ) const = 0;
};

} // namespace platform

#endif // PLATFORM_CONTRACTS_MODULE_RESOLVER_H
