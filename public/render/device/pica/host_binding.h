//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.pica: how the composition root hands the render
//			core's one PICA200 device to the 3DS shader API (RFC 0026
//			decision 9). The launcher's render core owns the device; the
//			shader API (materialsystem/shaderapicore) borrows it, so the 3DS
//			product has one device and one owner of citro3d. Names no native
//			type (CAP007).
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_HOST_BINDING_H
#define RENDER_DEVICE_PICA_HOST_BINDING_H

#include <cstdint>

namespace render::device
{
class IRenderDevice2;
}

// Exported by the 3DS shader API. The root binds the core's device before
// the material system initializes the shader API, and keeps it alive until
// the material system has shut down; without one, Init fails with a named
// error (the shader API never creates a device of its own).
extern "C" void CoreShaderBackend_BindDevice( render::device::IRenderDevice2 *device );

namespace render::legacy
{
class ICorePassRecorder;
}

// Exported by the 3DS shader API (RFC 0026 P3): the core's passes record at
// the stream's slots, into the frame's encoder on the shared device. Without
// one the shader API marks no slot.
extern "C" void CoreShaderBackend_BindCorePassRecorder(
    render::legacy::ICorePassRecorder *recorder );

// Off the 3DS (RFC 0029: the shader API on any render core device), the
// presenter that shows each frame's colour target: the root's, for its
// device's presentation (the WebGPU adapter's canvas). Without one frames
// are drawn and not shown.
using PicaShaderBackendPresenter = bool ( * )( void *context,
    render::device::IRenderDevice2 &device, std::uint64_t color, unsigned int width,
    unsigned int height );
extern "C" void CoreShaderBackend_BindPresenter( PicaShaderBackendPresenter presenter, void *context );

#endif // RENDER_DEVICE_PICA_HOST_BINDING_H
