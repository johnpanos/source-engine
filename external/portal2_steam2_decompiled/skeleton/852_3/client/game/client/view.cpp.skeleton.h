// DWARF declaration skeleton for game/client/view.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x87580 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
}

// game/client/view.cpp:89
ConVar zoom_sensitivity_ratio;

// game/client/view.cpp:92
IViewRender *view;

// game/client/view.cpp:98
static Vector g_vecRenderOrigin[2];

// game/client/view.cpp:99
static QAngle g_vecRenderAngles[2];

// game/client/view.cpp:100
static Vector g_vecPrevRenderOrigin[2];

// game/client/view.cpp:101
static QAngle g_vecPrevRenderAngles[2];

// game/client/view.cpp:102
static Vector g_vecVForward[2];

// game/client/view.cpp:102
static Vector g_vecVRight[2];

// game/client/view.cpp:102
static Vector g_vecVUp[2];

// game/client/view.cpp:103
static VMatrix g_matCamInverse[2];

// game/client/view.cpp:107
static ConVar v_centermove;

// game/client/view.cpp:108
static ConVar v_centerspeed;

// game/client/view.cpp:116
ConVar v_viewmodel_fov;

// game/client/view.cpp:119
static ConVar mat_viewportscale;

// game/client/view.cpp:121
ConVar cl_leveloverview;

// game/client/view.cpp:123
ConVar r_mapextents;

// game/client/view.cpp:126
static ConVar cl_camera_follow_bone_index;

// game/client/view.cpp:127
Vector g_cameraFollowPos;

// game/client/view.cpp:130
ConVar gl_clear;

// game/client/view.cpp:131
ConVar gl_clear_randomcolor;

// game/client/view.cpp:133
static ConVar r_farz;

// game/client/view.cpp:134
static ConVar cl_demoviewoverride;

// game/client/view.cpp:136
static Vector s_DemoView;

// game/client/view.cpp:137
static QAngle s_DemoAngle;

// game/client/view.cpp:139 (declaration)
void CalcDemoViewOverride( Vector &origin, QAngle &angles );

// game/client/view.cpp:164 (declaration)
void GetView( int nSlot );

// game/client/view.cpp:164 @0x6535e0 _ZN11CViewRender7GetViewEi
CViewSetup &CViewRender::GetView( int nSlot )
{
}

// game/client/view.cpp:175 @0x653640 _ZNK11CViewRender7GetViewEi
const CViewSetup &CViewRender::GetView( int nSlot )
{
}

// game/client/view.cpp:189 (declaration)
const Vector &MainViewOrigin( int nSlot );

// game/client/view.cpp:189 @0x6536a0 _Z14MainViewOrigini
const Vector &MainViewOrigin( int nSlot )
{
}

// game/client/view.cpp:194 (declaration)
const QAngle &MainViewAngles( int nSlot );

// game/client/view.cpp:194 @0x6536c0 _Z14MainViewAnglesi
const QAngle &MainViewAngles( int nSlot )
{
}

// game/client/view.cpp:199 @0x6536e0 _Z15MainViewForwardi
const Vector &MainViewForward( int nSlot )
{
}

// game/client/view.cpp:204 @0x653700 _Z13MainViewRighti
const Vector &MainViewRight( int nSlot )
{
}

// game/client/view.cpp:209 @0x653720 _Z10MainViewUpi
const Vector &MainViewUp( int nSlot )
{
}

// game/client/view.cpp:214 @0x653740 _Z21MainWorldToViewMatrixi
const VMatrix &MainWorldToViewMatrix( int nSlot )
{
}

// game/client/view.cpp:219 @0x653760 _Z18PrevMainViewOrigini
const Vector &PrevMainViewOrigin( int nSlot )
{
}

// game/client/view.cpp:224 @0x653780 _Z18PrevMainViewAnglesi
const QAngle &PrevMainViewAngles( int nSlot )
{
}

// game/client/view.cpp:233 (declaration)
void ComputeCameraVariables( const Vector &vecOrigin, const QAngle &vecAngles, Vector *pVecForward, Vector *pVecRight, Vector *pVecUp, VMatrix *pMatCamInverse );

// game/client/view.cpp:233 @0x653c20 _Z22ComputeCameraVariablesRK6VectorRK6QAnglePS_S5_S5_P7VMatrix
ComputeCameraVariables( const Vector &vecOrigin, const QAngle &vecAngles, Vector *pVecForward, Vector *pVecRight, Vector *pVecUp, VMatrix *pMatCamInverse )
{
	{
		int i;  // line 238
	}
}

// game/client/view.cpp:256 @0x6537a0 _Z12R_CullSpherePK6VPlaneiPK6Vectorf
bool R_CullSphere( const VPlane *pPlanes, int nPlanes, const Vector *pCenter, float radius )
{
	{
		int i;  // line 258
		// inlined VPlane::DistTo() at line 259
	}
}

// game/client/view.cpp:269 @0x653860 _ZL15StartPitchDriftv
StartPitchDrift()
{
}

// game/client/view.cpp:274
static ConCommand centerview;

// game/client/view.cpp:279 @0x6541d0 _ZN11CViewRender4InitEv
void CViewRender::Init()
{
	CMaterialReference g_material_WriteZ;  // line 295
	{
		int i;  // line 298
		// inlined VMatrix::Identity() at line 307
		// inlined Vector::Init() at line 306
		// inlined Vector::Init() at line 305
		// inlined Vector::Init() at line 304
		// inlined QAngle::Init() at line 303
		// inlined Vector::Init() at line 302
		// inlined QAngle::Init() at line 301
		// inlined Vector::Init() at line 300
	}
}

// game/client/view.cpp:311 @0x653880 _ZN11CViewRender8GetWhiteEv
CMaterialReference &CViewRender::GetWhite()
{
}

// game/client/view.cpp:319 @0x653dd0 _ZN11CViewRender9LevelInitEv
void CViewRender::LevelInit()
{
	{
		int i;  // line 327
	}
}

// game/client/view.cpp:345 @0x653890 _ZN11CViewRender13LevelShutdownEv
void CViewRender::LevelShutdown()
{
}

// game/client/view.cpp:353 @0x653d50 _ZN11CViewRender8ShutdownEv
void CViewRender::Shutdown()
{
}

// game/client/viewrender.h:362 @0x656940 _ZN11CViewRender14RenderPreSceneERK10CViewSetup
void CViewRender::RenderPreScene( const CViewSetup &view )
{
}

// game/client/viewrender.h:363 @0x656950 _ZN11CViewRender16PreViewDrawSceneERK10CViewSetup
void CViewRender::PreViewDrawScene( const CViewSetup &view )
{
}

// game/client/viewrender.h:364 @0x656960 _ZN11CViewRender17PostViewDrawSceneERK10CViewSetup
void CViewRender::PostViewDrawScene( const CViewSetup &view )
{
}

// game/client/viewrender.h:368 @0x656ba0 _ZN11CViewRenderD1Ev
CViewRender::~CViewRender()
{
	// inlined CViewRender::~CViewRender() at line 368
}

// game/client/viewrender.h:368 @0x656d40 _ZN11CViewRenderD0Ev
CViewRender::~CViewRender()
{
	// inlined CViewRender::CCommandMemberInitializer_OnScreenFadeMaxSize::~CCommandMemberInitializer_OnScreenFadeMaxSize() at line 368
	// inlined CViewRender::CCommandMemberInitializer_OnScreenFadeMinSize::~CCommandMemberInitializer_OnScreenFadeMinSize() at line 368
	// inlined CViewRender::CCommandMemberInitializer_OnScreenFadeMinSize::~CCommandMemberInitializer_OnScreenFadeMinSize() at line 368
}

// game/client/view.cpp:369 @0x6538b0 _ZNK11CViewRender21BuildWorldListsNumberEv
int CViewRender::BuildWorldListsNumber()
{
}

// game/client/view.cpp:377 @0x6538c0 _ZN11CViewRender15StartPitchDriftEv
void CViewRender::StartPitchDrift()
{
}

// game/client/view.cpp:396 @0x653930 _ZN11CViewRender14StopPitchDriftEv
void CViewRender::StopPitchDrift()
{
}

// game/client/view.cpp:408 @0x653f30 _ZN11CViewRender10DriftPitchEv
void CViewRender::DriftPitch()
{
	float delta;  // line 410
	float move;  // line 410
	C_BasePlayer *player;  // line 412
	// inlined CPrediction::GetIdealPitch() at line 447
	// inlined QAngle::QAngle() at line 467
	// inlined QAngle::operator+() at line 467
	// inlined QAngle::operator QAngleByValue&() at line 467
	// inlined QAngle::QAngle() at line 476
	// inlined QAngle::operator-() at line 476
	// inlined QAngle::operator QAngleByValue&() at line 476
}

// game/client/viewrender.h:420 @0x656970 _ZN11CViewRender16ShouldForceNoVisEv
bool CViewRender::ShouldForceNoVis()
{
}

// game/client/viewrender.h:427 @0x656b70 _ZN11CViewRender10GetFrustumEv
VPlane *CViewRender::GetFrustum()
{
}

// game/client/viewrender.h:430 @0x656980 _ZN11CViewRender12GetDrawFlagsEv
int CViewRender::GetDrawFlags()
{
}

// game/client/view.cpp:482 @0x655240 _ZN11CViewRender13OnRenderStartEv
void CViewRender::OnRenderStart()
{
	// inlined FrustumCache_t::SetUpdated() at line 551
	{
		int iSlot;  // line 546
		{
			const CViewSetup &view;  // line 548
		}
	}
	{
		int hh;  // line 486
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuardNoVgui;  // line 488
			C_BasePlayer *player;  // line 494
			{
				int iDefaultFOV;  // line 500
				int localFOV;  // line 501
				int min_fov;  // line 502
				// inlined ConVar::GetFloat() at line 511
				// inlined ConVar::GetInt() at line 500
			}
		}
	}
}

// game/client/view.cpp:562 @0x653970 _ZNK11CViewRender12GetViewSetupEv
const CViewSetup *CViewRender::GetViewSetup()
{
}

// game/client/view.cpp:571 @0x653980 _ZNK11CViewRender18GetPlayerViewSetupEi
const CViewSetup *CViewRender::GetPlayerViewSetup( int nSlot )
{
}

// game/client/view.cpp:585 @0x6539e0 _ZN11CViewRender10DisableVisEv
void CViewRender::DisableVis()
{
}

// game/client/view.cpp:598 @0x6539f0 _ZN11CViewRender8GetZNearEv
float CViewRender::GetZNear()
{
}

// game/client/view.cpp:603 @0x653e70 _ZN11CViewRender7GetZFarEv
float CViewRender::GetZFar()
{
	float farZ;  // line 606
	// inlined ConVar::GetFloat() at line 607
	{
		C_BasePlayer *pPlayer;  // line 612
	}
}

// game/client/view.cpp:633 @0x654540 _ZN11CViewRender9SetUpViewEv
void CViewRender::SetUpView()
{
	int nSlot;  // line 636
	CVProfScope VProf_;  // line 639
	float farZ;  // line 641
	CViewSetup &view;  // line 643
	C_BasePlayer *pPlayer;  // line 658
	bool bNoViewEnt;  // line 660
	float flFOVOffset;  // line 733
	AudioState_t audioState;  // line 743
	// inlined CVProfScope::CVProfScope() at line 639
	// inlined CViewRender::GetView() at line 643
	{
		int viewentity;  // line 686
		{
			C_BaseEntity *ve;  // line 690
			// inlined CClientEntityList::GetEnt() at line 690
			// inlined VectorCopy() at line 693
			// inlined VectorCopy() at line 694
		}
		// inlined VectorCopy() at line 703
	}
	// inlined CViewRender::GetView() at line 709
	// inlined CalcDemoViewOverride() at line 720
	// inlined ComputeCameraVariables() at line 740
	// inlined AudioState_t::AudioState_t() at line 743
	// inlined Vector::operator=() at line 744
	// inlined QAngle::operator=() at line 745
	// inlined Vector::operator=() at line 750
	// inlined QAngle::operator=() at line 751
	// inlined Vector::operator=() at line 756
	// inlined QAngle::operator=() at line 757
	// inlined Vector::operator=() at line 758
	// inlined QAngle::operator=() at line 759
	// inlined CVProfScope::~CVProfScope() at line 766
	// inlined Vector::operator=() at line 724
	// inlined QAngle::operator=() at line 725
	// inlined CVProfScope::~CVProfScope() at line 766
}

// game/client/view.cpp:769 @0x6554f0 _ZN11CViewRender29WriteSaveGameScreenshotOfSizeEPKcii
void CViewRender::WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height )
{
	CMatRenderContextPtr pRenderContext;  // line 771
	CViewSetup viewSetup;  // line 785
	unsigned char *pImage;  // line 799
	int iMaxTGASize;  // line 805
	void *pTGA;  // line 806
	CUtlBuffer buffer;  // line 807
	char szPathedFileName[1024];  // line 817
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 832
	// inlined CUtlBuffer::~CUtlBuffer() at line 832
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 829
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 828
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 826
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 825
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 823
	// inlined MemAlloc_Alloc() at line 806
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 802
	// inlined MemAlloc_Alloc() at line 799
	// inlined ScaleFOVByWidthRatio() at line 790
	// inlined CViewRender::GetView() at line 790
	// inlined CViewRender::GetView() at line 785
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 782
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 776
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 775
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 773
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 772
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 771
	// inlined CUtlBuffer::~CUtlBuffer() at line 832
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 832
}

// game/client/view.cpp:838 @0x653a00 _ZN11CViewRender23WriteSaveGameScreenshotEPKc
void CViewRender::WriteSaveGameScreenshot( const char *pFilename )
{
}

// game/client/view.cpp:844 (declaration)
float ScaleFOVByWidthRatio( float fovDegrees, float ratio );

// game/client/view.cpp:844 @0x653bb0 _Z20ScaleFOVByWidthRatioff
float ScaleFOVByWidthRatio( float fovDegrees, float ratio )
{
	float halfAngleRadians;  // line 846
	float t;  // line 847
	float retDegrees;  // line 849
}

// game/client/view.cpp:857 @0x6559d0 _ZN11CViewRender13SetUpOverViewEv
void CViewRender::SetUpOverView()
{
	float aspect;  // line 863
	int size_y;  // line 865
	int size_x;  // line 866
	int newCRC;  // line 879
	CMatRenderContextPtr pRenderContext;  // line 887
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 888
	// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 888
	// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 887
	// inlined CViewRender::GetView() at line 883
	// inlined CViewRender::GetView() at line 883
	// inlined CViewRender::GetView() at line 879
	// inlined CViewRender::GetView() at line 879
	// inlined CViewRender::GetView() at line 879
	// inlined QAngle::operator=() at line 876
	// inlined CViewRender::GetView() at line 876
	// inlined QAngle::QAngle() at line 876
	// inlined CViewRender::GetView() at line 874
	// inlined CViewRender::GetView() at line 873
	// inlined CViewRender::GetView() at line 872
	// inlined CViewRender::GetView() at line 871
	// inlined CViewRender::GetView() at line 869
	// inlined CViewRender::GetView() at line 868
	// inlined CViewRender::GetView() at line 863
	// inlined CViewRender::GetView() at line 863
	// inlined CViewRender::GetView() at line 861
	// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 888
	int oldCRC;  // line 859
}

// game/client/view.cpp:897
ConVar ss_debug_draw_player;

// game/client/view.cpp:898 @0x655d80 _ZN11CViewRender6RenderEP7vrect_t
void CViewRender::Render( vrect_t *rect )
{
	CVProfScope VProf_;  // line 900
	CUtlVector<vgui::Panel*,CUtlMemory<vgui::Panel*, int> > roots;  // line 904
	CMatStubHandler matStub;  // line 908
	float flViewportScale;  // line 914
	vrect_t engineRect;  // line 916
	// inlined CVProfScope::CVProfScope() at line 900
	// inlined CUtlVector<vgui::Panel*,CUtlMemory<vgui::Panel*, int> >::CUtlVector() at line 904
	// inlined ConVar::GetFloat() at line 914
	{
		int hh;  // line 922
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuardNoVgui;  // line 924
			CViewSetup &view;  // line 926
			float engineAspectRatio;  // line 928
			int insetX;  // line 934
			int insetY;  // line 934
			float aspectRatio;  // line 937
			int nClearFlags;  // line 956
			bool drawViewModel;  // line 971
			C_BasePlayer *pPlayer;  // line 973
			int flags;  // line 992
			{
				CViewSetup hudViewSetup;  // line 1003
				// inlined CViewSetup::CViewSetup() at line 1003
			}
			// inlined ConVar::GetInt() at line 1001
			// inlined CViewRender::GetView() at line 926
			// inlined ScaleFOVByWidthRatio() at line 938
			// inlined ScaleFOVByWidthRatio() at line 939
			{
				CMatRenderContextPtr pRenderContext;  // line 960
				// inlined CMatRenderContextPtr::CMatRenderContextPtr() at line 960
				// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 961
				// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 962
				// inlined CBaseAutoPtr<IMatRenderContext>::operator->() at line 963
				// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 963
				// inlined CMatRenderContextPtr::~CMatRenderContextPtr() at line 963
			}
		}
	}
	{
		CViewSetup view2d;  // line 1048
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 1057
		}
		// inlined CViewSetup::CViewSetup() at line 1048
	}
	// inlined CUtlVector<vgui::Panel*,CUtlMemory<vgui::Panel*, int> >::~CUtlVector() at line 1062
	// inlined CVProfScope::~CVProfScope() at line 1062
	// inlined CUtlVector<vgui::Panel*,CUtlMemory<vgui::Panel*, int> >::~CUtlVector() at line 1062
	// inlined CVProfScope::~CVProfScope() at line 1062
}

// public/tier1/convar.h:899 @0x656a90 _ZN25CConCommandMemberAccessorI11CViewRenderED0Ev
CConCommandMemberAccessor<CViewRender>::~CConCommandMemberAccessor()
{
}

// public/tier1/convar.h:899 @0x656b00 _ZN25CConCommandMemberAccessorI11CViewRenderED1Ev
CConCommandMemberAccessor<CViewRender>::~CConCommandMemberAccessor()
{
}

// public/tier1/convar.h:909 @0x6569c0 _ZN25CConCommandMemberAccessorI11CViewRenderE15CommandCallbackERK8CCommand
void CConCommandMemberAccessor<CViewRender>::CommandCallback( const CCommand &command )
{
}

// public/tier1/convar.h:915 @0x656a30 _ZN25CConCommandMemberAccessorI11CViewRenderE25CommandCompletionCallbackEPKcR10CUtlVectorI10CUtlString10CUtlMemoryIS5_iEE
int CConCommandMemberAccessor<CViewRender>::CommandCompletionCallback( const char *pPartial, CUtlVector<CUtlString,CUtlMemory<CUtlString, int> > &commands )
{
}

// game/client/view.cpp:1065 @0x653a30 _ZL6GetPosRK8CCommandR6VectorR6QAngle
GetPos( const CCommand &args, Vector &vecOrigin, QAngle &angles )
{
	int nSlot;  // line 1067
	{
		C_BasePlayer *pPlayer;  // line 1072
		// inlined Vector::operator=() at line 1075
		// inlined QAngle::operator=() at line 1076
	}
	// inlined FStrEq() at line 1070
	// inlined CCommand::operator[]() at line 1070
	// inlined CCommand::ArgC() at line 1070
	// inlined QAngle::operator=() at line 1069
	// inlined Vector::operator=() at line 1068
}

// game/client/view.cpp:1081 @0x653b30 _ZL8spec_posRK8CCommand
spec_pos( const CCommand &args )
{
	Vector vecOrigin;  // line 1083
	QAngle angles;  // line 1084
}

// game/client/view.cpp:1081
static ConCommand spec_pos_command;

// game/client/view.cpp:1090 @0x654410 _ZL6getposRK8CCommand
getpos( const CCommand &args )
{
	Vector vecOrigin;  // line 1092
	QAngle angles;  // line 1093
	const char *pCommand1;  // line 1096
	const char *pCommand2;  // line 1097
	// inlined FStrEq() at line 1098
	// inlined CCommand::operator[]() at line 1098
	// inlined CCommand::ArgC() at line 1098
}

// game/client/view.cpp:1090
static ConCommand getpos_command;

// game/client/view.cpp:1108
ConCommand getpos_exact;
