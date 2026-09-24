// DWARF declaration skeleton for game/client/portal2/c_item_photo.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x743f0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientClass::ClientClass() at line 28
	// inlined CPhotoMaterialFoo::CPhotoMaterialFoo() at line 78
	// inlined CPlacementPhotoFoo::CPlacementPhotoFoo() at line 165
}

// game/client/portal2/c_item_photo.cpp:18 sizeof=0xe88 (i386)
struct C_Photograph : public C_BaseAnimating
{
public:
	const char *GetTextureName();  // line 23
	char m_szTextureName[260]; // +0xd84  // line 24
};

// game/client/portal2/c_item_photo.cpp:18 (declaration)
~C_Photograph();

// game/client/portal2/c_item_photo.cpp:18 @0x522210 _ZN12C_PhotographD0Ev
C_Photograph::~C_Photograph()
{
}

// game/client/portal2/c_item_photo.cpp:18 @0x522290 _ZN12C_PhotographD1Ev
C_Photograph::~C_Photograph()
{
}

// game/client/portal2/c_item_photo.cpp:18 (declaration)
void C_Photograph();

// game/client/portal2/c_item_photo.cpp:20 @0x521ac0 _ZN12C_Photograph40YouForgotToImplementOrDeclareClientClassEv
int C_Photograph::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_item_photo.cpp:20 @0x521ae0 _ZN12C_Photograph14GetClientClassEv
ClientClass *C_Photograph::GetClientClass()
{
}

// game/client/portal2/c_item_photo.cpp:28 @0x521d30 _ZL26_C_Photograph_CreateObjectii
IClientNetworkable *_C_Photograph_CreateObject( int entnum, int serialNum )
{
	C_Photograph *pRet;  // line 28
	// inlined C_Photograph::C_Photograph() at line 28
}

// game/client/portal2/c_item_photo.cpp:28 @0x74290 _Z15ClientClassInitIN13DT_Photograph7ignoredEEiPT_
int ClientClassInit<DT_Photograph::ignored>( DT_Photograph::ignored * )
{
	char *pRecvTableName;  // line 28
	RecvTable &RecvTable;  // line 28
	RecvProp RecvProps[3];  // line 28
}

// game/client/portal2/c_item_photo.cpp:28
RecvTable g_RecvTable;

// game/client/portal2/c_item_photo.cpp:28
int g_RecvTableInit;

// game/client/portal2/c_item_photo.cpp:28
ClientClass __g_C_PhotographClientClass;

// game/client/portal2/c_item_photo.cpp:36 sizeof=0x8 (i386)
struct CPhotoMaterialProxy : public CEntityMaterialProxy
{
public:
	CPhotoMaterialProxy();  // line 38
	virtual bool Init( IMaterial *, KeyValues * );  // line 39
	virtual void OnBind( C_BaseEntity * );  // line 40
	virtual IMaterial *GetMaterial();  // line 41
private:
	IMaterialVar *m_BaseTextureVar; // +0x4  // line 44
};

// game/client/portal2/c_item_photo.cpp:36 (declaration)
~CPhotoMaterialProxy();

// game/client/portal2/c_item_photo.cpp:36 @0x5222f0 _ZN19CPhotoMaterialProxyD0Ev
CPhotoMaterialProxy::~CPhotoMaterialProxy()
{
	// inlined CEntityMaterialProxy::~CEntityMaterialProxy() at line 36
}

// game/client/portal2/c_item_photo.cpp:36 @0x522320 _ZN19CPhotoMaterialProxyD1Ev
CPhotoMaterialProxy::~CPhotoMaterialProxy()
{
	// inlined CEntityMaterialProxy::~CEntityMaterialProxy() at line 36
}

// game/client/portal2/c_item_photo.cpp:38 @0x521af0 _ZN19CPhotoMaterialProxyC2Ev
CPhotoMaterialProxy::CPhotoMaterialProxy()
{
}

// game/client/portal2/c_item_photo.cpp:38 @0x521b10 _ZN19CPhotoMaterialProxyC1Ev
CPhotoMaterialProxy::CPhotoMaterialProxy()
{
}

// game/client/portal2/c_item_photo.cpp:39 @0x521b30 _ZN19CPhotoMaterialProxy4InitEP9IMaterialP9KeyValues
bool CPhotoMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool foundVar;  // line 54
}

// game/client/portal2/c_item_photo.cpp:40 @0x521de0 _ZN19CPhotoMaterialProxy6OnBindEP12C_BaseEntity
void CPhotoMaterialProxy::OnBind( C_BaseEntity *pC_BaseEntity )
{
	C_Photograph *pPhoto;  // line 61
	ITexture *pTexture;  // line 66
}

// game/client/portal2/c_item_photo.cpp:41 @0x521b70 _ZN19CPhotoMaterialProxy11GetMaterialEv
IMaterial *CPhotoMaterialProxy::GetMaterial()
{
}

// game/client/portal2/c_item_photo.cpp:78 sizeof=0x1 (i386)
struct CPhotoMaterialFoo
{
public:
	CPhotoMaterialFoo();  // line 78
};

// game/client/portal2/c_item_photo.cpp:78 @0x521ed0 _ZL27CCPhotoMaterialProxyFactoryv
IMaterialProxy *CCPhotoMaterialProxyFactory()
{
	// inlined CPhotoMaterialProxy::CPhotoMaterialProxy() at line 78
}

// game/client/portal2/c_item_photo.cpp:84 sizeof=0x20 (i386)
struct CPlacementPhotoMaterialProxy : public CEntityMaterialProxy
{
public:
	CPlacementPhotoMaterialProxy();  // line 86
	virtual void OnBind( C_BaseEntity * );  // line 87
	virtual bool Init( IMaterial *, KeyValues * );  // line 88
	virtual IMaterial *GetMaterial();  // line 89
private:
	float GetAlphaFade();  // line 92
	IMaterial *m_pMaterial; // +0x4  // line 94
	IMaterialVar *m_pAlphaVar; // +0x8  // line 95
	bool m_bLastState; // +0xc  // line 96
	float m_flStartAlpha; // +0x10  // line 97
	float m_flTargetAlpha; // +0x14  // line 98
	float m_flFadeStartTime; // +0x18  // line 99
	float m_flFadeDuration; // +0x1c  // line 100
};

// game/client/portal2/c_item_photo.cpp:84 (declaration)
~CPlacementPhotoMaterialProxy();

// game/client/portal2/c_item_photo.cpp:84 @0x5221e0 _ZN28CPlacementPhotoMaterialProxyD1Ev
CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy()
{
	// inlined CEntityMaterialProxy::~CEntityMaterialProxy() at line 84
}

// game/client/portal2/c_item_photo.cpp:84 @0x522350 _ZN28CPlacementPhotoMaterialProxyD0Ev
CPlacementPhotoMaterialProxy::~CPlacementPhotoMaterialProxy()
{
	// inlined CEntityMaterialProxy::~CEntityMaterialProxy() at line 84
}

// game/client/portal2/c_item_photo.cpp:86 @0x521ba0 _ZN28CPlacementPhotoMaterialProxyC1Ev
CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy()
{
}

// game/client/portal2/c_item_photo.cpp:86 @0x521e90 _ZN28CPlacementPhotoMaterialProxyC2Ev
CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy()
{
}

// game/client/portal2/c_item_photo.cpp:87 @0x521f50 _ZN28CPlacementPhotoMaterialProxy6OnBindEP12C_BaseEntity
void CPlacementPhotoMaterialProxy::OnBind( C_BaseEntity *pC_BaseEntity )
{
	C_Portal_Player *pPlayer;  // line 139
	// inlined CPlacementPhotoMaterialProxy::GetAlphaFade() at line 146
	// inlined CPlacementPhotoMaterialProxy::GetAlphaFade() at line 162
}

// game/client/portal2/c_item_photo.cpp:88 @0x521be0 _ZN28CPlacementPhotoMaterialProxy4InitEP9IMaterialP9KeyValues
bool CPlacementPhotoMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool found;  // line 117
}

// game/client/portal2/c_item_photo.cpp:89 @0x5221d0 _ZN28CPlacementPhotoMaterialProxy11GetMaterialEv
IMaterial *CPlacementPhotoMaterialProxy::GetMaterial()
{
}

// game/client/portal2/c_item_photo.cpp:92 @0x521c70 _ZN28CPlacementPhotoMaterialProxy12GetAlphaFadeEv
float CPlacementPhotoMaterialProxy::GetAlphaFade()
{
	// inlined SimpleSplineRemapValClamped() at line 134
}

// game/client/portal2/c_item_photo.cpp:165 sizeof=0x1 (i386)
struct CPlacementPhotoFoo
{
public:
	CPlacementPhotoFoo();  // line 165
};

// game/client/portal2/c_item_photo.cpp:165 @0x521f00 _ZL36CCPlacementPhotoMaterialProxyFactoryv
IMaterialProxy *CCPlacementPhotoMaterialProxyFactory()
{
	// inlined CPlacementPhotoMaterialProxy::CPlacementPhotoMaterialProxy() at line 165
}
