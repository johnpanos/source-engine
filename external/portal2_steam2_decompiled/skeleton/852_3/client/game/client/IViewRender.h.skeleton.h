// DWARF declaration skeleton for game/client/IViewRender.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/IViewRender.h:71 sizeof=0x4 (i386)
struct IViewRender
{
public:
	int (**_vptr$IViewRender)(); // +0x0  // line 0
	virtual void Init();  // line 75
	virtual void LevelInit();  // line 78
	virtual void LevelShutdown();  // line 79
	virtual void Shutdown();  // line 82
	virtual void OnRenderStart();  // line 87
	virtual void Render( vrect_t * );  // line 90
	virtual void RenderView( const CViewSetup &, const CViewSetup &, int, int );  // line 94
	virtual int GetDrawFlags();  // line 97
	virtual void StartPitchDrift();  // line 101
	virtual void StopPitchDrift();  // line 102
	virtual VPlane *GetFrustum();  // line 105
	virtual bool ShouldDrawBrushModels();  // line 107
	virtual const CViewSetup *GetPlayerViewSetup( int ) const;  // line 109
	virtual const CViewSetup *GetViewSetup() const;  // line 110
	virtual void DisableVis();  // line 112
	virtual int BuildWorldListsNumber() const;  // line 114
	virtual void SetCheapWaterStartDistance( float );  // line 116
	virtual void SetCheapWaterEndDistance( float );  // line 117
	virtual void GetWaterLODParams( float &, float & );  // line 119
	virtual void DriftPitch();  // line 121
	virtual void SetScreenOverlayMaterial( IMaterial * );  // line 123
	virtual IMaterial *GetScreenOverlayMaterial();  // line 124
	virtual void WriteSaveGameScreenshot( const char * );  // line 126
	virtual void WriteSaveGameScreenshotOfSize( const char *, int, int );  // line 127
	virtual void QueueOverlayRenderView( const CViewSetup &, int, int );  // line 130
	virtual float GetZNear();  // line 133
	virtual float GetZFar();  // line 134
	virtual void GetScreenFadeDistances( float *, float * );  // line 136
	virtual C_BaseEntity *GetCurrentlyDrawingEntity();  // line 138
	virtual void SetCurrentlyDrawingEntity( C_BaseEntity * );  // line 139
	virtual bool UpdateShadowDepthTexture( ITexture *, ITexture *, const CViewSetup & );  // line 141
	virtual void FreezeFrame( float );  // line 143
	virtual void InitFadeData();  // line 145
};
