// DWARF declaration skeleton for game/client/portal2/c_paintblob.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/portal2/c_paintblob.h:12 sizeof=0xac (i386)
struct C_PaintBlob : public CBasePaintBlob
{
public:
	C_PaintBlob();  // line 14
	~C_PaintBlob();  // line 15
private:
	virtual void PaintBlobCleanse( const trace_t & );  // line 19
	virtual bool PaintBlobStreakPaint( const Vector & );  // line 20
	virtual void ResolveCollision( bool &, const BlobCollisionRecord &, const Vector &, Vector &, float );  // line 21
	void PaintBlobPaint( const trace_t & );  // line 23
};
