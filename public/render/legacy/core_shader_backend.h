//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: how the composition root hands the render core's device, pass
//			recorder and presenter to the core shader API
//			(materialsystem/shaderapicore). The launcher's render core owns the
//			device; the shader API borrows it, so there is one device and one
//			owner. Names no native type (CAP007).
//
//=============================================================================//

#ifndef RENDER_LEGACY_CORE_SHADER_BACKEND_H
#define RENDER_LEGACY_CORE_SHADER_BACKEND_H

#include <cstdint>

namespace render::device
{
class IRenderDevice2;
}

namespace render::legacy
{
class ICorePassRecorder;
}

// Exported by the core shader API. The root binds the core's device before
// the material system initializes the shader API, and keeps it alive until
// the material system has shut down; without one, Init fails with a named
// error (the shader API never creates a device of its own).
extern "C" void CoreShaderBackend_BindDevice( render::device::IRenderDevice2 *device );

// Exported by the core shader API: the core's passes record at the stream's
// slots, into the frame's encoder on the shared device. Without one the
// shader API marks no slot.
extern "C" void CoreShaderBackend_BindCorePassRecorder(
    render::legacy::ICorePassRecorder *recorder );

// The presenter that shows each frame's colour target: the root's, for its
// device's presentation. Without one frames are drawn and not shown.
using CoreShaderBackendPresenter = bool ( * )( void *context,
    render::device::IRenderDevice2 &device, std::uint64_t color, unsigned int width,
    unsigned int height );
extern "C" void CoreShaderBackend_BindPresenter( CoreShaderBackendPresenter presenter, void *context );

#endif // RENDER_LEGACY_CORE_SHADER_BACKEND_H
