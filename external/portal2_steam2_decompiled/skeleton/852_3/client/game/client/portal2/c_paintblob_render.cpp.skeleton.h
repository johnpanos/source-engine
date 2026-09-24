// DWARF declaration skeleton for game/client/portal2/c_paintblob_render.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x75260 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::CUtlVector() at line 59
	// inlined QAngle::QAngle() at line 60
}

// game/client/portal2/c_paintblob_render.cpp:21
ConVar r_paintblob_draw_isosurface;

// game/client/portal2/c_paintblob_render.cpp:24
ConVar r_paintblob_rotate;

// game/client/portal2/c_paintblob_render.cpp:25
ConVar r_paintblob_rotate_by90;

// game/client/portal2/c_paintblob_render.cpp:27
ConVar r_paintblob_blr_scale;

// game/client/portal2/c_paintblob_render.cpp:28
ConVar r_paintblob_blr_cubewidth;

// game/client/portal2/c_paintblob_render.cpp:29
ConVar r_paintblob_blr_render_radius;

// game/client/portal2/c_paintblob_render.cpp:30
ConVar r_paintblob_blr_cutoff_radius;

// game/client/portal2/c_paintblob_render.cpp:32
ConVar r_paintblob_calc_uv_and_tan;

// game/client/portal2/c_paintblob_render.cpp:33
ConVar r_paintblob_calc_tan_only;

// game/client/portal2/c_paintblob_render.cpp:34
ConVar r_paintblob_calc_color;

// game/client/portal2/c_paintblob_render.cpp:35
ConVar r_paintblob_calc_hifreq_color;

// game/client/portal2/c_paintblob_render.cpp:36
ConVar r_paintblob_calc_tile_color;

// game/client/portal2/c_paintblob_render.cpp:38
ConVar r_paintblob_debug_use_tiler;

// game/client/portal2/c_paintblob_render.cpp:39
ConVar r_paintblob_debug_max_tiles;

// game/client/portal2/c_paintblob_render.cpp:40
ConVar r_paintblob_debug_max_slices;

// game/client/portal2/c_paintblob_render.cpp:41
ConVar r_paintblob_debug_tile;

// game/client/portal2/c_paintblob_render.cpp:42
ConVar r_paintblob_debug_draw_margin;

// game/client/portal2/c_paintblob_render.cpp:43
ConVar r_paintblob_debug_draw_tile_boundaries;

// game/client/portal2/c_paintblob_render.cpp:45
ConVar r_paintblob_wireframe;

// game/client/portal2/c_paintblob_render.cpp:46
ConVar r_paintblob_material;

// game/client/portal2/c_paintblob_render.cpp:47
ConVar r_paintblob_shader;

// game/client/portal2/c_paintblob_render.cpp:49
ConVar r_paintblob_distance_fading_enabled;

// game/client/portal2/c_paintblob_render.cpp:50
ConVar r_paintblob_start_fading_distance;

// game/client/portal2/c_paintblob_render.cpp:51
ConVar r_paintblob_max_fading_distance;

// game/client/portal2/c_paintblob_render.cpp:53
ConVar r_threaded_blobulator;

// game/client/portal2/c_paintblob_render.cpp:54
ConVar r_no_low_res_blobs;

// game/client/portal2/c_paintblob_render.cpp:56
ConVar r_paintblob_only_mainview_highres;

// game/client/portal2/c_paintblob_render.cpp:57
ConVar r_paintblob_only_mainview_displayed;

// game/client/portal2/c_paintblob_render.cpp:59
BlobParticles_t g_PaintblobRenderParticles;

// game/client/portal2/c_paintblob_render.cpp:60
const QAngle g_PaintblobRenderAnglesAngles;

// game/client/portal2/c_paintblob_render.cpp:62
typedef CUtlEnvelope<ImpParticleWithFourInterpolants> BlobParticlesEnvelope_t;

// game/client/portal2/c_paintblob_render.cpp:71
float g_FastSpherePosData[51][8];

// game/client/portal2/c_paintblob_render.cpp:126
int g_FastSphereTriData[84][3];

// game/client/portal2/c_paintblob_render.cpp:214 @0x524940 _ZN14NPaintRenderer24Paintblob_DrawFastSphereER12CMeshBuilderRK6Vectorffff
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

// game/client/portal2/c_paintblob_render.cpp:255 @0x527520 _ZN14NPaintRenderer21Paintblob_DrawSpheresEP9IMaterialf
void NPaintRenderer::Paintblob_DrawSpheres( IMaterial *pMaterial, float flRadius )
{
	Point3D eye;  // line 257
	SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16> sort_particles;  // line 259
	CMatRenderContextPtr pRenderContext;  // line 269
	IMesh *pMesh;  // line 274
	int vertMax;  // line 276
	int j;  // line 278
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::~SmartArray() at line 302
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 302
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 280
	{
		int total;  // line 282
		CMeshBuilder meshBuilder;  // line 284
		int i;  // line 287
		// inlined CMeshBuilder::~CMeshBuilder() at line 280
		// inlined CMeshBuilder::~CMeshBuilder() at line 280
		{
			ImpParticleWithOneInterpolant *imp_particle;  // line 290
		}
		// inlined CMeshBuilder::CMeshBuilder() at line 284
		// inlined CMeshBuilder::Begin() at line 285
		// inlined CMeshBuilder::End() at line 300
	}
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 274
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 272
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 271
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 269
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>() at line 267
	{
		int i;  // line 262
		// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::operator[]() at line 265
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::operator[]() at line 265
		// inlined Point3D::length() at line 265
	}
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::ensureCapacity() at line 260
	// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 260
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::SmartArray() at line 259
	// inlined Point3D::Point3D() at line 257
	// inlined SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::~SmartArray() at line 302
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 302
}

// game/client/portal2/c_paintblob_render.cpp:306 @0x5250d0 _ZN14NPaintRenderer29Paintblob_SafeLightCubeUpdateERK6VectorP8Vector4D
void NPaintRenderer::Paintblob_SafeLightCubeUpdate( const Vector &vecRenderOrigin, Vector4D *cachedCubeColours )
{
	Vector boxColors[6];  // line 313
	bool invalidOrigin;  // line 317
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

// game/client/portal2/c_paintblob_render.cpp:360 @0x524c30 _ZN14NPaintRenderer15GetDrawMaterialEv
IMaterial *NPaintRenderer::GetDrawMaterial()
{
	// inlined ConVar::GetString() at line 373
	// inlined ConVar::GetString() at line 375
}

// game/client/portal2/c_paintblob_render.cpp:412 @0x5250c0 _ZN14NPaintRenderer32SetUpImpRendererUserDefinedFuncsEv
void NPaintRenderer::SetUpImpRendererUserDefinedFuncs()
{
}

// game/client/portal2/c_paintblob_render.cpp:453 @0x525f00 _ZN14NPaintRenderer18TransformParticlesEfRK6VectorRS0_S3_R7VMatrixS5_P31ImpParticleWithFourInterpolantsi
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

// common/blobulator/SmartArray.h:478 @0x5285a0 _ZN10SmartArrayIN14NPaintRenderer37PaintBlob_DrawSpheres_sortParticles_tELb0ELi16EE4sortINS1_1CEEEvii
void SmartArray<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t,false,16>::sort<NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t::C>( int lo0, int hi0 )
{
	int c0;  // line 484
	NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t t;  // line 487
	NPaintRenderer::PaintBlob_DrawSpheres_sortParticles_t mid;  // line 505
	int lo;  // line 511
	int hi;  // line 512
	int lSizeM1;  // line 553
	int rSizeM1;  // line 566
}

// game/client/portal2/c_paintblob_render.cpp:489 @0x5264d0 _ZN14NPaintRenderer24Paintblob_DrawIsoSurfaceEP17IClientRenderableRK6VectorP9IMaterialf9view_id_tP31ImpParticleWithFourInterpolantsi
void NPaintRenderer::Paintblob_DrawIsoSurface( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, view_id_t viewID, ImpParticleWithFourInterpolants *blobParticles, int blobCount )
{
	CVProfScope VProf_;  // line 491
	IMaterial *pSpecialMaterial;  // line 493
	CMatRenderContextPtr pRenderContext;  // line 503
	Vector transformedCenter;  // line 513
	VMatrix rotationMatrix;  // line 517
	VMatrix invRotationMatrix;  // line 518
	Vector transformedEye;  // line 519
	float angle;  // line 520
	float flPaintResolution;  // line 551
	const float flMaxPaintScale;  // line 554
	float flPaintResolutionLow;  // line 555
	{
		ImpTiler *tiler;  // line 594
		Blobulator::IBlobRenderer *sweepRenderer;  // line 595
		{
			int i;  // line 613
		}
		// inlined Point3D::Point3D() at line 611
		// inlined ImpTiler::setMaxNoTilesToDraw() at line 603
		// inlined ImpTiler::getRenderer() at line 595
		// inlined Point3D::Point3D() at line 622
		{
			Vector overlayCenter;  // line 650
			Vector mins;  // line 651
			Vector maxs;  // line 652
			// inlined ImpTiler::getLastTilesOffset() at line 650
			// inlined VMatrix::ApplyRotation() at line 650
			// inlined Vector::operator VectorByValue&() at line 650
			// inlined ImpTiler::getRenderDim() at line 651
			// inlined Vector::operator*() at line 651
			// inlined ImpTiler::getRenderDim() at line 652
			// inlined Vector::operator*() at line 652
			// inlined QAngle::QAngle() at line 653
			// inlined QAngle::operator QAngleByValue&() at line 653
			// inlined Vector::operator+() at line 653
			// inlined Vector::operator VectorByValue&() at line 653
		}
		{
			int i;  // line 633
			{
				Vector overlayCenter;  // line 635
				Vector mins;  // line 636
				Vector maxs;  // line 637
				// inlined QAngle::operator QAngleByValue&() at line 638
				// inlined Vector::operator VectorByValue&() at line 638
				// inlined VMatrix::ApplyRotation() at line 635
				// inlined Vector::operator VectorByValue&() at line 635
				// inlined ImpTiler::getRenderDim() at line 636
				// inlined Vector::operator*() at line 636
				// inlined ImpTiler::getRenderDim() at line 637
				// inlined Vector::operator*() at line 637
				// inlined QAngle::QAngle() at line 638
				// inlined Vector::operator+() at line 638
			}
		}
	}
	// inlined Vector::Init() at line 543
	// inlined VMatrix::Identity() at line 542
	// inlined VMatrix::Identity() at line 541
	// inlined CVProfScope::CVProfScope() at line 491
	// inlined IsErrorMaterial() at line 495
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 503
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 504
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 505
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 506
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 507
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 515
	{
		Vector eye;  // line 535
		// inlined Vector::operator=() at line 537
		// inlined VMatrix::ApplyRotation() at line 537
		// inlined Vector::operator-() at line 536
		// inlined Vector::operator VectorByValue&() at line 534
		// inlined Vector::Vector() at line 534
		// inlined Vector::operator VectorByValue&() at line 533
		// inlined Vector::Vector() at line 533
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 531
	}
	// inlined ConVar::GetFloat() at line 551
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 702
	{
		Blobulator::IBlobRenderer *sweepRenderer;  // line 675
		// inlined Point3D::Point3D() at line 685
		{
			int i;  // line 688
		}
	}
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 702
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 702
	// inlined CVProfScope::~CVProfScope() at line 702
	// inlined CVProfScope::~CVProfScope() at line 702
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 702
}

// public/tier1/functors.h:544 @0x528840 _ZN9CFunctor7IPFvP17IClientRenderableRK6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEiES1_S2_S6_fS7_SA_i12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEEclEv
void CFunctor7<void (*)(IClientRenderable*, const Vector&, IMaterial*, float, view_id_t, CUtlEnvelope<ImpParticleWithFourInterpolants>&, int),IClientRenderable*,Vector,IMaterial*,float,view_id_t,CUtlEnvelope<ImpParticleWithFourInterpolants>,int,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::operator()()
{
}

// public/tier1/functors.h:544 @0x528950 _ZN9CFunctor7IPFvP17IClientRenderableRK6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEiES1_S2_S6_fS7_SA_i12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED0Ev
CFunctor7<void (*)(IClientRenderable*, const Vector&, IMaterial*, float, view_id_t, CUtlEnvelope<ImpParticleWithFourInterpolants>&, int),IClientRenderable*,Vector,IMaterial*,float,view_id_t,CUtlEnvelope<ImpParticleWithFourInterpolants>,int,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor7()
{
	// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 544
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 544
}

// public/tier1/functors.h:544 @0x5289f0 _ZN9CFunctor7IPFvP17IClientRenderableRK6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEiES1_S2_S6_fS7_SA_i12CRefCounted1I8CFunctor20CRefCountServiceBaseILb1E6CRefMTEEED1Ev
CFunctor7<void (*)(IClientRenderable*, const Vector&, IMaterial*, float, view_id_t, CUtlEnvelope<ImpParticleWithFourInterpolants>&, int),IClientRenderable*,Vector,IMaterial*,float,view_id_t,CUtlEnvelope<ImpParticleWithFourInterpolants>,int,CRefCounted1<CFunctor, CRefCountServiceBase<true, CRefMT> > >::~CFunctor7()
{
	// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 544
	// inlined CRefCounted1<CFunctor,CRefCountServiceBase<true, CRefMT> >::~CRefCounted1() at line 544
}

// game/client/portal2/c_paintblob_render.cpp:706 @0x5263a0 _ZN14NPaintRenderer32Paintblob_CullOutOfViewParticlesEv
void NPaintRenderer::Paintblob_CullOutOfViewParticles()
{
	int iParticlesRemoved;  // line 708
	{
		int i;  // line 710
		{
			ImpParticleWithOneInterpolant *imp_particle;  // line 712
			Vector vCenter;  // line 713
			// inlined Vector::Vector() at line 713
			{
				ImpParticleWithOneInterpolant *last_particle;  // line 717
			}
		}
	}
}

// game/client/portal2/c_paintblob_render.cpp:732 @0x5274b0 _ZN14NPaintRendererL26s_Paintblob_DrawIsoSurfaceEP17IClientRenderableRK6VectorP9IMaterialf9view_id_tR12CUtlEnvelopeI31ImpParticleWithFourInterpolantsEi
void NPaintRenderer::s_Paintblob_DrawIsoSurface( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, view_id_t viewID, BlobParticlesEnvelope_t &blobParticles, int blobCount )
{
	// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::operator ImpParticleWithFourInterpolants*() at line 734
}

// game/client/portal2/c_paintblob_render.cpp:738 @0x5280d0 _ZN14NPaintRenderer14Paintblob_DrawEP17IClientRenderableRK6VectorP9IMaterialfb
void NPaintRenderer::Paintblob_Draw( IClientRenderable *pClientRenderable, const Vector &vecRenderOrigin, IMaterial *pMaterial, float flCubeWidth, bool bSurfaceNoParticleCull )
{
	{
		CMatRenderContextPtr pRenderContext;  // line 753
		ICallQueue *pQueue;  // line 754
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 762
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Base() at line 761
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 761
		// inlined ICallQueue::QueueCall<void, IClientRenderable*, const Vector&, IMaterial*, float, view_id_t, BlobParticlesEnvelope_t&, int, IClientRenderable*, Vector, IMaterial*, float, view_id_t, BlobParticlesEnvelope_t, int>() at line 757
		// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::CUtlEnvelope() at line 757
		// inlined CUtlVector<ImpParticleWithFourInterpolants,CUtlMemoryAligned<ImpParticleWithFourInterpolants, 16> >::Count() at line 757
		// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 754
		// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 753
		// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 757
		// inlined CUtlEnvelope<ImpParticleWithFourInterpolants>::~CUtlEnvelope() at line 757
		// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 762
	}
	// inlined NPaintRenderer::Paintblob_CullOutOfViewParticles() at line 743
}

// public/tier1/utlmemory.h:959 @0x528880 _ZN17CUtlMemoryAlignedI31ImpParticleWithFourInterpolantsLi16EED1Ev
CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::~CUtlMemoryAligned()
{
	// inlined CUtlMemoryAligned<ImpParticleWithFourInterpolants,16>::Purge() at line 961
	// inlined CUtlMemory<ImpParticleWithFourInterpolants,int>::~CUtlMemory() at line 961
	// inlined CUtlMemory<ImpParticleWithFourInterpolants,int>::~CUtlMemory() at line 961
}
