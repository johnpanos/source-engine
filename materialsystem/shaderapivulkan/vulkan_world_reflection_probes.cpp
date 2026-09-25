//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RPRB reflection probe texture upload (R50-PARALLAX).
//
//===========================================================================//

#include "vulkan_world_lightmap.h"

#include "vulkan_device.h"
#include "mapcontainer/probe_volume.h"
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
	if ( !request.texels )
	{
		context.SetReflectionProbes( nullptr, 0, 0, 0 );
		return true;
	}
	if ( !context.WorldMeshResident() )
		return fail( "reflection probes require a resident world mesh" );
	if ( request.width < 16 || request.height < 3 || request.width > 16384 ||
	     request.height > 16384 || request.probeCount == 0 ||
	     request.probeCount > mapcontainer::kReflectionProbesMaxProbes )
		return fail( "reflection probe upload request is malformed" );
	// The texture must carry its header: count, mip-0 width and the marker.
	const float count = mapcontainer::HalfToFloat( request.texels[0] );
	const float width0 = mapcontainer::HalfToFloat( request.texels[2] );
	const float marker = mapcontainer::HalfToFloat( request.texels[3] );
	if ( marker != mapcontainer::kReflectionProbeTextureMarker ||
	     count != float( request.probeCount ) || 2.0f * width0 != float( request.width ) )
		return fail( "reflection probe texture header does not match the request" );
	return context.SetReflectionProbes(
	    request.texels, request.width, request.height, request.probeCount, error );
}

} // namespace render_vulkan
