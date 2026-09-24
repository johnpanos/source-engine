// DWARF declaration skeleton for game/shared/portal2/paint_saverestore.cpp
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// None:0 @0x1be00 _Z41__static_initialization_and_destruction_0ii
__static_initialization_and_destruction_0( int __initialize_p, int __priority )
{
	// inlined Color::Color() at line 126
	// inlined Vector2D::Vector2D() at line 146
	// inlined Vector2D::Vector2D() at line 147
	// inlined Vector4D::Vector4D() at line 137
	// inlined Vector4D::Vector4D() at line 138
	// inlined CSharedVarSaveDataOps::CSharedVarSaveDataOps() at line 1154
	// inlined CPaintSaveRestoreBlockHandler::CPaintSaveRestoreBlockHandler() at line 211
}

// game/shared/portal2/paint_saverestore.cpp:21
ConVar save_paintblob;

// game/shared/portal2/paint_saverestore.cpp:25 sizeof=0x4 (i386)
struct CPaintSaveRestoreBlockHandler : public CDefSaveRestoreBlockHandler
{
private:
	virtual const char *GetBlockName();  // line 26
	virtual void PreSave( CSaveRestoreData * );  // line 28
	virtual void Save( ISave * );  // line 30
	virtual void WriteSaveHeaders( ISave * );  // line 110
	virtual void PostSave();  // line 112
	virtual void PreRestore();  // line 114
	virtual void ReadRestoreHeaders( IRestore * );  // line 121
	virtual void Restore( IRestore *, bool );  // line 123
	virtual void PostRestore();  // line 206
};

// game/shared/portal2/paint_saverestore.cpp:25 (declaration)
void CPaintSaveRestoreBlockHandler();

// game/shared/portal2/paint_saverestore.cpp:26 @0x24c980 _ZN29CPaintSaveRestoreBlockHandler12GetBlockNameEv
const char *CPaintSaveRestoreBlockHandler::GetBlockName()
{
}

// game/shared/portal2/paint_saverestore.cpp:28 @0x24c990 _ZN29CPaintSaveRestoreBlockHandler7PreSaveEP16CSaveRestoreData
void CPaintSaveRestoreBlockHandler::PreSave( CSaveRestoreData * )
{
}

// game/shared/portal2/paint_saverestore.cpp:30 @0x24ca00 _ZN29CPaintSaveRestoreBlockHandler4SaveEP5ISave
void CPaintSaveRestoreBlockHandler::Save( ISave *pSave )
{
	const ProjectedWallVector_t *pProjectedWalls;  // line 47
	{
		int count;  // line 50
		{
			int i;  // line 53
			{
				CProjectedWallEntity *pWall;  // line 55
				{
					bool bHasSourcePortal;  // line 61
					int nSegments;  // line 74
					int nPaintedSegments;  // line 76
					float flSegmentLength;  // line 87
					Vector vDirection;  // line 88
					// inlined Vector::operator-() at line 88
					{
						int s;  // line 77
						{
							PaintPowerType type;  // line 79
						}
					}
					{
						EHANDLE hPortal;  // line 65
						// inlined CHandle<CBaseEntity>::CHandle() at line 65
					}
					// inlined Vector::NormalizeInPlace() at line 89
					{
						int s;  // line 90
						{
							int type;  // line 92
							{
								Vector vSegmentCenter;  // line 95
								// inlined operator*() at line 95
								// inlined Vector::operator+() at line 95
							}
						}
					}
					{
						EHANDLE hProjectedWall;  // line 70
						// inlined CHandle<CBaseEntity>::CHandle() at line 70
					}
				}
			}
		}
	}
}

// game/shared/portal2/paint_saverestore.cpp:110 @0x24c9a0 _ZN29CPaintSaveRestoreBlockHandler16WriteSaveHeadersEP5ISave
void CPaintSaveRestoreBlockHandler::WriteSaveHeaders( ISave * )
{
}

// game/shared/portal2/paint_saverestore.cpp:112 @0x24c9b0 _ZN29CPaintSaveRestoreBlockHandler8PostSaveEv
void CPaintSaveRestoreBlockHandler::PostSave()
{
}

// game/shared/portal2/paint_saverestore.cpp:114 @0x24c9e0 _ZN29CPaintSaveRestoreBlockHandler10PreRestoreEv
void CPaintSaveRestoreBlockHandler::PreRestore()
{
}

// game/shared/portal2/paint_saverestore.cpp:121 @0x24c9c0 _ZN29CPaintSaveRestoreBlockHandler18ReadRestoreHeadersEP8IRestore
void CPaintSaveRestoreBlockHandler::ReadRestoreHeaders( IRestore * )
{
}

// game/shared/portal2/paint_saverestore.cpp:123 @0x24ce90 _ZN29CPaintSaveRestoreBlockHandler7RestoreEP8IRestoreb
void CPaintSaveRestoreBlockHandler::Restore( IRestore *pRestore, bool fCreatePlayers )
{
	ProjectedWallVector_t projectedWalls;  // line 140
	CBaseEntity *pEnt;  // line 141
	int nWalls;  // line 146
	int paintedWallCount;  // line 149
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::~CUtlVector() at line 201
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::Purge() at line 201
	{
		int i;  // line 150
		{
			bool bHasSourcePortal;  // line 154
			CProjectedWallEntity *pPaintedWall;  // line 158
			int nPaintedSegments;  // line 186
			PaintPowerType type;  // line 187
			trace_t tr;  // line 188
			{
				int s;  // line 190
			}
			{
				EHANDLE hWall;  // line 179
				// inlined CHandle<CBaseEntity>::CHandle() at line 179
				// inlined CHandle<CBaseEntity>::Get() at line 181
			}
			{
				EHANDLE hPortal;  // line 161
				CPortal_Base2D *pSourcePortal;  // line 163
				// inlined CHandle<CBaseEntity>::CHandle() at line 161
				// inlined CHandle<CBaseEntity>::Get() at line 163
				{
					int w;  // line 167
				}
			}
		}
	}
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::Count() at line 146
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::AddToTail() at line 144
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::CUtlVector() at line 140
	// inlined CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> >::~CUtlVector() at line 201
}

// game/shared/portal2/paint_saverestore.cpp:206 @0x24c9d0 _ZN29CPaintSaveRestoreBlockHandler11PostRestoreEv
void CPaintSaveRestoreBlockHandler::PostRestore()
{
}

// game/shared/portal2/paint_saverestore.cpp:211
CPaintSaveRestoreBlockHandler g_PaintSaveRestoreBlockHandler;

// game/shared/portal2/paint_saverestore.cpp:213 @0x24c970 _Z31GetPaintSaveRestoreBlockHandlerv
ISaveRestoreBlockHandler *GetPaintSaveRestoreBlockHandler()
{
}

// public/tier1/utlmemory.h:707 @0x24cdb0 _ZN10CUtlMemoryIP20CProjectedWallEntityiE4GrowEi
void CUtlMemory<CProjectedWallEntity*,int>::Grow( int num )
{
	int nAllocationRequested;  // line 720
	int nNewAllocationCount;  // line 724
	// inlined UtlMemory_CalcNewAllocationCount() at line 724
	// inlined CUtlMemory<CProjectedWallEntity*,int>::IsExternallyAllocated() at line 711
	// inlined MemAlloc_Alloc() at line 761
}
