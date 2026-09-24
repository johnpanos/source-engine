// DWARF declaration skeleton for game/shared/portal/paint_power_user.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/shared/portal/paint_power_user.h:52 (declaration)
const std::pair<PaintPowerInfo_t*,PaintPowerInfo_t*> GetRange<PaintPowerInfo_t>( CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > &v );

// game/shared/portal/paint_power_user.h:59 (declaration)
const std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> GetConstRange<PaintPowerInfo_t>( const CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > &v );

// game/shared/portal/paint_power_user.h:85 sizeof=0x1368 (i386)
struct PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer> > : public CPaintableEntity<CBaseMultiplayerPlayer>, public IPaintPowerUser
{
public:
	struct ContextSurfacePaintPowerInfo_t; // declaration only in this unit  // line 0
private:
	PaintPowerInfo_t m_PaintPowers[5]; // +0x1264  // line 162
	CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > m_SurfacePaintPowerInfo; // +0x1340  // line 163
	CUtlVector<PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CBaseMultiplayerPlayer> >::ContextSurfacePaintPowerInfo_t, int> > m_ContextSurfacePaintPowerInfo; // +0x1354  // line 164
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 175
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 176
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 177
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 179
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 180
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 181
public:
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

// game/shared/portal/paint_power_user.h:85 sizeof=0x874 (i386)
struct PaintPowerUser<CPaintableEntity<CPhysicsProp> > : public CPaintableEntity<CPhysicsProp>, public IPaintPowerUser
{
public:
	struct ContextSurfacePaintPowerInfo_t; // declaration only in this unit  // line 0
private:
	PaintPowerInfo_t m_PaintPowers[5]; // +0x770  // line 162
	CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > m_SurfacePaintPowerInfo; // +0x84c  // line 163
	CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> > m_ContextSurfacePaintPowerInfo; // +0x860  // line 164
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 175
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 176
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 177
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 179
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 180
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 181
public:
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

// game/shared/portal/paint_power_user.h:85 sizeof=0x10e0 (i386)
struct PaintPowerUser<CPaintableEntity<CNPC_FloorTurret> > : public CPaintableEntity<CNPC_FloorTurret>, public IPaintPowerUser
{
public:
	struct ContextSurfacePaintPowerInfo_t; // declaration only in this unit  // line 0
private:
	PaintPowerInfo_t m_PaintPowers[5]; // +0xfdc  // line 162
	CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > m_SurfacePaintPowerInfo; // +0x10b8  // line 163
	CUtlVector<PaintPowerUser<CPaintableEntity<CNPC_FloorTurret> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CNPC_FloorTurret> >::ContextSurfacePaintPowerInfo_t, int> > m_ContextSurfacePaintPowerInfo; // +0x10cc  // line 164
	virtual PaintPowerState ActivateSpeedPower( PaintPowerInfo_t & );  // line 175
	virtual PaintPowerState UseSpeedPower( PaintPowerInfo_t & );  // line 176
	virtual PaintPowerState DeactivateSpeedPower( PaintPowerInfo_t & );  // line 177
	virtual PaintPowerState ActivateBouncePower( PaintPowerInfo_t & );  // line 179
	virtual PaintPowerState UseBouncePower( PaintPowerInfo_t & );  // line 180
	virtual PaintPowerState DeactivateBouncePower( PaintPowerInfo_t & );  // line 181
public:
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

// game/shared/portal/paint_power_user.h:152 sizeof=0x18 (i386)
struct ContextSurfacePaintPowerInfo_t
{
public:
	CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > paintPowerInfo; // +0x0  // line 153
	string_t context; // +0x14  // line 154
};

// game/shared/portal/paint_power_user.h:152 (declaration)
~ContextSurfacePaintPowerInfo_t();

// game/shared/portal/paint_power_user.h:152 (declaration)
void ContextSurfacePaintPowerInfo_t();

// game/shared/portal/paint_power_user.h:241 @0x76a9b0 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEEC2Ev
void PaintPowerUser<CPaintableEntity<CPhysicsProp> >::PaintPowerUser()
{
	{
		unsigned int i;  // line 243
		// inlined PaintPowerInfo_t::operator=() at line 245
	}
	// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::CUtlVector() at line 241
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::CUtlVector() at line 241
	// inlined CPaintableEntity<CPhysicsProp>::CPaintableEntity() at line 241
	// inlined CPaintableEntity<CPhysicsProp>::~CPaintableEntity() at line 246
	// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::~CUtlVector() at line 246
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 246
}

// game/shared/portal/paint_power_user.h:252 @0x769fb0 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEED2Ev
PaintPowerUser<CPaintableEntity<CPhysicsProp> >::~PaintPowerUser()
{
	// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::~CUtlVector() at line 253
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 253
	// inlined CPaintableEntity<CPhysicsProp>::~CPaintableEntity() at line 253
	// inlined CPaintableEntity<CPhysicsProp>::~CPaintableEntity() at line 253
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 253
}

// game/shared/portal/paint_power_user.h:252 @0x76a5b0 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEED1Ev
PaintPowerUser<CPaintableEntity<CPhysicsProp> >::~PaintPowerUser()
{
}

// game/shared/portal/paint_power_user.h:252 @0x76b120 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEED0Ev
PaintPowerUser<CPaintableEntity<CPhysicsProp> >::~PaintPowerUser()
{
	// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::~CUtlVector() at line 253
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 253
	// inlined CPaintableEntity<CPhysicsProp>::~CPaintableEntity() at line 253
	// inlined CPaintableEntity<CPhysicsProp>::~CPaintableEntity() at line 253
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 253
}

// game/shared/portal/paint_power_user.h:258 @0x7674a0 _ZNK14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE14GetPaintPowersEv
const std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> PaintPowerUser<CPaintableEntity<CPhysicsProp> >::GetPaintPowers()
{
}

// game/shared/portal/paint_power_user.h:265 @0x767380 _ZNK14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE13GetPaintPowerEj
const PaintPowerInfo_t &PaintPowerUser<CPaintableEntity<CPhysicsProp> >::GetPaintPower( unsigned int powerType )
{
}

// game/shared/portal/paint_power_user.h:273 @0x7673c0 _ZNK14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE35FindHighestPriorityActivePaintPowerEv
const PaintPowerInfo_t *PaintPowerUser<CPaintableEntity<CPhysicsProp> >::FindHighestPriorityActivePaintPower()
{
	const PaintPowerInfo_t *pPower;  // line 275
	{
		unsigned int i;  // line 276
	}
}

// game/shared/portal/paint_power_user.h:290 @0x768380 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE24AddSurfacePaintPowerInfoERK16PaintPowerInfo_tPKc
void PaintPowerUser<CPaintableEntity<CPhysicsProp> >::AddSurfacePaintPowerInfo( const PaintPowerInfo_t &contact, const char *context )
{
	{
		int index;  // line 307
		// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::operator[]() at line 314
		// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::EnsureCount() at line 313
		// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::FindContextSurfacePaintPowerInfo() at line 307
		// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::operator[]() at line 318
		// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::AddToTail() at line 318
	}
	// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::AddToTail() at line 301
}

// game/shared/portal/paint_power_user.h:342 @0x7695e0 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE17UpdatePaintPowersEv
void PaintPowerUser<CPaintableEntity<CPhysicsProp> >::UpdatePaintPowers()
{
	{
		CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> > activePowers;  // line 348
		PaintPowerInfo_t cachedPowers[5];  // line 352
		std::pair<const PaintPowerInfo_t*,const PaintPowerInfo_t*> activeRange;  // line 364
		// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::CUtlVector() at line 348
		// inlined std::copy<PaintPowerInfo_t*, PaintPowerInfo_t*>() at line 354
		{
			unsigned int i;  // line 357
			{
				PaintPowerInfo_t &power;  // line 359
			}
		}
		// inlined GetConstRange<PaintPowerInfo_t>() at line 364
		{
			PaintPowerConstIter i;  // line 365
			{
				const PaintPowerInfo_t &newPower;  // line 368
				const unsigned int index;  // line 369
				const PaintPowerInfo_t &oldPower;  // line 373
				const bool stayActive;  // line 374
				// inlined PaintPowerInfo_t::operator=() at line 370
			}
		}
		// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 398
		// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ClearSurfacePaintPowerInfo() at line 381
		{
			unsigned int i;  // line 384
			// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::UsePaintPower() at line 393
			// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ActivatePaintPower() at line 389
			// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::DeactivatePaintPower() at line 397
		}
		// inlined CUtlVector<PaintPowerInfo_t,CUtlMemory<PaintPowerInfo_t, int> >::~CUtlVector() at line 398
	}
}

// game/shared/portal/paint_power_user.h:433 (declaration)
void MapSurfacesToPowers<PaintPowerInfo_t*>( PaintPowerInfo_t *begin, PaintPowerInfo_t *end );

// game/shared/portal/paint_power_user.h:443 @0x767cf0 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE19MapSurfacesToPowersEv
void PaintPowerUser<CPaintableEntity<CPhysicsProp> >::MapSurfacesToPowers()
{
	std::pair<PaintPowerInfo_t*,PaintPowerInfo_t*> range;  // line 445
	const int count;  // line 448
	// inlined PaintPowerUser<CPaintableEntity<CPhysicsProp> >::GetNonConstSurfacePaintPowerInfo() at line 445
	// inlined MapSurfacesToPowers<PaintPowerInfo_t*>() at line 446
	// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::Count() at line 448
	{
		int i;  // line 449
		{
			std::pair<PaintPowerInfo_t*,PaintPowerInfo_t*> contextRange;  // line 451
			// inlined CUtlVector<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t,CUtlMemory<PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ContextSurfacePaintPowerInfo_t, int> >::operator[]() at line 451
			// inlined GetRange<PaintPowerInfo_t>() at line 451
			// inlined MapSurfacesToPowers<PaintPowerInfo_t*>() at line 452
		}
	}
}

// game/shared/portal/paint_power_user.h:628 @0x767460 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE15ActivateNoPowerER16PaintPowerInfo_t
PaintPowerState PaintPowerUser<CPaintableEntity<CPhysicsProp> >::ActivateNoPower( PaintPowerInfo_t &powerInfo )
{
}

// game/shared/portal/paint_power_user.h:635 @0x767470 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE10UseNoPowerER16PaintPowerInfo_t
PaintPowerState PaintPowerUser<CPaintableEntity<CPhysicsProp> >::UseNoPower( PaintPowerInfo_t &powerInfo )
{
}

// game/shared/portal/paint_power_user.h:642 @0x767480 _ZN14PaintPowerUserI16CPaintableEntityI12CPhysicsPropEE17DeactivateNoPowerER16PaintPowerInfo_t
PaintPowerState PaintPowerUser<CPaintableEntity<CPhysicsProp> >::DeactivateNoPower( PaintPowerInfo_t &powerInfo )
{
}
