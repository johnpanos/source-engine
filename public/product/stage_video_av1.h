//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.stage.video-av1 (RFC 0027 L1): the `video-av1` content
//			stage, the `video.av1` compiler of the shared-concerns table. It
//			transcodes the installed game's Bink movies (`<game>/media/*.bik`
//			under the `steam-portal2` locator) to AV1/Opus WebM in the
//			`av1-media` location, as tools/video/transcode_av1.py did: a CRF
//			ladder per clip, kept only when frame count, size and audio tracks
//			match and the mean SSIM against the Bink decode reaches 0.97, and a
//			manifest.json keyed by source size, modification time and encoder
//			settings, so an unchanged clip is never encoded again.
//
//			ffmpeg and ffprobe are external tools run through the process
//			provider; a host without them reports the stage unavailable.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_STAGE_VIDEO_AV1_H
#define PUBLIC_PRODUCT_STAGE_VIDEO_AV1_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kVideoAv1Stage = "video-av1";

std::unique_ptr<IProductStage> CreateVideoAv1Stage();

} // namespace product

#endif // PUBLIC_PRODUCT_STAGE_VIDEO_AV1_H
