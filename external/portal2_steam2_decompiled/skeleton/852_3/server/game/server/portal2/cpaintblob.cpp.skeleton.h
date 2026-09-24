// DWARF declaration skeleton for game/server/portal2/cpaintblob.cpp
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xbd860 _Z41__static_initialization_and_destruction_0ii
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

// game/server/portal2/cpaintblob.cpp:15
ConVar sv_paintblob_damage;

// game/server/portal2/cpaintblob.cpp:17
ConVar paintblob_applies_impulse;

// game/server/portal2/cpaintblob.cpp:19 (declaration)
void CPaintBlob();

// game/server/portal2/cpaintblob.cpp:19 @0x6afa70 _ZN10CPaintBlobC1Ev
CPaintBlob::CPaintBlob()
{
}

// game/server/portal2/cpaintblob.cpp:19 @0x6afab0 _ZN10CPaintBlobC2Ev
CPaintBlob::CPaintBlob()
{
}

// game/server/portal2/cpaintblob.cpp:24 (declaration)
~CPaintBlob();

// game/server/portal2/cpaintblob.cpp:24 @0x6afa30 _ZN10CPaintBlobD1Ev
CPaintBlob::~CPaintBlob()
{
}

// game/server/portal2/cpaintblob.cpp:24 @0x6afa50 _ZN10CPaintBlobD2Ev
CPaintBlob::~CPaintBlob()
{
}

// game/server/portal2/cpaintblob.cpp:29 @0x6afaf0 _ZN10CPaintBlob18AddToPaintDatabaseEv
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

// game/server/portal2/cpaintblob.cpp:44 @0x6af9d0 _ZN10CPaintBlob14SetupPaintInfoERK10CGameTrace
void CPaintBlob::SetupPaintInfo( const trace_t &tr )
{
	// inlined Vector::operator=() at line 49
}

// game/server/portal2/cpaintblob.cpp:54 (declaration)
Vector GetPaintBlobDamage( const Vector &vecDir );

// game/server/portal2/cpaintblob.cpp:54 @0x6af900 _Z18GetPaintBlobDamageRK6Vector
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

// game/server/portal2/cpaintblob.cpp:66 @0x6afb70 _ZN10CPaintBlob14PaintBlobPaintERK10CGameTrace
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
