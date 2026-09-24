// DWARF declaration skeleton for game/shared/portal2/hitbox_damaged_entity_shared.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x16110 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/shared/portal2/hitbox_damaged_entity_shared.cpp:26 @0x1b2100 _Z13EmitParticlesP15C_BaseAnimating17DamagedEntityTypeii
EmitParticles( C_BaseAnimating *pOwnerEntity, DamagedEntityType entityType, int damageEffectIndex, int particleEffectIndex )
{
	const DamageInfoVector &damageInfoVector;  // line 28
	const DamageInfo &damageInfo;  // line 32
	// inlined Vector::Vector() at line 46
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 32
	{
		int attachmentIndex;  // line 38
	}
}

// game/shared/portal2/hitbox_damaged_entity_shared.cpp:55 @0x1b22b0 _Z8EmitGibsP15C_BaseAnimating17DamagedEntityTypei
EmitGibs( C_BaseAnimating *pParentEntity, DamagedEntityType entityType, int damageEffectIndex )
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
			C_Gib *pGib;  // line 146
			// inlined CStudioHdr::~CStudioHdr() at line 100
			// inlined Vector::operator-=() at line 125
			// inlined Vector::operator-() at line 128
			// inlined Vector::operator VectorByValue&() at line 128
			// inlined VectorTransform() at line 128
			// inlined C_BaseEntity::GetAbsVelocity() at line 131
			// inlined C_BaseEntity::VPhysicsGetObject() at line 132
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
			// inlined CStudioHdr::~CStudioHdr() at line 100
		}
	}
	{
		matrix3x4_t burstCenterTransform;  // line 93
	}
	// inlined C_BaseEntity::EntityToWorldTransform() at line 86
	// inlined CUtlVector<DamageInfo,CUtlMemory<DamageInfo, int> >::operator[]() at line 67
}
