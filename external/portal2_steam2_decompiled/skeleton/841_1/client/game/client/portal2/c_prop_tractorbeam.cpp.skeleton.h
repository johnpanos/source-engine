// DWARF declaration skeleton for game/client/portal2/c_prop_tractorbeam.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x79500 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined ClientClass::ClientClass() at line 144
	// inlined PredMapInit<C_PropTractorBeamProjector>() at line 151
}

// game/client/portal2/c_prop_tractorbeam.cpp:17 sizeof=0xddc (i386)
struct C_PropTractorBeamProjector : public C_BaseAnimating
{
public:
	static typedescription_t m_PredDesc[];  // line 21
	virtual void Spawn();  // line 23
	virtual void ClientThink();  // line 24
	virtual void UpdateOnRemove();  // line 25
	static void RecvProxy_LinearForce( const CRecvProxyData *, void *, void * );  // line 27
	static void RecvProxy_Enabled( const CRecvProxyData *, void *, void * );  // line 28
	float CalculateRotationPose();  // line 30
	float CalculateArmaturePose();  // line 31
	virtual void OnToolStartRecording();  // line 33
protected:
	void CreateEffect();  // line 37
	void UpdateEffect();  // line 38
	void StopEffect();  // line 39
	float m_flLinearForce; // +0xd84  // line 41
	Vector m_vEndPos; // +0xd88  // line 42
	bool m_bEffectsActive; // +0xd94  // line 43
	bool m_bActivated; // +0xd95  // line 44
	bool m_bEnabled; // +0xd96  // line 45
	float m_flArmatureStart; // +0xd98  // line 48
	float m_flArmatureTarget; // +0xd9c  // line 49
	float m_flArmatureDuration; // +0xda0  // line 50
	float m_flArmatureStartTime; // +0xda4  // line 51
	float m_flRotationStart; // +0xda8  // line 54
	float m_flRotationTarget; // +0xdac  // line 55
	float m_flRotationDuration; // +0xdb0  // line 56
	float m_flRotationStartTime; // +0xdb4  // line 57
	CUtlReference<CNewParticleEffect> m_hEmitterEffect[3]; // +0xdb8  // line 59
};

// game/client/portal2/c_prop_tractorbeam.cpp:17 (declaration)
~C_PropTractorBeamProjector();

// game/client/portal2/c_prop_tractorbeam.cpp:17 @0x53c6a0 _ZN26C_PropTractorBeamProjectorD0Ev
C_PropTractorBeamProjector::~C_PropTractorBeamProjector()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 17
}

// game/client/portal2/c_prop_tractorbeam.cpp:17 (declaration)
void C_PropTractorBeamProjector();

// game/client/portal2/c_prop_tractorbeam.cpp:17 @0x53c780 _ZN26C_PropTractorBeamProjectorD1Ev
C_PropTractorBeamProjector::~C_PropTractorBeamProjector()
{
	// inlined CUtlReference<CNewParticleEffect>::~CUtlReference() at line 17
}

// game/client/portal2/c_prop_tractorbeam.cpp:20 @0x53a8c0 _ZN26C_PropTractorBeamProjector40YouForgotToImplementOrDeclareClientClassEv
int C_PropTractorBeamProjector::YouForgotToImplementOrDeclareClientClass()
{
}

// game/client/portal2/c_prop_tractorbeam.cpp:20 @0x53a8e0 _ZN26C_PropTractorBeamProjector14GetClientClassEv
ClientClass *C_PropTractorBeamProjector::GetClientClass()
{
}

// game/client/portal2/c_prop_tractorbeam.cpp:21 @0x53a8f0 _ZN26C_PropTractorBeamProjector14GetPredDescMapEv
datamap_t *C_PropTractorBeamProjector::GetPredDescMap()
{
}

// game/client/portal2/c_prop_tractorbeam.cpp:23 @0x53ad60 _ZN26C_PropTractorBeamProjector5SpawnEv
void C_PropTractorBeamProjector::Spawn()
{
}

// game/client/portal2/c_prop_tractorbeam.cpp:24 @0x53ba80 _ZN26C_PropTractorBeamProjector11ClientThinkEv
void C_PropTractorBeamProjector::ClientThink()
{
	float flArmPose;  // line 291
	int nPoseParam;  // line 292
	float flAnimSpeed;  // line 296
	// inlined C_BaseAnimating::SetPlaybackRate() at line 297
	// inlined C_PropTractorBeamProjector::CalculateRotationPose() at line 296
	// inlined C_BaseAnimating::SetPoseParameter() at line 293
	// inlined C_BaseAnimating::LookupPoseParameter() at line 292
	// inlined C_PropTractorBeamProjector::CalculateArmaturePose() at line 291
}

// game/client/portal2/c_prop_tractorbeam.cpp:25 @0x53bdb0 _ZN26C_PropTractorBeamProjector14UpdateOnRemoveEv
void C_PropTractorBeamProjector::UpdateOnRemove()
{
	// inlined C_PropTractorBeamProjector::StopEffect() at line 174
}

// game/client/portal2/c_prop_tractorbeam.cpp:27 @0x53b1a0 _ZN26C_PropTractorBeamProjector21RecvProxy_LinearForceEPK14CRecvProxyDataPvS3_
void C_PropTractorBeamProjector::RecvProxy_LinearForce( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PropTractorBeamProjector *pBeam;  // line 65
	float flLinearForce;  // line 69
	// inlined C_PropTractorBeamProjector::CalculateRotationPose() at line 75
	// inlined C_PropTractorBeamProjector::CalculateArmaturePose() at line 81
}

// game/client/portal2/c_prop_tractorbeam.cpp:28 @0x53bfd0 _ZN26C_PropTractorBeamProjector17RecvProxy_EnabledEPK14CRecvProxyDataPvS3_
void C_PropTractorBeamProjector::RecvProxy_Enabled( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_PropTractorBeamProjector *pBeam;  // line 93
	bool bEnabled;  // line 97
	// inlined C_PropTractorBeamProjector::CalculateArmaturePose() at line 112
	// inlined C_PropTractorBeamProjector::CalculateRotationPose() at line 106
	// inlined C_PropTractorBeamProjector::CalculateRotationPose() at line 126
	// inlined C_PropTractorBeamProjector::CalculateArmaturePose() at line 132
	// inlined C_PropTractorBeamProjector::StopEffect() at line 138
}

// game/client/portal2/c_prop_tractorbeam.cpp:30 @0x53a930 _ZN26C_PropTractorBeamProjector21CalculateRotationPoseEv
float C_PropTractorBeamProjector::CalculateRotationPose()
{
	float flRotationPose;  // line 312
	float flForcePerc;  // line 317
	bool bNegativeForce;  // line 318
	// inlined SimpleSplineRemapVal() at line 314
}

// game/client/portal2/c_prop_tractorbeam.cpp:31 @0x53adb0 _ZN26C_PropTractorBeamProjector21CalculateArmaturePoseEv
float C_PropTractorBeamProjector::CalculateArmaturePose()
{
	float flArmPose;  // line 347
	// inlined SimpleSplineRemapVal() at line 349
}

// game/client/portal2/c_prop_tractorbeam.cpp:33 @0x53ba40 _ZN26C_PropTractorBeamProjector20OnToolStartRecordingEv
void C_PropTractorBeamProjector::OnToolStartRecording()
{
}

// game/client/portal2/c_prop_tractorbeam.cpp:37 @0x53b410 _ZN26C_PropTractorBeamProjector12CreateEffectEv
void C_PropTractorBeamProjector::CreateEffect()
{
	char *pAttachmentNames[3];  // line 185
	{
		int i;  // line 187
		{
			Vector color;  // line 202
			Vector vVelocity;  // line 219
			// inlined Vector::operator*() at line 219
			// inlined C_BaseEntity::Forward() at line 219
		}
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 196
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 192
		// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 189
	}
	// inlined C_PropTractorBeamProjector::StopEffect() at line 183
}

// game/client/portal2/c_prop_tractorbeam.cpp:38 @0x53aea0 _ZN26C_PropTractorBeamProjector12UpdateEffectEv
void C_PropTractorBeamProjector::UpdateEffect()
{
	{
		int i;  // line 233
		{
			Vector color;  // line 237
			Vector vVelocity;  // line 254
			// inlined Vector::operator*() at line 254
			// inlined C_BaseEntity::Forward() at line 254
		}
	}
}

// game/client/portal2/c_prop_tractorbeam.cpp:39 @0x53ab40 _ZN26C_PropTractorBeamProjector10StopEffectEv
void C_PropTractorBeamProjector::StopEffect()
{
	{
		int i;  // line 265
		// inlined CUtlReference<CNewParticleEffect>::operator=() at line 270
		// inlined CUtlReference<CNewParticleEffect>::operator CNewParticleEffect*() at line 267
	}
}

// game/client/portal2/c_prop_tractorbeam.cpp:144 @0x53aa50 _ZL40_C_PropTractorBeamProjector_CreateObjectii
IClientNetworkable *_C_PropTractorBeamProjector_CreateObject( int entnum, int serialNum )
{
	C_PropTractorBeamProjector *pRet;  // line 144
	// inlined C_PropTractorBeamProjector::C_PropTractorBeamProjector() at line 144
}

// game/client/portal2/c_prop_tractorbeam.cpp:144 @0x79330 _Z15ClientClassInitIN27DT_PropTractorBeamProjector7ignoredEEiPT_
int ClientClassInit<DT_PropTractorBeamProjector::ignored>( DT_PropTractorBeamProjector::ignored * )
{
	char *pRecvTableName;  // line 144
	RecvTable &RecvTable;  // line 144
	RecvProp RecvProps[5];  // line 144
}

// game/client/portal2/c_prop_tractorbeam.cpp:144
RecvTable g_RecvTable;

// game/client/portal2/c_prop_tractorbeam.cpp:144
int g_RecvTableInit;

// game/client/portal2/c_prop_tractorbeam.cpp:144
ClientClass __g_C_PropTractorBeamProjectorClientClass;

// game/client/portal2/c_prop_tractorbeam.cpp:151 (declaration)
datamap_t *PredMapInit<C_PropTractorBeamProjector>( C_PropTractorBeamProjector * );

// game/client/portal2/c_prop_tractorbeam.cpp:151 @0x53a900 _Z11PredMapInitI26C_PropTractorBeamProjectorEP9datamap_tPT_
datamap_t *PredMapInit<C_PropTractorBeamProjector>( C_PropTractorBeamProjector * )
{
}

// game/client/portal2/c_prop_tractorbeam.cpp:151
datamap_t *g_PredMapHolder;
