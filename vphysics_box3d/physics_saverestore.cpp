//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D save/restore (RFC 0004 C5).
//
// The game writes each physics interface through IPhysicsEnvironment::Save
// into its save stream and reads the records back, in the same order, into
// a fresh environment. The stream contents are the provider's own; these are
// versioned Box3D records written with ISave primitives. Coverage matches
// IVP (vphysics/vphysics_saverestore.cpp): objects (with their shadow
// controller), springs, constraint groups, constraints and motion
// controllers; fluids, player controllers and standalone shadow controllers
// are recreated by the game and report false, as IVP does.
//
// Records refer to each other by the pointer each interface had when saved.
// Every record starts with that pointer, and restoring maps it to the new
// object; PreRestore seeds the map with objects the game re-created itself
// (the world). References resolve through the map; unknown ones become NULL.
//
//=============================================================================//
#include "physics_environment.h"

#include <string.h>

#include "box3d/box3d.h"
#include "isaverestore.h"
#include "physics_constraint.h"
#include "physics_controllers.h"
#include "physics_fluid.h"
#include "physics_material.h"
#include "physics_object.h"
#include "physics_vehicle.h"
#include "utlmap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const int kRecordVersion = 1;

// Old -> new interface pointers for the restore in progress.
CUtlMap<void *, void *> s_pointerMap( 0, 0, DefLessFunc( void * ) );
// Deferred to PostRestore, as IVP defers them: collisions are enabled once
// every dependent record (controllers, constraints) has been restored, and
// groups activate once their constraints exist.
CUtlVector<CPhysicsObjectBox3D *> s_enableCollisions;
CUtlVector<CConstraintGroupBox3D *> s_activateGroups;

void *Remap( void *pOld )
{
	if ( !pOld )
		return NULL;
	int index = s_pointerMap.Find( pOld );
	return index != s_pointerMap.InvalidIndex() ? s_pointerMap[index] : NULL;
}

void WritePointer( ISave *pSave, const void *pPointer )
{
	pSave->WriteData( (const char *)&pPointer, sizeof( pPointer ) );
}

void *ReadPointer( IRestore *pRestore )
{
	void *pPointer = NULL;
	pRestore->ReadData( (char *)&pPointer, sizeof( pPointer ), 0 );
	return pPointer;
}

void WriteMaterial( ISave *pSave, int materialIndex )
{
	const char *pName = g_SurfaceDatabase.GetPropName( materialIndex );
	if ( !pName )
		pName = g_SurfaceDatabase.GetPropName( 0 );
	int length = (int)strlen( pName ) + 1;
	pSave->WriteInt( &length );
	pSave->WriteString( pName );
}

int ReadMaterial( IRestore *pRestore )
{
	char name[256];
	int length = pRestore->ReadInt();
	pRestore->ReadString( name, sizeof( name ), length );
	int index = g_SurfaceDatabase.GetSurfaceIndex( name );
	return index < 0 ? 0 : index;
}

template <typename T>
void WriteBlock( ISave *pSave, const T &value )
{
	pSave->WriteData( (const char *)&value, sizeof( T ) );
}

template <typename T>
void ReadBlock( IRestore *pRestore, T &value )
{
	pRestore->ReadData( (char *)&value, sizeof( T ), 0 );
}
}

bool CPhysicsEnvironmentBox3D::Save( const physsaveparams_t &params )
{
	ISave *pSave = params.pSave;
	if ( !pSave || !params.pObject )
		return false;
	switch ( params.type )
	{
	case PIID_IPHYSICSOBJECT:
	{
		CPhysicsObjectBox3D *pObject = ToBox3D( (IPhysicsObject *)params.pObject );
		WritePointer( pSave, params.pObject );
		pSave->WriteInt( &kRecordVersion );
		CPhysicsObjectStateBox3D state;
		pObject->WriteState( state );
		state.pCollide = NULL;		// the game supplies the model on restore
		WriteBlock( pSave, state );
		WriteMaterial( pSave, state.materialIndex );
		CShadowControllerBox3D *pShadow = pObject->GetShadow();
		bool hasShadow = pShadow != NULL;
		pSave->WriteBool( &hasShadow );
		if ( pShadow )
		{
			CShadowControllerBox3D::State_t shadow;
			pShadow->WriteState( shadow );
			WriteBlock( pSave, shadow );
			WriteMaterial( pSave, shadow.savedMaterialIndex );
		}
		return true;
	}
	case PIID_IPHYSICSCONSTRAINTGROUP:
	{
		CConstraintGroupBox3D *pGroup = static_cast<CConstraintGroupBox3D *>( (IPhysicsConstraintGroup *)params.pObject );
		WritePointer( pSave, params.pObject );
		pSave->WriteInt( &kRecordVersion );
		WriteBlock( pSave, pGroup->GetParams() );
		bool active = pGroup->IsActive();
		pSave->WriteBool( &active );
		return true;
	}
	case PIID_IPHYSICSCONSTRAINT:
	{
		CConstraintBox3D *pConstraint = static_cast<CConstraintBox3D *>( (IPhysicsConstraint *)params.pObject );
		WritePointer( pSave, params.pObject );
		pSave->WriteInt( &kRecordVersion );
		// An inert constraint (its object was deleted) saves only its type.
		int type = pConstraint->IsInert() ? CONSTRAINT_BOX3D_UNKNOWN : pConstraint->GetType();
		pSave->WriteInt( &type );
		if ( type == CONSTRAINT_BOX3D_UNKNOWN )
			return true;
		WritePointer( pSave, pConstraint->GetGroup() );
		WritePointer( pSave, pConstraint->GetReferenceObject() );
		WritePointer( pSave, pConstraint->GetAttachedObject() );
		switch ( type )
		{
		case CONSTRAINT_BOX3D_RAGDOLL:
		{
			constraint_ragdollparams_t ragdoll;
			pConstraint->WriteRagdoll( ragdoll );
			WriteBlock( pSave, ragdoll );
			break;
		}
		case CONSTRAINT_BOX3D_HINGE:
		{
			constraint_hingeparams_t hinge;
			pConstraint->WriteHinge( hinge );
			WriteBlock( pSave, hinge );
			break;
		}
		case CONSTRAINT_BOX3D_FIXED:
		{
			constraint_fixedparams_t fixed;
			pConstraint->WriteFixed( fixed );
			WriteBlock( pSave, fixed );
			break;
		}
		case CONSTRAINT_BOX3D_BALLSOCKET:
		{
			constraint_ballsocketparams_t ballsocket;
			pConstraint->WriteBallsocket( ballsocket );
			WriteBlock( pSave, ballsocket );
			break;
		}
		case CONSTRAINT_BOX3D_SLIDING:
		{
			constraint_slidingparams_t sliding;
			pConstraint->WriteSliding( sliding );
			WriteBlock( pSave, sliding );
			break;
		}
		case CONSTRAINT_BOX3D_PULLEY:
		{
			constraint_pulleyparams_t pulley;
			pConstraint->WritePulley( pulley );
			WriteBlock( pSave, pulley );
			break;
		}
		case CONSTRAINT_BOX3D_LENGTH:
		{
			constraint_lengthparams_t length;
			pConstraint->WriteLength( length );
			WriteBlock( pSave, length );
			break;
		}
		}
		return true;
	}
	case PIID_IPHYSICSSPRING:
	{
		CPhysicsSpringBox3D *pSpring = static_cast<CPhysicsSpringBox3D *>( (IPhysicsSpring *)params.pObject );
		WritePointer( pSave, params.pObject );
		pSave->WriteInt( &kRecordVersion );
		springparams_t spring;
		pSpring->WriteParams( spring );
		WriteBlock( pSave, spring );
		WritePointer( pSave, pSpring->GetStartObject() );
		WritePointer( pSave, pSpring->GetEndObject() );
		return true;
	}
	case PIID_IPHYSICSMOTIONCONTROLLER:
	{
		CMotionControllerBox3D *pMotion = static_cast<CMotionControllerBox3D *>( (IPhysicsMotionController *)params.pObject );
		WritePointer( pSave, params.pObject );
		pSave->WriteInt( &kRecordVersion );
		int priority = pMotion->GetPriority();
		pSave->WriteInt( &priority );
		int count = pMotion->CountObjects();
		pSave->WriteInt( &count );
		CUtlVector<IPhysicsObject *> objects;
		objects.SetCount( count );
		if ( count )
			pMotion->GetObjects( objects.Base() );
		for ( int i = 0; i < count; i++ )
			WritePointer( pSave, objects[i] );
		return true;
	}
	case PIID_IPHYSICSVEHICLECONTROLLER:
		return SaveVehicle( params );
	default:
		// Fluids, player controllers and standalone shadows: not saved (IVP).
		return false;
	}
}

void CPhysicsEnvironmentBox3D::PreRestore( const physprerestoreparams_t &params )
{
	s_pointerMap.RemoveAll();
	s_enableCollisions.RemoveAll();
	s_activateGroups.RemoveAll();
	for ( int i = 0; i < params.recreatedObjectCount; i++ )
		s_pointerMap.InsertOrReplace( params.recreatedObjectList[i].pOldObject, params.recreatedObjectList[i].pNewObject );
}

bool CPhysicsEnvironmentBox3D::Restore( const physrestoreparams_t &params )
{
	IRestore *pRestore = params.pRestore;
	if ( !pRestore || !params.ppObject )
		return false;
	*params.ppObject = NULL;
	if ( params.type != PIID_IPHYSICSOBJECT && params.type != PIID_IPHYSICSCONSTRAINTGROUP &&
		params.type != PIID_IPHYSICSCONSTRAINT && params.type != PIID_IPHYSICSSPRING &&
		params.type != PIID_IPHYSICSMOTIONCONTROLLER && params.type != PIID_IPHYSICSVEHICLECONTROLLER )
	{
		return false;
	}

	void *pOld = ReadPointer( pRestore );
	if ( pRestore->ReadInt() != kRecordVersion )
		return false;

	switch ( params.type )
	{
	case PIID_IPHYSICSOBJECT:
	{
		CPhysicsObjectStateBox3D state;
		ReadBlock( pRestore, state );
		state.materialIndex = ReadMaterial( pRestore );
		state.pCollide = params.pCollisionModel;
		if ( params.pName )
			V_strncpy( state.name, params.pName, sizeof( state.name ) );
		bool hasShadow = false;
		pRestore->ReadBool( &hasShadow );
		CShadowControllerBox3D::State_t shadow;
		if ( hasShadow )
		{
			ReadBlock( pRestore, shadow );
			shadow.savedMaterialIndex = ReadMaterial( pRestore );
		}
		// Collisions come on in PostRestore, once dependents exist.
		bool collisions = state.collisionEnabled;
		state.collisionEnabled = false;
		CPhysicsObjectBox3D *pObject = CPhysicsObjectBox3D::CreateFromState( this, params.pGameData, state, false );
		if ( !pObject )
			return false;
		TrackObject( pObject );
		if ( collisions )
			s_enableCollisions.AddToTail( pObject );
		if ( hasShadow )
			pObject->AttachShadowController( new CShadowControllerBox3D( pObject, shadow ) );
		*params.ppObject = pObject;
		break;
	}
	case PIID_IPHYSICSCONSTRAINTGROUP:
	{
		constraint_groupparams_t groupParams;
		ReadBlock( pRestore, groupParams );
		bool active = false;
		pRestore->ReadBool( &active );
		CConstraintGroupBox3D *pGroup = static_cast<CConstraintGroupBox3D *>( CreateConstraintGroup( groupParams ) );
		if ( active )
			s_activateGroups.AddToTail( pGroup );
		*params.ppObject = static_cast<IPhysicsConstraintGroup *>( pGroup );
		break;
	}
	case PIID_IPHYSICSCONSTRAINT:
	{
		int type = pRestore->ReadInt();
		if ( type == CONSTRAINT_BOX3D_UNKNOWN )
		{
			// An inert shell, as IVP restores a constraint whose object died.
			constraint_breakableparams_t breakable;
			breakable.Defaults();
			CConstraintBox3D *pInert = new CConstraintBox3D( this, NULL, NULL, NULL, CONSTRAINT_BOX3D_UNKNOWN, breakable );
			m_constraints.AddToTail( pInert );
			*params.ppObject = static_cast<IPhysicsConstraint *>( pInert );
			break;
		}
		IPhysicsConstraintGroup *pGroup = (IPhysicsConstraintGroup *)Remap( ReadPointer( pRestore ) );
		IPhysicsObject *pReference = (IPhysicsObject *)Remap( ReadPointer( pRestore ) );
		IPhysicsObject *pAttached = (IPhysicsObject *)Remap( ReadPointer( pRestore ) );
		IPhysicsConstraint *pConstraint = NULL;
		switch ( type )
		{
		case CONSTRAINT_BOX3D_RAGDOLL:
		{
			constraint_ragdollparams_t ragdoll;
			ReadBlock( pRestore, ragdoll );
			if ( pReference && pAttached )
				pConstraint = CreateRagdollConstraint( pReference, pAttached, pGroup, ragdoll );
			break;
		}
		case CONSTRAINT_BOX3D_HINGE:
		{
			constraint_hingeparams_t hinge;
			ReadBlock( pRestore, hinge );
			if ( pReference && pAttached )
				pConstraint = CreateHingeConstraint( pReference, pAttached, pGroup, hinge );
			break;
		}
		case CONSTRAINT_BOX3D_FIXED:
		{
			constraint_fixedparams_t fixed;
			ReadBlock( pRestore, fixed );
			if ( pReference && pAttached )
				pConstraint = CreateFixedConstraint( pReference, pAttached, pGroup, fixed );
			break;
		}
		case CONSTRAINT_BOX3D_BALLSOCKET:
		{
			constraint_ballsocketparams_t ballsocket;
			ReadBlock( pRestore, ballsocket );
			if ( pReference && pAttached )
				pConstraint = CreateBallsocketConstraint( pReference, pAttached, pGroup, ballsocket );
			break;
		}
		case CONSTRAINT_BOX3D_SLIDING:
		{
			constraint_slidingparams_t sliding;
			ReadBlock( pRestore, sliding );
			if ( pReference && pAttached )
				pConstraint = CreateSlidingConstraint( pReference, pAttached, pGroup, sliding );
			break;
		}
		case CONSTRAINT_BOX3D_PULLEY:
		{
			constraint_pulleyparams_t pulley;
			ReadBlock( pRestore, pulley );
			if ( pReference && pAttached )
				pConstraint = CreatePulleyConstraint( pReference, pAttached, pGroup, pulley );
			break;
		}
		case CONSTRAINT_BOX3D_LENGTH:
		{
			constraint_lengthparams_t length;
			ReadBlock( pRestore, length );
			if ( pReference && pAttached )
				pConstraint = CreateLengthConstraint( pReference, pAttached, pGroup, length );
			break;
		}
		default:
			return false;
		}
		if ( pConstraint )
			pConstraint->SetGameData( params.pGameData );
		*params.ppObject = pConstraint;
		break;
	}
	case PIID_IPHYSICSSPRING:
	{
		springparams_t spring;
		ReadBlock( pRestore, spring );
		IPhysicsObject *pStart = (IPhysicsObject *)Remap( ReadPointer( pRestore ) );
		IPhysicsObject *pEnd = (IPhysicsObject *)Remap( ReadPointer( pRestore ) );
		if ( pStart && pEnd )
			*params.ppObject = CreateSpring( pStart, pEnd, &spring );
		else
			DevMsg( "Failed to restore spring endpoints\n" );
		break;
	}
	case PIID_IPHYSICSMOTIONCONTROLLER:
	{
		int priority = pRestore->ReadInt();
		int count = pRestore->ReadInt();
		// The game sets the handler after restoring.
		CMotionControllerBox3D *pMotion = static_cast<CMotionControllerBox3D *>( CreateMotionController( NULL ) );
		pMotion->SetPriority( (IPhysicsMotionController::priority_t)priority );
		for ( int i = 0; i < count; i++ )
		{
			IPhysicsObject *pObject = (IPhysicsObject *)Remap( ReadPointer( pRestore ) );
			if ( pObject )
				pMotion->AttachObject( pObject, true );
		}
		*params.ppObject = static_cast<IPhysicsMotionController *>( pMotion );
		break;
	}
	case PIID_IPHYSICSVEHICLECONTROLLER:
		if ( !RestoreVehicle( params ) )
			return false;
		break;
	default:
		return false;
	}

	if ( *params.ppObject )
		s_pointerMap.InsertOrReplace( pOld, *params.ppObject );
	return true;
}

void CPhysicsEnvironmentBox3D::PostRestore()
{
	for ( int i = 0; i < s_enableCollisions.Count(); i++ )
		s_enableCollisions[i]->EnableCollisions( true );
	for ( int i = 0; i < s_activateGroups.Count(); i++ )
		s_activateGroups[i]->Activate();
	s_enableCollisions.RemoveAll();
	s_activateGroups.RemoveAll();
	s_pointerMap.RemoveAll();
}

//-----------------------------------------------------------------------------
// Vehicles: the controller state, then the surface names its material
// indices refer to (restored by name, as IVP's MaterialIndexDataOps does).
//-----------------------------------------------------------------------------
bool CPhysicsEnvironmentBox3D::SaveVehicle( const physsaveparams_t &params )
{
	CVehicleControllerBox3D *pVehicle = static_cast<CVehicleControllerBox3D *>( (IPhysicsVehicleController *)params.pObject );
	ISave *pSave = params.pSave;
	WritePointer( pSave, params.pObject );
	pSave->WriteInt( &kRecordVersion );
	CVehicleStateBox3D state;
	pVehicle->WriteState( state );
	WriteBlock( pSave, state );
	for ( int i = 0; i < VEHICLE_MAX_AXLE_COUNT; i++ )
	{
		const vehicle_wheelparams_t &wheels = state.vehicleData.axles[i].wheels;
		WriteMaterial( pSave, wheels.materialIndex );
		WriteMaterial( pSave, wheels.brakeMaterialIndex < 0 ? 0 : wheels.brakeMaterialIndex );
		WriteMaterial( pSave, wheels.skidMaterialIndex < 0 ? 0 : wheels.skidMaterialIndex );
	}
	WriteMaterial( pSave, state.currentState.skidMaterial );
	return true;
}

bool CPhysicsEnvironmentBox3D::RestoreVehicle( const physrestoreparams_t &params )
{
	IRestore *pRestore = params.pRestore;
	CVehicleStateBox3D state;
	ReadBlock( pRestore, state );
	for ( int i = 0; i < VEHICLE_MAX_AXLE_COUNT; i++ )
	{
		vehicle_wheelparams_t &wheels = state.vehicleData.axles[i].wheels;
		wheels.materialIndex = ReadMaterial( pRestore );
		int brake = ReadMaterial( pRestore ), skid = ReadMaterial( pRestore );
		wheels.brakeMaterialIndex = wheels.brakeMaterialIndex < 0 ? wheels.brakeMaterialIndex : brake;
		wheels.skidMaterialIndex = wheels.skidMaterialIndex < 0 ? wheels.skidMaterialIndex : skid;
	}
	state.currentState.skidMaterial = ReadMaterial( pRestore );
	state.pBody = (IPhysicsObject *)Remap( state.pBody );
	for ( int i = 0; i < VEHICLE_MAX_WHEEL_COUNT; i++ )
		state.pWheels[i] = (IPhysicsObject *)Remap( state.pWheels[i] );
	if ( !state.pBody )
	{
		DevMsg( "Failed to restore vehicle body\n" );
		return false;
	}
	CVehicleControllerBox3D *pVehicle = CVehicleControllerBox3D::CreateFromState( this, params.pGameTrace, state );
	m_vehicles.AddToTail( pVehicle );
	*params.ppObject = static_cast<IPhysicsVehicleController *>( pVehicle );
	return true;
}
