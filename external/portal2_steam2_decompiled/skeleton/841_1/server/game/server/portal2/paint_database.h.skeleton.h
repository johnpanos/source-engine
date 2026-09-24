// DWARF declaration skeleton for game/server/portal2/paint_database.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/paint_database.h:13 sizeof=0x20 (i386)
struct PaintLocationData_t
{
public:
	CBaseEntity *pBrushEntity; // +0x0  // line 14
	PaintPowerType type; // +0x4  // line 15
	Vector location; // +0x8  // line 16
	Vector normal; // +0x14  // line 17
};

// game/server/portal2/paint_database.h:13 (declaration)
~PaintLocationData_t();

// game/server/portal2/paint_database.h:13 (declaration)
void operator=( const PaintLocationData_t & );

// game/server/portal2/paint_database.h:21 sizeof=0x4 (i386)
struct PaintEntityData_t
{
public:
	CBaseHandle hEnt; // +0x0  // line 22
};

// game/server/portal2/paint_database.h:21 (declaration)
~PaintEntityData_t();

// game/server/portal2/paint_database.h:21 (declaration)
void PaintEntityData_t( const PaintEntityData_t & );

// game/server/portal2/paint_database.h:27
typedef const PaintLocationData_t *PaintLocationConstIter;

// game/server/portal2/paint_database.h:29
typedef CUtlVector<PaintLocationData_t,CUtlMemory<PaintLocationData_t, int> > PaintLocationVector_t;

// game/server/portal2/paint_database.h:30
typedef CUtlVector<PaintEntityData_t,CUtlMemory<PaintEntityData_t, int> > PaintEntityVector_t;

// game/server/portal2/paint_database.h:31
typedef CUtlVector<CProjectedWallEntity*,CUtlMemory<CProjectedWallEntity*, int> > ProjectedWallVector_t;

// game/server/portal2/paint_database.h:33
typedef CUtlVector<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >,CUtlMemory<CUtlVector<unsigned char, CUtlMemory<unsigned char, int> >, int> > PaintmapDataVector_t;

// game/server/portal2/paint_database.h:36 sizeof=0x6c (i386)
struct CPaintDatabase : public CAutoGameSystemPerFrame, public CGameEventListener
{
public:
	CPaintDatabase( const char * );  // line 38
	virtual ~CPaintDatabase();  // line 39
	virtual const char *Name();  // line 42
	virtual void LevelInitPreEntity();  // line 43
	virtual void LevelInitPostEntity();  // line 44
	virtual void LevelShutdownPostEntity();  // line 45
	virtual void FireGameEvent( IGameEvent * );  // line 48
	void AddPaint( const trace_t &, PaintPowerType );  // line 50
	void PaintEntity( CBaseEntity *, PaintPowerType, const Vector & );  // line 51
	void RemovePaintedEntity( const CBaseEntity * );  // line 53
	void RemovePaintedEntity( const CBaseEntity *, bool );  // line 54
	void RemovePaintedWall( CProjectedWallEntity *, bool );  // line 55
	void RemoveAllPaint();  // line 57
	virtual void PreClientUpdate();  // line 59
	const ProjectedWallVector_t *GetPaintedProjectedWalls() const;  // line 61
	void SavePaintmapData( ISave * );  // line 63
	void RestorePaintmapData( IRestore * );  // line 64
	void SendPaintDataTo( CBasePlayer * );  // line 66
private:
	void PaintProjectedWall( CProjectedWallEntity *, PaintPowerType, const Vector & );  // line 69
	void ClearPaintData();  // line 71
	void RemovePaintedEntity( int, bool );  // line 72
	int FindPaintedEntity( const CBaseEntity * ) const;  // line 73
	void SendPaintDataToEngine();  // line 75
	PaintLocationVector_t m_PaintThisFrame; // +0x18  // line 77
	PaintEntityVector_t m_PaintedEntities; // +0x2c  // line 78
	ProjectedWallVector_t m_PaintedProjectedWalls; // +0x40  // line 79
	bool m_bCanPaint; // +0x54  // line 80
	bool m_bSentClientPaintData; // +0x55  // line 81
	PaintmapDataVector_t m_Paintmaps; // +0x58  // line 83
};

// game/server/portal2/paint_database.h:42 @0x736df0 _ZN14CPaintDatabase4NameEv
const char *CPaintDatabase::Name()
{
}
