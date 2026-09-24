//========= Portal 2 reconstruction ============================================//
//
// Purpose: Save/restore block handler for paintmaps and painted projected walls.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "isaverestore.h"
#include "paint_saverestore.h"

#if !defined( CLIENT_DLL )
#include "paint_database.h"
#include "paint_stream_manager.h"
#include "projectedwallentity.h"
#include "portal_base2d.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if !defined( CLIENT_DLL )
ConVar save_paintblob( "save_paintblob", "0", FCVAR_DEVELOPMENTONLY );
#endif


class CPaintSaveRestoreBlockHandler : public CDefSaveRestoreBlockHandler
{
	virtual const char *GetBlockName() { return "PaintDatabase"; }

	virtual void PreSave( CSaveRestoreData * ) {}

	virtual void Save( ISave *pSave )
	{
		if ( !engine->HasPaintmap() )
			return;

#if !defined( CLIENT_DLL )
		if ( save_paintblob.GetBool() )
		{
			PaintStreamManager.SavePaintBlobState( pSave );
		}

		PaintDatabase.SavePaintmapData( pSave );

		// Painted projected walls aren't part of the paintmap, so save their painted segments
		const ProjectedWallVector_t *pProjectedWalls = PaintDatabase.GetPaintedProjectedWalls();

		{
			int count = pProjectedWalls->Count();
			pSave->WriteInt( &count );

			for ( int i = 0; i < count; ++i )
			{
				CProjectedWallEntity *pWall = (*pProjectedWalls)[i];
				if ( !pWall )
					continue;

				pSave->StartBlock();

				// Walls projected through a portal are recreated on load, so identify them by that portal
				bool bHasSourcePortal = pWall->GetSourcePortal() != NULL;
				pSave->WriteBool( &bHasSourcePortal );
				if ( bHasSourcePortal )
				{
					EHANDLE hPortal = pWall->GetSourcePortal();
					pSave->WriteEHandle( &hPortal );
				}
				else
				{
					EHANDLE hProjectedWall = pWall;
					pSave->WriteEHandle( &hProjectedWall );
				}

				int nSegments = pWall->GetNumSegments();

				int nPaintedSegments = 0;
				for ( int s = 0; s < nSegments; ++s )
				{
					PaintPowerType type = pWall->GetPaintPowerAtSegment( s );
					if ( type != NO_POWER )
					{
						++nPaintedSegments;
					}
				}

				pSave->WriteInt( &nPaintedSegments );

				float flSegmentLength = pWall->GetSegmentLength();
				Vector vDirection = pWall->GetEndPoint() - pWall->GetStartPoint();
				vDirection.NormalizeInPlace();
				for ( int s = 0; s < nSegments; ++s )
				{
					int type = pWall->GetPaintPowerAtSegment( s );
					if ( type != NO_POWER )
					{
						Vector vSegmentCenter = pWall->GetStartPoint() + ( ( s + 0.5f ) * flSegmentLength ) * vDirection;
						pSave->WriteInt( &type );
						pSave->WriteVector( &vSegmentCenter );
					}
				}

				pSave->EndBlock();
			}
		}
#endif
	}

	virtual void WriteSaveHeaders( ISave * ) {}

	virtual void PostSave() {}

	virtual void PreRestore()
	{
#if !defined( CLIENT_DLL )
		PaintDatabase.RemoveAllPaint();
#endif
	}

	virtual void ReadRestoreHeaders( IRestore * ) {}

	virtual void Restore( IRestore *pRestore, bool fCreatePlayers )
	{
		if ( !engine->HasPaintmap() )
			return;

#if !defined( CLIENT_DLL )
		if ( save_paintblob.GetBool() )
		{
			PaintStreamManager.RestorePaintBlobState( pRestore );
		}

		PaintDatabase.RestorePaintmapData( pRestore );

		// Get all the projected walls in the level
		ProjectedWallVector_t projectedWalls;
		CBaseEntity *pEnt = NULL;
		while ( ( pEnt = gEntList.FindEntityByClassname( pEnt, "projected_wall_entity" ) ) != NULL )
		{
			projectedWalls.AddToTail( dynamic_cast< CProjectedWallEntity* >( pEnt ) );
		}

		int nWalls = projectedWalls.Count();

		// Repaint the saved wall segments
		int paintedWallCount = pRestore->ReadInt();
		for ( int i = 0; i < paintedWallCount; ++i )
		{
			pRestore->StartBlock();

			bool bHasSourcePortal;
			pRestore->ReadBool( &bHasSourcePortal );

			CProjectedWallEntity *pPaintedWall = NULL;
			if ( bHasSourcePortal )
			{
				EHANDLE hPortal;
				pRestore->ReadEHandle( &hPortal );

				CPortal_Base2D *pSourcePortal = dynamic_cast< CPortal_Base2D* >( hPortal.Get() );
				if ( pSourcePortal )
				{
					for ( int w = 0; w < nWalls; ++w )
					{
						if ( projectedWalls[w]->GetSourcePortal() == pSourcePortal )
						{
							pPaintedWall = projectedWalls[w];
							break;
						}
					}
				}
			}
			else
			{
				EHANDLE hWall;
				pRestore->ReadEHandle( &hWall );

				pPaintedWall = dynamic_cast< CProjectedWallEntity* >( hWall.Get() );
			}

			int nPaintedSegments = pRestore->ReadInt();
			PaintPowerType type;
			trace_t tr;
			tr.m_pEnt = pPaintedWall;
			for ( int s = 0; s < nPaintedSegments; ++s )
			{
				type = static_cast< PaintPowerType >( pRestore->ReadInt() );
				pRestore->ReadVector( &tr.endpos, 1 );
				PaintDatabase.AddPaint( tr, type );
			}

			pRestore->EndBlock();
		}
#endif
	}

	virtual void PostRestore() {}
};


CPaintSaveRestoreBlockHandler g_PaintSaveRestoreBlockHandler;

ISaveRestoreBlockHandler *GetPaintSaveRestoreBlockHandler()
{
	return &g_PaintSaveRestoreBlockHandler;
}
