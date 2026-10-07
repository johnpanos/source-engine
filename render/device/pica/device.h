//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica's private declarations (RFC 0026).
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_DEVICE_H
#define RENDER_DEVICE_PICA_DEVICE_H

#include "render/device/facts.h"

namespace render::device::pica
{

// The facts every device of this adapter reports (RFC 0026 decision 4).
const DeviceFacts &AdapterFacts();

} // namespace render::device::pica

#endif // RENDER_DEVICE_PICA_DEVICE_H
