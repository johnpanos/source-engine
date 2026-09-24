// DWARF declaration skeleton for game/server/portal2/cpaintblob.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xa84a0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/server/portal2/cpaintblob.cpp:15
ConVar sv_paintblob_damage;

// game/server/portal2/cpaintblob.cpp:17
ConVar paintblob_applies_impulse;

// game/server/portal2/cpaintblob.cpp:19 (declaration)
void CPaintBlob();

// game/server/portal2/cpaintblob.cpp:19 @0x7086d0 _ZN10CPaintBlobC2Ev
CPaintBlob::CPaintBlob()
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::CUtlVector() at line 19
}

// game/server/portal2/cpaintblob.cpp:19 @0x708b10 _ZN10CPaintBlobC1Ev
CPaintBlob::CPaintBlob()
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::CUtlVector() at line 19
}

// game/server/portal2/cpaintblob.cpp:24 (declaration)
~CPaintBlob();

// game/server/portal2/cpaintblob.cpp:24 @0x708a00 _ZN10CPaintBlobD2Ev
CPaintBlob::~CPaintBlob()
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::~CUtlVector() at line 26
}

// game/server/portal2/cpaintblob.cpp:24 @0x708e30 _ZN10CPaintBlobD1Ev
CPaintBlob::~CPaintBlob()
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::~CUtlVector() at line 26
}

// game/server/portal2/cpaintblob.cpp:29 @0x708750 _ZN10CPaintBlob18AddToPaintDatabaseEv
void CPaintBlob::AddToPaintDatabase()
{
	{
		trace_t tr;  // line 33
		// inlined Vector::operator=() at line 34
		// inlined Vector::operator=() at line 35
	}
}

// game/server/portal2/cpaintblob.cpp:44 (declaration)
void SetupPaintInfo( const trace_t &tr );

// game/server/portal2/cpaintblob.cpp:44 @0x708670 _ZN10CPaintBlob14SetupPaintInfoERK10CGameTrace
void CPaintBlob::SetupPaintInfo( const trace_t &tr )
{
	// inlined Vector::operator=() at line 49
}

// game/server/portal2/cpaintblob.cpp:54 (declaration)
Vector GetPaintBlobDamage( const Vector &vecDir );

// game/server/portal2/cpaintblob.cpp:54 @0x7085a0 _Z18GetPaintBlobDamageRK6Vector
Vector GetPaintBlobDamage( const Vector &vecDir )
{
	Vector vecForce;  // line 57
	float flForceScale;  // line 56
	Vector vecForce;  // line 57
	// inlined ConVar::GetFloat() at line 56
	// inlined Vector::operator*=() at line 59
	// inlined ConVar::GetFloat() at line 60
	// inlined Vector::operator*=() at line 60
}

// game/server/portal2/cpaintblob.cpp:66 @0x708b90 _ZN10CPaintBlob14PaintBlobPaintERK10CGameTrace
void CPaintBlob::PaintBlobPaint( const trace_t &tr )
{
	Vector vecTouchPos;  // line 68
	Vector vecNormal;  // line 69
	{
		CBaseEntity *pOther;  // line 78
		{
			IPhysicsObject *pPhysics;  // line 83
			{
				Vector vVel;  // line 86
				Vector aVel;  // line 87
				CRagdollProp *pRagdoll;  // line 90
				{
					ragdoll_t *pRagdollPhys;  // line 97
					{
						int j;  // line 98
					}
				}
				// inlined Vector::operator/() at line 86
				// inlined GetPaintBlobDamage() at line 86
				// inlined Vector::Normalized() at line 86
			}
			// inlined CBaseEntity::VPhysicsGetObject() at line 83
		}
	}
	// inlined CPaintBlob::SetupPaintInfo() at line 74
}

// game/server/portal2/cpaintblob.cpp:109 @0x7087d0 _ZN10CPaintBlob27AddBlobTeleportationHistoryERK26BlobTeleportationHistory_t
void CPaintBlob::AddBlobTeleportationHistory( const BlobTeleportationHistory_t &history )
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::AddToTail() at line 111
}

// game/server/portal2/cpaintblob.cpp:115 @0x708590 _ZN10CPaintBlob25ClearTeleportationHistoryEv
void CPaintBlob::ClearTeleportationHistory()
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::RemoveAll() at line 117
}

// game/server/portal2/cpaintblob.cpp:121 @0x7088a0 _ZN10CPaintBlob23GetTeleportationHistoryER10CUtlVectorI26BlobTeleportationHistory_t10CUtlMemoryIS1_iEE
void CPaintBlob::GetTeleportationHistory( BlobTeleportationHistoryVector_t &historyList )
{
	int nHistoryCount;  // line 123
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::Count() at line 123
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::Base() at line 126
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::AddMultipleToTail() at line 126
}

// game/server/portal2/cpaintblob.cpp:130 @0x708560 _ZN10CPaintBlob26SetBlobTeleportedThisFrameEb
void CPaintBlob::SetBlobTeleportedThisFrame( bool bTeleported )
{
}

// game/server/portal2/cpaintblob.cpp:135 @0x708580 _ZN10CPaintBlob26HasBlobTeleportedThisFrameEv
bool CPaintBlob::HasBlobTeleportedThisFrame()
{
}

// public/tier1/utlmemory.h:707 @0x708f40 _ZN10CUtlMemoryI26BlobTeleportationHistory_tiE4GrowEi
void CUtlMemory<BlobTeleportationHistory_t,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<BlobTeleportationHistory_t,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
