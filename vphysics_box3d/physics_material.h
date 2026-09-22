#ifndef PHYSICS_MATERIAL_H
#define PHYSICS_MATERIAL_H

#include "vphysics_interface.h"
#include "utlsymbol.h"
#include "utlvector.h"

// Surface-property database for the Box3D provider.
//
// This is authored-content bookkeeping (surfaceprops*.txt -> friction, sounds,
// game material per named surface), not physics simulation, so it is a full
// working database rather than a stub. Game code unconditionally dereferences
// the surfacedata_t returned by GetSurfaceData() (player CategorizePosition,
// footsteps, impacts), so it must always resolve to a valid record. The parse
// rules mirror the shared IVP database exactly; only the IVP_Material coupling
// is dropped. It also registers itself with the shared vcollide parser so that
// authored surfaceprop names inside .phy blocks resolve through this database.

// One named surface record. IVP's version derives from IVP_Material; the Box3D
// database stores the same authored data without that coupling.
struct CSurfaceBox3D
{
	CUtlSymbol m_name;
	surfacedata_t data;
};

class CPhysicsSurfacePropsBox3D : public IPhysicsSurfaceProps
{
public:
	CPhysicsSurfacePropsBox3D();
	virtual ~CPhysicsSurfacePropsBox3D();

	virtual int ParseSurfaceData( const char *pFilename, const char *pTextfile ) override;
	virtual int SurfacePropCount( void ) const override;

	virtual int GetSurfaceIndex( const char *pSurfacePropName ) const override;
	virtual void GetPhysicsProperties( int surfaceDataIndex, float *density, float *thickness, float *friction, float *elasticity ) const override;

	virtual surfacedata_t *GetSurfaceData( int surfaceDataIndex ) override;
	virtual const char *GetString( unsigned short stringTableIndex ) const override;
	virtual const char *GetPropName( int surfaceDataIndex ) const override;

	virtual void SetWorldMaterialIndexTable( int *pMapArray, int mapSize ) override;
	virtual void GetPhysicsParameters( int surfaceDataIndex, surfacephysicsparams_t *pParamsOut ) const override;

private:
	const CSurfaceBox3D *GetInternalSurface( int materialIndex ) const;
	CSurfaceBox3D *GetInternalSurface( int materialIndex );
	void CopyPhysicsProperties( CSurfaceBox3D *pOut, int baseIndex );
	bool AddFileToDatabase( const char *pFilename );
	bool IsReservedMaterialIndex( int materialIndex ) const;
	const char *GetReservedMaterialName( int materialIndex ) const;
	int GetReservedFallBack( int materialIndex ) const;
	int GetReservedSurfaceIndex( const char *pPropertyName ) const;

	CUtlSymbolTable m_strings;
	CUtlVector<CSurfaceBox3D> m_props;
	CUtlVector<CUtlSymbol> m_fileList;
	bool m_init;
	int m_shadowFallback;
};

// The module's single surface database (exported as VPhysicsSurfaceProps001);
// objects read friction/elasticity from it when they are created.
extern CPhysicsSurfacePropsBox3D g_SurfaceDatabase;

#endif // PHYSICS_MATERIAL_H
