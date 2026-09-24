// DWARF declaration skeleton for game/client/portal2/c_paint_messages.cpp
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x75c60 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined C_PaintInitHelper::C_PaintInitHelper() at line 170
}

// game/client/portal2/c_paint_messages.cpp:21 @0x5243e0 _Z20__MsgFunc_PaintWorldR7bf_read
__MsgFunc_PaintWorld( bf_read &msg )
{
	{
		PaintPowerType power;  // line 28
		C_BaseEntity *pBrushEntity;  // line 29
		int nPaintCount;  // line 31
		Vector vCenter;  // line 34
		Vector vContactPoint;  // line 40
		// inlined CBitRead::ReadLong() at line 29
		// inlined CBitRead::ReadByte() at line 28
		// inlined CBitRead::ReadByte() at line 31
		// inlined CBitRead::ReadFloat() at line 35
		// inlined CBitRead::ReadFloat() at line 36
		// inlined CBitRead::ReadFloat() at line 37
		{
			int i;  // line 41
			// inlined CBitRead::ReadShort() at line 44
			// inlined CBitRead::ReadShort() at line 45
			// inlined CBitRead::ReadShort() at line 46
		}
	}
}

// game/client/portal2/c_paint_messages.cpp:54 @0x525020 _Z21__MsgFunc_PaintEntityR7bf_read
__MsgFunc_PaintEntity( bf_read &msg )
{
	IPaintableEntity *pPaintableEnt;  // line 56
	const PaintPowerType power;  // line 57
	Vector pos;  // line 58
	// inlined CBitRead::ReadFloat() at line 61
	// inlined CBitRead::ReadFloat() at line 60
	// inlined CBitRead::ReadFloat() at line 59
	// inlined CBitRead::ReadByte() at line 57
	// inlined CBitRead::ReadLong() at line 56
}

// game/client/portal2/c_paint_messages.cpp:67 @0x523f70 _Z26__MsgFunc_ChangePaintColorR7bf_read
__MsgFunc_ChangePaintColor( bf_read &msg )
{
	C_BaseEntity *pEntity;  // line 70
	C_WeaponPaintGun *pPaintGun;  // line 77
	{
		int power;  // line 80
		// inlined CBitRead::ReadByte() at line 80
	}
	// inlined CBitRead::ReadLong() at line 70
}

// game/client/portal2/c_paint_messages.cpp:88 @0x523f10 _Z24__MsgFunc_RemoveAllPaintR7bf_read
__MsgFunc_RemoveAllPaint( bf_read &msg )
{
}

// game/client/portal2/c_paint_messages.cpp:97 @0x524280 _Z26__MsgFunc_PaintAllSurfacesR7bf_read
__MsgFunc_PaintAllSurfaces( bf_read &msg )
{
	{
		uint8 power;  // line 101
		// inlined CBitRead::ReadByte() at line 101
	}
}

// game/client/portal2/c_paint_messages.cpp:107 @0x525cc0 _Z26__MsgFunc_LoadPaintmapDataR7bf_read
__MsgFunc_LoadPaintmapData( bf_read &msg )
{
	int nPaintmapID;  // line 110
	int nPaintmapOffset;  // line 111
	int nNumRLE;  // line 113
	CUtlVector<unsigned char,CUtlMemory<unsigned char, int> > data;  // line 114
	int offset;  // line 115
	// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::~CUtlVector() at line 130
	{
		int n;  // line 120
		{
			int nElements;  // line 122
			uint8 val;  // line 123
			// inlined CBitRead::ReadFloat() at line 122
			// inlined CBitRead::ReadByte() at line 123
			// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::EnsureCount() at line 125
		}
	}
	// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::CUtlVector() at line 114
	// inlined CBitRead::ReadFloat() at line 113
	// inlined CBitRead::ReadFloat() at line 111
	// inlined CBitRead::ReadByte() at line 110
	// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::~CUtlVector() at line 130
}

// game/client/portal2/c_paint_messages.cpp:134 @0x525790 _Z26__MsgFunc_LoadPaintmapBitsR7bf_read
__MsgFunc_LoadPaintmapBits( bf_read &msg )
{
	CVarBitVec paintSurfBits;  // line 136
	int numBits;  // line 138
	// inlined CVarBitVec::~CVarBitVec() at line 146
	{
		int i;  // line 141
		// inlined CBitRead::ReadFloat() at line 143
		// inlined CBitVecT<CVarBitVecBase>::SetDWord() at line 143
	}
	// inlined CVarBitVecBase::Resize() at line 139
	// inlined CBitRead::ReadFloat() at line 138
	// inlined CVarBitVec::CVarBitVec() at line 136
	// inlined CVarBitVec::~CVarBitVec() at line 146
}

// game/client/portal2/c_paint_messages.cpp:152 sizeof=0xc (i386)
struct C_PaintInitHelper : public CAutoGameSystem
{
private:
	virtual bool Init();  // line 153
};

// game/client/portal2/c_paint_messages.cpp:152 (declaration)
void C_PaintInitHelper();

// game/client/portal2/c_paint_messages.cpp:152 (declaration)
~C_PaintInitHelper();

// game/client/portal2/c_paint_messages.cpp:152 @0x5268a0 _ZN17C_PaintInitHelperD1Ev
C_PaintInitHelper::~C_PaintInitHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 152
}

// game/client/portal2/c_paint_messages.cpp:152 @0x5268c0 _ZN17C_PaintInitHelperD0Ev
C_PaintInitHelper::~C_PaintInitHelper()
{
	// inlined CAutoGameSystem::~CAutoGameSystem() at line 152
}

// game/client/portal2/c_paint_messages.cpp:153 @0x526550 _ZN17C_PaintInitHelper4InitEv
bool C_PaintInitHelper::Init()
{
	{
		int i;  // line 155
		{
			CSetActiveSplitScreenPlayerGuard g_SSGuard;  // line 157
		}
	}
}

// game/client/portal2/c_paint_messages.cpp:170
static C_PaintInitHelper s_PaintInitHelper;

// public/tier1/utlmemory.h:707 @0x5267c0 _ZN10CUtlMemoryIhiE4GrowEi
void CUtlMemory<unsigned char,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<unsigned char,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
