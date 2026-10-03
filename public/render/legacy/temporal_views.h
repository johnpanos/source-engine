//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_LEGACY_TEMPORAL_VIEWS_H
#define RENDER_LEGACY_TEMPORAL_VIEWS_H
#define RENDER_TEMPORAL_VIEWS_INTERFACE_VERSION "RenderTemporalViews001"
#define RENDER_TEMPORAL_VIEWS2_INTERFACE_VERSION "RenderTemporalViews002"
class IRenderTemporalViews
{
public:
	// Producer-selected semantic identity, including the portal renderer's
	// complete recursion-chain ID. Zero/unknown never borrows another view.
	virtual void SelectView( unsigned long long identity ) = 0;
	virtual void ResetHistory() = 0;
	virtual bool Enabled() const = 0;
	// Scale selected at the frame boundary, relative to the output viewport.
	virtual float RenderScale() const = 0;
	virtual bool Reconstruct( int x, int y, int renderWidth, int renderHeight, int outputWidth,
	    int outputHeight, float deltaMilliseconds ) = 0;

protected:
	~IRenderTemporalViews() = default;
};
// A separate version leaves the original view ABI intact for older clients.
class IRenderTemporalViews2 : public IRenderTemporalViews
{
public:
	virtual bool Available() const = 0;

protected:
	~IRenderTemporalViews2() = default;
};
#endif
