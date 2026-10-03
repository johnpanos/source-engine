//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_LEGACY_TEMPORAL_VIEWS_H
#define RENDER_LEGACY_TEMPORAL_VIEWS_H
#define RENDER_TEMPORAL_VIEWS_INTERFACE_VERSION "RenderTemporalViews001"
class IRenderTemporalViews
{
public:
	// Producer-selected semantic identity, including the portal renderer's
	// complete recursion-chain ID. Zero/unknown never borrows another view.
	virtual void SelectView( unsigned long long identity ) = 0;
	virtual void ResetHistory() = 0;
	virtual bool Enabled() const = 0;
	virtual bool Reconstruct( int x, int y, int renderWidth, int renderHeight, int outputWidth,
	    int outputHeight, float deltaMilliseconds ) = 0;

protected:
	~IRenderTemporalViews() = default;
};
#endif
