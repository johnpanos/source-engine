// DWARF declaration skeleton for game/shared/sequence_transitioner.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/sequence_transitioner.h:19 sizeof=0x14 (i386)
struct CSequenceTransitioner
{
public:
	void CheckForSequenceChange( CStudioHdr *, int, bool, bool );  // line 32
	void UpdateCurrent( CStudioHdr *, int, float, float, float );  // line 41
	void RemoveAll();  // line 43
	CUtlVector<C_AnimationLayer,CUtlMemory<C_AnimationLayer, int> > m_animationQueue; // +0x0  // line 46
};
