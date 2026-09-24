// DWARF declaration skeleton for game/server/portal2/cpaintblob.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/cpaintblob.h:12 sizeof=0xdc (i386)
struct CPaintBlob : public CBasePaintBlob
{
public:
	CPaintBlob();  // line 14
	~CPaintBlob();  // line 15
	void AddToPaintDatabase();  // line 17
	void AssignBlobID( int );  // line 19
	int GetBlobID();  // line 20
	void SetBlobTeleportedThisFrame( bool );  // line 22
	bool HasBlobTeleportedThisFrame();  // line 23
	void AddBlobTeleportationHistory( const BlobTeleportationHistory_t & );  // line 25
	void ClearTeleportationHistory();  // line 26
	void GetTeleportationHistory( BlobTeleportationHistoryVector_t & );  // line 27
private:
	virtual void PaintBlobCleanse( const trace_t & );  // line 31
	virtual bool PaintBlobStreakPaint( const Vector & );  // line 32
	virtual void ResolveCollision( bool &, const BlobCollisionRecord &, Vector &, float );  // line 33
	void PaintBlobPaint( const trace_t & );  // line 35
	void SetupPaintInfo( const trace_t & );  // line 36
	CBaseEntity *m_pPaintingEntity; // +0xb8  // line 39
	short unsigned int m_nSurfaceFlags; // +0xbc  // line 40
	int m_ID; // +0xc0  // line 41
	bool m_bTeleportedThisFrame; // +0xc4  // line 42
	BlobTeleportationHistoryVector_t m_teleportationHistory; // +0xc8  // line 43
};
