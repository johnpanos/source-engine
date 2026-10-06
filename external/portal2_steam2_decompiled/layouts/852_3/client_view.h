// Class layouts from /home/john/Downloads/portal2-steam2-research/852_3/portal2/bin/client.dylib.dSYM/Contents/Resources/DWARF/client.dylib
// Unit game/client/view.cpp; i386 offsets; reconstruction aid.

// game/client/viewrender.h:316
// game/client/viewrender.h:316 sizeof=0x54c (i386)
struct CViewRender : public IViewRender
{
public:
	virtual void Init();  // line 319
	virtual void Shutdown();  // line 320
	virtual const CViewSetup *GetPlayerViewSetup( int ) const;  // line 322
	virtual void StartPitchDrift();  // line 324
	virtual void StopPitchDrift();  // line 325
	virtual float GetZNear();  // line 327
	virtual float GetZFar();  // line 328
	virtual void OnRenderStart();  // line 330
	virtual void DriftPitch();  // line 331
	static CViewRender *GetMainView();  // line 333
	void AddViewToScene( CRendering3dView * );  // line 335
	CMaterialReference &GetWhite();  // line 337
	virtual void InitFadeData();  // line 339
protected:
	void SetUpView();  // line 343
	void SetUpOverView();  // line 346
	virtual void WriteSaveGameScreenshotOfSize( const char *, int, int );  // line 349
	virtual void WriteSaveGameScreenshot( const char * );  // line 350
	CViewSetup &GetView( int );  // line 354
	const CViewSetup &GetView( int ) const;  // line 355
	CViewSetup m_UserView[2]; // +0x4  // line 357
	bool m_bAllowViewAccess; // +0x1ec  // line 358
	CPitchDrift m_PitchDrift; // +0x1f0  // line 360
	virtual void RenderPreScene( const CViewSetup & );  // line 362
	virtual void PreViewDrawScene( const CViewSetup & );  // line 363
	virtual void PostViewDrawScene( const CViewSetup & );  // line 364
public:
	CViewRender();  // line 367
	virtual ~CViewRender();  // line 368
	void SetupVis( const CViewSetup &, unsigned int &, ViewCustomVisibility_t * );  // line 373
	virtual void Render( vrect_t * );  // line 377
	virtual void RenderView( const CViewSetup &, const CViewSetup &, int, int );  // line 378
	virtual void RenderPlayerSprites();  // line 379
	virtual void Render2DEffectsPreHUD( const CViewSetup & );  // line 380
	virtual void Render2DEffectsPostHUD( const CViewSetup & );  // line 381
	void DisableFog();  // line 384
	virtual void LevelInit();  // line 387
	virtual void LevelShutdown();  // line 388
	bool ShouldDrawEntities();  // line 392
	virtual bool ShouldDrawBrushModels();  // line 393
	virtual const CViewSetup *GetViewSetup() const;  // line 395
	virtual void DisableVis();  // line 397
	void MoveViewModels();  // line 400
	void GetViewModelPosition( int, Vector *, QAngle * );  // line 403
	virtual void SetCheapWaterStartDistance( float );  // line 405
	virtual void SetCheapWaterEndDistance( float );  // line 406
	virtual void GetWaterLODParams( float &, float & );  // line 408
	virtual void QueueOverlayRenderView( const CViewSetup &, int, int );  // line 410
	virtual void GetScreenFadeDistances( float *, float * );  // line 412
	virtual C_BaseEntity *GetCurrentlyDrawingEntity();  // line 414
	virtual void SetCurrentlyDrawingEntity( C_BaseEntity * );  // line 415
	virtual bool UpdateShadowDepthTexture( ITexture *, ITexture *, const CViewSetup & );  // line 417
	int GetBaseDrawFlags();  // line 419
	virtual bool ShouldForceNoVis();  // line 420
	int BuildRenderablesListsNumber() const;  // line 421
	int IncRenderablesListsNumber();  // line 422
	virtual int BuildWorldListsNumber() const;  // line 424
	int IncWorldListsNumber();  // line 425
	virtual VPlane *GetFrustum();  // line 427
	virtual int GetDrawFlags();  // line 430
	CBase3dView *GetActiveRenderer();  // line 432
	CBase3dView *SetActiveRenderer( CBase3dView * );  // line 433
	virtual void FreezeFrame( float );  // line 435
	void SetWaterOverlayMaterial( IMaterial * );  // line 437
protected:
	int m_BuildWorldListsNumber; // +0x204  // line 442
	void ViewDrawScene( bool, SkyboxVisibility_t, const CViewSetup &, int, view_id_t, bool, int, ViewCustomVisibility_t * );  // line 447
	void DrawMonitors( const CViewSetup & );  // line 449
	bool DrawOneMonitor( ITexture *, int, C_PointCamera *, const CViewSetup &, C_BasePlayer *, int, int, int, int );  // line 452
	bool ShouldDrawViewModel( bool );  // line 455
	void DrawViewModels( const CViewSetup &, bool );  // line 456
	void PerformScreenSpaceEffects( int, int, int, int );  // line 458
	virtual void SetScreenOverlayMaterial( IMaterial * );  // line 461
	virtual IMaterial *GetScreenOverlayMaterial();  // line 462
	void PerformScreenOverlay( int, int, int, int );  // line 463
	void DrawUnderwaterOverlay();  // line 465
	void DrawWorldAndEntities( bool, const CViewSetup &, int, ViewCustomVisibility_t * );  // line 468
	virtual void ViewDrawScene_Intro( const CViewSetup &, int, const IntroData_t & );  // line 470
	void ViewDrawScene_PortalStencil( const CViewSetup &, ViewCustomVisibility_t * );  // line 474
	void Draw3dSkyboxworld_Portal( const CViewSetup &, int &, bool &, SkyboxVisibility_t &, ITexture * );  // line 475
	void ViewDrawPhoto( ITexture *, C_BaseEntity * );  // line 479
	void DetermineWaterRenderInfo( const VisibleFogVolumeInfo_t &, WaterRenderInfo_t & );  // line 483
	bool UpdateRefractIfNeededByList( CUtlVectorFixedGrowable<CViewModelRenderablesList::CEntry,32ul> & );  // line 485
	void DrawRenderablesInList( CUtlVectorFixedGrowable<CViewModelRenderablesList::CEntry,32ul> &, int );  // line 486
	void SetupMain3DView( int, const CViewSetup &, const CViewSetup &, int &, ITexture * );  // line 489
	void CleanupMain3DView( const CViewSetup & );  // line 490
	void GetLetterBoxRectangles( int, const CViewSetup &, CUtlVector<vrect_t,CUtlMemory<vrect_t, int> > & );  // line 492
	void DrawLetterBoxRectangles( int, const CUtlVector<vrect_t,CUtlMemory<vrect_t, int> > & );  // line 493
	CViewSetup m_CurrentView; // +0x208  // line 496
	bool m_bForceNoVis; // +0x2fc  // line 500
	const ConVar *m_pDrawEntities; // +0x300  // line 503
	const ConVar *m_pDrawBrushModels; // +0x304  // line 504
	CMaterialReference m_TranslucentSingleColor; // +0x308  // line 507
	CMaterialReference m_ModulateSingleColor; // +0x30c  // line 508
	CMaterialReference m_ScreenOverlayMaterial; // +0x310  // line 509
	CMaterialReference m_UnderWaterOverlayMaterial; // +0x314  // line 510
	Vector m_vecLastFacing; // +0x318  // line 512
	float m_flCheapWaterStartDistance; // +0x324  // line 513
	float m_flCheapWaterEndDistance; // +0x328  // line 514
	CViewSetup m_OverlayViewSetup; // +0x32c  // line 516
	int m_OverlayClearFlags; // +0x420  // line 517
	int m_OverlayDrawFlags; // +0x424  // line 518
	bool m_bDrawOverlay; // +0x428  // line 519
	int m_BaseDrawFlags; // +0x42c  // line 521
	C_BaseEntity *m_pCurrentlyDrawingEntity; // +0x430  // line 522
	int m_BuildRenderableListsNumber; // +0x434  // line 528
	Frustum m_Frustum; // +0x438  // line 532
	CBase3dView *m_pActiveRenderer; // +0x498  // line 534
	CSimpleRenderExecutor m_SimpleExecutor; // +0x49c  // line 535
public:
	// game/client/viewrender.h:538 sizeof=0x8 (i386)
	struct FreezeParams_t
	{
	public:
		FreezeParams_t();  // line 539
		bool m_bTakeFreezeFrame; // +0x0  // line 541
		float m_flFreezeFrameUntil; // +0x4  // line 542
	};  // line 538
protected:
	CViewRender::FreezeParams_t m_FreezeParams[2]; // +0x4a4  // line 545
	FadeData_t m_FadeData; // +0x4b4  // line 547
	CMaterialReference m_WhiteMaterial; // +0x4c8  // line 549
public:
	// game/client/viewrender.h:551 sizeof=0x40 (i386)
	struct CCommandMemberInitializer_OnScreenFadeMinSize
	{
	private:
		CConCommandMemberAccessor<CViewRender> m_ConCommandAccessor; // +0x0  // line 551
	public:
		CCommandMemberInitializer_OnScreenFadeMinSize();  // line 551
	};  // line 551
protected:
	CViewRender::CCommandMemberInitializer_OnScreenFadeMinSize m_OnScreenFadeMinSize_register; // +0x4cc  // line 551
	void OnScreenFadeMinSize( const CCommand & );  // line 551
public:
	// game/client/viewrender.h:552 sizeof=0x40 (i386)
	struct CCommandMemberInitializer_OnScreenFadeMaxSize
	{
	private:
		CConCommandMemberAccessor<CViewRender> m_ConCommandAccessor; // +0x0  // line 552
	public:
		CCommandMemberInitializer_OnScreenFadeMaxSize();  // line 552
	};  // line 552
protected:
	CViewRender::CCommandMemberInitializer_OnScreenFadeMaxSize m_OnScreenFadeMaxSize_register; // +0x50c  // line 552
	void OnScreenFadeMaxSize( const CCommand & );  // line 552
};

// public/view_shared.h:44
// public/view_shared.h:44 sizeof=0xf4 (i386)
struct CViewSetup
{
public:
	CViewSetup();  // line 46
	int x; // +0x0  // line 75
	int y; // +0x4  // line 77
	int width; // +0x8  // line 79
	int height; // +0xc  // line 81
	bool m_bOrtho; // +0x10  // line 86
	float m_OrthoLeft; // +0x14  // line 88
	float m_OrthoTop; // +0x18  // line 89
	float m_OrthoRight; // +0x1c  // line 90
	float m_OrthoBottom; // +0x20  // line 91
	bool m_bCustomViewMatrix; // +0x24  // line 93
	matrix3x4_t m_matCustomViewMatrix; // +0x28  // line 94
	float fov; // +0x58  // line 97
	float fovViewmodel; // +0x5c  // line 99
	Vector origin; // +0x60  // line 102
	QAngle angles; // +0x6c  // line 105
	float zNear; // +0x78  // line 107
	float zFar; // +0x7c  // line 109
	float zNearViewmodel; // +0x80  // line 112
	float zFarViewmodel; // +0x84  // line 114
	float m_flAspectRatio; // +0x88  // line 118
	float m_flNearBlurDepth; // +0x8c  // line 121
	float m_flNearFocusDepth; // +0x90  // line 122
	float m_flFarFocusDepth; // +0x94  // line 123
	float m_flFarBlurDepth; // +0x98  // line 124
	float m_flNearBlurRadius; // +0x9c  // line 125
	float m_flFarBlurRadius; // +0xa0  // line 126
	int m_nDoFQuality; // +0xa4  // line 127
	MotionBlurMode_t m_nMotionBlurMode; // +0xa8  // line 130
	float m_flShutterTime; // +0xac  // line 131
	Vector m_vShutterOpenPosition; // +0xb0  // line 132
	QAngle m_shutterOpenAngles; // +0xbc  // line 133
	Vector m_vShutterClosePosition; // +0xc8  // line 134
	QAngle m_shutterCloseAngles; // +0xd4  // line 135
	float m_flOffCenterTop; // +0xe0  // line 138
	float m_flOffCenterBottom; // +0xe4  // line 139
	float m_flOffCenterLeft; // +0xe8  // line 140
	float m_flOffCenterRight; // +0xec  // line 141
	bool m_bOffCenter : 1; // +0xf0  // line 142
	bool m_bRenderToSubrectOfLargerScreen : 1; // +0xf0  // line 146
	bool m_bDoBloomAndToneMapping : 1; // +0xf0  // line 149
	bool m_bDoDepthOfField : 1; // +0xf0  // line 150
	bool m_bHDRTarget : 1; // +0xf0  // line 151
	bool m_bDrawWorldNormal : 1; // +0xf0  // line 152
	bool m_bCullFrontFaces : 1; // +0xf0  // line 153
	bool m_bCacheFullSceneState : 1; // +0xf0  // line 156
};

// CRenderView: not found as a complete record in this unit
