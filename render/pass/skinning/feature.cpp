//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.skinning's frame features; see feature.h.
//
//=============================================================================//

#include "render/pass/skinning/feature.h"

namespace render::pass::skinning
{

namespace
{

class SkinningFeature final : public frame::IRenderFeature
{
public:
	explicit SkinningFeature( bool device ) : m_Device( device ) {}

	const char *Name() const override { return m_Device ? kSkinningFeature : kCpuSkinningFeature; }

	frame::FeatureRequirements Requirements() const override
	{
		frame::FeatureRequirements requirements;
		if ( m_Device )
		{
			requirements.required = {
			    device::Capability::kCompute, device::Capability::kStorageBuffers };
			requirements.fallback = kCpuSkinningFeature;
		}
		return requirements;
	}

	// No skinned meshes reach the frame yet (K6 product skinning).
	void AddPasses( frame::FeatureContext & ) override {}

private:
	bool m_Device;
};

} // namespace

std::unique_ptr<frame::IRenderFeature> CreateSkinningFeature()
{
	return std::make_unique<SkinningFeature>( true );
}

std::unique_ptr<frame::IRenderFeature> CreateCpuSkinningFeature()
{
	return std::make_unique<SkinningFeature>( false );
}

} // namespace render::pass::skinning
