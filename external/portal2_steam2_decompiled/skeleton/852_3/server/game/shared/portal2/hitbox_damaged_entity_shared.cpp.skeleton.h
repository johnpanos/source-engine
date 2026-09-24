// DWARF declaration skeleton for game/shared/portal2/hitbox_damaged_entity_shared.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x1e5e0 _Z41__static_initialization_and_destruction_0ii
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
}

// game/shared/portal2/hitbox_damaged_entity_shared.cpp:26 @0x2446b0 _Z13EmitParticlesP14CBaseAnimating17DamagedEntityTypeii
EmitParticles( CBaseAnimating *pOwnerEntity, DamagedEntityType entityType, int damageEffectIndex, int particleEffectIndex )
{
	const DamageInfoVector &damageInfoVector;  // line 28
	const DamageInfo &damageInfo;  // line 32
	// inlined Vector::Vector() at line 46
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 32
	{
		int attachmentIndex;  // line 38
	}
}

// game/shared/portal2/hitbox_damaged_entity_shared.cpp:55 @0x244860 _Z8EmitGibsP14CBaseAnimating17DamagedEntityTypei
EmitGibs( CBaseAnimating *pParentEntity, DamagedEntityType entityType, int damageEffectIndex )
{
	const DamageInfoVector &damageInfoVector;  // line 62
	const DamageInfo &damageInfo;  // line 67
	matrix3x4_t localToWorld;  // line 72
	Vector parentOrigin;  // line 73
	int parentAttachment;  // line 74
	const matrix3x4_t &parentTransform;  // line 86
	Vector burstCenter;  // line 87
	const int parentBurstCenterAttachment;  // line 88
	{
		int gibIndex;  // line 100
		{
			const char *gibModelName;  // line 102
			const int gibModelIndex;  // line 103
			CStudioHdr gibStudioHdr;  // line 110
			const model_t *gibModel;  // line 111
			Vector position;  // line 117
			Vector placementOrigin;  // line 118
			QAngle angles;  // line 119
			const int placementIndex;  // line 120
			Vector velocity;  // line 131
			IPhysicsObject *pPhysics;  // line 132
			CGib *pGib;  // line 139
			{
				Vector separationDirection;  // line 188
				// inlined Vector::operator VectorByValue&() at line 195
				// inlined operator*() at line 195
				// inlined Vector::NormalizeInPlace() at line 194
				// inlined Vector::operator==() at line 191
				// inlined Vector::operator-() at line 188
				// inlined Vector::operator-() at line 192
				// inlined Vector::operator=() at line 192
			}
			// inlined CNetworkVarBase<int,CBaseAnimating::NetworkVar_m_nBody>::operator=<int>() at line 141
			// inlined _CreateEntity<CGib>() at line 139
			// inlined CBaseEntity::GetAbsAngles() at line 119
			// inlined Vector::operator-=() at line 125
			// inlined Vector::operator-() at line 128
			// inlined Vector::operator VectorByValue&() at line 128
			// inlined VectorTransform() at line 128
			// inlined CBaseEntity::GetAbsVelocity() at line 131
			// inlined CBaseEntity::VPhysicsGetObject() at line 132
		}
	}
	{
		matrix3x4_t burstCenterTransform;  // line 93
	}
	// inlined CBaseEntity::EntityToWorldTransform() at line 86
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 67
	// inlined CBaseEntity::GetAbsOrigin() at line 87
}
