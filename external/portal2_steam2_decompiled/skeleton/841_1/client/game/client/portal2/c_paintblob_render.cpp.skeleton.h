// DWARF declaration skeleton for game/client/portal2/c_paintblob_render.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x75130 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::CUtlVector() at line 59
	// inlined QAngle::QAngle() at line 60
}

// game/client/portal2/c_paintblob_render.cpp:22
ConVar r_paintblob_draw_isosurface;

// game/client/portal2/c_paintblob_render.cpp:25
ConVar r_paintblob_rotate;

// game/client/portal2/c_paintblob_render.cpp:26
ConVar r_paintblob_rotate_by90;

// game/client/portal2/c_paintblob_render.cpp:28
ConVar r_paintblob_blr_scale;

// game/client/portal2/c_paintblob_render.cpp:29
ConVar r_paintblob_blr_cubewidth;

// game/client/portal2/c_paintblob_render.cpp:30
ConVar r_paintblob_blr_render_radius;

// game/client/portal2/c_paintblob_render.cpp:31
ConVar r_paintblob_blr_cutoff_radius;

// game/client/portal2/c_paintblob_render.cpp:33
ConVar r_paintblob_calc_uv_and_tan;

// game/client/portal2/c_paintblob_render.cpp:34
ConVar r_paintblob_calc_tan_only;

// game/client/portal2/c_paintblob_render.cpp:35
ConVar r_paintblob_calc_color;

// game/client/portal2/c_paintblob_render.cpp:36
ConVar r_paintblob_calc_hifreq_color;

// game/client/portal2/c_paintblob_render.cpp:37
ConVar r_paintblob_calc_tile_color;

// game/client/portal2/c_paintblob_render.cpp:39
ConVar r_paintblob_tile_index_to_draw;

// game/client/portal2/c_paintblob_render.cpp:40
ConVar r_paintblob_debug_draw_margin;

// game/client/portal2/c_paintblob_render.cpp:41
ConVar r_paintblob_debug_draw_tile_boundaries;

// game/client/portal2/c_paintblob_render.cpp:43
ConVar r_paintblob_wireframe;

// game/client/portal2/c_paintblob_render.cpp:44
ConVar r_paintblob_material;

// game/client/portal2/c_paintblob_render.cpp:45
ConVar r_paintblob_shader;

// game/client/portal2/c_paintblob_render.cpp:47
ConVar r_paintblob_distance_fading_enabled;

// game/client/portal2/c_paintblob_render.cpp:48
ConVar r_paintblob_start_fading_distance;

// game/client/portal2/c_paintblob_render.cpp:49
ConVar r_paintblob_max_fading_distance;

// game/client/portal2/c_paintblob_render.cpp:51
ConVar r_threaded_blobulator;

// game/client/portal2/c_paintblob_render.cpp:52
ConVar r_no_low_res_blobs;

// game/client/portal2/c_paintblob_render.cpp:54
ConVar r_paintblob_only_mainview_highres;

// game/client/portal2/c_paintblob_render.cpp:55
ConVar r_paintblob_only_mainview_displayed;

// game/client/portal2/c_paintblob_render.cpp:57
ConVar r_paintblob_force_single_pass;

// game/client/portal2/c_paintblob_render.cpp:59
BlobParticles_t g_PaintblobRenderParticles;

// game/client/portal2/c_paintblob_render.cpp:60
const QAngle g_PaintblobRenderAnglesAngles;

// game/client/portal2/c_paintblob_render.cpp:62
typedef CUtlEnvelope<ImpParticleWithFourInterpolants> BlobParticlesEnvelope_t;

// game/client/portal2/c_paintblob_render.cpp:71
float g_FastSpherePosData[51][8];

// common/blobulator/SmartArray.h:98 @0x51fb80 _ZN6HelperL4sortIN14NPaintRenderer37PaintBlob_DrawSpheres_sortParticles_tENS2_1CEEEvPT_ii
void Helper::sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t, NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>( NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t *a, int lo0, int hi0 )
{
	int c0;  // line 104
	NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t t;  // line 107
	NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t mid;  // line 125
	int lo;  // line 131
	int hi;  // line 132
	int lSizeM1;  // line 173
	int rSizeM1;  // line 186
}

// game/client/portal2/c_paintblob_render.cpp:126
int g_FastSphereTriData[84][3];

// game/client/portal2/c_paintblob_render.cpp:214 @0x51f390 _ZN14NPaintRenderer24Paintblob_DrawFastSphereER12CMeshBuilderRK6Vectorffff
void NPaintRenderer::Paintblob_DrawFastSphere( CMeshBuilder &meshBuilder, const Vector &center, float radius, float r, float g, float b )
{
	int offset;  // line 216
	Vector pos;  // line 218
	// inlined CMeshBuilder::GetCurrentVertex() at line 216
	{
		int i;  // line 219
		// inlined CMeshBuilder::Position3fv() at line 225
		// inlined CMeshBuilder::Normal3fv() at line 226
		// inlined CMeshBuilder::TexCoord2fv() at line 227
		// inlined CMeshBuilder::Color3f() at line 228
		// inlined CMeshBuilder::AdvanceVertex() at line 229
	}
	{
		int i;  // line 232
		// inlined CMeshBuilder::FastIndex() at line 234
		// inlined CMeshBuilder::FastIndex() at line 235
		// inlined CMeshBuilder::FastIndex() at line 236
	}
}

// game/client/portal2/c_paintblob_render.cpp:241 sizeof=0x8 (i386)
struct PaintBlob_DrawSpheres_sortParticles_t
{
public:
	int no; // +0x0  // line 242
	float dist; // +0x4  // line 243
};

// game/client/portal2/c_paintblob_render.cpp:255 @0x521de0 _ZN14NPaintRenderer21Paintblob_DrawSpheresEP9IMaterialf
void NPaintRenderer::Paintblob_DrawSpheres( IMaterial *pMaterial, float flRadius )
{
	Point3D eye;  // line 257
	SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16> sort_particles;  // line 259
	CMatRenderContextPtr pRenderContext;  // line 269
	IMesh *pMesh;  // line 274
	int vertMax;  // line 276
	int j;  // line 278
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::~SmartArray() at line 301
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 301
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 279
	{
		int total;  // line 281
		CMeshBuilder meshBuilder;  // line 283
		int i;  // line 286
		// inlined CMeshBuilder::~CMeshBuilder() at line 279
		{
			ImpParticleWithOneInterpolant *imp_particle;  // line 289
		}
		// inlined CMeshBuilder::CMeshBuilder() at line 283
		// inlined CMeshBuilder::Begin() at line 284
		// inlined CMeshBuilder::End() at line 299
		// inlined CMeshBuilder::~CMeshBuilder() at line 279
	}
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 274
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 272
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 271
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 269
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>() at line 267
	{
		int i;  // line 262
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::operator[]() at line 265
		// inlined Point3D::length() at line 265
	}
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::ensureCapacity() at line 260
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 260
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::SmartArray() at line 259
	// inlined Point3D::Point3D() at line 257
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 301
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::~SmartArray() at line 301
}

// game/client/portal2/c_paintblob_render.cpp:305 @0x51fd70 _ZN14NPaintRenderer29Paintblob_SafeLightCubeUpdateERK6VectorP8Vector4D
void NPaintRenderer::Paintblob_SafeLightCubeUpdate( const Vector &vecRenderOrigin, Vector4D *cachedCubeColours )
{
	Vector boxColors[6];  // line 312
	bool invalidOrigin;  // line 316
	bool bBadLightCube;  // line 340
	{
		int i;  // line 341
		// inlined IsFinite() at line 343
		// inlined IsFinite() at line 343
		// inlined IsFinite() at line 343
	}
	{
		int i;  // line 329
		// inlined Lerp<Vector>() at line 337
		// inlined Vector::operator=() at line 337
	}
	{
		int i;  // line 319
		// inlined IsFinite() at line 321
		// inlined IsFinite() at line 321
		// inlined IsFinite() at line 321
	}
}

// game/client/portal2/c_paintblob_render.cpp:360 @0x51f680 _ZN14NPaintRenderer15GetDrawMaterialEv
IMaterial *NPaintRenderer::GetDrawMaterial()
{
	// inlined ConVar::GetString() at line 373
	// inlined ConVar::GetString() at line 375
}

// game/client/portal2/c_paintblob_render.cpp:412 @0x51fb10 _ZN14NPaintRenderer32SetUpImpRendererUserDefinedFuncsEv
void NPaintRenderer::SetUpImpRendererUserDefinedFuncs()
{
}

// game/client/portal2/c_paintblob_render.cpp:453 @0x520ba0 _ZN14NPaintRenderer18TransformParticlesEfRK6VectorRS0_S3_R7VMatrixS5_P31ImpParticleWithFourInterpolantsi
void NPaintRenderer::TransformParticles( float angle, const Vector &center, Vector &transformedCenter, Vector &transformedEye, VMatrix &rotationMatrix, VMatrix &invRotationMatrix, ImpParticleWithFourInterpolants *blobParticles, int blobCount )
{
	{
		int i;  // line 455
		{
			ImpParticleWithFourInterpolants *imp_particle;  // line 457
			Vector vParticle;  // line 459
			Vector transformedParticle;  // line 460
			Point3D pParticle;  // line 462
			Point3D pCenter;  // line 476
			Point3D vec;  // line 477
			// inlined Point3D::AsVector() at line 459
			// inlined Vector::operator-() at line 460
			// inlined VMatrix::ApplyRotation() at line 462
			// inlined Point3D::Point3D() at line 462
			// inlined SetWSIMD() at line 464
			// inlined Point3D::Point3D() at line 476
			// inlined Point3D::operator-() at line 477
			// inlined Point3D::unit() at line 479
			// inlined Point3D::set() at line 480
			// inlined Point3D::crossProduct() at line 481
			// inlined Point3D::normalize() at line 482
			// inlined Point3D::crossProduct() at line 483
			// inlined Point3D::normalize() at line 484
		}
	}
}

// game/client/portal2/c_paintblob_render.cpp:489 sizeof=0x10 (i386)
struct CDrawInfo
{
public:
	CDrawInfo( ImpTiler *, float, view_id_t );  // line 491
	~CDrawInfo();  // line 500
	ImpTiler *GetTiler() const;  // line 505
	float GetAngle() const;  // line 510
	CMatRenderContextPtr &SetRenderContext( IMaterialSystem * );  // line 515
	CMatRenderContextPtr &GetRenderContext();  // line 521
	view_id_t GetViewId() const;  // line 526
private:
	CDrawInfo( const NPaintRenderer::CDrawInfo & );  // line 533
	NPaintRenderer::CDrawInfo &operator=( const NPaintRenderer::CDrawInfo & );  // line 534
	ImpTiler *m_pTiler; // +0x0  // line 537
	float m_fAngle; // +0x4  // line 538
	CMatRenderContextPtr m_pRenderContext; // +0x8  // line 539
	view_id_t m_ViewId; // +0xc  // line 540
};

// game/client/portal2/c_paintblob_render.cpp:543 @0x51fb20 _ZN14NPaintRenderer11GetMaterialEP9IMaterial
IMaterial *NPaintRenderer::GetMaterial( IMaterial *pMaterial )
{
	IMaterial *pSpecialMaterial;  // line 545
	// inlined IsErrorMaterial() at line 546
}

// public/tier1/functors.h:545 @0x522fb0 _ZN9CFunctor8IPFvP17IClientRenderable6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEibES1_S2_S4_fS5_S8_ib12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEEclEv
void CFunctor8<void (*)(IClientRenderable*, Vector, IMaterial*, float, view_id_t, CUtlEnvelope<ImpParticleWithFourInterpolants>&, int, bool),IClientRenderable*,Vector,IMaterial*,float,view_id_t,CUtlEnvelope<ImpParticleWithFourInterpolants>,int,bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::operator()()
{
}

// public/tier1/functors.h:545 @0x523030 _ZN9CFunctor5IPFvP17IClientRenderable6VectorP9IMaterialPN14NPaintRenderer9CDrawInfoEbES1_S2_S4_S7_b12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEEclEv
void CFunctor5<void (*)(IClientRenderable*, Vector, IMaterial*, NPaintRenderer::CDrawInfo*, bool),IClientRenderable*,Vector,IMaterial*,NPaintRenderer::CDrawInfo*,bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::operator()()
{
}

// public/tier1/functors.h:545 @0x5230a0 _ZN9CFunctor8IPFvP17IClientRenderable6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEibES1_S2_S4_fS5_S8_ib12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED1Ev
CFunctor8<void (*)(IClientRenderable*, Vector, IMaterial*, float, view_id_t, CUtlEnvelope<ImpParticleWithFourInterpolants>&, int, bool),IClientRenderable*,Vector,IMaterial*,float,view_id_t,CUtlEnvelope<ImpParticleWithFourInterpolants>,int,bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor8()
{
	// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 545
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 545
}

// public/tier1/functors.h:545 @0x523340 _ZN9CFunctor5IPFvP17IClientRenderable6VectorP9IMaterialPN14NPaintRenderer9CDrawInfoEbES1_S2_S4_S7_b12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED0Ev
CFunctor5<void (*)(IClientRenderable*, Vector, IMaterial*, NPaintRenderer::CDrawInfo*, bool),IClientRenderable*,Vector,IMaterial*,NPaintRenderer::CDrawInfo*,bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor5()
{
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 545
}

// public/tier1/functors.h:545 @0x523380 _ZN9CFunctor5IPFvP17IClientRenderable6VectorP9IMaterialPN14NPaintRenderer9CDrawInfoEbES1_S2_S4_S7_b12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED1Ev
CFunctor5<void (*)(IClientRenderable*, Vector, IMaterial*, NPaintRenderer::CDrawInfo*, bool),IClientRenderable*,Vector,IMaterial*,NPaintRenderer::CDrawInfo*,bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor5()
{
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 545
}

// public/tier1/functors.h:545 @0x5233c0 _ZN9CFunctor8IPFvP17IClientRenderable6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEibES1_S2_S4_fS5_S8_ib12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED0Ev
CFunctor8<void (*)(IClientRenderable*, Vector, IMaterial*, float, view_id_t, CUtlEnvelope<ImpParticleWithFourInterpolants>&, int, bool),IClientRenderable*,Vector,IMaterial*,float,view_id_t,CUtlEnvelope<ImpParticleWithFourInterpolants>,int,bool,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor8()
{
	// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 545
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 545
}

// game/client/portal2/c_paintblob_render.cpp:553 @0x521040 _ZN14NPaintRenderer18SetupRenderContextEP17IClientRenderableRK6VectorP9IMaterialRNS_9CDrawInfoE
void NPaintRenderer::SetupRenderContext( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, NPaintRenderer::CDrawInfo &drawInfo )
{
	CMatRenderContextPtr &pRenderContext;  // line 557
	// inlined CDrawInfo::SetRenderContext() at line 557
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 558
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 559
	// inlined NPaintRenderer::GetMaterial() at line 559
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 560
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 561
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 562
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 563
}

// game/client/portal2/c_paintblob_render.cpp:566 @0x51fb60 _ZN14NPaintRenderer20CleanupRenderContextERNS_9CDrawInfoE
void NPaintRenderer::CleanupRenderContext( NPaintRenderer::CDrawInfo &drawInfo )
{
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 568
}

// game/client/portal2/c_paintblob_render.cpp:571 @0x521160 _ZN14NPaintRenderer19Paintblob_BeginDrawEP17IClientRenderableRK6VectorP9IMaterialf9view_id_tP31ImpParticleWithFourInterpolantsib
NPaintRenderer::CDrawInfo *NPaintRenderer::Paintblob_BeginDraw( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, view_id_t viewID, ImpParticleWithFourInterpolants *blobParticles, int blobCount, bool bUseTwoPassesToDraw )
{
	CVProfScope VProf_;  // line 573
	Vector transformedCenter;  // line 586
	VMatrix rotationMatrix;  // line 588
	VMatrix invRotationMatrix;  // line 589
	Vector transformedEye;  // line 590
	float fAngle;  // line 591
	float flPaintResolution;  // line 616
	const float flMaxPaintScale;  // line 617
	float flPaintResolutionLow;  // line 618
	ImpTiler *pTiler;  // line 642
	NPaintRenderer::CDrawInfo *pDrawInfo;  // line 650
	{
		int i;  // line 679
		{
			Vector overlayCenter;  // line 681
			Vector mins;  // line 682
			Vector maxs;  // line 683
			// inlined QAngle::operator QAngleByValue&() at line 684
			// inlined Vector::operator VectorByValue&() at line 684
			// inlined VMatrix::ApplyRotation() at line 681
			// inlined Vector::operator VectorByValue&() at line 681
			// inlined ImpTiler::getRenderDim() at line 682
			// inlined Vector::operator*() at line 682
			// inlined ImpTiler::getRenderDim() at line 683
			// inlined Vector::operator*() at line 683
			// inlined QAngle::QAngle() at line 684
			// inlined Vector::operator+() at line 684
		}
	}
	{
		Vector eye;  // line 604
		// inlined Vector::Vector() at line 602
		// inlined Vector::operator VectorByValue&() at line 602
		// inlined Vector::Vector() at line 603
		// inlined Vector::operator VectorByValue&() at line 603
		// inlined Vector::operator-() at line 605
		// inlined VMatrix::ApplyRotation() at line 606
		// inlined Vector::operator=() at line 606
	}
	// inlined CVProfScope::~CVProfScope() at line 688
	// inlined CVProfScope::CVProfScope() at line 573
	// inlined VMatrix::Identity() at line 610
	// inlined VMatrix::Identity() at line 611
	// inlined Vector::Init() at line 612
	// inlined ConVar::GetFloat() at line 616
	// inlined ImpTiler::setTileIndexToDraw() at line 648
	// inlined CDrawInfo::CDrawInfo() at line 650
	// inlined Point3D::Point3D() at line 659
	{
		int i;  // line 661
	}
	// inlined Point3D::Point3D() at line 668
	// inlined CVProfScope::~CVProfScope() at line 688
}

// game/client/portal2/c_paintblob_render.cpp:691 @0x5226f0 _ZN14NPaintRenderer23PaintBlob_WaitEndOfDrawEP17IClientRenderable6VectorP9IMaterialPNS_9CDrawInfoEb
void NPaintRenderer::PaintBlob_WaitEndOfDraw( IClientRenderable *pClientRenderable, const Vector vecRenderOrigin, IMaterial *pMaterial, NPaintRenderer::CDrawInfo *pDrawInfo, bool bUseTwoPassesToDraw )
{
	ImpTiler *pTiler;  // line 693
	// inlined CDrawInfo::~CDrawInfo() at line 706
	// inlined NPaintRenderer::CleanupRenderContext() at line 703
	// inlined CDrawInfo::GetTiler() at line 693
}

// game/client/portal2/c_paintblob_render.cpp:710 @0x521bf0 _ZN14NPaintRenderer32Paintblob_CullOutOfViewParticlesEv
void NPaintRenderer::Paintblob_CullOutOfViewParticles()
{
	int iParticlesRemoved;  // line 712
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::SetCountNonDestructively() at line 731
	{
		int i;  // line 714
		{
			ImpParticleWithOneInterpolant *imp_particle;  // line 716
			Vector vCenter;  // line 717
			// inlined Vector::Vector() at line 717
			{
				ImpParticleWithOneInterpolant *last_particle;  // line 721
				// inlined ImpParticleWithOneInterpolant::operator=() at line 722
			}
		}
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 714
	}
}

// game/client/portal2/c_paintblob_render.cpp:735 @0x5227f0 _ZN14NPaintRenderer36Paintblob_DrawIsoSurface_SynchronousEP17IClientRenderable6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEib
void NPaintRenderer::Paintblob_DrawIsoSurface_Synchronous( IClientRenderable *pClientRenderable, const Vector vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, view_id_t viewID, BlobParticlesEnvelope_t &particles, int blobCount, bool bUseTwoPassesToDraw )
{
	NPaintRenderer::CDrawInfo *pDrawInfo;  // line 737
	// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::operator ImpParticleWithFourInterpolants*() at line 737
}

// game/client/portal2/c_paintblob_render.cpp:813
ConVar cl_debug_paint_clip_box;

// game/client/portal2/c_paintblob_render.cpp:815 @0x522890 _ZN14NPaintRenderer14Paintblob_DrawEiP17IClientRenderableRK6VectorP9IMaterialfRK10CUtlVectorI11matrix3x4_t10CUtlMemoryIS8_iEEb
void NPaintRenderer::Paintblob_Draw( int renderMode, IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, const PortalMatrixList_t &portalMatrixList, bool bSurfaceNoParticleCull )
{
	Vector backwardPortalMins;  // line 817
	Vector backwardPortalMaxs;  // line 818
	const Vector &nonConstVecRenderOrigin;  // line 844
	{
		CMatRenderContextPtr pRenderContext;  // line 854
		ICallQueue *pQueue;  // line 855
		bool bUseTwoPassesToDraw;  // line 857
		const int mTotalNumberOfFragments;  // line 860
		const int mNumberOfRendererInUse;  // line 861
		const int mNumberOfJobsPerRenderer;  // line 862
		bool bPassedFlag;  // line 875
		view_id_t viewId;  // line 882
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 916
		{
			NPaintRenderer::CDrawInfo *pDrawInfo;  // line 887
			// inlined ICallQueue::QueueCall<void, IClientRenderable*, Vector, IMaterial*, NPaintRenderer::CDrawInfo*, bool, IClientRenderable*, Vector, IMaterial*, NPaintRenderer::CDrawInfo*, bool>() at line 895
		}
		{
			BlobParticlesEnvelope_t particles;  // line 907
			// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 915
			// inlined NPaintRenderer::Paintblob_DrawIsoSurface_Synchronous() at line 914
			// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 914
			// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 907
			// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::CUtlEnvelope() at line 907
			// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 910
			// inlined ICallQueue::QueueCall<void, IClientRenderable*, Vector, IMaterial*, float, view_id_t, BlobParticlesEnvelope_t&, int, bool, IClientRenderable*, Vector, IMaterial*, float, view_id_t, BlobParticlesEnvelope_t, int, bool>() at line 910
			// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 915
		}
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 855
		// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 854
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 916
	}
	// inlined Vector::Vector() at line 817
	// inlined Vector::Vector() at line 818
	{
		int i;  // line 821
		{
			Vector pos;  // line 823
			QAngle ang;  // line 824
			// inlined CUtlVector<matrix3x4_t,CUtlMemory<matrix3x4_t, int> >::operator[]() at line 825
			// inlined MatrixAngles() at line 825
		}
	}
}

// public/tier1/utlmemory.h:959 @0x523120 _ZN17CUtlMemoryAlignedI31ImpParticleWithFourInterpolantsLi16EED1Ev
CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::~CUtlMemoryAligned()
{
	// inlined CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Purge() at line 961
	// inlined CUtlMemory<ImpParticleWithFourInterpolants,int>::~CUtlMemory() at line 961
	// inlined CUtlMemory<ImpParticleWithFourInterpolants,int>::~CUtlMemory() at line 961
}

// public/tier1/utlmemory.h:999 @0x5231e0 _ZN17CUtlMemoryAlignedI31ImpParticleWithFourInterpolantsLi16EE4GrowEi
void CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Grow( int num )
{
	int nAllocationRequested;  // line 1014
	// inlined MemAlloc_ReallocAligned() at line 1023
	// inlined UtlMemory_CalcNewAllocationCount() at line 1016
	// inlined CUtlMemory<ImpParticleWithFourInterpolants,int>::IsExternallyAllocated() at line 1003
	// inlined MemAlloc_AllocAlignedUnattributed() at line 1029
}
