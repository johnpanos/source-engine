// DWARF declaration skeleton for game/server/portal2/paint_database.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0xad940 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CPaintDatabase::CPaintDatabase() at line 31
}

// game/server/portal2/paint_database.cpp:31
CPaintDatabase PaintDatabase;

// game/server/portal2/paint_database.cpp:33 (declaration)
void CPaintDatabase( const char *name );

// game/server/portal2/paint_database.cpp:33 @0xad860 _ZN14CPaintDatabaseC2EPKc
CPaintDatabase::CPaintDatabase( const char *name )
{
	// inlined CGameEventListener::CGameEventListener() at line 33
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::CUtlVector() at line 33
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::CUtlVector() at line 33
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::CUtlVector() at line 33
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::CUtlVector() at line 33
	// inlined CPaintDatabase::ClearPaintData() at line 36
}

// game/server/portal2/paint_database.cpp:33 @0x733db0 _ZN14CPaintDatabaseC1EPKc
CPaintDatabase::CPaintDatabase( const char *name )
{
}

// game/server/portal2/paint_database.cpp:40 (declaration)
~CPaintDatabase();

// game/server/portal2/paint_database.cpp:40 @0x7360c0 _ZN14CPaintDatabaseD0Ev
CPaintDatabase::~CPaintDatabase()
{
	// inlined CPaintDatabase::ClearPaintData() at line 42
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::~CUtlVector() at line 43
	// inlined CGameEventListener::~CGameEventListener() at line 43
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 43
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 43
	// inlined CGameEventListener::~CGameEventListener() at line 43
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::~CUtlVector() at line 43
}

// game/server/portal2/paint_database.cpp:40 @0x736750 _ZN14CPaintDatabaseD2Ev
CPaintDatabase::~CPaintDatabase()
{
	// inlined CPaintDatabase::ClearPaintData() at line 42
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::~CUtlVector() at line 43
	// inlined CGameEventListener::~CGameEventListener() at line 43
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 43
	// inlined CAutoGameSystemPerFrame::~CAutoGameSystemPerFrame() at line 43
	// inlined CGameEventListener::~CGameEventListener() at line 43
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::~CUtlVector() at line 43
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::~CUtlVector() at line 43
}

// game/server/portal2/paint_database.cpp:40 @0x736de0 _ZN14CPaintDatabaseD1Ev
CPaintDatabase::~CPaintDatabase()
{
}

// game/server/portal2/paint_database.cpp:45 @0x731d80 _ZN14CPaintDatabase18LevelInitPreEntityEv
void CPaintDatabase::LevelInitPreEntity()
{
	// inlined CPaintDatabase::ClearPaintData() at line 47
}

// game/server/portal2/paint_database.cpp:50 @0x732460 _ZN14CPaintDatabase19LevelInitPostEntityEv
void CPaintDatabase::LevelInitPostEntity()
{
	// inlined CPaintDatabase::SendPaintDataToEngine() at line 54
}

// game/server/portal2/paint_database.cpp:57 @0x731ae0 _ZN14CPaintDatabase23LevelShutdownPostEntityEv
void CPaintDatabase::LevelShutdownPostEntity()
{
}

// game/server/portal2/paint_database.cpp:62 @0x733290 _ZN14CPaintDatabase8AddPaintERK10CGameTrace14PaintPowerType
void CPaintDatabase::AddPaint( const trace_t &tr, PaintPowerType powerType )
{
	CBaseEntity *pPaintedEntity;  // line 64
	Vector vecPaintLocation;  // line 65
	Vector vecNormal;  // line 66
	{
		PaintLocationData_t data;  // line 71
		// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::AddToTail() at line 78
		// inlined Vector::operator+() at line 73
		// inlined operator*() at line 73
	}
	{
		CProjectedWallEntity *pPaintableWall;  // line 84
	}
}

// game/server/portal2/paint_database.cpp:98 @0x732900 _ZN14CPaintDatabase11PaintEntityEP11CBaseEntity14PaintPowerTypeRK6Vector
void CPaintDatabase::PaintEntity( CBaseEntity *pPaintedEntity, PaintPowerType newPowerType, const Vector &vecPosition )
{
	IPaintableEntity *pPaintableEntity;  // line 101
	PaintPowerType paintedPowerType;  // line 102
	const bool bPainted;  // line 129
	{
		IPhysicsObject *pPhysics;  // line 154
		// inlined CBaseEntity::VPhysicsGetObject() at line 154
	}
	// inlined CPaintDatabase::FindPaintedEntity() at line 140
	{
		PaintEntityData_t data;  // line 142
		// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::AddToTail() at line 146
		// inlined CBaseHandle::operator=() at line 143
	}
	{
		CBroadcastRecipientFilter filter;  // line 110
		// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 110
		// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 118
		// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 118
	}
	{
		const color24 otherColor;  // line 123
		const Color paintedColor;  // line 124
		// inlined CBaseEntity::GetRenderColor() at line 123
		// inlined Color::Color() at line 124
	}
	// inlined CPaintDatabase::RemovePaintedEntity() at line 134
	{
		Color newColor;  // line 165
		// inlined CBaseEntity::SetRenderColor() at line 166
	}
}

// game/server/portal2/paint_database.cpp:171 @0x731bb0 _ZN14CPaintDatabase18PaintProjectedWallEP20CProjectedWallEntity14PaintPowerTypeRK6Vector
void CPaintDatabase::PaintProjectedWall( CProjectedWallEntity *pWall, PaintPowerType powerType, const Vector &vecPosition )
{
	const bool bWallPainted;  // line 173
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::AddToTail() at line 182
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::Find() at line 180
}

// game/server/portal2/paint_database.cpp:188 @0x733850 _ZN14CPaintDatabase14RemoveAllPaintEv
void CPaintDatabase::RemoveAllPaint()
{
	const int nEntityCount;  // line 191
	const int nPaintedProjectedWallCount;  // line 202
	CBroadcastRecipientFilter filter;  // line 212
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 215
	// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 212
	// inlined CPaintDatabase::ClearPaintData() at line 208
	{
		int i;  // line 192
		{
			CBaseEntity *pEnt;  // line 194
			// inlined EntityFromEntityHandle() at line 194
			// inlined CBaseHandle::Get() at line 194
		}
	}
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::Count() at line 191
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 215
}

// game/server/portal2/paint_database.cpp:236 @0x7326c0 _ZN14CPaintDatabase19RemovePaintedEntityEPK11CBaseEntity
void CPaintDatabase::RemovePaintedEntity( const CBaseEntity *pPaintedEntity )
{
	const int index;  // line 239
	// inlined CPaintDatabase::FindPaintedEntity() at line 239
}

// game/server/portal2/paint_database.cpp:248 (declaration)
void RemovePaintedEntity( const CBaseEntity *pPaintedEntity, bool bDeleteData );

// game/server/portal2/paint_database.cpp:248 @0x7325f0 _ZN14CPaintDatabase19RemovePaintedEntityEPK11CBaseEntityb
void CPaintDatabase::RemovePaintedEntity( const CBaseEntity *pPaintedEntity, bool bDeleteData )
{
	const int index;  // line 250
	// inlined CPaintDatabase::FindPaintedEntity() at line 250
}

// game/server/portal2/paint_database.cpp:258 @0x731da0 _ZN14CPaintDatabase19RemovePaintedEntityEib
void CPaintDatabase::RemovePaintedEntity( int index, bool bDeleteData )
{
	CBaseEntity *pEnt;  // line 260
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::Remove() at line 276
	// inlined EntityFromEntityHandle() at line 260
	// inlined CBaseHandle::Get() at line 260
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::operator[]() at line 260
	{
		IPaintableEntity *pPaintableEntity;  // line 263
		// inlined CBaseEntity::SetRenderColor() at line 270
	}
}

// game/server/portal2/paint_database.cpp:281 (declaration)
void RemovePaintedWall( CProjectedWallEntity *pWall, bool bDeleteData );

// game/server/portal2/paint_database.cpp:281 @0x731b30 _ZN14CPaintDatabase17RemovePaintedWallEP20CProjectedWallEntityb
void CPaintDatabase::RemovePaintedWall( CProjectedWallEntity *pWall, bool bDeleteData )
{
	{
		const int index;  // line 314
		// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::Remove() at line 317
		// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::Find() at line 314
	}
}

// game/server/portal2/paint_database.cpp:350 (declaration)
void FindPaintedEntity( const CBaseEntity *pPaintedEntity );

// game/server/portal2/paint_database.cpp:350 @0x7323c0 _ZNK14CPaintDatabase17FindPaintedEntityEPK11CBaseEntity
int CPaintDatabase::FindPaintedEntity( const CBaseEntity *pPaintedEntity )
{
	const int nPaintCount;  // line 352
	{
		int i;  // line 353
		{
			CBaseEntity *pEnt;  // line 355
			// inlined EntityFromEntityHandle() at line 355
			// inlined CBaseHandle::Get() at line 355
		}
	}
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::Count() at line 352
}

// game/server/portal2/paint_database.cpp:366 @0x731b00 _ZN14CPaintDatabase13FireGameEventEP10IGameEvent
void CPaintDatabase::FireGameEvent( IGameEvent *event )
{
}

// game/server/portal2/paint_database.cpp:381 (declaration)
void ClearPaintData();

// game/server/portal2/paint_database.cpp:381 @0x731b10 _ZN14CPaintDatabase14ClearPaintDataEv
void CPaintDatabase::ClearPaintData()
{
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::RemoveAll() at line 383
	// inlined CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> >::RemoveAll() at line 384
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::RemoveAll() at line 385
}

// game/server/portal2/paint_database.cpp:389 @0x736e00 _Z29AscendingPaintLocationComparePK19PaintLocationData_tS1_
int AscendingPaintLocationCompare( const PaintLocationData_t *a, const PaintLocationData_t *b )
{
}

// game/server/portal2/paint_database.cpp:395
ConVar paint_location_distance_threshold_square;

// game/server/portal2/paint_database.cpp:396 (declaration)
void RemoveRepeatedPaintData( PaintLocationVector_t &dataList );

// game/server/portal2/paint_database.cpp:418 sizeof=0x8 (i386)
struct PaingDatabase_PreClientUpdate_DifferentType_t : public std::unary_function<const PaintLocationData_t&,bool>
{
public:
	PaintPowerType m_PowerType; // +0x0  // line 419
	CBaseEntity *m_pBrushEntity; // +0x4  // line 420
	bool operator()( const PaintLocationData_t & ) const;  // line 422
	PaingDatabase_PreClientUpdate_DifferentType_t( CBaseEntity *, PaintPowerType );  // line 427
};

// game/server/portal2/paint_database.cpp:432 sizeof=0x198 (i386)
struct CChangedPaintBoundsCache
{
public:
	// game/server/portal2/paint_database.cpp:435 sizeof=0x18 (i386)
	struct Bounds_t
	{
	public:
		Vector m_vecMins; // +0x0  // line 436
		Vector m_vecMaxs; // +0xc  // line 437
	};  // line 435
	void AddChangedBounds( const Vector &, float );  // line 440
	CUtlVectorFixedGrowable<CChangedPaintBoundsCache::Bounds_t,16ul> m_ChangedBoxes; // +0x0  // line 470
};

// game/server/portal2/paint_database.cpp:432 (declaration)
void CChangedPaintBoundsCache();

// game/server/portal2/paint_database.cpp:432 (declaration)
~CChangedPaintBoundsCache();

// game/server/portal2/paint_database.cpp:474 sizeof=0x9c (i386)
struct CPaintAffectedEntityList : public IEntityEnumerator
{
public:
	virtual bool EnumEntity( IHandleEntity * );  // line 476
	CUtlVectorFixedGrowable<CBaseEntity*,32ul> m_EntitiesToUpdate; // +0x4  // line 495
};

// game/server/portal2/paint_database.cpp:474 (declaration)
void CPaintAffectedEntityList();

// game/server/portal2/paint_database.cpp:474 (declaration)
~CPaintAffectedEntityList();

// game/server/portal2/paint_database.cpp:476 @0x736e30 _ZN24CPaintAffectedEntityList10EnumEntityEP13IHandleEntity
bool CPaintAffectedEntityList::EnumEntity( IHandleEntity *pHandleEntity )
{
	CBaseEntity *pEntity;  // line 481
	// inlined CGlobalEntityList::GetBaseEntity() at line 481
	// inlined CBaseHandle::CBaseHandle() at line 481
	// inlined CUtlVector<CBaseEntity*,CUtlMemoryFixedGrowable<CBaseEntity*, 32ul, int> >::AddToTail() at line 491
}

// game/server/portal2/paint_database.cpp:498 @0x734840 _ZN14CPaintDatabase15PreClientUpdateEv
void CPaintDatabase::PreClientUpdate()
{
	CVProfScope VProf_;  // line 500
	int count;  // line 506
	CChangedPaintBoundsCache boundsCache;  // line 515
	CPaintAffectedEntityList paintEnum;  // line 526
	{
		CBasePlayer *pOtherPlayer;  // line 546
		CSingleUserRecipientFilter filter;  // line 572
		PaintLocationConstIter cbegin;  // line 575
		PaintLocationConstIter cend;  // line 576
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 631
		{
			PaintLocationConstIter currentLocation;  // line 579
			{
				PaintPowerType powerType;  // line 585
				CBaseEntity *pBrushEntity;  // line 588
				const int SIZEOF_FLOAT_PAINT_POSITION;  // line 592
				const int SIZEOF_SHORT_PAINT_POSITION;  // line 593
				const int SIZEOF_PAINT_ENTITY_MSG_HEADER;  // line 594
				const int SIZE_OF_PAINT_ENTITY_MSG_BODY;  // line 595
				PaintLocationConstIter firstDifferent;  // line 597
				const int paintLocationCount;  // line 598
				Vector center;  // line 605
				// inlined std::find_if<const PaintLocationData_t*, PaingDatabase_PreClientUpdate_DifferentType_t>() at line 597
				{
					int j;  // line 619
					{
						const Vector offset;  // line 621
						// inlined Vector::operator-() at line 621
					}
				}
				{
					int j;  // line 606
					// inlined Vector::operator+=() at line 608
				}
				// inlined Vector::operator*=() at line 611
			}
		}
		// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 572
		// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::Sort() at line 569
		// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::RemoveAll() at line 552
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 631
	}
	{
		int i;  // line 532
		{
			CBaseEntity *pEntity;  // line 534
		}
	}
	{
		int i;  // line 527
		// inlined CUtlVector<CChangedPaintBoundsCache::Bounds_t,CUtlMemoryFixedGrowable<CChangedPaintBoundsCache::Bounds_t, 16ul, int> >::operator[]() at line 529
	}
	// inlined CPaintAffectedEntityList::CPaintAffectedEntityList() at line 526
	{
		int i;  // line 516
		{
			PaintLocationData_t data;  // line 518
			float flChangedPaintRadius;  // line 520
			// inlined CChangedPaintBoundsCache::AddChangedBounds() at line 521
		}
	}
	// inlined CChangedPaintBoundsCache::CChangedPaintBoundsCache() at line 515
	// inlined CVProfScope::CVProfScope() at line 500
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::Count() at line 506
	// inlined RemoveRepeatedPaintData() at line 510
	// inlined CVProfScope::~CVProfScope() at line 635
	// inlined CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> >::RemoveAll() at line 635
	// inlined CPaintAffectedEntityList::~CPaintAffectedEntityList() at line 635
	// inlined CChangedPaintBoundsCache::~CChangedPaintBoundsCache() at line 635
	// inlined CPaintAffectedEntityList::~CPaintAffectedEntityList() at line 635
	// inlined CChangedPaintBoundsCache::~CChangedPaintBoundsCache() at line 635
	// inlined CVProfScope::~CVProfScope() at line 635
	// inlined CPaintAffectedEntityList::~CPaintAffectedEntityList() at line 635
	// inlined CChangedPaintBoundsCache::~CChangedPaintBoundsCache() at line 635
}

// game/server/portal2/paint_database.cpp:639 @0x7339b0 _Z17CC_RemoveAllPaintv
CC_RemoveAllPaint()
{
}

// game/server/portal2/paint_database.cpp:644
static ConCommand removeallpaint;

// game/server/portal2/paint_database.cpp:648 @0x7335d0 _Z10CC_PaintAtRK8CCommand
CC_PaintAt( const CCommand &args )
{
	PaintPowerType power;  // line 656
	Vector pos;  // line 658
	trace_t tr;  // line 660
	// inlined CCommand::operator[]() at line 658
	// inlined CCommand::operator[]() at line 658
	// inlined CCommand::operator[]() at line 658
	// inlined Vector::Vector() at line 658
	// inlined Vector::operator=() at line 662
	// inlined Vector::Vector() at line 663
	// inlined Vector::operator=() at line 663
}

// public/tier1/utlvector.h:655 @0x7370e0 _ZN10CUtlVectorIh10CUtlMemoryIhiEE10GrowVectorEi
void CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::GrowVector( int num )
{
	// inlined CUtlMemory<unsigned char,int>::NumAllocated() at line 657
	// inlined CUtlMemory<unsigned char,int>::Grow() at line 660
	// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::ResetDbgInfo() at line 664
}

// game/server/portal2/paint_database.cpp:672
static ConCommand paintat;

// game/server/portal2/paint_database.cpp:676 (declaration)
void RLEEncodeSave<unsigned char>( ISave *pSave, unsigned char *pArray, int count );

// game/server/portal2/paint_database.cpp:705 @0x735b90 _ZN14CPaintDatabase16SavePaintmapDataEP5ISave
void CPaintDatabase::SavePaintmapData( ISave *pSave )
{
	CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> > data;  // line 707
	int nPaintData;  // line 709
	CVarBitVec paintSurfBits;  // line 723
	int numBits;  // line 727
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::~CUtlVector() at line 733
	// inlined CVarBitVec::~CVarBitVec() at line 733
	// inlined ISave::WriteInt() at line 731
	// inlined CVarBitVec::CVarBitVec() at line 723
	{
		int i;  // line 712
		{
			int count;  // line 716
			// inlined RLEEncodeSave<unsigned char>() at line 718
			// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::Base() at line 718
		}
	}
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::CUtlVector() at line 707
	// inlined CVarBitVec::~CVarBitVec() at line 733
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::~CUtlVector() at line 733
}

// game/server/portal2/paint_database.cpp:738 (declaration)
void RLEDecodeRestore<unsigned char>( IRestore *pRestore, unsigned char *pArray, int count );

// game/server/portal2/paint_database.cpp:758 @0x7339d0 _ZN14CPaintDatabase19RestorePaintmapDataEP8IRestore
void CPaintDatabase::RestorePaintmapData( IRestore *pRestore )
{
	int nPaintData;  // line 760
	CVarBitVec paintSurfBits;  // line 775
	// inlined CVarBitVec::~CVarBitVec() at line 795
	{
		int numBits;  // line 779
		int numIntsInStream;  // line 782
		int readSize;  // line 783
		{
			int ignored;  // line 789
		}
		// inlined IRestore::ReadInt() at line 784
		// inlined CVarBitVecBase::Resize() at line 780
	}
	// inlined CVarBitVec::CVarBitVec() at line 775
	{
		int i;  // line 764
		{
			int count;  // line 768
			// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::Base() at line 770
			// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::operator[]() at line 769
			// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::SetSize() at line 769
			// inlined RLEDecodeRestore<unsigned char>() at line 770
		}
	}
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::SetSize() at line 761
	// inlined CVarBitVec::~CVarBitVec() at line 795
}

// game/server/portal2/paint_database.cpp:799 @0x733dc0 _ZN14CPaintDatabase15SendPaintDataToEP11CBasePlayer
void CPaintDatabase::SendPaintDataTo( CBasePlayer *pPlayer )
{
	{
		CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> > data;  // line 803
		CSingleUserRecipientFilter filter;  // line 806
		CUtlVector<float,CUtlMemory<float, int> > msgLength;  // line 809
		CUtlVector<unsigned char,CUtlMemory<unsigned char, int> > msgVal;  // line 810
		const int SIZEOF_PAINT_ID;  // line 812
		const int SIZEOF_PAINT_OFFSET;  // line 813
		const int SIZEOF_RLE_DATA;  // line 814
		int MAX_RLE;  // line 815
		int nPaintmap;  // line 818
		CVarBitVec paintSurfBits;  // line 873
		int numBits;  // line 875
		// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::~CUtlVector() at line 885
		{
			int i;  // line 880
		}
		// inlined CVarBitVecBase::GetNumBits() at line 875
		// inlined CVarBitVec::CVarBitVec() at line 873
		{
			int n;  // line 819
			{
				int count;  // line 821
				int total;  // line 822
				int nRLE;  // line 824
				int nOffset;  // line 825
				// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::Count() at line 821
				// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::operator[]() at line 821
				{
					int i;  // line 826
					{
						int length;  // line 828
						uint8 val;  // line 829
						// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::RemoveAll() at line 865
						{
							int m;  // line 854
						}
						// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::AddToTail() at line 840
						// inlined CUtlVector<float,CUtlMemory<float, int> >::AddToTail() at line 839
						// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::operator[]() at line 829
					}
				}
			}
		}
		// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::Count() at line 818
		// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::CUtlVector() at line 810
		// inlined CUtlVector<float,CUtlMemory<float, int> >::CUtlVector() at line 809
		// inlined CSingleUserRecipientFilter::CSingleUserRecipientFilter() at line 806
		// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::CUtlVector() at line 803
		// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::~CUtlVector() at line 885
		// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::~CUtlVector() at line 885
		// inlined CUtlVector<unsigned char,CUtlMemory<unsigned char, int> >::~CUtlVector() at line 885
		// inlined CUtlVector<float,CUtlMemory<float, int> >::~CUtlVector() at line 885
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 885
		// inlined CUtlVector<float,CUtlMemory<float, int> >::~CUtlVector() at line 885
		// inlined CVarBitVec::~CVarBitVec() at line 885
		// inlined CSingleUserRecipientFilter::~CSingleUserRecipientFilter() at line 885
		// inlined CVarBitVec::~CVarBitVec() at line 885
	}
}

// public/tier1/utlvector.h:865 @0x737200 _ZN10CUtlVectorIS_Ih10CUtlMemoryIhiEES0_IS2_iEE8SetCountEi
void CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::SetCount( int count )
{
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::RemoveAll() at line 867
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::AddMultipleToTail() at line 868
}

// game/server/portal2/paint_database.cpp:891 (declaration)
void SendPaintDataToEngine();

// game/server/portal2/paint_database.cpp:891 @0x732780 _ZN14CPaintDatabase21SendPaintDataToEngineEv
void CPaintDatabase::SendPaintDataToEngine()
{
	{
		int i;  // line 893
		// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::Count() at line 893
	}
	// inlined CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> >::Purge() at line 898
}

// game/server/portal2/paint_database.cpp:902 @0x733760 _Z19CC_PaintAllSurfacesRK8CCommand
CC_PaintAllSurfaces( const CCommand &args )
{
	PaintPowerType power;  // line 904
	CBroadcastRecipientFilter filter;  // line 913
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 920
	// inlined CBroadcastRecipientFilter::CBroadcastRecipientFilter() at line 913
	// inlined CBroadcastRecipientFilter::~CBroadcastRecipientFilter() at line 920
}

// game/server/portal2/paint_database.cpp:923
static ConCommand paintallsurfaces;
