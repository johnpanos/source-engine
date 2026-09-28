//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-artifacts.v1 source port (RFC 0016). Materials and
//			passes find artifacts through it. Adapters: the build-time artifact
//			store (ArtifactStore, here) and, in development compositions, a
//			reload source (RFC 0014). Shipping products never compile shaders.
//
//=============================================================================//

#ifndef RENDER_SHADERLIB_ARTIFACT_SOURCE_H
#define RENDER_SHADERLIB_ARTIFACT_SOURCE_H

#include "foundation/expected.h"
#include "render/shaderlib/artifact.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace render::shaderlib
{

enum class ShaderLibraryStatus : std::uint8_t
{
	kDuplicateArtifact = 1,
	kMissingArtifact,
	kInvalidArtifact,
	kPermutationOutOfRange
};

struct ShaderLibraryError
{
	ShaderLibraryStatus status = ShaderLibraryStatus::kMissingArtifact;
	ArtifactKey key; // the artifact it concerns, when there is one
};

class IShaderArtifactSource
{
public:
	virtual ~IShaderArtifactSource() = default;
	// nullptr when absent. The artifact lives as long as the source.
	virtual const ShaderArtifact *Find( const ArtifactKey &key ) const = 0;
	// Changes whenever an artifact is added or replaced.
	virtual std::uint64_t Revision() const = 0;
};

// The build-time store: artifacts added once, found by key.
class ArtifactStore final : public IShaderArtifactSource
{
public:
	foundation::Expected<void, ShaderLibraryError> Add( ShaderArtifact artifact );
	const ShaderArtifact *Find( const ArtifactKey &key ) const override;
	std::uint64_t Revision() const override { return m_Revision; }
	std::size_t Count() const { return m_Artifacts.size(); }

private:
	std::vector<std::unique_ptr<ShaderArtifact>> m_Artifacts;
	std::uint64_t m_Revision = 0;
};

} // namespace render::shaderlib

#endif // RENDER_SHADERLIB_ARTIFACT_SOURCE_H
