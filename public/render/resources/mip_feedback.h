//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CPU texture-mip feedback for visible materials (RFC 0016 resource
//          residency work). The frame producer supplies projected coverage
//          and UV span from its own geometry representation; this helper
//          does not inspect or mutate residency.
//
//=============================================================================//

#ifndef RENDER_RESOURCES_MIP_FEEDBACK_H
#define RENDER_RESOURCES_MIP_FEEDBACK_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace render::resources
{

struct VisibleTextureFootprint
{
	std::uint64_t material = 0;
	std::uint32_t textureSlot = 0;
	std::uint32_t textureWidth = 0;
	std::uint32_t textureHeight = 0;
	std::uint32_t mipLevels = 0;
	// Extent of the visible material region on screen, in pixels.
	float screenWidth = 0.0f;
	float screenHeight = 0.0f;
	// UV span of that region. For example, uvWidth=2 over screenWidth=100
	// means 0.02 U per screen pixel.
	float uvWidth = 0.0f;
	float uvHeight = 0.0f;
};

struct MipRequest
{
	std::uint64_t material = 0;
	std::uint32_t textureSlot = 0;
	std::uint32_t mip = 0;
};

// One instance belongs to one frame. Repeated visible regions of the same
// material texture are merged to the finest requested mip. Output order is
// deterministic by (material, textureSlot).
class MipFeedbackFrame
{
public:
	explicit MipFeedbackFrame( std::uint64_t frame ) : m_Frame( frame ) {}
	std::uint64_t Frame() const { return m_Frame; }
	// Returns false for malformed or non-finite footprint data.
	bool AddVisible( const VisibleTextureFootprint &footprint );
	std::vector<MipRequest> Requests() const;

private:
	std::uint64_t m_Frame = 0;
	std::map<std::pair<std::uint64_t, std::uint32_t>, std::uint32_t> m_Requests;
};

} // namespace render::resources

#endif // RENDER_RESOURCES_MIP_FEEDBACK_H
