// DWARF declaration skeleton for game/shared/portal/paint_stream.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_stream.h:39 sizeof=0x44 (i386)
struct CPaintStream
{
public:
	CPaintStream();  // line 43
	~CPaintStream();  // line 44
	void AddPaintBlob( CBasePaintBlob * );  // line 49
	void RemoveAllPaintBlobs();  // line 50
	const Vector &WorldAlignMins() const;  // line 52
	const Vector &WorldAlignMaxs() const;  // line 53
	void UpdateRenderBoundsAndOriginWorldspace();  // line 55
	unsigned int GetBlobsCount() const;  // line 57
	CBasePaintBlob *GetBlob( int );  // line 58
	void UpdateBlobs();  // line 60
	void QueryBlobSphere( const Vector &, float, PaintBlobVector_t & );  // line 63
	void SavePaintBlobState( ISave * );  // line 66
	void RestorePaintBlobState( IRestore * );  // line 67
	void *GetBlobData();  // line 69
	void SetPaintType( int );  // line 72
private:
	void DebugDrawBlobs();  // line 76
	void DeleteDeadBlobs();  // line 79
	void QueuePaintEffect();  // line 81
	Vector m_vCachedRenderOrigin; // +0x0  // line 83
	Vector m_vCachedWorldMins; // +0xc  // line 84
	Vector m_vCachedWorldMaxs; // +0x18  // line 85
	int m_iCachedWorldBoundsUpdateTick; // +0x24  // line 86
	int m_nCurrentUpdateFrame; // +0x28  // line 88
	int m_nPaintType; // +0x2c  // line 90
	PaintBlobVector_t m_blobs; // +0x30  // line 91
	void AddPaintToDatabase() const;  // line 98
};
