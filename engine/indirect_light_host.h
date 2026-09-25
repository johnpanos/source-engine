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

namespace light_set
{
struct Snapshot;
}
namespace mapcontainer
{
class ProbeVolumeView;
}

// Map lifetime. BeginMap takes the map's validated PRBV bytes, its RTRN
// radiosity transfer bytes and its SDFV signed distance volume, if any (all
// copied; the transfer is validated against the volume, and radiosity is
// offered only with a valid one; the SDF producer only with a valid SDFV and
// a renderer compute service), and starts the Baked producer, then the saved
// r_indirect_producer (an unoffered value is reported and Baked kept).
void IndirectLight_BeginMap( const unsigned char *prbv, size_t size, const unsigned char *rtrn,
    size_t rtrnSize, const unsigned char *sdfv, size_t sdfvSize );
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
