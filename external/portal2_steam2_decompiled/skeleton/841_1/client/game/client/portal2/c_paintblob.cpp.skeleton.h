// DWARF declaration skeleton for game/client/portal2/c_paintblob.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x74ee0 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
}

// game/client/portal2/c_paintblob.cpp:14
ConVar paintbomb_draw_num_paint_blobs;

// game/client/portal2/c_paintblob.cpp:15
ConVar paintbomb_draw_sphere_radius;

// game/client/portal2/c_paintblob.cpp:16
ConVar paintbomb_draw_min_blob_radius;

// game/client/portal2/c_paintblob.cpp:17
ConVar paintbomb_draw_max_blob_radius;

// game/client/portal2/c_paintblob.cpp:18
ConVar paintbomb_draw_blob_speed_min;

// game/client/portal2/c_paintblob.cpp:19
ConVar paintbomb_draw_blob_speed_max;

// game/client/portal2/c_paintblob.cpp:21 (declaration)
void C_PaintBlob();

// game/client/portal2/c_paintblob.cpp:21 @0x51ee00 _ZN11C_PaintBlobC1Ev
C_PaintBlob::C_PaintBlob()
{
}

// game/client/portal2/c_paintblob.cpp:21 @0x51ee40 _ZN11C_PaintBlobC2Ev
C_PaintBlob::C_PaintBlob()
{
}

// game/client/portal2/c_paintblob.cpp:26 (declaration)
~C_PaintBlob();

// game/client/portal2/c_paintblob.cpp:26 @0x51edc0 _ZN11C_PaintBlobD1Ev
C_PaintBlob::~C_PaintBlob()
{
}

// game/client/portal2/c_paintblob.cpp:26 @0x51ede0 _ZN11C_PaintBlobD2Ev
C_PaintBlob::~C_PaintBlob()
{
}

// game/client/portal2/c_paintblob.cpp:31 @0x51ee80 _Z23CreateBlobsForPaintBombRK6VectoriR10CUtlVectorIP14CBasePaintBlob10CUtlMemoryIS4_iEE
CreateBlobsForPaintBomb( const Vector &vecBombPos, int paintType, PaintBlobVector_t &blobs )
{
	int numBlobs;  // line 34
	// inlined ConVar::GetInt() at line 34
	{
		int i;  // line 36
		{
			Vector vecRand;  // line 39
			Vector vecBlobPos;  // line 41
			Vector vecBlobDir;  // line 44
			Vector vecBlobVelocity;  // line 46
			CBasePaintBlob *pBlob;  // line 49
			// inlined CUtlVector<CBasePaintBlob*,CUtlMemory<CBasePaintBlob*, int> >::AddToTail() at line 57
			// inlined RandomVector() at line 39
			// inlined ConVar::GetFloat() at line 41
			// inlined Vector::operator*() at line 41
			// inlined Vector::operator+() at line 41
			// inlined Vector::operator-() at line 44
			// inlined Vector::operator-() at line 46
			// inlined Vector::operator*() at line 46
		}
	}
}

// game/client/portal2/c_paintblob.cpp:62 @0x51ed70 _ZN11C_PaintBlob14PaintBlobPaintERK10CGameTrace
void C_PaintBlob::PaintBlobPaint( const trace_t &tr )
{
	Vector vecTouchPos;  // line 64
	Vector vecNormal;  // line 65
}

// public/tier1/utlmemory.h:707 @0x51f2b0 _ZN10CUtlMemoryIP14CBasePaintBlobiE4GrowEi
void CUtlMemory<CBasePaintBlob*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<CBasePaintBlob*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
