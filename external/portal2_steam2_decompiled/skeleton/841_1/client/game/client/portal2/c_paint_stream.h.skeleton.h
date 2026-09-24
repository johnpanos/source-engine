// DWARF declaration skeleton for game/client/portal2/c_paint_stream.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_paint_stream.h:12 sizeof=0xe38 (i386)
struct C_PaintStream : public C_BaseAnimating
{
public:
	C_PaintStream();  // line 17
	virtual ~C_PaintStream();  // line 18
	virtual void Spawn();  // line 20
	virtual void InitSharedVars();  // line 21
	virtual const QAngle &GetRenderAngles();  // line 23
	virtual const Vector &GetRenderOrigin();  // line 24
	virtual void GetRenderBoundsWorldspace( Vector &, Vector & );  // line 25
	virtual void GetRenderBounds( Vector &, Vector & );  // line 26
	virtual const Vector &WorldSpaceCenter();  // line 29
	virtual void ComputeWorldSpaceSurroundingBox( Vector *, Vector * );  // line 30
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 32
	virtual bool ShouldDraw();  // line 33
	virtual void UpdateOnRemove();  // line 35
	void AddPaintBlob( CBasePaintBlob * );  // line 38
	void RemoveAllPaintBlobs();  // line 39
	virtual const Vector &WorldAlignMins() const;  // line 41
	virtual const Vector &WorldAlignMaxs() const;  // line 42
	void UpdateRenderBoundsAndOriginWorldspace();  // line 44
	unsigned int GetBlobsCount() const;  // line 46
	CBasePaintBlob *GetBlob( int );  // line 47
	void Update();  // line 49
	void UpdateBlobs();  // line 50
	void OnBlobPaused();  // line 51
	void SetPaintType( int );  // line 53
private:
	void DebugDrawBlobs();  // line 58
	void DeleteDeadBlobs();  // line 61
	void QueuePaintEffect();  // line 63
	Vector m_vCachedRenderOrigin; // +0xd84  // line 65
	Vector m_vCachedWorldMins; // +0xd90  // line 66
	Vector m_vCachedWorldMaxs; // +0xd9c  // line 67
	int m_iCachedWorldBoundsUpdateTick; // +0xda8  // line 68
	int m_nCurrentUpdateFrame; // +0xdac  // line 70
	PaintBlobVector_t m_blobs; // +0xdb0  // line 72
	void GetCurrentBlobData( BlobDataVector_t & );  // line 75
	CSharedUtlVectorBase<BlobDataPerUpdate_t,C_PaintStream::SharedVar_m_sharedBlobData> m_sharedBlobData; // +0xdc4  // line 76
	CNetworkVar( int, m_nPaintType ); // +0xdd0  // line 78
	CNetworkVar( int, m_nRenderMode ); // +0xdd4  // line 79
	void DrawBlobs( IMaterial * );  // line 82
	Vector4D m_lightingBoxColors[6]; // +0xdd8  // line 83
};

// game/client/portal2/c_paint_stream.h:33 @0x52ae10 _ZN13C_PaintStream10ShouldDrawEv
bool C_PaintStream::ShouldDraw()
{
}

// game/client/portal2/c_paint_stream.h:86
typedef C_PaintStream CPaintStream;
