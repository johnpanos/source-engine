// DWARF declaration skeleton for game/server/portal2/rocketprojectile.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xcef00 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSteamID::CSteamID() at line 650
	// inlined CSteamID::CSteamID() at line 654
	// inlined CSteamID::CSteamID() at line 656
	// inlined CSteamID::CSteamID() at line 659
	// inlined CSteamID::CSteamID() at line 662
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined CEntityFactory<CRocket_Turret_Projectile>::CEntityFactory() at line 31
}

// game/server/portal2/rocketprojectile.cpp:23 (declaration)
void GetBaseMap();

// game/server/portal2/rocketprojectile.cpp:23 @0x6f8240 _ZN25CRocket_Turret_Projectile14GetDataDescMapEv
datamap_t *CRocket_Turret_Projectile::GetDataDescMap()
{
}

// game/server/portal2/rocketprojectile.cpp:23 @0x6f8250 _ZN25CRocket_Turret_Projectile10GetBaseMapEv
datamap_t *CRocket_Turret_Projectile::GetBaseMap()
{
}

// game/server/portal2/rocketprojectile.cpp:23 @0xcece0 _Z11DataMapInitI25CRocket_Turret_ProjectileEP9datamap_tPT_
datamap_t *DataMapInit<CRocket_Turret_Projectile>( CRocket_Turret_Projectile * )
{
	// inlined CDatadescGeneratedNameHolder::GenerateName() at line 29
	// inlined CDatadescGeneratedNameHolder::CDatadescGeneratedNameHolder() at line 23
	CDatadescGeneratedNameHolder nameHolder;  // line 23
	typedescription_t dataDesc[4];  // line 23
}

// game/server/portal2/rocketprojectile.cpp:23
datamap_t *g_DataMapHolder;

// game/server/portal2/rocketprojectile.cpp:23
void CRocket_Turret_Projectile::m_DataMap;

// game/server/portal2/rocketprojectile.cpp:31
static CEntityFactory<CRocket_Turret_Projectile> rocket_turret_projectile;

// game/server/portal2/rocketprojectile.cpp:36 @0x6f8420 _ZN25CRocket_Turret_Projectile5SpawnEv
void CRocket_Turret_Projectile::Spawn()
{
	// inlined CBaseCombatCharacter::Spawn() at line 39
	// inlined CBaseEntity::SetSolid() at line 41
	// inlined Vector::Vector() at line 43
	// inlined Vector::operator VectorByValue&() at line 43
	// inlined Vector::operator-() at line 43
	// inlined Vector::operator VectorByValue&() at line 43
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_takedamage>::operator=<int>() at line 48
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iMaxHealth>::operator=<int>() at line 49
	// inlined CNetworkVarBase<int,CBaseEntity::NetworkVar_m_iHealth>::operator=<int>() at line 49
}

// game/server/portal2/rocketprojectile.cpp:64 @0x6f9ac0 _ZN25CRocket_Turret_Projectile12MissileTouchEP11CBaseEntity
void CRocket_Turret_Projectile::MissileTouch( CBaseEntity *pOther )
{
	Vector vVel;  // line 67
	// inlined CHandle<CBaseEntity>::Get() at line 70
	// inlined CBaseEntity::GetAbsVelocity() at line 67
	{
		Vector vBounceVel;  // line 73
		QAngle vBounceAngles;  // line 75
		// inlined Vector::Vector() at line 73
		// inlined Vector::operator*() at line 74
		// inlined Vector::operator VectorByValue&() at line 74
		// inlined QAngle::QAngle() at line 78
		// inlined QAngle::operator QAngleByValue&() at line 78
		// inlined CHandle<RocketTrail>::operator RocketTrail*() at line 79
		// inlined CBaseEntity::SetSolid() at line 81
		// inlined CHandle<RocketTrail>::operator RocketTrail*() at line 83
		// inlined CHandle<RocketTrail>::operator=() at line 86
	}
}

// game/server/util.h:99 @0x6f9df0 _ZN14CEntityFactoryI25CRocket_Turret_ProjectileE6CreateEPKc
IServerNetworkable *CEntityFactory<CRocket_Turret_Projectile>::Create( const char *pClassName )
{
	CRocket_Turret_Projectile *pEnt;  // line 101
	// inlined _CreateEntityTemplate<CRocket_Turret_Projectile>() at line 101
}

// game/server/portal2/rocketprojectile.cpp:101 @0x6f9620 _ZN25CRocket_Turret_Projectile7ExplodeEv
void CRocket_Turret_Projectile::Explode()
{
	Vector forward;  // line 105
	trace_t tr;  // line 109
	// inlined CHandle<RocketTrail>::operator RocketTrail*() at line 130
	{
		CTakeDamageInfo info;  // line 122
		// inlined CTakeDamageInfo::SetDamageForce() at line 125
		// inlined Vector::operator*() at line 125
		// inlined CBaseEntity::Forward() at line 125
		// inlined CTakeDamageInfo::SetDamagePosition() at line 124
		// inlined CBaseEntity::GetAbsOrigin() at line 124
	}
	// inlined FClassnameIs() at line 120
	// inlined CBaseEntity::SetSolid() at line 113
	// inlined CNetworkVarBase<char,CBaseEntity::NetworkVar_m_takedamage>::operator=<int>() at line 112
	// inlined UTIL_TraceLine() at line 110
	// inlined CBaseEntity::GetAbsOrigin() at line 110
	// inlined Vector::operator+() at line 110
	// inlined CBaseEntity::GetAbsOrigin() at line 110
	// inlined Vector::operator*() at line 110
	// inlined CHandle<RocketTrail>::operator=() at line 133
}

// game/server/util.h:105 @0x6f9d50 _ZN14CEntityFactoryI25CRocket_Turret_ProjectileE7DestroyEP18IServerNetworkable
void CEntityFactory<CRocket_Turret_Projectile>::Destroy( IServerNetworkable *pNetworkable )
{
}

// game/server/util.h:113 @0x6f9d80 _ZN14CEntityFactoryI25CRocket_Turret_ProjectileE13GetEntitySizeEv
size_t CEntityFactory<CRocket_Turret_Projectile>::GetEntitySize()
{
}

// game/server/portal2/rocketprojectile.cpp:139 @0x6f82b0 _ZN25CRocket_Turret_Projectile8PrecacheEv
void CRocket_Turret_Projectile::Precache()
{
}

// game/server/portal2/rocketprojectile.cpp:152 @0x6f8380 _ZN25CRocket_Turret_Projectile17NotifySystemEventEP11CBaseEntity21notify_system_event_tRK28notify_system_event_params_t
void CRocket_Turret_Projectile::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	// inlined CHandle<RocketTrail>::operator RocketTrail*() at line 163
	// inlined CHandle<RocketTrail>::operator=() at line 164
}

// game/server/portal2/rocketprojectile.cpp:169 @0x6f8260 _ZN25CRocket_Turret_Projectile11SetLauncherE7CHandleI11CBaseEntityE
void CRocket_Turret_Projectile::SetLauncher( EHANDLE &hLauncher )
{
}

// game/server/portal2/rocketprojectile.cpp:174 @0x6f8760 _ZN25CRocket_Turret_Projectile11DoExplosionEv
void CRocket_Turret_Projectile::DoExplosion()
{
	string_t strTurretName;  // line 180
	string_t strPropName;  // line 181
	string_t strCubeName;  // line 182
	Vector vecRadius;  // line 184
	CBaseEntity *pEnts[128];  // line 185
	int nNumFound;  // line 186
	// inlined UTIL_EntitiesInBox() at line 186
	// inlined Vector::operator VectorByValue&() at line 186
	// inlined Vector::operator VectorByValue&() at line 186
	// inlined Vector::operator-() at line 186
	// inlined CBaseEntity::GetAbsOrigin() at line 186
	// inlined Vector::operator+() at line 186
	// inlined CBaseEntity::GetAbsOrigin() at line 186
	// inlined CBaseEntity::GetAbsAngles() at line 178
	{
		int i;  // line 187
		{
			trace_t tr;  // line 198
			Vector vForward;  // line 205
			Vector vDamagePos;  // line 207
			CTakeDamageInfo info;  // line 209
			// inlined string_t::operator!=() at line 193
			// inlined CBaseEntity::GetAbsOrigin() at line 199
			// inlined UTIL_TraceLine() at line 199
			// inlined Vector::operator*() at line 207
			// inlined CBaseEntity::GetAbsOrigin() at line 207
			// inlined Vector::operator-() at line 207
			// inlined CTakeDamageInfo::SetDamagePosition() at line 210
			// inlined CBaseEntity::GetAbsOrigin() at line 211
			// inlined Vector::operator-() at line 211
			// inlined Vector::operator VectorByValue&() at line 211
		}
	}
	// inlined CBaseEntity::GetOwnerEntity() at line 178
	// inlined CBaseEntity::GetAbsOrigin() at line 178
}

// game/server/portal2/rocketprojectile.cpp:217 @0x6f9490 _ZN25CRocket_Turret_Projectile12CreateSoundsEv
void CRocket_Turret_Projectile::CreateSounds()
{
	{
		CSoundEnvelopeController &controller;  // line 221
		CPASAttenuationFilter filter;  // line 223
		// inlined CPASAttenuationFilter::CPASAttenuationFilter() at line 223
		// inlined CBaseEntity::entindex() at line 225
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 226
		// inlined CPASAttenuationFilter::~CPASAttenuationFilter() at line 226
	}
}

// game/server/portal2/rocketprojectile.cpp:231 @0x6f8280 _ZN25CRocket_Turret_Projectile17StopLoopingSoundsEv
void CRocket_Turret_Projectile::StopLoopingSounds()
{
	CSoundEnvelopeController &controller;  // line 233
}

// game/server/portal2/rocketprojectile.cpp:242 @0x6f8d80 _ZN25CRocket_Turret_Projectile16CreateSmokeTrailEv
void CRocket_Turret_Projectile::CreateSmokeTrail()
{
	// inlined CHandle<RocketTrail>::operator RocketTrail*() at line 244
	// inlined CHandle<RocketTrail>::operator=() at line 248
	// inlined CBaseHandle::operator!=() at line 248
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_Opacity>::operator=<float>() at line 250
	// inlined CHandle<RocketTrail>::operator->() at line 251
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_SpawnRate>::operator=<int>() at line 251
	// inlined CHandle<RocketTrail>::operator->() at line 252
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_ParticleLifetime>::operator=<float>() at line 252
	// inlined CHandle<RocketTrail>::operator->() at line 253
	// inlined CNetworkVectorBase<Vector,RocketTrail::NetworkVar_m_StartColor>::Init() at line 253
	// inlined CHandle<RocketTrail>::operator->() at line 254
	// inlined CNetworkVectorBase<Vector,RocketTrail::NetworkVar_m_EndColor>::Init() at line 254
	// inlined CHandle<RocketTrail>::operator->() at line 255
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_StartSize>::operator=<int>() at line 255
	// inlined CHandle<RocketTrail>::operator->() at line 256
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_EndSize>::operator=<int>() at line 256
	// inlined CHandle<RocketTrail>::operator->() at line 257
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_SpawnRadius>::operator=<int>() at line 257
	// inlined CHandle<RocketTrail>::operator->() at line 258
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_MinSpeed>::operator=<int>() at line 258
	// inlined CHandle<RocketTrail>::operator->() at line 259
	// inlined CNetworkVarBase<float,RocketTrail::NetworkVar_m_MaxSpeed>::operator=<int>() at line 259
	// inlined CHandle<RocketTrail>::operator->() at line 261
	// inlined CHandle<RocketTrail>::operator->() at line 262
}

// game/server/portal2/rocketprojectile.cpp:266 @0x6f8300 _ZN25CRocket_Turret_Projectile14UpdateOnRemoveEv
void CRocket_Turret_Projectile::UpdateOnRemove()
{
	// inlined CHandle<RocketTrail>::operator RocketTrail*() at line 269
}

// game/server/portal2/rocketprojectile.cpp:280 @0x6f85e0 _ZL24fire_rocket_projectile_fv
fire_rocket_projectile_f()
{
	CBasePlayer *pPlayer;  // line 282
	Vector ptEyes;  // line 284
	Vector vForward;  // line 284
	QAngle vLookAng;  // line 285
	CRocket_Turret_Projectile *pRocket;  // line 290
	// inlined Vector::operator=() at line 286
	// inlined QAngle::operator=() at line 288
	// inlined Vector::operator*() at line 303
	// inlined Vector::operator VectorByValue&() at line 303
	// inlined CHandle<CBaseEntity>::CHandle() at line 304
}

// game/server/portal2/rocketprojectile.cpp:307
ConCommand fire_rocket_projectile;
