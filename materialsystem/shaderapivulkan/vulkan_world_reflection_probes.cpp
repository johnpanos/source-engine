//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RPRB (v8) reflection probe upload (R50-PARALLAX): the BC6H radiance
//          cube array and the probe table, read natively by the shaders.
//
//===========================================================================//

#include "vulkan_world_lightmap.h"

#include "vulkan_device.h"
#include "mapcontainer/reflection_probes.h"

namespace render_vulkan
{

bool UploadWorldReflectionProbes( CVulkanContext &context,
    const world_mesh_gpu::ReflectionProbesUploadRequest &request, std::string *error )
{
	const auto fail = [&]( const std::string &message )
	{
		if ( error )
			*error = message;
		return false;
	};
	if ( !request.data )
	{
		context.SetReflectionProbes( nullptr, 0, 0, 0, 0, nullptr );
		return true;
	}
	if ( !context.WorldMeshResident() )
		return fail( "reflection probes require a resident world mesh" );
	if ( request.probeCount == 0 || request.probeCount > mapcontainer::kReflectionProbesMaxProbes ||
	     request.mipCount == 0 || request.mipCount > mapcontainer::kReflectionProbesMaxMips ||
	     request.faceSize < mapcontainer::kReflectionProbesMinFace ||
	     request.faceSize > mapcontainer::kReflectionProbesMaxFace || !request.buffer ||
	     request.bufferWords < mapcontainer::kReflectionProbeBufferMasksWord ||
	     request.radianceBytes == 0 || request.radianceBytes > request.dataBytes ||
	     request.buffer[0] != request.probeCount )
		return fail( "reflection probe upload request is malformed" );
	// The radiance cube array must be exactly the BC6H blocks of every mip.
	uint64_t expected = 0;
	for ( uint32_t level = 0; level < request.mipCount; ++level )
	{
		const uint64_t blocks = ( ( request.faceSize >> level ) + 3 ) / 4;
		expected += uint64_t( request.probeCount ) * 6 * blocks * blocks * 16;
	}
	if ( expected != request.radianceBytes )
		return fail( "reflection probe radiance size does not match its cubes" );
	if ( request.baseMip >= request.mipCount || request.buffer[7] != request.baseMip )
		return fail( "reflection probe base mip does not match its buffer" );
	// The cube array holds the mips from the base on (a texture setting).
	uint64_t skipped = 0;
	for ( uint32_t level = 0; level < request.baseMip; ++level )
	{
		const uint64_t blocks = ( ( request.faceSize >> level ) + 3 ) / 4;
		skipped += uint64_t( request.probeCount ) * 6 * blocks * blocks * 16;
	}
	return context.SetReflectionProbes( request.buffer, request.bufferWords, request.probeCount,
	    request.faceSize >> request.baseMip, request.mipCount - request.baseMip,
	    static_cast<const uint8_t *>( request.data ) + skipped, error );
}

} // namespace render_vulkan
