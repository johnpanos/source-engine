// DWARF declaration skeleton for game/shared/portal/paint_blobs_shared.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_blobs_shared.h:13
enum BlobTraceResult
{
	BLOB_TRACE_HIT_NOTHING = 0,
	BLOB_TRACE_HIT_PORTAL = 1,
	BLOB_TRACE_HIT_WORLD = 2,
	BLOB_TRACE_HIT_PAINT_CLEANSER = 3,
	BLOB_TRACE_HIT_SOMETHING = 4,
	BLOB_TRACE_HIT_PLAYER = 5,
	BLOB_TRACE_HIT_WATER = 6,
	BLOB_TRACE_HIT_TRACTORBEAM = 7,
	BLOB_TRACE_HIT_PROP_PORTAL = 8,
};

// game/shared/portal/paint_blobs_shared.h:27 sizeof=0x64 (i386)
struct BlobCollisionRecord
{
public:
	trace_t trace; // +0x0  // line 28
	Vector targetEndPos; // +0x54  // line 29
	BlobTraceResult traceResultType; // +0x60  // line 30
};

// game/shared/portal/paint_blobs_shared.h:33
enum PaintBlobMoveState
{
	PAINT_BLOB_AIR_MOVE = 0,
	PAINT_BLOB_STREAK_MOVE = 1,
	PAINT_BLOB_TRACTOR_BEAM_MOVE = 2,
};

// game/shared/portal/paint_blobs_shared.h:42 sizeof=0xb8 (i386)
struct CBasePaintBlob
{
public:
	int (**_vptr$CBasePaintBlob)(); // +0x0  // line 0
	CBasePaintBlob();  // line 44
	~CBasePaintBlob();  // line 45
	void Init( const Vector &, const Vector &, int, float, float, bool );  // line 47
	bool IsStreaking() const;  // line 49
	void SetTractorBeam( CTrigger_TractorBeam * );  // line 51
	const Vector &GetPosition() const;  // line 53
	void SetPosition( const Vector & );  // line 54
	const Vector &GetPrevPosition() const;  // line 56
	void SetPrevPosition( const Vector & );  // line 57
	const Vector &GetVelocity() const;  // line 59
	void SetVelocity( const Vector & );  // line 60
	PaintPowerType GetPaintPowerType() const;  // line 62
	PaintBlobMoveState GetMoveState() const;  // line 64
	void SetMoveState( PaintBlobMoveState );  // line 65
	float GetAccumulatedTime() const;  // line 67
	void SetAccumulatedTime( float );  // line 68
	float GetLastUpdateTime() const;  // line 69
	void SetLastUpdateTime( float );  // line 70
	float GetVortexDirection() const;  // line 72
	bool ShouldDeleteThis() const;  // line 74
	void SetDeletionFlag( bool );  // line 75
	float GetLifeTime() const;  // line 77
	void UpdateLifeTime( float );  // line 78
	void UpdateBlobPreCollision( float, Vector &, Vector & );  // line 80
	void UpdateBlobCollision( float, const Vector &, Vector &, bool & );  // line 81
	void UpdateBlobPostCollision( float );  // line 82
	const Vector &GetContactNormal() const;  // line 84
	float GetStreakTime() const;  // line 86
	float GetStreakSpeedDampenRate() const;  // line 87
	void SetRadiusScale( float );  // line 89
	float GetRadiusScale() const;  // line 90
	bool ShouldPlayEffect() const;  // line 92
	bool IsSilent() const;  // line 94
	bool IsGhosting() const;  // line 96
	const Vector &GetGhostPosition() const;  // line 97
protected:
	virtual void PaintBlobCleanse( const trace_t & );  // line 100
	virtual bool PaintBlobStreakPaint( const Vector & );  // line 101
	void PaintBlobMove( float, Vector &, Vector & );  // line 103
	void PaintBlobTractorBeamMove( float, Vector &, Vector & );  // line 104
	void PaintBlobMoveThroughPortal( float, C_Portal_Base2D *, const Vector &, const Vector & );  // line 106
	BlobTraceResult BlobHitSolid( const trace_t & );  // line 108
	int CheckCollision( BlobCollisionRecord *, int, const Vector & );  // line 109
	virtual void ResolveCollision( bool &, const BlobCollisionRecord &, Vector &, float );  // line 110
	bool PaintBlobCheckShouldStreak( const trace_t &, Vector & );  // line 112
	void DecayVortexSpeed( float );  // line 114
	void PlayEffect( const Vector &, const Vector & );  // line 115
	Vector m_vecPosition; // +0x4  // line 117
	Vector m_vecPrevPosition; // +0x10  // line 118
	Vector m_vecVelocity; // +0x1c  // line 119
	Vector m_vContactNormal; // +0x28  // line 122
	PaintPowerType m_paintType; // +0x34  // line 124
	C_BaseEntity *m_pOwner; // +0x38  // line 125
	PaintBlobMoveState m_MoveState; // +0x3c  // line 126
	float m_flLifeTime; // +0x40  // line 129
	Vector m_vecStreakDir; // +0x44  // line 132
	bool m_bStreakDirChanged; // +0x50  // line 133
	float m_flStreakTimer; // +0x54  // line 134
	float m_flStreakSpeedDampenRate; // +0x58  // line 135
	bool m_bInTractorBeam; // +0x5c  // line 138
	Vector m_vBeamStart; // +0x60  // line 139
	Vector m_vBeamEnd; // +0x6c  // line 140
	bool m_bIsBeamReversed; // +0x78  // line 141
	bool m_bIsBeamDirTowardsPortal; // +0x79  // line 142
	float m_flBeamRadius; // +0x7c  // line 143
	float m_flCurrentVortexRadius; // +0x80  // line 145
	float m_flDestVortexRadius; // +0x84  // line 146
	float m_flVortexDirection; // +0x88  // line 147
	float m_flCurrentVortexSpeed; // +0x8c  // line 148
	bool m_bDeleteFlag; // +0x90  // line 150
	float m_flAccumulatedTime; // +0x94  // line 152
	float m_flLastUpdateTime; // +0x98  // line 153
	float m_flRadiusScale; // +0x9c  // line 157
	bool m_bShouldPlayEffect; // +0xa0  // line 159
	CTrigger_TractorBeam *m_pBeam; // +0xa4  // line 162
	bool m_bSilent; // +0xa8  // line 165
	bool m_bGhosting; // +0xa9  // line 168
	Vector m_vecGhostPosition; // +0xac  // line 169
};

// game/shared/portal/paint_blobs_shared.h:195
typedef CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> > PaintBlobVector_t;

// game/shared/portal/paint_blobs_shared.h:202 sizeof=0x18 (i386)
struct BlobTeleportationHistory_t
{
public:
	BlobTeleportationHistory_t();  // line 203
	BlobTeleportationHistory_t( const Vector &, const Vector & );  // line 208
	Vector m_vEnterPosition; // +0x0  // line 214
	Vector m_vExitPosition; // +0xc  // line 215
};

// game/shared/portal/paint_blobs_shared.h:202 (declaration)
void operator=( const BlobTeleportationHistory_t & );

// game/shared/portal/paint_blobs_shared.h:218
typedef CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> > BlobTeleportationHistoryVector_t;

// game/shared/portal/paint_blobs_shared.h:221 sizeof=0x2c (i386)
struct BlobData_t
{
public:
	BlobData_t();  // line 222
	BlobData_t( const BlobData_t & );  // line 230
	~BlobData_t();  // line 238
	int m_blobID; // +0x0  // line 243
	Vector m_vPosition; // +0x4  // line 244
	float m_flScale; // +0x10  // line 245
	bool m_bTeleportedThisFrame; // +0x14  // line 246
	BlobTeleportationHistoryVector_t m_teleportationHistory; // +0x18  // line 247
};

// game/shared/portal/paint_blobs_shared.h:221 (declaration)
void operator=( const BlobData_t & );

// game/shared/portal/paint_blobs_shared.h:238 @0x52afd0 _ZN10BlobData_tD1Ev
BlobData_t::~BlobData_t()
{
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::Purge() at line 240
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::~CUtlVector() at line 241
	// inlined CUtlVector<BlobTeleportationHistory_t,CUtlMemory<BlobTeleportationHistory_t, int> >::~CUtlVector() at line 241
}

// game/shared/portal/paint_blobs_shared.h:250
typedef CUtlVector<BlobData_t,CUtlMemory<BlobData_t, int> > BlobDataVector_t;

// game/shared/portal/paint_blobs_shared.h:253 sizeof=0x18 (i386)
struct BlobDataPerUpdate_t
{
public:
	BlobDataPerUpdate_t();  // line 254
	~BlobDataPerUpdate_t();  // line 259
	float m_flUpdateTime; // +0x0  // line 264
	BlobDataVector_t m_blobData; // +0x4  // line 265
};
