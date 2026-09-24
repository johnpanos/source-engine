// DWARF declaration skeleton for game/client/portal2/c_hitbox_damaged_entity.cpp
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x73970 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1142
	// inlined C_HitBoxDamageInitHelper::C_HitBoxDamageInitHelper() at line 43
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:14 (declaration)
~IClientHitboxDamagedEntity();

// game/client/portal2/c_hitbox_damaged_entity.cpp:14 @0x520af0 _ZN26IClientHitboxDamagedEntityD0Ev
IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity()
{
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:14 @0x520b10 _ZN26IClientHitboxDamagedEntityD1Ev
IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity()
{
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:14 @0x520b30 _ZN26IClientHitboxDamagedEntityD2Ev
IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity()
{
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:18 @0x520b50 _Z33__MsgFunc_ApplyHitBoxDamageEffectR7bf_read
__MsgFunc_ApplyHitBoxDamageEffect( bf_read &msg )
{
	IClientHitboxDamagedEntity *pDamagedEntity;  // line 20
	{
		const int damageEffectIndex;  // line 23
		const int particleEffectIndex;  // line 24
		// inlined CBitRead::ReadByte() at line 23
		// inlined CBitRead::ReadByte() at line 24
	}
	// inlined CBitRead::ReadLong() at line 20
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:30 sizeof=0xc (i386)
struct C_HitBoxDamageInitHelper : public CAutoGameSystem
{
private:
	virtual bool Init();  // line 31
};

// game/client/portal2/c_hitbox_damaged_entity.cpp:30 (declaration)
void C_HitBoxDamageInitHelper();

// game/client/portal2/c_hitbox_damaged_entity.cpp:30 (declaration)
~C_HitBoxDamageInitHelper();

// game/client/portal2/c_hitbox_damaged_entity.cpp:30 @0x5210d0 _ZN24C_HitBoxDamageInitHelperD1Ev
C_HitBoxDamageInitHelper::~C_HitBoxDamageInitHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 30
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:30 @0x5210f0 _ZN24C_HitBoxDamageInitHelperD0Ev
C_HitBoxDamageInitHelper::~C_HitBoxDamageInitHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 30
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:31 @0x520fc0 _ZN24C_HitBoxDamageInitHelper4InitEv
bool C_HitBoxDamageInitHelper::Init()
{
	{
		int i;  // line 33
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 35
		}
	}
}

// game/client/portal2/c_hitbox_damaged_entity.cpp:43
C_HitBoxDamageInitHelper g_HitBoxDamageInitHelper;
