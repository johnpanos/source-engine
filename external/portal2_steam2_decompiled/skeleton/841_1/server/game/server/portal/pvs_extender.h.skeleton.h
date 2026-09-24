// DWARF declaration skeleton for game/server/portal/pvs_extender.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal/pvs_extender.h:15 sizeof=0x8 (i386)
struct CPVS_Extender
{
public:
	int (**_vptr$CPVS_Extender)(); // +0x0  // line 0
	CPVS_Extender();  // line 17
	virtual ~CPVS_Extender();  // line 18
	// game/server/portal/pvs_extender.h:21 sizeof=0xc (i386)
	struct VisExtensionChain_t
	{
	public:
		CPVS_Extender::VisExtensionChain_t *pParentChain; // +0x0  // line 22
		int m_nArea; // +0x4  // line 23
		CPVS_Extender *pExtender; // +0x8  // line 24
	};  // line 21
	virtual CServerNetworkProperty *GetExtenderNetworkProp();  // line 27
	virtual const edict_t *GetExtenderEdict() const;  // line 28
	virtual bool IsExtenderValid();  // line 29
	virtual Vector GetExtensionPVSOrigin();  // line 30
	static void ComputeExtendedPVS( const CBaseEntity *, const Vector &, unsigned char *, int, int );  // line 33
	virtual void ComputeSubVisibility( CPVS_Extender **, int, unsigned char *, int, const Vector &, const VPlane *, int, CPVS_Extender::VisExtensionChain_t *, int *, int );  // line 36
	// game/server/portal/pvs_extender.h:40 sizeof=0x2001 (i386)
	struct ExtenderInstanceData_t
	{
	public:
		unsigned char iPVSBits[8192]; // +0x0  // line 41
		bool bAddedToPVSAlready; // +0x2000  // line 42
	};  // line 40
protected:
	CPVS_Extender::ExtenderInstanceData_t *m_pExtenderData; // +0x4  // line 46
};
