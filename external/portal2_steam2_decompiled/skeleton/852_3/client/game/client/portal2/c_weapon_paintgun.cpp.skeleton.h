// DWARF declaration skeleton for game/client/portal2/c_weapon_paintgun.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x79e40 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined ClientClass::ClientClass() at line 40
	// inlined PredMapInit<C_WeaponPaintGun>() at line 51
	// inlined Cweapon_paintgunFoo::Cweapon_paintgunFoo() at line 58
}

// game/client/portal2/c_weapon_paintgun.cpp:19
ConVar cl_erase_stream_color;

// game/client/portal2/c_weapon_paintgun.cpp:21
ConVar draw_paint_particle_stream;

// game/client/portal2/c_weapon_paintgun.cpp:27 (declaration)
void ChangePaintColor( PaintPowerType power );

// game/client/portal2/c_weapon_paintgun.cpp:40 @0x53e3d0 _ZN16C_WeaponPaintGun40YouForgotToImplementOrDeclareClientClassEv
int C_WeaponPaintGun::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:40 @0x53e3f0 _ZN16C_WeaponPaintGun14GetClientClassEv
ClientClass *C_WeaponPaintGun::GetClientClass()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:40 @0x540670 _ZL30_C_WeaponPaintGun_CreateObjectii
IClientNetworkable *_C_WeaponPaintGun_CreateObject( int entnum, int serialNum )
{
	C_WeaponPaintGun *pRet;  // line 40
	// inlined C_WeaponPaintGun::C_WeaponPaintGun() at line 40
}

// game/client/portal2/c_weapon_paintgun.cpp:40
ClientClass __g_C_WeaponPaintGunClientClass;

// game/client/portal2/c_weapon_paintgun.cpp:40
void C_WeaponPaintGun::m_pClassRecvTable;

// game/client/portal2/c_weapon_paintgun.cpp:41 @0x79c00 _Z15ClientClassInitIN17DT_WeaponPaintGun7ignoredEEiPT_
int ClientClassInit<DT_WeaponPaintGun::ignored>( DT_WeaponPaintGun::ignored * )
{
	char *pRecvTableName;  // line 41
	RecvTable &RecvTable;  // line 41
	RecvProp RecvProps[6];  // line 41
}

// game/client/portal2/c_weapon_paintgun.cpp:41
RecvTable g_RecvTable;

// game/client/portal2/c_weapon_paintgun.cpp:41
int g_RecvTableInit;

// game/client/portal2/c_weapon_paintgun.cpp:51 @0x53e400 _ZN16C_WeaponPaintGun14GetPredDescMapEv
datamap_t *C_WeaponPaintGun::GetPredDescMap()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:51 (declaration)
datamap_t *PredMapInit<C_WeaponPaintGun>( C_WeaponPaintGun * );

// game/client/portal2/c_weapon_paintgun.cpp:51 @0x53e410 _Z11PredMapInitI16C_WeaponPaintGunEP9datamap_tPT_
datamap_t *PredMapInit<C_WeaponPaintGun>( C_WeaponPaintGun * )
{
}

// game/client/portal2/c_weapon_paintgun.cpp:51
datamap_t *g_PredMapHolder;

// game/client/portal2/c_weapon_paintgun.cpp:51
void C_WeaponPaintGun::m_PredMap;

// game/client/portal2/c_weapon_paintgun.cpp:58 sizeof=0x1 (i386)
struct Cweapon_paintgunFoo
{
public:
	Cweapon_paintgunFoo();  // line 58
};

// game/client/portal2/c_weapon_paintgun.cpp:58 @0x540300 _ZL24CC_WeaponPaintGunFactoryv
C_BaseEntity *CC_WeaponPaintGunFactory()
{
	// inlined C_WeaponPaintGun::C_WeaponPaintGun() at line 58
}

// game/client/portal2/c_weapon_paintgun.cpp:63 (declaration)
void C_WeaponPaintGun();

// game/client/portal2/c_weapon_paintgun.cpp:63 @0x53e590 _ZN16C_WeaponPaintGunC2Ev
C_WeaponPaintGun::C_WeaponPaintGun()
{
	// inlined CUtlReference<CNewParticleEffect>::CUtlReference() at line 72
	// inlined CUtlReference<CNewParticleEffect>::CUtlReference() at line 72
}

// game/client/portal2/c_weapon_paintgun.cpp:63 @0x540230 _ZN16C_WeaponPaintGunC1Ev
C_WeaponPaintGun::C_WeaponPaintGun()
{
	// inlined C_WeaponPaintGun::C_WeaponPaintGun() at line 73
}

// game/client/portal2/c_weapon_paintgun.cpp:84 @0x53e530 _ZN16C_WeaponPaintGun8PrecacheEv
void C_WeaponPaintGun::Precache()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:94 @0x53f410 _ZN16C_WeaponPaintGun5SpawnEv
void C_WeaponPaintGun::Spawn()
{
}

// game/client/portal2/c_weapon_paintgun.cpp:107 @0x53e770 _ZN16C_WeaponPaintGun11ClientThinkEv
void C_WeaponPaintGun::ClientThink()
{
	float flDeltaTime;  // line 109
	C_Portal_Player *pPlayer;  // line 112
	Color color;  // line 134
	// inlined C_BaseEntity::SetRenderColor() at line 138
	// inlined Color::r() at line 138
	// inlined Color::g() at line 138
	// inlined Color::b() at line 138
	// inlined Vector::operator=() at line 131
	// inlined ToPortalPlayer() at line 112
	// inlined Color::Color() at line 136
	// inlined Color::operator=() at line 136
	{
		PaintPowerType blobType;  // line 121
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:144 @0x53ed10 _ZN16C_WeaponPaintGun8SimulateEv
bool C_WeaponPaintGun::Simulate()
{
	bool result;  // line 146
}

// game/client/portal2/c_weapon_paintgun.cpp:158 @0x53e670 _ZN16C_WeaponPaintGun16OnPreDataChangedE16DataUpdateType_t
void C_WeaponPaintGun::OnPreDataChanged( DataUpdateType_t type )
{
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 163
}

// game/client/portal2/c_weapon_paintgun.cpp:167 @0x53ffc0 _ZN16C_WeaponPaintGun13OnDataChangedE16DataUpdateType_t
void C_WeaponPaintGun::OnDataChanged( DataUpdateType_t updateType )
{
	{
		C_BaseCombatCharacter *pCurrentOwner;  // line 206
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 208
	}
	{
		C_Portal_Player *pOwner;  // line 176
		// inlined ToPortalPlayer() at line 176
		// inlined Vector::operator=() at line 180
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:215 @0x53e970 _ZN16C_WeaponPaintGun17ChangeRenderColorEb
void C_WeaponPaintGun::ChangeRenderColor( bool bForce )
{
	C_BasePlayer *pPlayer;  // line 217
	Color color;  // line 218
	// inlined C_BaseEntity::SetRenderColor() at line 231
	{
		C_BaseViewModel *pGunModel;  // line 224
		// inlined Color::b() at line 228
		// inlined Color::g() at line 228
		// inlined Color::r() at line 228
		// inlined C_BaseEntity::SetRenderColor() at line 228
	}
	// inlined ToBasePlayer() at line 217
	// inlined Color::Color() at line 220
	// inlined Color::operator=() at line 220
}

// game/client/portal2/c_weapon_paintgun.cpp:235 @0x53ed40 _ZN16C_WeaponPaintGun15StartHoseEffectEv
void C_WeaponPaintGun::StartHoseEffect()
{
	const char *particleEffectName;  // line 250
	Color paintColor;  // line 252
	C_BasePlayer *pOwner;  // line 254
	// inlined Color::Color() at line 252
	// inlined ToBasePlayer() at line 254
	// inlined Color::operator=() at line 274
	// inlined Vector::Vector() at line 277
	// inlined Vector::operator VectorByValue&() at line 277
	{
		CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 257
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 262
		// inlined Color::operator=() at line 265
	}
	// inlined CUtlReference<CNewParticleEffect>::operator=() at line 271
}

// game/client/portal2/c_weapon_paintgun.cpp:281 @0x53eac0 _ZN16C_WeaponPaintGun14StopHoseEffectEv
void C_WeaponPaintGun::StopHoseEffect()
{
	{
		C_BasePlayer *pOwner;  // line 286
		// inlined ToBasePlayer() at line 286
		{
			CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 289
			// inlined CUtlReference<CNewParticleEffect>::operator=() at line 294
			// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 293
		}
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 300
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:306 @0x53e440 _ZN16C_WeaponPaintGun18GetEffectViewModelEv
C_BaseViewModel *C_WeaponPaintGun::GetEffectViewModel()
{
	C_BaseViewModel *pViewModel;  // line 309
	C_BasePlayer *pOwner;  // line 310
	{
		CSetActiveSplitScreenPlayerGuard g_SSEGuard;  // line 313
	}
	// inlined ToBasePlayer() at line 310
}

// game/client/portal2/c_weapon_paintgun.cpp:321 @0x53f7b0 _ZN16C_WeaponPaintGun15AddViewmodelBobEP15C_BaseViewModelR6VectorR6QAngle
void C_WeaponPaintGun::AddViewmodelBob( C_BaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector forward;  // line 323
	Vector right;  // line 323
	Vector up;  // line 323
	C_Portal_Player *pPortalPlayer;  // line 328
	float rollAngle;  // line 342
	VMatrix rotMatrix;  // line 343
	Vector rotAxis;  // line 344
	float pitchAngle;  // line 351
	float yawAngle;  // line 357
	// inlined ToPortalPlayer() at line 328
	// inlined VectorMA() at line 333
	// inlined operator*() at line 336
	// inlined Vector::operator+=() at line 336
	// inlined VectorMA() at line 339
	// inlined CrossProduct() at line 344
	// inlined Vector::Normalized() at line 344
	// inlined VMatrix::operator*() at line 347
	// inlined Vector::operator=() at line 347
	// inlined VMatrix::operator*() at line 348
	// inlined Vector::operator=() at line 348
	// inlined VMatrix::operator*() at line 349
	// inlined Vector::operator=() at line 349
	// inlined Vector::operator=() at line 352
	// inlined VMatrix::operator*() at line 354
	// inlined Vector::operator=() at line 354
	// inlined VMatrix::operator*() at line 355
	// inlined Vector::operator=() at line 355
	// inlined Vector::operator=() at line 358
	// inlined VMatrix::operator*() at line 360
	// inlined Vector::operator=() at line 360
}

// game/client/portal2/c_weapon_paintgun.cpp:366 @0x53f4a0 _ZN16C_WeaponPaintGun16CalcViewmodelBobEv
float C_WeaponPaintGun::CalcViewmodelBob()
{
	float cycle;  // line 370
	C_BasePlayer *player;  // line 372
	float speed;  // line 384
	float bob_offset;  // line 388
	// inlined RemapVal() at line 388
	// inlined C_BasePlayer::MaxSpeed() at line 386
	// inlined Vector::Length() at line 384
	// inlined ToBasePlayer() at line 372
	float bobtime;  // line 368
	float lastbobtime;  // line 369
}

// game/client/portal2/c_weapon_paintgun.cpp:434 @0x53f120 _ZN16C_WeaponPaintGun13SetPaintPowerE14PaintPowerType
void C_WeaponPaintGun::SetPaintPower( PaintPowerType type )
{
	{
		IGameEvent *event;  // line 441
		{
			C_BasePlayer *pPlayer;  // line 445
			// inlined ToBasePlayer() at line 445
		}
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:457
static const PaintPowerType paintPowersInGunOrder[4];

// game/client/portal2/c_weapon_paintgun.cpp:459 @0x53f220 _ZN16C_WeaponPaintGun15CyclePaintPowerEb
void C_WeaponPaintGun::CyclePaintPower( bool bForward )
{
	int nCurrentPowerIndex;  // line 467
	int nCurrentColor;  // line 468
	int nNextPowerIndex;  // line 480
	int nCounter;  // line 481
	{
		int i;  // line 471
	}
}

// game/client/portal2/c_weapon_paintgun.cpp:501 @0x53f3a0 _ZL9NextPaintv
NextPaint()
{
	C_BasePlayer *pPlayer;  // line 503
	C_WeaponPaintGun *pPaintGun;  // line 507
}

// game/client/portal2/c_weapon_paintgun.cpp:514
static ConCommand nextpaint;

// game/client/portal2/c_weapon_paintgun.cpp:517 @0x53f330 _ZL9PrevPaintv
PrevPaint()
{
	C_BasePlayer *pPlayer;  // line 519
	C_WeaponPaintGun *pPaintGun;  // line 523
}

// game/client/portal2/c_weapon_paintgun.cpp:530
static ConCommand prevpaint;

// game/client/portal2/c_weapon_paintgun.cpp:533 @0x540400 _ZL13ChangePaintToRK8CCommand
ChangePaintTo( const CCommand &args )
{
	// inlined CCommand::operator[]() at line 545
	// inlined CCommand::operator[]() at line 549
	// inlined CCommand::operator[]() at line 553
	// inlined ChangePaintColor() at line 547
	// inlined ChangePaintColor() at line 543
	// inlined ChangePaintColor() at line 555
	// inlined ChangePaintColor() at line 551
}

// game/client/portal2/c_weapon_paintgun.cpp:558
static ConCommand changepaintto;
