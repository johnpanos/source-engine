// DWARF declaration skeleton for game/server/portal2/cpaintblob.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/cpaintblob.h:12 sizeof=0xb4 (i386)
struct CPaintBlob : public CBasePaintBlob
{
public:
	CPaintBlob();  // line 14
	~CPaintBlob();  // line 15
	void AddToPaintDatabase();  // line 17
private:
	virtual void PaintBlobCleanse( const trace_t & );  // line 21
	virtual bool PaintBlobStreakPaint( const Vector & );  // line 22
	virtual void ResolveCollision( bool &, const BlobCollisionRecord &, const Vector &, Vector &, float );  // line 23
	void PaintBlobPaint( const trace_t & );  // line 25
	void SetupPaintInfo( const trace_t & );  // line 26
	CBaseEntity *m_pPaintingEntity; // +0xac  // line 29
	short unsigned int m_nSurfaceFlags; // +0xb0  // line 30
};
