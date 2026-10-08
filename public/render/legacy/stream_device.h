//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What a legacy shader backend still does beneath the material
//			system's IShaderDevice facade (RFC 0016 legacy device facade, F4):
//			the legacy draw stream's resources and frame. The facade answers
//			the device and presentation questions itself (from the render
//			core's device, adapter and presentation sources), so no backend
//			implements IShaderDevice or IShaderDeviceMgr; this narrow
//			interface retires with the legacy stream (R91).
//
//=============================================================================//

#ifndef RENDER_LEGACY_STREAM_DEVICE_H
#define RENDER_LEGACY_STREAM_DEVICE_H

#include "shaderapi/IShaderDevice.h"

namespace render::legacy
{

class ILegacyStreamDevice
{
public:
	// The legacy frame: records and submits what the stream drew.
	virtual void Present() = 0;

	// Alt-tab and device-reset style release and restore of the stream's
	// device resources.
	virtual void ReleaseResources() = 0;
	virtual void ReacquireResources() = 0;

	// Child windows the tools render into.
	virtual bool AddView( void *hWnd ) = 0;
	virtual void RemoveView( void *hWnd ) = 0;
	virtual void SetView( void *hWnd ) = 0;

	// The stream's meshes and buffers.
	virtual IMesh *CreateStaticMesh(
	    VertexFormat_t format, const char *pTextureBudgetGroup, IMaterial *pMaterial ) = 0;
	virtual void DestroyStaticMesh( IMesh *pMesh ) = 0;
	virtual IVertexBuffer *CreateVertexBuffer( ShaderBufferType_t type, VertexFormat_t format,
	    int nVertexCount, const char *pBudgetGroup ) = 0;
	virtual void DestroyVertexBuffer( IVertexBuffer *pVertexBuffer ) = 0;
	virtual IIndexBuffer *CreateIndexBuffer( ShaderBufferType_t type, MaterialIndexFormat_t format,
	    int nIndexCount, const char *pBudgetGroup ) = 0;
	virtual void DestroyIndexBuffer( IIndexBuffer *pIndexBuffer ) = 0;
	virtual IVertexBuffer *GetDynamicVertexBuffer(
	    int nStreamID, VertexFormat_t format, bool bBuffered ) = 0;
	virtual IIndexBuffer *GetDynamicIndexBuffer( MaterialIndexFormat_t format, bool bBuffered ) = 0;

	// Runtime-compiled shaders (none of the current backends compile any).
	virtual IShaderBuffer *CompileShader(
	    const char *pProgram, size_t nBufLen, const char *pShaderVersion ) = 0;
	virtual VertexShaderHandle_t CreateVertexShader( IShaderBuffer *pShaderBuffer ) = 0;
	virtual void DestroyVertexShader( VertexShaderHandle_t hShader ) = 0;
	virtual GeometryShaderHandle_t CreateGeometryShader( IShaderBuffer *pShaderBuffer ) = 0;
	virtual void DestroyGeometryShader( GeometryShaderHandle_t hShader ) = 0;
	virtual PixelShaderHandle_t CreatePixelShader( IShaderBuffer *pShaderBuffer ) = 0;
	virtual void DestroyPixelShader( PixelShaderHandle_t hShader ) = 0;

	// Loading screens drawn while the main thread is busy.
	virtual void EnableNonInteractiveMode(
	    MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t *pInfo ) = 0;
	virtual void RefreshFrontBufferNonInteractive() = 0;
	virtual void HandleThreadEvent( uint32 threadEvent ) = 0;

protected:
	~ILegacyStreamDevice() = default;
};

} // namespace render::legacy

#endif // RENDER_LEGACY_STREAM_DEVICE_H
