//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica: how the composition root hands the render
//			core's one PICA200 device to the 3DS shader API (RFC 0026
//			decision 9). The launcher's render core owns the device; the
//			shader API (materialsystem/shaderapipica) borrows it, so the 3DS
//			product has one device and one owner of citro3d. Names no native
//			type (CAP007).
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_HOST_BINDING_H
#define RENDER_DEVICE_PICA_HOST_BINDING_H

namespace render::device
{
class IRenderDevice2;
}

// Exported by the 3DS shader API. The root binds the core's device before
// the material system initializes the shader API, and keeps it alive until
// the material system has shut down; without one, Init fails with a named
// error (the shader API never creates a device of its own).
extern "C" void PicaShaderBackend_BindDevice( render::device::IRenderDevice2 *device );

#endif // RENDER_DEVICE_PICA_HOST_BINDING_H
