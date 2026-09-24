// DWARF declaration skeleton for game/shared/portal/paint_power_user.h
// Source: Steam2 depot 841_1 client.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_power_user.h:85 sizeof=0x1b60 (i386)
struct PaintPowerUser<CPaintableEntity<C_BasePlayer> > : public CPaintableEntity<C_BasePlayer>, public IPaintPowerUser
{
public:
	struct ContextSurfacePaintPowerInfo_t; // declaration only in this unit  // line 0
private:
	PaintPowerInfo_t m_PaintPowers[5]; // +0x1a5c  // line 162
	CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > m_SurfacePaintPowerInfo; // +0x1b38  // line 163
	CUtlVector<PaintPowerUser<CPaintableEntity<C_BasePlayer> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<C_BasePlayer> >::ContextSurfacePaintPowerInfo_t, int> > m_ContextSurfacePaintPowerInfo; // +0x1b4c  // line 164
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 175
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 176
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 177
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 179
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 180
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 181
public:
	static typedescription_t m_PredDesc[1];  // line 211
	void PaintPowerUser();  // line 241
	virtual ~PaintPowerUser();  // line 252
	virtual const std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> GetPaintPowers() const;  // line 258
	virtual const PaintPowerInfo_t &GetPaintPower( unsigned int ) const;  // line 265
	virtual const PaintPowerInfo_t *FindHighestPriorityActivePaintPower() const;  // line 273
	virtual void AddSurfacePaintPowerInfo( const PaintPowerInfo_t &, const char * );  // line 290
protected:
	virtual void ChooseActivePaintPowers( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > & );  // line 324
public:
	virtual void UpdatePaintPowers();  // line 342
protected:
	void ClearSurfacePaintPowerInfo();  // line 406
	void PrioritySortSurfacePaintPowerInfo( int (*)(const PaintPowerInfo_t *, const PaintPowerInfo_t *) );  // line 419
	void MapSurfacesToPowers();  // line 443
	bool SurfaceInfoContainsPower( const PaintPowerInfo_t &, const char * ) const;  // line 458
private:
	const std::pair<PaintPowerInfo_t*,PaintPowerInfo_t*> GetNonConstSurfacePaintPowerInfo( const char * );  // line 472
	int FindContextSurfacePaintPowerInfo( const char * ) const;  // line 487
protected:
	PaintPowerState ActivatePaintPower( PaintPowerInfo_t & );  // line 506
	PaintPowerState UsePaintPower( PaintPowerInfo_t & );  // line 534
	PaintPowerState DeactivatePaintPower( PaintPowerInfo_t & );  // line 559
	const std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> GetSurfacePaintPowerInfo( const char * ) const;  // line 584
	bool HasAnySurfacePaintPowerInfo() const;  // line 599
	void ForceSetPaintPower( const PaintPowerInfo_t & );  // line 613
	void ForcePaintPowerToState( PaintPowerType, PaintPowerState );  // line 620
private:
	virtual PaintPowerState ActivateNoPower( PaintPowerInfo_t & );  // line 628
	virtual PaintPowerState UseNoPower( PaintPowerInfo_t & );  // line 635
	virtual PaintPowerState DeactivateNoPower( PaintPowerInfo_t & );  // line 642
};
