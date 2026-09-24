// DWARF declaration skeleton for game/shared/portal/paint_blobs_shared.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
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
};

// game/shared/portal/paint_blobs_shared.h:26 sizeof=0x64 (i386)
struct BlobCollisionRecord
{
public:
	trace_t trace; // +0x0  // line 27
	Vector targetEndPos; // +0x54  // line 28
	BlobTraceResult traceResultType; // +0x60  // line 29
};

// game/shared/portal/paint_blobs_shared.h:32
enum PaintBlobMoveState
{
	PAINT_BLOB_AIR_MOVE = 0,
	PAINT_BLOB_STREAK_MOVE = 1,
	PAINT_BLOB_TRACTOR_BEAM_MOVE = 2,
};

// game/shared/portal/paint_blobs_shared.h:41 sizeof=0xac (i386)
struct CBasePaintBlob
{
public:
	int (**_vptr$CBasePaintBlob)(); // +0x0  // line 0
	CBasePaintBlob();  // line 43
	~CBasePaintBlob();  // line 44
	void Init( const Vector &, const Vector &, int, float, float, bool );  // line 46
	bool IsStreaking() const;  // line 48
	void SetTractorBeam( CTrigger_TractorBeam * );  // line 50
	const Vector &GetPosition() const;  // line 52
	void SetPosition( const Vector & );  // line 53
	const Vector &GetPrevPosition() const;  // line 55
	void SetPrevPosition( const Vector & );  // line 56
	const Vector &GetVelocity() const;  // line 58
	void SetVelocity( const Vector & );  // line 59
	PaintPowerType GetPaintPowerType() const;  // line 61
	PaintBlobMoveState GetMoveState() const;  // line 63
	void SetMoveState( PaintBlobMoveState );  // line 64
	float GetAccumulatedTime() const;  // line 66
	void SetAccumulatedTime( float );  // line 67
	float GetLastUpdateTime() const;  // line 68
	void SetLastUpdateTime( float );  // line 69
	float GetVortexDirection() const;  // line 71
	bool ShouldDeleteThis() const;  // line 73
	void SetDeletionFlag( bool );  // line 74
	float GetLifeTime() const;  // line 76
	void UpdateLifeTime( float );  // line 77
	void UpdateBlobPreCollision( float, Vector &, Vector & );  // line 79
	void UpdateBlobCollision( float, const Vector &, Vector &, bool & );  // line 80
	void UpdateBlobPostCollision( float );  // line 81
	const Vector &GetContactNormal() const;  // line 83
	float GetStreakTime() const;  // line 85
	float GetStreakSpeedDampenRate() const;  // line 86
	void SetRadiusScale( float );  // line 88
	float GetRadiusScale() const;  // line 89
	bool ShouldPlayEffect() const;  // line 91
	bool IsSilent() const;  // line 93
protected:
	virtual void PaintBlobCleanse( const trace_t & );  // line 96
	virtual bool PaintBlobStreakPaint( const Vector & );  // line 97
	void PaintBlobMove( float, Vector &, Vector & );  // line 99
	void PaintBlobTractorBeamMove( float, Vector &, Vector & );  // line 100
	void PaintBlobMoveThroughPortal( float, C_Portal_Base2D *, const Vector &, const Vector & );  // line 102
	int CheckCollision( BlobCollisionRecord *, int, const Vector & );  // line 104
	virtual void ResolveCollision( bool &, const BlobCollisionRecord &, const Vector &, Vector &, float );  // line 105
	bool PaintBlobCheckShouldStreak( const trace_t &, Vector & );  // line 107
	void DecayVortexSpeed( float );  // line 109
	void PlayEffect( const Vector &, const Vector & );  // line 110
	Vector m_vecPosition; // +0x4  // line 112
	Vector m_vecPrevPosition; // +0x10  // line 113
	Vector m_vecVelocity; // +0x1c  // line 114
	Vector m_vContactNormal; // +0x28  // line 117
	PaintPowerType m_paintType; // +0x34  // line 119
	C_BaseEntity *m_pOwner; // +0x38  // line 120
	PaintBlobMoveState m_MoveState; // +0x3c  // line 121
	float m_flLifeTime; // +0x40  // line 124
	Vector m_vecStreakDir; // +0x44  // line 127
	bool m_bStreakDirChanged; // +0x50  // line 128
	float m_flStreakTimer; // +0x54  // line 129
	float m_flStreakSpeedDampenRate; // +0x58  // line 130
	Vector m_vBeamStart; // +0x5c  // line 133
	Vector m_vBeamEnd; // +0x68  // line 134
	bool m_bIsBeamReversed; // +0x74  // line 135
	bool m_bIsBeamDirTowardsPortal; // +0x75  // line 136
	float m_flBeamRadius; // +0x78  // line 137
	float m_flCurrentVortexRadius; // +0x7c  // line 139
	float m_flDestVortexRadius; // +0x80  // line 140
	float m_flVortexDirection; // +0x84  // line 141
	float m_flCurrentVortexSpeed; // +0x88  // line 142
	bool m_bDeleteFlag; // +0x8c  // line 144
	int m_nTraceTimer; // +0x90  // line 147
	float m_flAccumulatedTime; // +0x94  // line 149
	float m_flLastUpdateTime; // +0x98  // line 150
	float m_flRadiusScale; // +0x9c  // line 154
	bool m_bShouldPlayEffect; // +0xa0  // line 156
	CTrigger_TractorBeam *m_pBeam; // +0xa4  // line 159
	bool m_bSilent; // +0xa8  // line 162
};
