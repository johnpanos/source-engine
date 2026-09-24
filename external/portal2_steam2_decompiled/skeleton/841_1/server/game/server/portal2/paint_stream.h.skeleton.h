// DWARF declaration skeleton for game/server/portal2/paint_stream.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/paint_stream.h:12 sizeof=0x56c (i386)
struct CPaintStream : public CBaseAnimating
{
public:
	CPaintStream();  // line 19
	virtual ~CPaintStream();  // line 20
	virtual void Spawn();  // line 22
	virtual void Precache();  // line 23
	virtual void InitSharedVars();  // line 24
	virtual void UpdateOnRemove();  // line 25
	virtual int UpdateTransmitState();  // line 26
	void AddPaintBlob( CBasePaintBlob * );  // line 30
	void RemoveAllPaintBlobs();  // line 31
	const Vector &WorldAlignMins() const;  // line 33
	const Vector &WorldAlignMaxs() const;  // line 34
	void UpdateRenderBoundsAndOriginWorldspace();  // line 36
	unsigned int GetBlobsCount() const;  // line 38
	CBasePaintBlob *GetBlob( int );  // line 39
	void Update();  // line 41
	void UpdateBlobs();  // line 42
	void OnBlobPaused();  // line 43
	void SetPaintType( int );  // line 45
	void SetStreamRenderMode( int );  // line 46
	void SavePaintBlobState( ISave * );  // line 49
	void RestorePaintBlobState( IRestore * );  // line 50
private:
	void DebugDrawBlobs();  // line 53
	void DeleteDeadBlobs();  // line 56
	void QueuePaintEffect();  // line 58
	Vector m_vCachedRenderOrigin; // +0x514  // line 60
	Vector m_vCachedWorldMins; // +0x520  // line 61
	Vector m_vCachedWorldMaxs; // +0x52c  // line 62
	int m_iCachedWorldBoundsUpdateTick; // +0x538  // line 63
	int m_nCurrentUpdateFrame; // +0x53c  // line 65
	int m_nBlobCounter; // +0x540  // line 67
	PaintBlobVector_t m_blobs; // +0x544  // line 68
	CSharedUtlVectorBase<BlobDataPerUpdate_t,CPaintStream::SharedVar_m_sharedBlobData> m_sharedBlobData; // +0x558  // line 71
	CNetworkVar( int, m_nPaintType ); // +0x564  // line 73
	CNetworkVar( int, m_nRenderMode ); // +0x568  // line 74
	void AddPaintToDatabase() const;  // line 77
	void UpdateBlobSharedData();  // line 78
};
