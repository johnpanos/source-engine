//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's RFC 0011 indirect-light host: the map's producer
//          switcher (render/indirect_switcher.h), r_indirect_producer, and
//          the volume every consumer samples.
//
//===========================================================================//

#ifndef ENGINE_INDIRECT_LIGHT_HOST_H
#define ENGINE_INDIRECT_LIGHT_HOST_H

#include <cstddef>
#include <cstdint>

namespace light_set
{
struct Snapshot;
}
namespace mapcontainer
{
class ProbeVolumeView;
}

// A map's indirect-light inputs, borrowed for BeginMap (it copies what it
// keeps): the validated PRBV, and when the map has them its RTRN radiosity
// transfer, SDFV signed distance volume and WMSH world mesh.
struct IndirectLightMapData
{
	const unsigned char *prbv = nullptr;
	size_t prbvSize = 0;
	const unsigned char *rtrn = nullptr;
	size_t rtrnSize = 0;
	const unsigned char *sdfv = nullptr;
	size_t sdfvSize = 0;
	const unsigned char *wmsh = nullptr;
	size_t wmshSize = 0;
	const unsigned char *lmap = nullptr; // the LMAP lump, for baked direct-light occlusion
	size_t lmapSize = 0;
	uint32_t lmapVersion = 0;
};

// Map lifetime. BeginMap validates the optional inputs (the transfer against
// the volume) and offers each producer only with what it needs: radiosity a
// valid transfer; the SDF producer a valid SDFV and a renderer compute
// service; the ray-query producer those, the world's triangles and a device
// with ray query. With an SDFV's lights, the WMSH and an LMAP direct layer it
// also removes the baked direct light moving geometry blocks (the proxies)
// from the world's lightmap. It starts the Baked producer, then the saved
// r_indirect_producer (an unoffered value is reported and Baked kept).
void IndirectLight_BeginMap( const IndirectLightMapData &map );
void IndirectLight_EndMap();

// Once per rendered frame on the main thread, before the view renders, with
// the frame's light set. Uploads the published volume when it changes.
void IndirectLight_Frame( const light_set::Snapshot &lights );

// The volume models sample on the CPU this frame (the ambient cube); null
// without a map volume. Valid until the next IndirectLight_Frame.
const mapcontainer::ProbeVolumeView *IndirectLight_CurrentVolume();

// Lifecycle: the app leaving and regaining the foreground; the device's
// resources lost and restored.
void IndirectLight_Background();
void IndirectLight_Resume();
void IndirectLight_DeviceLost();
void IndirectLight_DeviceRestored();

#endif // ENGINE_INDIRECT_LIGHT_HOST_H
