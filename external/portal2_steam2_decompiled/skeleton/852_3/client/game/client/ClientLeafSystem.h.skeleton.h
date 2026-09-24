// DWARF declaration skeleton for game/client/ClientLeafSystem.h
// Source: Steam2 depot 852_3 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/client/ClientLeafSystem.h:64 sizeof=0xc (i386)
struct DistanceFadeInfo_t
{
public:
	float m_flMaxDistSqr; // +0x0  // line 65
	float m_flMinDistSqr; // +0x4  // line 66
	float m_flFalloffFactor; // +0x8  // line 67
};

// game/client/ClientLeafSystem.h:73 sizeof=0x18124 (i386)
struct CClientRenderablesList : public CRefCounted<CRefCountServiceBase<true, CRefMT> >
{
private:
	static CUtlMemoryPool s_Allocator;  // line 74
public:
	static void *operator new( size_t );  // line 74
	static void *operator new( size_t, int, const char *, int );  // line 74
	static void operator delete( void * );  // line 74
	static void operator delete( void *, int, const char *, int );  // line 74
	// game/client/ClientLeafSystem.h:84 sizeof=0x8 (i386)
	struct CEntry
	{
	public:
		IClientRenderable *m_pRenderable; // +0x0  // line 85
		short unsigned int m_iWorldListInfoLeaf; // +0x4  // line 86
		RenderableInstance_t m_InstanceData; // +0x6  // line 87
		uint8 m_nModelType : 7; // +0x7  // line 88
		uint8 m_TwoPass : 1; // +0x7  // line 89
	};  // line 84
	DistanceFadeInfo_t m_DetailFade; // +0x8  // line 93
	CClientRenderablesList::CEntry m_RenderGroups[3][4096]; // +0x14  // line 94
	int m_RenderGroupCounts[3]; // +0x18014  // line 95
	int m_nBoneSetupDependencyCount; // +0x18020  // line 96
	IClientRenderable *m_pBoneSetupDependency[64]; // +0x18024  // line 97
};

// game/client/ClientLeafSystem.h:105 sizeof=0x230 (i386)
struct CViewModelRenderablesList
{
public:
	// game/client/ClientLeafSystem.h:115 sizeof=0x8 (i386)
	struct CEntry
	{
	public:
		IClientRenderable *m_pRenderable; // +0x0  // line 116
		RenderableInstance_t m_InstanceData; // +0x4  // line 117
	};  // line 115
	CUtlVectorFixedGrowable<CViewModelRenderablesList::CEntry,32ul> m_RenderGroups[2]; // +0x0  // line 123
};
