//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server-side record of painted surfaces, entities and projected walls.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PAINT_DATABASE_H
#define PAINT_DATABASE_H
#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "GameEventListener.h"
#include "utlvector.h"
#include "paint_color_manager.h"

class CBaseEntity;
class CBasePlayer;
class CProjectedWallEntity;
class ISave;
class IRestore;

struct PaintLocationData_t
{
	CBaseEntity *pBrushEntity;
	PaintPowerType type;
	Vector location;
	Vector normal;
};


struct PaintEntityData_t
{
	CBaseHandle hEnt;
};


typedef const PaintLocationData_t *PaintLocationConstIter;

typedef CUtlVector<PaintLocationData_t> PaintLocationVector_t;
typedef CUtlVector<PaintEntityData_t> PaintEntityVector_t;
typedef CUtlVector<CProjectedWallEntity *> ProjectedWallVector_t;

class CPaintDatabase : public CAutoGameSystemPerFrame, public CGameEventListener
{
public:
	CPaintDatabase( const char *name );
	virtual ~CPaintDatabase();

	// CAutoGameSystemPerFrame
	virtual char const *Name() { return "PaintDatabase"; }
	virtual void LevelInitPreEntity();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPostEntity();

	// CGameEventListener
	virtual void FireGameEvent( IGameEvent *event );

	void AddPaint( const trace_t& tr, PaintPowerType powerType );
	void PaintEntity( CBaseEntity *pPaintedEntity, PaintPowerType newPowerType, const Vector &vecPosition );

	void RemovePaintedEntity( const CBaseEntity *pPaintedEntity );
	void RemovePaintedEntity( const CBaseEntity *pPaintedEntity, bool bDeleteData );
	void RemovePaintedWall( CProjectedWallEntity *pWall, bool bDeleteData );

	void RemoveAllPaint();

	virtual void PreClientUpdate();

	const ProjectedWallVector_t *GetPaintedProjectedWalls() const { return &m_PaintedProjectedWalls; }

	void SavePaintmapData( ISave *pSave );
	void RestorePaintmapData( IRestore *pRestore );

	void SendPaintDataTo( CBasePlayer *pPlayer );

private:
	void PaintProjectedWall( CProjectedWallEntity *pWall, PaintPowerType powerType, const Vector &vecPosition );

	void ClearPaintData();
	void RemovePaintedEntity( int index, bool bDeleteData );
	int FindPaintedEntity( const CBaseEntity *pPaintedEntity ) const;

	void SendPaintDataToEngine();

	PaintLocationVector_t m_PaintThisFrame;
	PaintEntityVector_t m_PaintedEntities;
	ProjectedWallVector_t m_PaintedProjectedWalls;
	bool m_bCanPaint;
	bool m_bSentClientPaintData;

	// Restored paint records, loaded into the engine once it has paint maps.
	CUtlVector<uint32> m_PendingPaintmapRLE;
};

extern CPaintDatabase PaintDatabase;

#endif // PAINT_DATABASE_H
