#ifndef PHYSICS_MATERIAL_H
#define PHYSICS_MATERIAL_H

#include "vphysics_interface.h"

class CPhysicsSurfacePropsBox3D : public IPhysicsSurfaceProps
{
public:
	CPhysicsSurfacePropsBox3D() {}
	virtual ~CPhysicsSurfacePropsBox3D() {}

	virtual int ParseSurfaceData( const char *pFilename, const char *pTextfile ) override { return 0; }
	virtual int SurfacePropCount( void ) const override { return 0; }

	virtual int GetSurfaceIndex( const char *pSurfacePropName ) const override { return 0; }
	virtual void GetPhysicsProperties( int surfaceDataIndex, float *density, float *thickness, float *friction, float *elasticity ) const override {}

	virtual surfacedata_t *GetSurfaceData( int surfaceDataIndex ) override { return nullptr; }
	virtual const char *GetString( unsigned short stringTableIndex ) const override { return ""; }
	virtual const char *GetPropName( int surfaceDataIndex ) const override { return ""; }

	virtual void SetWorldMaterialIndexTable( int *pMapArray, int mapSize ) override {}
	virtual void GetPhysicsParameters( int surfaceDataIndex, surfacephysicsparams_t *pParamsOut ) const override {}
};

#endif // PHYSICS_MATERIAL_H
