//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: CPU mip demand calculation for visible material texture regions.
//
//=============================================================================//

#include "render/resources/mip_feedback.h"

#include <algorithm>
#include <cmath>

namespace render::resources
{

bool MipFeedbackFrame::AddVisible( const VisibleTextureFootprint &footprint )
{
	if ( footprint.textureWidth == 0 || footprint.textureHeight == 0 ||
	     footprint.mipLevels == 0 || !std::isfinite( footprint.screenWidth ) ||
	     !std::isfinite( footprint.screenHeight ) || !std::isfinite( footprint.uvWidth ) ||
	     !std::isfinite( footprint.uvHeight ) || footprint.screenWidth <= 0.0f ||
	     footprint.screenHeight <= 0.0f || footprint.uvWidth < 0.0f || footprint.uvHeight < 0.0f )
		return false;

	const double texelsPerPixel = std::max(
	    double( footprint.textureWidth ) * footprint.uvWidth / footprint.screenWidth,
	    double( footprint.textureHeight ) * footprint.uvHeight / footprint.screenHeight );
	const double lod = texelsPerPixel > 1.0 ? std::ceil( std::log2( texelsPerPixel ) ) : 0.0;
	const std::uint32_t mip = static_cast<std::uint32_t>( std::min(
	    lod, double( footprint.mipLevels - 1 ) ) );
	const auto key = std::pair{ footprint.material, footprint.textureSlot };
	auto [found, inserted] = m_Requests.emplace( key, mip );
	if ( !inserted )
		found->second = std::min( found->second, mip );
	return true;
}

std::vector<MipRequest> MipFeedbackFrame::Requests() const
{
	std::vector<MipRequest> requests;
	requests.reserve( m_Requests.size() );
	for ( const auto &[key, mip] : m_Requests )
		requests.push_back( { key.first, key.second, mip } );
	return requests;
}

} // namespace render::resources
