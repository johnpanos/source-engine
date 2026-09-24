// DWARF declaration skeleton for game/client/portal2/c_weapon_paintgun.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x7a5c0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 39
	// inlined PredMapInit<C_WeaponPaintGun>() at line 52
	// inlined Cweapon_paintgunFoo::Cweapon_paintgunFoo() at line 59
}

// game/client/portal2/c_weapon_paintgun.cpp:18
ConVar cl_erase_stream_color;

// game/client/portal2/c_weapon_paintgun.cpp:20
ConVar draw_paint_particle_stream;

// game/client/portal2/c_weapon_paintgun.cpp:26 (declaration)
void ChangePaintColor( PaintPowerType power );

// game/client/portal2/c_weapon_paintgun.cpp:39 @0x540c30 _ZN16C_WeaponPaintGun40YouForgotToImplementOrDeclareClientClassEv
int C_WeaponPaintGun::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:39 @0x540c50 _ZN16C_WeaponPaintGun14GetClientClassEv
ClientClass *C_WeaponPaintGun::GetClientClass()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:39 @0x541160 _ZL30_C_WeaponPaintGun_CreateObjectii
IClientNetworkable *_C_WeaponPaintGun_CreateObject( int entnum, int serialNum )
{
	C_WeaponPaintGun *pRet;  // line 39
	// inlined C_WeaponPaintGun::C_WeaponPaintGun() at line 39
}

// game/client/portal2/c_weapon_paintgun.cpp:39
ClientClass __g_C_WeaponPaintGunClientClass;

// game/client/portal2/c_weapon_paintgun.cpp:39
void C_WeaponPaintGun::m_pClassRecvTable;

// game/client/portal2/c_weapon_paintgun.cpp:40 @0x7a2a0 _Z15ClientClassInitIN17DT_WeaponPaintGun7ignoredEEiPT_
int ClientClassInit<DT_WeaponPaintGun::ignored>( DT_WeaponPaintGun::ignored * )
{
	char *pRecvTableName;  // line 40
	RecvTable &RecvTable;  // line 40
	RecvProp RecvProps[8];  // line 40
}

// game/client/portal2/c_weapon_paintgun.cpp:40
RecvTable g_RecvTable;

// game/client/portal2/c_weapon_paintgun.cpp:40
int g_RecvTableInit;

// game/client/portal2/c_weapon_paintgun.cpp:52 @0x540c60 _ZN16C_WeaponPaintGun14GetPredDescMapEv
datamap_t *C_WeaponPaintGun::GetPredDescMap()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:52 (declaration)
datamap_t *PredMapInit<C_WeaponPaintGun>( C_WeaponPaintGun * );

// game/client/portal2/c_weapon_paintgun.cpp:52 @0x540c70 _Z11PredMapInitI16C_WeaponPaintGunEP9datamap_tPT_
datamap_t *PredMapInit<C_WeaponPaintGun>( C_WeaponPaintGun * )
{
}

// game/client/portal2/c_weapon_paintgun.cpp:52
datamap_t *g_PredMapHolder;

// game/client/portal2/c_weapon_paintgun.cpp:52
void C_WeaponPaintGun::m_PredMap;

// game/client/portal2/c_weapon_paintgun.cpp:59 sizeof=0x1 (i386)
struct Cweapon_paintgunFoo
{
public:
	Cweapon_paintgunFoo();  // line 59
};

// game/client/portal2/c_weapon_paintgun.cpp:59 @0x541120 _ZL24CC_WeaponPaintGunFactoryv
C_BaseEntity *CC_WeaponPaintGunFactory()
{
	// inlined C_WeaponPaintGun::C_WeaponPaintGun() at line 59
}

// game/client/portal2/c_weapon_paintgun.cpp:64 (declaration)
void C_WeaponPaintGun();

// game/client/portal2/c_weapon_paintgun.cpp:64 @0x540d00 _ZN16C_WeaponPaintGunC2Ev
C_WeaponPaintGun::C_WeaponPaintGun()
{
	// inlined CUtlReference<CNewParticleEffect>::CUtlReference() at line 73
	// inlined C_WeaponPaintGun::NetworkVar_m_hPaintStream::NetworkVar_m_hPaintStream() at line 73
	// inlined CUtlReference<CNewParticleEffect>::CUtlReference() at line 73
}

// game/client/portal2/c_weapon_paintgun.cpp:64 @0x540e00 _ZN16C_WeaponPaintGunC1Ev
C_WeaponPaintGun::C_WeaponPaintGun()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:85 @0x540ca0 _ZN16C_WeaponPaintGun8PrecacheEv
void C_WeaponPaintGun::Precache()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:95 @0x5414b0 _ZN16C_WeaponPaintGun5SpawnEv
void C_WeaponPaintGun::Spawn()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:108 @0x540f20 _ZN16C_WeaponPaintGun11ClientThinkEv
void C_WeaponPaintGun::ClientThink()
{
	float flDeltaTime;  // line 110
	C_Portal_Player *pPlayer;  // line 113
	Color color;  // line 135
	// inlined C_BaseEntity::SetRenderColor() at line 139
	// inlined Color::r() at line 139
	// inlined Color::g() at line 139
	// inlined Color::b() at line 139
	// inlined Vector::operator=() at line 132
	// inlined ToPortalPlayer() at line 113
	// inlined Color::Color() at line 137
	// inlined Color::operator=() at line 137
	{
		PaintPowerType blobType;  // line 122
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:145 @0x541ce0 _ZN16C_WeaponPaintGun8SimulateEv
bool C_WeaponPaintGun::Simulate()
{
	bool result;  // line 147
}

// game/client/portal2/c_weapon_paintgun.cpp:159 @0x540e20 _ZN16C_WeaponPaintGun16OnPreDataChangedE16DataUpdateType_t
void C_WeaponPaintGun::OnPreDataChanged( DataUpdateType_t type )
{
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 164
}

// game/client/portal2/c_weapon_paintgun.cpp:168 @0x542900 _ZN16C_WeaponPaintGun13OnDataChangedE16DataUpdateType_t
void C_WeaponPaintGun::OnDataChanged( DataUpdateType_t updateType )
{
	{
		C_BaseCombatCharacter *pCurrentOwner;  // line 207
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 209
	}
	{
		C_Portal_Player *pOwner;  // line 177
		// inlined ToPortalPlayer() at line 177
		// inlined Vector::operator=() at line 181
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:216 @0x541940 _ZN16C_WeaponPaintGun17ChangeRenderColorEb
void C_WeaponPaintGun::ChangeRenderColor( bool bForce )
{
	C_BasePlayer *pPlayer;  // line 218
	Color color;  // line 219
	// inlined C_BaseEntity::SetRenderColor() at line 232
	{
		C_BaseViewModel *pGunModel;  // line 225
		// inlined Color::b() at line 229
		// inlined Color::g() at line 229
		// inlined Color::r() at line 229
		// inlined C_BaseEntity::SetRenderColor() at line 229
	}
	// inlined ToBasePlayer() at line 218
	// inlined Color::Color() at line 221
	// inlined Color::operator=() at line 221
}

// game/client/portal2/c_weapon_paintgun.cpp:236 @0x541d10 _ZN16C_WeaponPaintGun15StartHoseEffectEv
void C_WeaponPaintGun::StartHoseEffect()
{
	const char *particleEffectName;  // line 251
	Color paintColor;  // line 253
	C_BasePlayer *pOwner;  // line 255
	// inlined Color::Color() at line 253
	// inlined ToBasePlayer() at line 255
	// inlined Color::operator=() at line 275
	// inlined Vector::Vector() at line 278
	// inlined Vector::operator VectorByValue&() at line 278
	{
		CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 258
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 263
		// inlined Color::operator=() at line 266
	}
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 272
}

// game/client/portal2/c_weapon_paintgun.cpp:282 @0x541a90 _ZN16C_WeaponPaintGun14StopHoseEffectEv
void C_WeaponPaintGun::StopHoseEffect()
{
	{
		C_BasePlayer *pOwner;  // line 287
		// inlined ToBasePlayer() at line 287
		{
			CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 290
			// inlined CUtlReference<CNewParticleEffect>::operator=() at line 295
			// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 294
		}
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 301
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:307 @0x541850 _ZN16C_WeaponPaintGun18GetEffectViewModelEv
C_BaseViewModel *C_WeaponPaintGun::GetEffectViewModel()
{
	C_BaseViewModel *pViewModel;  // line 310
	C_BasePlayer *pOwner;  // line 311
	{
		CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 314
	}
	// inlined ToBasePlayer() at line 311
}

// game/client/portal2/c_weapon_paintgun.cpp:322 @0x5420f0 _ZN16C_WeaponPaintGun15AddViewmodelBobEP15C_BaseViewModelR6VectorR6QAngle
void C_WeaponPaintGun::AddViewmodelBob( C_BaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector forward;  // line 324
	Vector right;  // line 324
	Vector up;  // line 324
	C_Portal_Player *pPortalPlayer;  // line 329
	float rollAngle;  // line 343
	VMatrix rotMatrix;  // line 344
	Vector rotAxis;  // line 345
	float pitchAngle;  // line 352
	float yawAngle;  // line 358
	// inlined ToPortalPlayer() at line 329
	// inlined VectorMA() at line 334
	// inlined operator*() at line 337
	// inlined Vector::operator+=() at line 337
	// inlined VectorMA() at line 340
	// inlined CrossProduct() at line 345
	// inlined Vector::Normalized() at line 345
	// inlined VMatrix::operator*() at line 348
	// inlined Vector::operator=() at line 348
	// inlined VMatrix::operator*() at line 349
	// inlined Vector::operator=() at line 349
	// inlined VMatrix::operator*() at line 350
	// inlined Vector::operator=() at line 350
	// inlined Vector::operator=() at line 353
	// inlined VMatrix::operator*() at line 355
	// inlined Vector::operator=() at line 355
	// inlined VMatrix::operator*() at line 356
	// inlined Vector::operator=() at line 356
	// inlined Vector::operator=() at line 359
	// inlined VMatrix::operator*() at line 361
	// inlined Vector::operator=() at line 361
}

// game/client/portal2/c_weapon_paintgun.cpp:367 @0x541540 _ZN16C_WeaponPaintGun16CalcViewmodelBobEv
float C_WeaponPaintGun::CalcViewmodelBob()
{
	float cycle;  // line 371
	C_BasePlayer *player;  // line 373
	float speed;  // line 385
	float bob_offset;  // line 389
	// inlined RemapVal() at line 389
	// inlined C_BasePlayer::MaxSpeed() at line 387
	// inlined Vector::Length() at line 385
	// inlined ToBasePlayer() at line 373
	float bobtime;  // line 369
	float lastbobtime;  // line 370
}

// game/client/portal2/c_weapon_paintgun.cpp:435 @0x5411c0 _ZN16C_WeaponPaintGun13SetPaintPowerE14PaintPowerType
void C_WeaponPaintGun::SetPaintPower( PaintPowerType type )
{
	{
		IGameEvent *event;  // line 442
		{
			C_BasePlayer *pPlayer;  // line 446
			// inlined ToBasePlayer() at line 446
		}
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:458
static const PaintPowerType paintPowersInGunOrder[4];

// game/client/portal2/c_weapon_paintgun.cpp:460 @0x5412c0 _ZN16C_WeaponPaintGun15CyclePaintPowerEb
void C_WeaponPaintGun::CyclePaintPower( bool bForward )
{
	int nCurrentPowerIndex;  // line 468
	int nCurrentColor;  // line 469
	int nNextPowerIndex;  // line 481
	int nCounter;  // line 482
	{
		int i;  // line 472
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:502 @0x541440 _ZL9NextPaintv
NextPaint()
{
	C_BasePlayer *pPlayer;  // line 504
	C_WeaponPaintGun *pPaintGun;  // line 508
}

// game/client/portal2/c_weapon_paintgun.cpp:515
static ConCommand nextpaint;

// game/client/portal2/c_weapon_paintgun.cpp:518 @0x5413d0 _ZL9PrevPaintv
PrevPaint()
{
	C_BasePlayer *pPlayer;  // line 520
	C_WeaponPaintGun *pPaintGun;  // line 524
}

// game/client/portal2/c_weapon_paintgun.cpp:531
static ConCommand prevpaint;

// game/client/portal2/c_weapon_paintgun.cpp:534 @0x542b70 _ZL13ChangePaintToRK8CCommand
ChangePaintTo( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 546
	// inlined CCommand::operator[]() at line 550
	// inlined CCommand::operator[]() at line 554
	// inlined ChangePaintColor() at line 548
	// inlined ChangePaintColor() at line 544
	// inlined ChangePaintColor() at line 556
	// inlined ChangePaintColor() at line 552
}

// game/client/portal2/c_weapon_paintgun.cpp:559
static ConCommand changepaintto;
