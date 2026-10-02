// The frozen Vulkan frontend still uses combined image samplers. Its probe
// sampling math is owned by render/shaders/common/probe_volume.glsl; this
// adapter remains only while K12 migrates the remaining native surfaces.
#define PROBE_VOLUME_COMBINED_SAMPLERS
#include "../../../render/shaders/common/probe_volume.glsl"
#undef PROBE_VOLUME_COMBINED_SAMPLERS

// The moving occluders after the grid rows are native-backend relight inputs.
// They do not belong to the core probe-volume sampling contract.
int ProbeOccluderCount()
{
	return int( texelFetch( probeGrids, ivec2( 5, 0 ), 0 ).z );
}

void ProbeOccluder( int k, out vec3 lo, out vec3 hi, out float reflectance )
{
	const int row = int( texelFetch( probeGrids, ivec2( 5, 0 ), 0 ).y ) + k;
	const vec4 low = texelFetch( probeGrids, ivec2( 0, row ), 0 );
	lo = low.xyz;
	reflectance = low.w;
	hi = texelFetch( probeGrids, ivec2( 1, row ), 0 ).xyz;
}
