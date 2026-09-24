// DWARF declaration skeleton for game/shared/portal/paint_stream.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_stream.h:15 sizeof=0xe28 (i386)
struct C_PaintStream : public C_BaseAnimating
{
public:
	C_PaintStream();  // line 19
	virtual ~C_PaintStream();  // line 20
	virtual void Spawn();  // line 22
	virtual const QAngle &GetRenderAngles();  // line 24
	virtual const Vector &GetRenderOrigin();  // line 25
	virtual void GetRenderBoundsWorldspace( Vector &, Vector & );  // line 26
	virtual void GetRenderBounds( Vector &, Vector & );  // line 27
	virtual const Vector &WorldSpaceCenter();  // line 30
	virtual void ComputeWorldSpaceSurroundingBox( Vector *, Vector * );  // line 31
	virtual int DrawModel( int, const RenderableInstance_t & );  // line 33
	virtual bool ShouldDrawForSplitScreenUser( int );  // line 34
	void AddPaintBlob( CBasePaintBlob * );  // line 49
	void RemoveAllPaintBlobs();  // line 50
	virtual const Vector &WorldAlignMins() const;  // line 52
	virtual const Vector &WorldAlignMaxs() const;  // line 53
	void UpdateRenderBoundsAndOriginWorldspace();  // line 55
	unsigned int GetBlobsCount() const;  // line 57
	CBasePaintBlob *GetBlob( int );  // line 58
	void UpdateBlobs();  // line 60
	void QueryBlobSphere( const Vector &, float, PaintBlobVector_t & );  // line 63
	void SetPaintType( int );  // line 72
private:
	void DebugDrawBlobs();  // line 76
	void DeleteDeadBlobs();  // line 79
	void QueuePaintEffect();  // line 81
	Vector m_vCachedRenderOrigin; // +0xd84  // line 83
	Vector m_vCachedWorldMins; // +0xd90  // line 84
	Vector m_vCachedWorldMaxs; // +0xd9c  // line 85
	int m_iCachedWorldBoundsUpdateTick; // +0xda8  // line 86
	int m_nCurrentUpdateFrame; // +0xdac  // line 88
	int m_nPaintType; // +0xdb0  // line 90
	PaintBlobVector_t m_blobs; // +0xdb4  // line 91
	void DrawBlobs( IMaterial * );  // line 94
	Vector4D m_lightingBoxColors[6]; // +0xdc8  // line 95
};
