//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_DIAGNOSTICS_H
#define RENDER_DIAGNOSTICS_H

#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>

// Private provider diagnostics. No SDK types cross this capture boundary.
namespace renderdiagnostics
{
struct DrawState
{
	const char *material = "";
	const char *kind = "";
	int primitives = 0;
	std::int32_t result = 0;
	unsigned viewportWidth = 0, viewportHeight = 0;
	unsigned targetWidth = 0, targetHeight = 0, targetFormat = 0;
	std::uintptr_t vertexShader = 0, pixelShader = 0, target = 0;
	unsigned textureMask = 0;
	unsigned depthTest = 0, depthWrite = 0, colorWrite = 0, cull = 0, blend = 0;
	unsigned queryErrors = 0;
	bool stateAvailable = false;
};

// Owns its output and bounds draw detail independently of frame summaries.
// Calls are serialized; this diagnostic mutex is never a renderer state lock.
class Capture
{
public:
	Capture( const char *path, unsigned firstFrame, unsigned frameCount, unsigned maxDraws );
	~Capture();
	bool Enabled() const { return m_Output != nullptr; }
	bool WantsDrawState() const;
	void BeginFrame();
	void EndFrame();
	void RequestShader(
	    bool pixel, const char *name, int staticIndex, int dynamicIndex, bool failed );
	void ShaderFailure( const char *name, int staticIndex, int dynamicCount );
	void Draw( const DrawState &state );
	void Skipped( const char *material, const char *reason );
	void Present( std::int32_t result, bool cropped );

private:
	struct Shader
	{
		std::string name;
		int staticIndex = 0, dynamicIndex = 0;
		bool failed = false;
	};
	void WriteShader( const char *stage, const Shader &shader );
	std::FILE *m_Output = nullptr;
	mutable std::mutex m_Mutex;
	Shader m_Vertex, m_Pixel;
	unsigned m_Frame = 0, m_FirstFrame, m_FrameCount, m_MaxDraws;
	unsigned m_Details = 0, m_Failures = 0;
	unsigned m_Draws = 0, m_Primitives = 0, m_Errors = 0, m_MissingShaders = 0;
	unsigned m_Skipped = 0;
};

Capture &Current();
} // namespace renderdiagnostics

#ifdef USE_DXVK
#include "locald3dtypes.h"
void TraceNativeDraw( IDirect3DDevice9 *device, const char *material, const char *kind,
    int primitives, HRESULT result );
#endif
#endif
