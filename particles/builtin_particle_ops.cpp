//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: particle system code
//
//===========================================================================//

#include "tier0/platform.h"
#include "particles/particles.h"
#include "filesystem.h"
#include "tier2/tier2.h"
#include "tier2/fileutils.h"
#include "tier2/renderutils.h"
#include "tier1/UtlStringMap.h"
#include "tier1/strtools.h"
#include "studio.h"
#include "bspflags.h"
#include "tier0/vprof.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#if MEASURE_PARTICLE_PERF

#if VPROF_LEVEL > 0
#define START_OP float flOpStartTime = Plat_FloatTime(); VPROF_ENTER_SCOPE(pOp->GetDefinition()->GetName())
#else
#define START_OP float flOpStartTime = Plat_FloatTime();
#endif

#if VPROF_LEVEL > 0
#define END_OP  if ( 1 ) {																						\
	float flETime = Plat_FloatTime() - flOpStartTime;									\
	IParticleOperatorDefinition *pDef = (IParticleOperatorDefinition *) pOp->GetDefinition();	\
	pDef->RecordExecutionTime( flETime );												\
} \
	VPROF_EXIT_SCOPE()
#else
#define END_OP  if ( 1 ) {																						\
	float flETime = Plat_FloatTime() - flOpStartTime;									\
	IParticleOperatorDefinition *pDef = (IParticleOperatorDefinition *) pOp->GetDefinition();	\
	pDef->RecordExecutionTime( flETime );												\
}
#endif
#else
#define START_OP
#define END_OP
#endif

//-----------------------------------------------------------------------------
// Standard movement operator
//-----------------------------------------------------------------------------
class C_OP_BasicMovement : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_BasicMovement );

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_PREV_XYZ_MASK | PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	Vector m_Gravity;
	float m_fDrag;
	int m_nMaxConstraintPasses;
};

DEFINE_PARTICLE_OPERATOR( C_OP_BasicMovement, "Movement Basic", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_BasicMovement ) 
	DMXELEMENT_UNPACK_FIELD( "gravity", "0 0 0", Vector, m_Gravity )
	DMXELEMENT_UNPACK_FIELD( "drag", "0", float, m_fDrag )
	DMXELEMENT_UNPACK_FIELD( "max constraint passes", "3", int, m_nMaxConstraintPasses )
END_PARTICLE_OPERATOR_UNPACK( C_OP_BasicMovement )


#define MAXIMUM_NUMBER_OF_CONSTRAINTS 100
//#define CHECKALL 1

#ifdef NDEBUG
#define CHECKSYSTEM( p ) 0
#else
#ifdef CHECKALL
static void CHECKSYSTEM( CParticleCollection *pParticles )
{
//	Assert( pParticles->m_nActiveParticles <= pParticles->m_pDef->m_nMaxParticles );
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		const float *xyz = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_XYZ, i );
		const float *xyz_prev = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_PREV_XYZ, i );
		Assert( IsFinite( xyz[0] ) );
		Assert( IsFinite( xyz[4] ) );
		Assert( IsFinite( xyz[8] ) );
		Assert( IsFinite( xyz_prev[0] ) );
		Assert( IsFinite( xyz_prev[4] ) );
		Assert( IsFinite( xyz_prev[8] ) );
	}
}
#else
#define CHECKSYSTEM( p ) 0
#endif
#endif

void C_OP_BasicMovement::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C4VAttributeWriteIterator prev_xyz( PARTICLE_ATTRIBUTE_PREV_XYZ, pParticles );
	C4VAttributeWriteIterator xyz( PARTICLE_ATTRIBUTE_XYZ, pParticles );

	// fltx4 adj_dt = ReplicateX4( (1.0-m_fDrag) * ( pParticles->m_flDt / pParticles->m_flPreviousDt ) );
	fltx4 adj_dt = ReplicateX4( ( pParticles->m_flDt / pParticles->m_flPreviousDt ) * ExponentialDecay( (1.0f-max(0.f,m_fDrag)), (1.0f/30.0f), pParticles->m_flDt ) );

	size_t nForceStride=0;
	Vector acc = m_Gravity;
	fltx4 accFactorX = ReplicateX4( acc.x );
	fltx4 accFactorY = ReplicateX4( acc.y );
	fltx4 accFactorZ = ReplicateX4( acc.z );

	int nAccumulators = pParticles->m_pDef->m_ForceGenerators.Count();

	FourVectors PerParticleForceAccumulator[MAX_PARTICLES_IN_A_SYSTEM / 4];	// xbox fixme - memory

	FourVectors *pAccOut = PerParticleForceAccumulator;
	if (nAccumulators)
	{
		// we do have per particle force accumulators
		nForceStride = 1;
		int nblocks = pParticles->m_nPaddedActiveParticles;
		for(int i=0;i<nblocks;i++)
		{
			pAccOut->x = accFactorX;
			pAccOut->y = accFactorY;
			pAccOut->z = accFactorZ;
			pAccOut++;
		} 
		// now, call all force accumulators
		for(int i=0;i < nAccumulators ; i++ )
		{
			float flStrengthOp;
			CParticleOperatorInstance *pOp = pParticles->m_pDef->m_ForceGenerators[i];
			if ( pParticles->CheckIfOperatorShouldRun( pOp, &flStrengthOp ))
			{
				START_OP;
				pParticles->m_pDef->m_ForceGenerators[i]->AddForces(
					PerParticleForceAccumulator,
					pParticles,
					nblocks,
					flStrengthOp,
					pParticles->m_pOperatorContextData + 
					pParticles->m_pDef->m_nForceGeneratorsCtxOffsets[i] );
				END_OP;
			}
		}
	}
	else
	{
		pAccOut->x = accFactorX;
		pAccOut->y = accFactorY;
		pAccOut->z = accFactorZ;
		// we just have gravity
	}
	
	CHECKSYSTEM( pParticles );
	fltx4 DtSquared = ReplicateX4( pParticles->m_flDt * pParticles->m_flDt );
	int ctr = pParticles->m_nPaddedActiveParticles;
	FourVectors *pAccIn = PerParticleForceAccumulator;
	do
	{
		accFactorX = MulSIMD( pAccIn->x, DtSquared );
		accFactorY = MulSIMD( pAccIn->y, DtSquared );
		accFactorZ = MulSIMD( pAccIn->z, DtSquared );
		
		// we will write prev xyz, and swap prev and cur at the end
		prev_xyz->x = AddSIMD( xyz->x,
							   AddSIMD( accFactorX, MulSIMD( adj_dt, SubSIMD( xyz->x, prev_xyz->x ) ) ) );
		prev_xyz->y = AddSIMD( xyz->y,
							   AddSIMD( accFactorY, MulSIMD( adj_dt, SubSIMD( xyz->y, prev_xyz->y ) ) ) );
		prev_xyz->z = AddSIMD( xyz->z,
							   AddSIMD( accFactorZ, MulSIMD( adj_dt, SubSIMD( xyz->z, prev_xyz->z ) ) ) );
		CHECKSYSTEM( pParticles );
		++prev_xyz;
		++xyz;
		pAccIn += nForceStride;
	} while (--ctr);

	CHECKSYSTEM( pParticles );
	pParticles->SwapPosAndPrevPos();
	// now, enforce constraints
	int nConstraints = pParticles->m_pDef->m_Constraints.Count();
	if ( nConstraints && pParticles->m_nPaddedActiveParticles )
	{
		bool bConstraintSatisfied[ MAXIMUM_NUMBER_OF_CONSTRAINTS ];
		bool bFinalConstraint[ MAXIMUM_NUMBER_OF_CONSTRAINTS ];
		for(int i=0;i<nConstraints; i++)
		{
			bFinalConstraint[i] = pParticles->m_pDef->m_Constraints[i]->IsFinalConstraint();

			bConstraintSatisfied[i] = false;
			pParticles->m_pDef->m_Constraints[i]->SetupConstraintPerFrameData(
				pParticles, pParticles->m_pOperatorContextData + 
				pParticles->m_pDef->m_nConstraintsCtxOffsets[i] );
		}

		// constraints get to see their own per psystem per op random #s
		for(int p=0; p < m_nMaxConstraintPasses ; p++ )
		{
//			int nSaveOffset=pParticles->m_nOperatorRandomSampleOffset;
			for(int i=0;i<nConstraints; i++)
			{
//				pParticles->m_nOperatorRandomSampleOffset += 23;
				if ( ! bConstraintSatisfied[i] )
				{
					CParticleOperatorInstance *pOp = pParticles->m_pDef->m_Constraints[i];
					bConstraintSatisfied[i] = true;
					if ( ( !bFinalConstraint[i] ) && ( pParticles->CheckIfOperatorShouldRun( pOp ) ) )
					{
						START_OP;
						bool bDidSomething = pOp->EnforceConstraint(
								0, pParticles->m_nPaddedActiveParticles, pParticles,
								pParticles->m_pOperatorContextData + 
								pParticles->m_pDef->m_nConstraintsCtxOffsets[i],
								pParticles->m_nActiveParticles );
						END_OP;
						CHECKSYSTEM( pParticles );
						if ( bDidSomething )
						{
							// other constraints now not satisfied, maybe
							for( int j=0; j<nConstraints; j++)
							{
								if ( i != j )
								{
									bConstraintSatisfied[ j ] = false;
								}
							}
						}
					}
				}
			}
//			pParticles->m_nOperatorRandomSampleOffset = nSaveOffset;
		}
		// now, run final constraints
		for(int i=0;i<nConstraints; i++)
		{
			CParticleOperatorInstance *pOp = pParticles->m_pDef->m_Constraints[i];
			if ( ( bFinalConstraint[i] ) &&
				 ( pParticles->CheckIfOperatorShouldRun( 
					 pOp ) ) )
			{
				START_OP;
				pOp->EnforceConstraint(
					0, pParticles->m_nPaddedActiveParticles, pParticles,
					pParticles->m_pOperatorContextData + 
					pParticles->m_pDef->m_nConstraintsCtxOffsets[i],
					pParticles->m_nActiveParticles );
				END_OP;
				CHECKSYSTEM( pParticles );
			}
		}
	}
	CHECKSYSTEM( pParticles );
}


//-----------------------------------------------------------------------------
// Fade and kill operator
//-----------------------------------------------------------------------------
class C_OP_FadeAndKill : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_FadeAndKill );

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement );
	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	float	m_flStartFadeInTime;
	float	m_flEndFadeInTime;
	float	m_flStartFadeOutTime;
	float	m_flEndFadeOutTime;
	float	m_flStartAlpha;
	float	m_flEndAlpha;
};

DEFINE_PARTICLE_OPERATOR( C_OP_FadeAndKill, "Alpha Fade and Decay", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_FadeAndKill ) 
	DMXELEMENT_UNPACK_FIELD( "start_alpha","1", float, m_flStartAlpha )
	DMXELEMENT_UNPACK_FIELD( "end_alpha","0", float, m_flEndAlpha )
	DMXELEMENT_UNPACK_FIELD( "start_fade_in_time","0", float, m_flStartFadeInTime )
	DMXELEMENT_UNPACK_FIELD( "end_fade_in_time","0.5", float, m_flEndFadeInTime )
	DMXELEMENT_UNPACK_FIELD( "start_fade_out_time","0.5", float, m_flStartFadeOutTime )
	DMXELEMENT_UNPACK_FIELD( "end_fade_out_time","1", float, m_flEndFadeOutTime )
END_PARTICLE_OPERATOR_UNPACK( C_OP_FadeAndKill )

void C_OP_FadeAndKill::InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
{
	// Cache off and validate values
	if ( m_flEndFadeInTime < m_flStartFadeInTime )
	{
		m_flEndFadeInTime = m_flStartFadeInTime;
	}
	if ( m_flEndFadeOutTime < m_flStartFadeOutTime )
	{
		m_flEndFadeOutTime = m_flStartFadeOutTime;
	}
	
	if ( m_flStartFadeOutTime < m_flStartFadeInTime )
	{
		V_swap( m_flStartFadeInTime, m_flStartFadeOutTime );
	}

	if ( m_flEndFadeOutTime < m_flEndFadeInTime )
	{
		V_swap( m_flEndFadeInTime, m_flEndFadeOutTime );
	}
}

void C_OP_FadeAndKill::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128InitialAttributeIterator pInitialAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeWriteIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );

	fltx4 fl4StartFadeInTime = ReplicateX4( m_flStartFadeInTime );
	fltx4 fl4StartFadeOutTime = ReplicateX4( m_flStartFadeOutTime );
	fltx4 fl4EndFadeInTime = ReplicateX4( m_flEndFadeInTime );
	fltx4 fl4EndFadeOutTime = ReplicateX4( m_flEndFadeOutTime );
	fltx4 fl4EndAlpha = ReplicateX4( m_flEndAlpha );
	fltx4 fl4StartAlpha = ReplicateX4( m_flStartAlpha );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;
	int nLimit = pParticles->m_nPaddedActiveParticles << 2;
	
	fltx4 fl4FadeInDuration = ReplicateX4( m_flEndFadeInTime - m_flStartFadeInTime );
	fltx4 fl4OOFadeInDuration = ReciprocalEstSIMD( fl4FadeInDuration );

	fltx4 fl4FadeOutDuration = ReplicateX4( m_flEndFadeOutTime - m_flStartFadeOutTime );
	fltx4 fl4OOFadeOutDuration = ReciprocalEstSIMD( fl4FadeOutDuration );

	for ( int i = 0; i < nLimit; i+= 4 )
	{
		fltx4 fl4Age = SubSIMD( fl4CurTime, *pCreationTime );
		fltx4 fl4ParticleLifeTime = *pLifeDuration;
		fltx4 fl4KillMask = CmpGeSIMD( fl4Age, *pLifeDuration );	// takes care of lifeduration = 0 div 0
		fl4Age = MulSIMD( fl4Age, ReciprocalEstSIMD( fl4ParticleLifeTime ) );	// age 0..1
		fltx4 fl4FadingInMask = AndNotSIMD( fl4KillMask, 
											AndSIMD(
												CmpLeSIMD( fl4StartFadeInTime, fl4Age ), CmpGtSIMD(fl4EndFadeInTime, fl4Age ) ) );
		fltx4 fl4FadingOutMask = AndNotSIMD( fl4KillMask,
										  AndSIMD( 
											  CmpLeSIMD( fl4StartFadeOutTime, fl4Age ), CmpGtSIMD(fl4EndFadeOutTime, fl4Age ) ) );
		if ( IsAnyNegative( fl4FadingInMask ) )
		{
			fltx4 fl4Goal = MulSIMD( *pInitialAlpha, fl4StartAlpha );
			fltx4 fl4NewAlpha = SimpleSplineRemapValWithDeltasClamped( fl4Age, fl4StartFadeInTime, fl4FadeInDuration, fl4OOFadeInDuration,
																	   fl4Goal, SubSIMD( *pInitialAlpha, fl4Goal ) );

			*pAlpha = MaskedAssign( fl4FadingInMask, fl4NewAlpha, *pAlpha );
		}
		if ( IsAnyNegative( fl4FadingOutMask ) )
		{
			fltx4 fl4Goal = MulSIMD( *pInitialAlpha, fl4EndAlpha );
			fltx4 fl4NewAlpha = SimpleSplineRemapValWithDeltasClamped( fl4Age, fl4StartFadeOutTime, fl4FadeOutDuration, fl4OOFadeOutDuration,
																	   *pInitialAlpha, SubSIMD( fl4Goal, *pInitialAlpha ) );
			*pAlpha = MaskedAssign( fl4FadingOutMask, fl4NewAlpha, *pAlpha );
		}
		if ( IsAnyNegative( fl4KillMask ) )
		{
			int nMask = TestSignSIMD( fl4KillMask );
			if ( nMask & 1 )
				pParticles->KillParticle( i );
			if ( nMask & 2 )
				pParticles->KillParticle( i + 1 );
			if ( nMask & 4 )
				pParticles->KillParticle( i + 2 );
			if ( nMask & 8 )
				pParticles->KillParticle( i + 3 );
		}
		++pCreationTime;
		++pLifeDuration;
		++pInitialAlpha;
		++pAlpha;
	}
}


//-----------------------------------------------------------------------------
// Fade In Operator
//-----------------------------------------------------------------------------
class C_OP_FadeIn : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_FadeIn );

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK | PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	float	m_flFadeInTimeMin;
	float	m_flFadeInTimeMax;
	float	m_flFadeInTimeExp;
	bool    m_bProportional;
};

DEFINE_PARTICLE_OPERATOR( C_OP_FadeIn, "Alpha Fade In Random", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_FadeIn ) 
	DMXELEMENT_UNPACK_FIELD( "fade in time min",".25", float, m_flFadeInTimeMin )
	DMXELEMENT_UNPACK_FIELD( "fade in time max",".25", float, m_flFadeInTimeMax )
	DMXELEMENT_UNPACK_FIELD( "fade in time exponent","1", float, m_flFadeInTimeExp )
	DMXELEMENT_UNPACK_FIELD( "proportional 0/1","1", bool, m_bProportional )
END_PARTICLE_OPERATOR_UNPACK( C_OP_FadeIn )


void C_OP_FadeIn::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128InitialAttributeIterator pInitialAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeWriteIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	C4IAttributeIterator pParticleID( PARTICLE_ATTRIBUTE_PARTICLE_ID, pParticles );
	int nRandomOffset = pParticles->OperatorRandomSampleOffset();

	fltx4 CurTime = pParticles->m_fl4CurTime;

	int nCtr = pParticles->m_nPaddedActiveParticles;
	int nSSEFixedExponent = m_flFadeInTimeExp*4.0;

	fltx4 FadeTimeMin = ReplicateX4( m_flFadeInTimeMin );
	fltx4 FadeTimeWidth = ReplicateX4( m_flFadeInTimeMax - m_flFadeInTimeMin );

	do 
	{
		fltx4 FadeInTime= Pow_FixedPoint_Exponent_SIMD(
			pParticles->RandomFloat( *pParticleID, nRandomOffset ),
			nSSEFixedExponent);
		FadeInTime = AddSIMD( FadeTimeMin, MulSIMD( FadeTimeWidth, FadeInTime ) );
		
		// Find our life percentage
		fltx4 flLifeTime = SubSIMD( CurTime, *pCreationTime );
		if ( m_bProportional )
		{
			flLifeTime =
				MaxSIMD( Four_Zeros,
						 MinSIMD( Four_Ones,
								  MulSIMD( flLifeTime, ReciprocalEstSIMD( *pLifeDuration ) ) ) );
		}
		
		fltx4 ApplyMask = CmpGtSIMD( FadeInTime, flLifeTime );
		if ( IsAnyNegative( ApplyMask ) )
		{
			// Fading in
			fltx4 NewAlpha =
				SimpleSplineRemapValWithDeltasClamped(
					flLifeTime, Four_Zeros,
					FadeInTime, ReciprocalEstSIMD( FadeInTime ), 
					Four_Zeros, *pInitialAlpha );
			*( pAlpha ) = MaskedAssign( ApplyMask, NewAlpha, *( pAlpha ) );
		}
		++pCreationTime;
		++pLifeDuration;
		++pInitialAlpha;
		++pAlpha;
		++pParticleID;
	} while( --nCtr );
}



//-----------------------------------------------------------------------------
// Fade Out Operator
//-----------------------------------------------------------------------------
class C_OP_FadeOut : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_FadeOut );

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK | PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		float flBias = ( m_flFadeBias != 0.0f ) ? m_flFadeBias : 0.5f;
		m_fl4BiasParam = PreCalcBiasParameter( ReplicateX4( flBias ) );
		if ( m_flFadeOutTimeMin == 0.0f && m_flFadeOutTimeMax == 0.0f )
		{
			m_flFadeOutTimeMin = m_flFadeOutTimeMax = FLT_EPSILON;
		}
	}

	float	m_flFadeOutTimeMin;
	float	m_flFadeOutTimeMax;
	float	m_flFadeOutTimeExp;
	float	m_flFadeBias;
	fltx4	m_fl4BiasParam;
	bool    m_bProportional;
	bool	m_bEaseInAndOut;
};

DEFINE_PARTICLE_OPERATOR( C_OP_FadeOut, "Alpha Fade Out Random", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_FadeOut ) 
	DMXELEMENT_UNPACK_FIELD( "fade out time min",".25", float, m_flFadeOutTimeMin )
	DMXELEMENT_UNPACK_FIELD( "fade out time max",".25", float, m_flFadeOutTimeMax )
	DMXELEMENT_UNPACK_FIELD( "fade out time exponent","1", float, m_flFadeOutTimeExp )
	DMXELEMENT_UNPACK_FIELD( "proportional 0/1","1", bool, m_bProportional )
	DMXELEMENT_UNPACK_FIELD( "ease in and out","1", bool, m_bEaseInAndOut )
	DMXELEMENT_UNPACK_FIELD( "fade bias", "0.5", float, m_flFadeBias )
END_PARTICLE_OPERATOR_UNPACK( C_OP_FadeOut )


void C_OP_FadeOut::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128InitialAttributeIterator pInitialAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeWriteIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	C4IAttributeIterator pParticleID( PARTICLE_ATTRIBUTE_PARTICLE_ID, pParticles );
	int nRandomOffset = pParticles->OperatorRandomSampleOffset();

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nCtr = pParticles->m_nPaddedActiveParticles;
	int nSSEFixedExponent = m_flFadeOutTimeExp*4.0;

	fltx4 FadeTimeMin = ReplicateX4( m_flFadeOutTimeMin );
	fltx4 FadeTimeWidth = ReplicateX4( m_flFadeOutTimeMax - m_flFadeOutTimeMin );

	do 
	{
		fltx4 fl4FadeOutTime = Pow_FixedPoint_Exponent_SIMD(
			pParticles->RandomFloat( *pParticleID, nRandomOffset ),
			nSSEFixedExponent );
		fl4FadeOutTime = AddSIMD( FadeTimeMin, MulSIMD( FadeTimeWidth, fl4FadeOutTime ) );

		fltx4 fl4Lifespan;

		// Find our life percentage
		fltx4 fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime );
		fltx4 fl4LifeDuration = *pLifeDuration;

		if ( m_bProportional )
		{
			fl4LifeTime = MulSIMD( fl4LifeTime, ReciprocalEstSIMD( fl4LifeDuration ) );
			fl4FadeOutTime = SubSIMD( Four_Ones, fl4FadeOutTime );
			fl4Lifespan = SubSIMD ( Four_Ones, fl4FadeOutTime );
		}
		else
		{
			fl4FadeOutTime = SubSIMD( *pLifeDuration, fl4FadeOutTime );
			fl4Lifespan = SubSIMD( *pLifeDuration, fl4FadeOutTime ) ;
		}

		fltx4 ApplyMask = CmpLtSIMD( fl4FadeOutTime, fl4LifeTime );
		if ( IsAnyNegative( ApplyMask ) )
		{
			// Fading out
			fltx4 NewAlpha;
			if ( m_bEaseInAndOut )
			{
				NewAlpha = SimpleSplineRemapValWithDeltasClamped(
					fl4LifeTime, fl4FadeOutTime,
					fl4Lifespan, ReciprocalEstSIMD( fl4Lifespan ), 
					*pInitialAlpha, SubSIMD ( Four_Zeros, *pInitialAlpha ) );
				NewAlpha = MaxSIMD( Four_Zeros, NewAlpha );
			}
			else
			{
				fltx4 fl4Frac = MulSIMD( SubSIMD( fl4LifeTime, fl4FadeOutTime ), ReciprocalEstSIMD( fl4Lifespan ) );
				fl4Frac = MinSIMD( Four_Ones, MaxSIMD( Four_Zeros, fl4Frac ) );
				fl4Frac = BiasSIMD( fl4Frac, m_fl4BiasParam );
				fl4Frac	= SubSIMD( Four_Ones, fl4Frac );
				NewAlpha = MulSIMD( *pInitialAlpha, fl4Frac );
			}
			
			*( pAlpha ) = MaskedAssign( ApplyMask, NewAlpha, *( pAlpha ) );
		}
		++pCreationTime;
		++pLifeDuration;
		++pInitialAlpha;
		++pAlpha;
		++pParticleID;
	} while( --nCtr );
}




//-----------------------------------------------------------------------------
// Oscillating Scalar operator
// performs an oscillation operation on any scalar (fade, radius, etc.)
//-----------------------------------------------------------------------------
class C_OP_OscillateScalar : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_OscillateScalar );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nField;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK |
			PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	float	m_RateMin;
	float	m_RateMax;
	float	m_FrequencyMin;
	float	m_FrequencyMax;
	int		m_nField;
	bool    m_bProportional, m_bProportionalOp;
	float	m_flStartTime_min;
	float	m_flStartTime_max;
	float	m_flEndTime_min;
	float	m_flEndTime_max;
	float	m_flOscMult;
	float	m_flOscAdd;
};

DEFINE_PARTICLE_OPERATOR( C_OP_OscillateScalar, "Oscillate Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_OscillateScalar )
	DMXELEMENT_UNPACK_FIELD_USERDATA( "oscillation field", "7", int, m_nField, "intchoice particlefield_scalar" )
	DMXELEMENT_UNPACK_FIELD( "oscillation rate min", "0", float, m_RateMin )
	DMXELEMENT_UNPACK_FIELD( "oscillation rate max", "0", float, m_RateMax )
	DMXELEMENT_UNPACK_FIELD( "oscillation frequency min", "1", float, m_FrequencyMin )
	DMXELEMENT_UNPACK_FIELD( "oscillation frequency max", "1", float, m_FrequencyMax )
	DMXELEMENT_UNPACK_FIELD( "proportional 0/1", "1", bool, m_bProportional )
	DMXELEMENT_UNPACK_FIELD( "start time min", "0", float, m_flStartTime_min )
	DMXELEMENT_UNPACK_FIELD( "start time max", "0", float, m_flStartTime_max )
	DMXELEMENT_UNPACK_FIELD( "end time min", "1", float, m_flEndTime_min )
	DMXELEMENT_UNPACK_FIELD( "end time max", "1", float, m_flEndTime_max )
	DMXELEMENT_UNPACK_FIELD( "start/end proportional", "1", bool, m_bProportionalOp )
	DMXELEMENT_UNPACK_FIELD( "oscillation multiplier", "2", float, m_flOscMult )
	DMXELEMENT_UNPACK_FIELD( "oscillation start phase", ".5", float, m_flOscAdd )
END_PARTICLE_OPERATOR_UNPACK( C_OP_OscillateScalar )
																    
void C_OP_OscillateScalar::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	C4IAttributeIterator pParticleId ( PARTICLE_ATTRIBUTE_PARTICLE_ID, pParticles );
	CM128AttributeWriteIterator pOscField ( m_nField, pParticles) ;

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nRandomOffset = pParticles->OperatorRandomSampleOffset();

	fltx4 fl4OscVal;

	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );

	fltx4 fl4CosFactorMultiplier = ReplicateX4( m_flOscMult );
	fltx4 fl4CosFactorAdd = ReplicateX4( m_flOscAdd );

	fltx4 fl4CosFactor = AddSIMD( MulSIMD( fl4CosFactorMultiplier, fl4CurTime ), fl4CosFactorAdd );
	fltx4 fl4CosFactorProp = fl4CosFactorMultiplier;

	fltx4 fl4StartTimeMin = ReplicateX4( m_flStartTime_min );
	fltx4 fl4StartTimeWidth = ReplicateX4( m_flStartTime_max - m_flStartTime_min );
	fltx4 fl4EndTimeMin = ReplicateX4( m_flEndTime_min );
	fltx4 fl4EndTimeWidth = ReplicateX4( m_flEndTime_max - m_flEndTime_min );

	fltx4 fl4FrequencyMin = ReplicateX4( m_FrequencyMin );
	fltx4 fl4FrequencyWidth = ReplicateX4( m_FrequencyMax - m_FrequencyMin );
	fltx4 fl4RateMin = ReplicateX4( m_RateMin );
	fltx4 fl4RateWidth = ReplicateX4( m_RateMax - m_RateMin );

	int nCtr = pParticles->m_nPaddedActiveParticles;


	do 
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );
		fltx4 fl4LifeTime;
		if ( m_bProportionalOp )
		{
			fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) ); // maybe need accurate div here?
		}
		else
		{
			fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime ); 
		}

		fltx4 fl4StartTime= pParticles->RandomFloat( *pParticleId, nRandomOffset + 11);
		fl4StartTime = AddSIMD( fl4StartTimeMin, MulSIMD( fl4StartTimeWidth, fl4StartTime ) );
		fltx4 fl4EndTime= pParticles->RandomFloat( *pParticleId, nRandomOffset + 12);
		fl4EndTime = AddSIMD( fl4EndTimeMin, MulSIMD( fl4EndTimeWidth, fl4EndTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );
		if ( IsAnyNegative( fl4GoodMask ) )
		{
			fltx4 fl4Frequency = pParticles->RandomFloat( *pParticleId, nRandomOffset );
			fl4Frequency = AddSIMD( fl4FrequencyMin, MulSIMD( fl4FrequencyWidth, fl4Frequency ) );
			fltx4 fl4Rate= pParticles->RandomFloat( *pParticleId, nRandomOffset + 1);
			fl4Rate = AddSIMD( fl4RateMin, MulSIMD( fl4RateWidth, fl4Rate ) );
			fltx4 fl4Cos;
			if ( m_bProportional )
			{
				fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) );
				fl4Cos = AddSIMD( MulSIMD( fl4CosFactorProp, MulSIMD( fl4LifeTime, fl4Frequency )), fl4CosFactorAdd );
			}
			else
			{
				fl4Cos = MulSIMD( fl4CosFactor, fl4Frequency );
			}
			fltx4 fl4OscMultiplier = MulSIMD( fl4Rate, fl4ScaleFactor);
			fl4OscVal = AddSIMD ( *pOscField, MulSIMD ( fl4OscMultiplier, SinEst01SIMD( fl4Cos ) ) );
			if ( m_nField == 7)
			{
				*pOscField = MaskedAssign( fl4GoodMask, 
					MaxSIMD( MinSIMD( fl4OscVal, Four_Ones), Four_Zeros ), *pOscField );
			}
			else
			{
				*pOscField = MaskedAssign( fl4GoodMask, fl4OscVal, *pOscField );
			}
		}
		++pCreationTime;
		++pLifeDuration;
		++pOscField;
		++pParticleId;
	} while (--nCtr );
	
};




//-----------------------------------------------------------------------------
// Oscillating Vector operator
// performs an oscillation operation on any vector (location, tint)
//-----------------------------------------------------------------------------
class C_OP_OscillateVector : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_OscillateVector );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nField;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK |
			PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;
	
	Vector	m_RateMin;
	Vector	m_RateMax;
	Vector	m_FrequencyMin;
	Vector	m_FrequencyMax;
	int		m_nField;
	bool    m_bProportional, m_bProportionalOp;
	bool	m_bAccelerator;
	float	m_flStartTime_min;
	float	m_flStartTime_max;
	float	m_flEndTime_min;
	float	m_flEndTime_max;
	float	m_flOscMult;
	float	m_flOscAdd;
};

DEFINE_PARTICLE_OPERATOR( C_OP_OscillateVector, "Oscillate Vector", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_OscillateVector )
	DMXELEMENT_UNPACK_FIELD_USERDATA( "oscillation field", "0", int, m_nField, "intchoice particlefield_vector" )
	DMXELEMENT_UNPACK_FIELD( "oscillation rate min", "0 0 0", Vector, m_RateMin )
	DMXELEMENT_UNPACK_FIELD( "oscillation rate max", "0 0 0", Vector, m_RateMax )
	DMXELEMENT_UNPACK_FIELD( "oscillation frequency min", "1 1 1", Vector, m_FrequencyMin )
	DMXELEMENT_UNPACK_FIELD( "oscillation frequency max", "1 1 1", Vector, m_FrequencyMax )
	DMXELEMENT_UNPACK_FIELD( "proportional 0/1", "1", bool, m_bProportional )
	DMXELEMENT_UNPACK_FIELD( "start time min", "0", float, m_flStartTime_min )
	DMXELEMENT_UNPACK_FIELD( "start time max", "0", float, m_flStartTime_max )
	DMXELEMENT_UNPACK_FIELD( "end time min", "1", float, m_flEndTime_min )
	DMXELEMENT_UNPACK_FIELD( "end time max", "1", float, m_flEndTime_max )
	DMXELEMENT_UNPACK_FIELD( "start/end proportional", "1", bool, m_bProportionalOp )
	DMXELEMENT_UNPACK_FIELD( "oscillation multiplier", "2", float, m_flOscMult )
	DMXELEMENT_UNPACK_FIELD( "oscillation start phase", ".5", float, m_flOscAdd )
END_PARTICLE_OPERATOR_UNPACK( C_OP_OscillateVector )

														    
void C_OP_OscillateVector::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	C4IAttributeIterator pParticleId ( PARTICLE_ATTRIBUTE_PARTICLE_ID, pParticles );
	C4VAttributeWriteIterator pOscField ( m_nField, pParticles) ;

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nRandomOffset = pParticles->OperatorRandomSampleOffset();

	FourVectors fvOscVal;

	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );

	fltx4 fl4CosFactorMultiplier = ReplicateX4( m_flOscMult );
	fltx4 fl4CosFactorAdd = ReplicateX4( m_flOscAdd );

	fltx4 fl4CosFactor = AddSIMD( MulSIMD( fl4CosFactorMultiplier, fl4CurTime ), fl4CosFactorAdd );
	fltx4 fl4CosFactorProp = fl4CosFactorMultiplier;

	fltx4 fl4StartTimeMin = ReplicateX4( m_flStartTime_min );
	fltx4 fl4StartTimeWidth = ReplicateX4( m_flStartTime_max - m_flStartTime_min );
	fltx4 fl4EndTimeMin = ReplicateX4( m_flEndTime_min );
	fltx4 fl4EndTimeWidth = ReplicateX4( m_flEndTime_max - m_flEndTime_min );

	FourVectors fvFrequencyMin;
	fvFrequencyMin.DuplicateVector( m_FrequencyMin );
	FourVectors fvFrequencyWidth;
	fvFrequencyWidth.DuplicateVector( m_FrequencyMax - m_FrequencyMin );
	FourVectors fvRateMin;
	fvRateMin.DuplicateVector( m_RateMin );
	FourVectors fvRateWidth;
	fvRateWidth.DuplicateVector( m_RateMax - m_RateMin );

	int nCtr = pParticles->m_nPaddedActiveParticles;


	do 
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );
		fltx4 fl4LifeTime;
		if ( m_bProportionalOp )
		{
			fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) ); // maybe need accurate div here?
		}
		else
		{
			fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime ); 
		}

		fltx4 fl4StartTime= pParticles->RandomFloat( *pParticleId, nRandomOffset + 11);
		fl4StartTime = AddSIMD( fl4StartTimeMin, MulSIMD( fl4StartTimeWidth, fl4StartTime ) );
		fltx4 fl4EndTime= pParticles->RandomFloat( *pParticleId, nRandomOffset + 12);
		fl4EndTime = AddSIMD( fl4EndTimeMin, MulSIMD( fl4EndTimeWidth, fl4EndTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );
		if ( IsAnyNegative( fl4GoodMask ) )
		{
			FourVectors fvFrequency;
			fvFrequency.x = pParticles->RandomFloat( *pParticleId, nRandomOffset + 8 );
			fvFrequency.y = pParticles->RandomFloat( *pParticleId, nRandomOffset + 12 );
			fvFrequency.z = pParticles->RandomFloat( *pParticleId, nRandomOffset + 15 );
			fvFrequency.VProduct( fvFrequencyWidth );
			fvFrequency += fvFrequencyMin; 

			FourVectors fvRate;
			fvRate.x = pParticles->RandomFloat( *pParticleId, nRandomOffset + 3);
			fvRate.y = pParticles->RandomFloat( *pParticleId, nRandomOffset + 7);
			fvRate.z = pParticles->RandomFloat( *pParticleId, nRandomOffset + 9);

			//fvRate = AddSIMD( fvRateMin, MulSIMD( fvRateWidth, fvRate ) );
			fvRate.VProduct( fvRateWidth );
			fvRate += fvRateMin;

			FourVectors fvCos;
			if ( m_bProportional )
			{
				fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) );
				fvCos.x = AddSIMD( MulSIMD( fl4CosFactorProp, MulSIMD( fvFrequency.x, fl4LifeTime )), fl4CosFactorAdd );
				fvCos.y = AddSIMD( MulSIMD( fl4CosFactorProp, MulSIMD( fvFrequency.y, fl4LifeTime )), fl4CosFactorAdd );
				fvCos.z = AddSIMD( MulSIMD( fl4CosFactorProp, MulSIMD( fvFrequency.z, fl4LifeTime )), fl4CosFactorAdd );
			}
			else
			{
				//fvCos = MulSIMD( fl4CosFactor, fvFrequency );
				fvCos.x = MulSIMD( fvFrequency.x, fl4CosFactor );
				fvCos.y = MulSIMD( fvFrequency.y, fl4CosFactor );
				fvCos.z = MulSIMD( fvFrequency.z, fl4CosFactor );
			}

			FourVectors fvOscMultiplier;
			fvOscMultiplier.x = MulSIMD( fvRate.x, fl4ScaleFactor);
			fvOscMultiplier.y = MulSIMD( fvRate.y, fl4ScaleFactor);
			fvOscMultiplier.z = MulSIMD( fvRate.z, fl4ScaleFactor);

			FourVectors fvOutput = *pOscField;

			fvOscVal.x = AddSIMD ( fvOutput.x, MulSIMD ( fvOscMultiplier.x, SinEst01SIMD( fvCos.x ) ) );
			fvOscVal.y = AddSIMD ( fvOutput.y, MulSIMD ( fvOscMultiplier.y, SinEst01SIMD( fvCos.y ) ) );
			fvOscVal.z = AddSIMD ( fvOutput.z, MulSIMD ( fvOscMultiplier.z, SinEst01SIMD( fvCos.z ) ) );

			if ( m_nField == 6)
			{
				pOscField->x = MaskedAssign( fl4GoodMask, 
					MaxSIMD( MinSIMD( fvOscVal.x, Four_Ones), Four_Zeros ), fvOutput.x );
				pOscField->y = MaskedAssign( fl4GoodMask, 
					MaxSIMD( MinSIMD( fvOscVal.y, Four_Ones), Four_Zeros ), fvOutput.y );
				pOscField->z = MaskedAssign( fl4GoodMask, 
					MaxSIMD( MinSIMD( fvOscVal.z, Four_Ones), Four_Zeros ), fvOutput.z );
			}
			else
			{
				pOscField->x = MaskedAssign( fl4GoodMask, fvOscVal.x, fvOutput.x );
				pOscField->y = MaskedAssign( fl4GoodMask, fvOscVal.y, fvOutput.y );
				pOscField->z = MaskedAssign( fl4GoodMask, fvOscVal.z, fvOutput.z );
			}
		}
		++pCreationTime;
		++pLifeDuration;
		++pOscField;
		++pParticleId;
	} while (--nCtr );
};




//-----------------------------------------------------------------------------
// Remap Scalar Operator
//-----------------------------------------------------------------------------
class C_OP_RemapScalar : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapScalar );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 1 << m_nFieldInput;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int		m_nFieldInput;
	int		m_nFieldOutput;
	float	m_flInputMin;
	float	m_flInputMax;
	float	m_flOutputMin;
	float	m_flOutputMax;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RemapScalar, "Remap Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapScalar )
	DMXELEMENT_UNPACK_FIELD_USERDATA( "input field", "7", int, m_nFieldInput, "intchoice particlefield_scalar" )
	DMXELEMENT_UNPACK_FIELD( "input minimum","0", float, m_flInputMin )
	DMXELEMENT_UNPACK_FIELD( "input maximum","1", float, m_flInputMax )
	DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
	DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
	DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapScalar )

void C_OP_RemapScalar::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	// clamp the result to 0 and 1 if it's alpha
	float flMin=m_flOutputMin;
	float flMax=m_flOutputMax;
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = clamp(m_flOutputMin, 0.0f, 1.0f );
		flMax = clamp(m_flOutputMax, 0.0f, 1.0f );
	}

	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		const float *pInput = pParticles->GetFloatAttributePtr( m_nFieldInput, i );
		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
		float flOutput = RemapValClamped( *pInput, m_flInputMin, m_flInputMax, flMin, flMax  );
		*pOutput = Lerp (flStrength, *pOutput, flOutput);
	}
}


//-----------------------------------------------------------------------------
// noise Operator
//-----------------------------------------------------------------------------
class C_OP_Noise : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_Noise );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int m_nFieldOutput;
	float	m_flOutputMin;
	float	m_flOutputMax;
	fltx4 m_fl4NoiseScale;
};

DEFINE_PARTICLE_OPERATOR( C_OP_Noise, "Noise Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_Noise )
	DMXELEMENT_UNPACK_FLTX4( "noise coordinate scale", "0.1", m_fl4NoiseScale)
	DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
	DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
	DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
END_PARTICLE_OPERATOR_UNPACK( C_OP_Noise );

void C_OP_Noise::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeWriteIterator pAttr( m_nFieldOutput, pParticles );
		
	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );

	fltx4 CoordScale=m_fl4NoiseScale;
	
	float fMin = m_flOutputMin;
	float fMax = m_flOutputMax;

	if ( ATTRIBUTES_WHICH_ARE_ANGLES & (1 << m_nFieldOutput ) )
	{
		fMin *= ( M_PI / 180.0f );
		fMax *= ( M_PI / 180.0f );
	}
	// calculate coefficients. noise retuns -1..1
	fltx4 ValueScale=ReplicateX4( 0.5*(fMax-fMin ) );
	fltx4 ValueBase=ReplicateX4( fMin + 0.5*( fMax - fMin ) );
	int nActive = pParticles->m_nPaddedActiveParticles;
	do
	{
		FourVectors Coord = *pXYZ;
		Coord *= CoordScale;
		*( pAttr )=AddSIMD( ValueBase, MulSIMD( ValueScale, NoiseSIMD( Coord ) ) );

		++pAttr;
		++pXYZ;
	} while( --nActive );
}
//-----------------------------------------------------------------------------
// vector noise Operator
//-----------------------------------------------------------------------------
class C_OP_VectorNoise : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_VectorNoise );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int m_nFieldOutput;
	Vector	m_vecOutputMin;
	Vector	m_vecOutputMax;
	fltx4 m_fl4NoiseScale;
};

DEFINE_PARTICLE_OPERATOR( C_OP_VectorNoise, "Noise Vector", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_VectorNoise )
	DMXELEMENT_UNPACK_FLTX4( "noise coordinate scale", "0.1", m_fl4NoiseScale)
	DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "6", int, m_nFieldOutput, "intchoice particlefield_vector" )
	DMXELEMENT_UNPACK_FIELD( "output minimum","0 0 0", Vector, m_vecOutputMin )
	DMXELEMENT_UNPACK_FIELD( "output maximum","1 1 1", Vector, m_vecOutputMax )
END_PARTICLE_OPERATOR_UNPACK( C_OP_VectorNoise );

void C_OP_VectorNoise::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C4VAttributeWriteIterator pAttr( m_nFieldOutput, pParticles );
		
	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );

	fltx4 CoordScale = m_fl4NoiseScale;
	
	// calculate coefficients. noise retuns -1..1
	fltx4 ValueScaleX = ReplicateX4( 0.5*(m_vecOutputMax.x-m_vecOutputMin.x ) );
	fltx4 ValueBaseX = ReplicateX4(m_vecOutputMin.x+0.5*( m_vecOutputMax.x-m_vecOutputMin.x ) );

	fltx4 ValueScaleY = ReplicateX4( 0.5*(m_vecOutputMax.y-m_vecOutputMin.y ) );
	fltx4 ValueBaseY = ReplicateX4(m_vecOutputMin.y+0.5*( m_vecOutputMax.y-m_vecOutputMin.y ) );

	fltx4 ValueScaleZ = ReplicateX4( 0.5*(m_vecOutputMax.z-m_vecOutputMin.z ) );
	fltx4 ValueBaseZ = ReplicateX4(m_vecOutputMin.z+0.5*( m_vecOutputMax.z-m_vecOutputMin.z ) );

	FourVectors ofs_y;
	ofs_y.DuplicateVector( Vector( 100000.5, 300000.25, 9000000.75 ) );
	FourVectors ofs_z;
	ofs_z.DuplicateVector( Vector( 110000.25, 310000.75, 9100000.5 ) );

	int nActive = pParticles->m_nActiveParticles;
	for( int i=0; i < nActive; i+=4 )
	{
		FourVectors Coord = *pXYZ;
		Coord *= CoordScale;
		pAttr->x=AddSIMD( ValueBaseX, MulSIMD( ValueScaleX, NoiseSIMD( Coord ) ) );
		Coord += ofs_y;
		pAttr->y=AddSIMD( ValueBaseY, MulSIMD( ValueScaleY, NoiseSIMD( Coord ) ) );
		Coord += ofs_z;
		pAttr->z=AddSIMD( ValueBaseZ, MulSIMD( ValueScaleZ, NoiseSIMD( Coord ) ) );

		++pAttr;
		++pXYZ;
	}
}

//-----------------------------------------------------------------------------
// Decay Operator (Lifespan limiter - kills dead particles)
//-----------------------------------------------------------------------------
class C_OP_Decay : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_Decay );

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_Decay, "Lifespan Decay", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_Decay ) 
END_PARTICLE_OPERATOR_UNPACK( C_OP_Decay )


void C_OP_Decay::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );

	int nLimit = pParticles->m_nPaddedActiveParticles << 2;

	for ( int i = 0; i < nLimit; i+= 4 )
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		
		fltx4 fl4KillMask = CmpLeSIMD( fl4LifeDuration, Four_Zeros );

		fltx4 fl4Age = SubSIMD( fl4CurTime, *pCreationTime );
		
		fl4KillMask = OrSIMD( fl4KillMask, CmpGeSIMD( fl4Age, fl4LifeDuration ) );
		if ( IsAnyNegative( fl4KillMask ) )
		{
			// not especially pretty - we need to kill some particles.
			int nMask = TestSignSIMD( fl4KillMask );
			if ( nMask & 1 )
				pParticles->KillParticle( i );
			if ( nMask & 2 )
				pParticles->KillParticle( i + 1 );
			if ( nMask & 4 )
				pParticles->KillParticle( i + 2 );
			if ( nMask & 8 )
				pParticles->KillParticle( i + 3 );

		}
		++pCreationTime;
		++pLifeDuration;
	}
}



//-----------------------------------------------------------------------------
// Lifespan Minimum Velocity Decay Operator (kills particles if they cease moving)
//-----------------------------------------------------------------------------
class C_OP_VelocityDecay : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_VelocityDecay );

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	float m_flMinVelocity;
};

DEFINE_PARTICLE_OPERATOR( C_OP_VelocityDecay, "Lifespan Minimum Velocity Decay", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_VelocityDecay )
	DMXELEMENT_UNPACK_FIELD( "minimum velocity","1", float, m_flMinVelocity )
END_PARTICLE_OPERATOR_UNPACK( C_OP_VelocityDecay )


void C_OP_VelocityDecay::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	fltx4 fl4MinVelocity = ReplicateX4( m_flMinVelocity );
	fltx4 fl4Dt = ReplicateX4( pParticles->m_flDt );

	CM128AttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	CM128AttributeIterator pPrevXYZ( PARTICLE_ATTRIBUTE_PREV_XYZ, pParticles );

	int nLimit = pParticles->m_nPaddedActiveParticles << 2;

	for ( int i = 0; i < nLimit; i+= 4 )
	{
		fltx4 fl4KillMask = CmpLeSIMD( DivSIMD ( SubSIMD( *pXYZ, *pPrevXYZ ), fl4Dt ), fl4MinVelocity );

		if ( IsAnyNegative( fl4KillMask ) )
		{
			// not especially pretty - we need to kill some particles.
			int nMask = TestSignSIMD( fl4KillMask );
			if ( nMask & 1 )
				pParticles->KillParticle( i );
			if ( nMask & 2 )
				pParticles->KillParticle( i + 1 );
			if ( nMask & 4 )
				pParticles->KillParticle( i + 2 );
			if ( nMask & 8 )
				pParticles->KillParticle( i + 3 );
		}
		++pXYZ;
		++pPrevXYZ;
	}
}


//-----------------------------------------------------------------------------
// Random Cull Operator - Randomly culls particles before their lifespan
//-----------------------------------------------------------------------------
class C_OP_Cull : public CParticleOperatorInstance
{
	float m_flCullPerc;
	float m_flCullStart;
	float m_flCullEnd;
	float m_flCullExp;

	DECLARE_PARTICLE_OPERATOR( C_OP_Cull );

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK | PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_Cull, "Cull Random", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_Cull ) 
	DMXELEMENT_UNPACK_FIELD( "Cull Start Time", "0", float, m_flCullStart )
	DMXELEMENT_UNPACK_FIELD( "Cull End Time", "1", float, m_flCullEnd )
	DMXELEMENT_UNPACK_FIELD( "Cull Time Exponent", "1", float, m_flCullExp )
	DMXELEMENT_UNPACK_FIELD( "Cull Percentage", "0.5", float, m_flCullPerc )
END_PARTICLE_OPERATOR_UNPACK( C_OP_Cull )


void C_OP_Cull::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	const float *pCreationTime;
	const float *pLifeDuration;
	float flLifeTime;
	int nRandomOffset = pParticles->OperatorRandomSampleOffset();
	
	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		pCreationTime = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );
		pLifeDuration = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_LIFE_DURATION, i );
		int nParticleId = *pParticles->GetIntAttributePtr( PARTICLE_ATTRIBUTE_PARTICLE_ID, i );
		float flCullRank = pParticles->RandomFloat( nParticleId + nRandomOffset + 15, 0.0f, 1.0f);
		float flCullTime = pParticles->RandomFloatExp( nParticleId + nRandomOffset + 12, m_flCullStart, m_flCullEnd, m_flCullExp );

		if ( flCullRank > ( m_flCullPerc * flStrength ) )
		{
			continue;
		}
		// Find our life percentage
		flLifeTime = clamp( ( pParticles->m_flCurTime - *pCreationTime ) / ( *pLifeDuration ), 0.0f, 1.0f );
		if ( flLifeTime >= m_flCullStart && flLifeTime <= m_flCullEnd && flLifeTime >= flCullTime  )
		{
			pParticles->KillParticle( i );
		}
	}
}


//-----------------------------------------------------------------------------
// generic spin operator
//-----------------------------------------------------------------------------
class CGeneralSpin : public CParticleOperatorInstance
{
protected:
	virtual int GetAttributeToSpin( void ) const =0;

	uint32 GetWrittenAttributes( void ) const
	{
		if ( m_nSpinRateDegrees != 0.0 )
			return (1 << GetAttributeToSpin() );
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_fSpinRateRadians = (float) m_nSpinRateDegrees * ( M_PI / 180.0f );
		m_fSpinRateMinRadians = (float) m_nSpinRateMinDegrees * ( M_PI / 180.0f );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int	m_nSpinRateDegrees;
	int m_nSpinRateMinDegrees;
	float m_fSpinRateRadians;
	float m_fSpinRateStopTime;
	float m_fSpinRateMinRadians;
};

void CGeneralSpin::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	float fCurSpinRate = m_fSpinRateRadians * flStrength;

	if ( fCurSpinRate == 0.0 )
		return;

	float dt = pParticles->m_flDt;
	float drot = dt * fabs( fCurSpinRate * 2.0f * M_PI );
	if ( m_fSpinRateStopTime == 0.0f )
	{
		drot = fmod( drot, (float)(2.0f * M_PI) );
	}
	if ( fCurSpinRate < 0.0f )
	{
		drot = -drot;
	}
	fltx4 Rot_Add = ReplicateX4( drot );
	fltx4 Pi_2 = ReplicateX4( 2.0*M_PI );
	fltx4 nPi_2 = ReplicateX4( -2.0*M_PI );

	// FIXME: This is wrong
	fltx4 minSpeedRadians = ReplicateX4( dt * fabs( m_fSpinRateMinRadians * 2.0f * M_PI ) );

	fltx4 now = pParticles->m_fl4CurTime;
	fltx4 SpinRateStopTime = ReplicateX4( m_fSpinRateStopTime ); 

	CM128AttributeIterator pCreationTimeStamp( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );

	CM128AttributeWriteIterator pRot( GetAttributeToSpin(), pParticles );
		
	int nActive = pParticles->m_nActiveParticles;
	for( int i=0; i < nActive; i+=4 )
	{
		// HACK: Rather than redo this, I'm simply remapping the stop time into the percentage of lifetime, rather than seconds
		fltx4 LifeSpan = *pLifeDuration;
		fltx4 SpinFadePerc = Four_Zeros;
		fltx4 OOSpinFadeRate = Four_Zeros;
		if ( m_fSpinRateStopTime )
		{
			SpinFadePerc = MulSIMD( LifeSpan, SpinRateStopTime );
			OOSpinFadeRate = DivSIMD( Four_Ones,  SpinFadePerc );
		}
		
		fltx4 Age = SubSIMD( now, *pCreationTimeStamp );
		fltx4 RScale = MaxSIMD( Four_Zeros, 
								  SubSIMD( Four_Ones, MulSIMD( Age, OOSpinFadeRate ) ) );

		// Cap the rotation at a minimum speed
		fltx4 deltaRot = MulSIMD( Rot_Add, RScale );
		fltx4 Tooslow = CmpLeSIMD( deltaRot, minSpeedRadians );
		deltaRot = OrSIMD( AndSIMD( Tooslow, minSpeedRadians ), AndNotSIMD( Tooslow, deltaRot ) );
		fltx4 NewRot = AddSIMD( *pRot, deltaRot );

		// now, cap at +/- 2*pi		
		fltx4 Toobig =CmpGeSIMD( NewRot, Pi_2 );
		fltx4 Toosmall = CmpLeSIMD( NewRot, nPi_2 );
		
		NewRot = OrSIMD( AndSIMD( Toobig, SubSIMD( NewRot, Pi_2 ) ),
							AndNotSIMD( Toobig, NewRot ) );
		
		NewRot = OrSIMD( AndSIMD( Toosmall, AddSIMD( NewRot, Pi_2 ) ),
							AndNotSIMD( Toosmall, NewRot ) );

		*( pRot )= NewRot;

		++pRot;
		++pCreationTimeStamp;
		++pLifeDuration;
	}
}


//-----------------------------------------------------------------------------
// generic spin operator, version 2. Uses rotation_speed
//-----------------------------------------------------------------------------
class CSpinUpdateBase : public CParticleOperatorInstance
{
protected:
	virtual int GetAttributeToSpin( void ) const =0;
	virtual int GetSpinSpeedAttribute( void ) const =0;

	uint32 GetWrittenAttributes( void ) const
	{
		return (1 << GetAttributeToSpin() );
	}

	uint32 GetReadAttributes( void ) const
	{
		return ( 1 << GetAttributeToSpin() ) | ( 1 << GetSpinSpeedAttribute() ) |
			PARTICLE_ATTRIBUTE_CREATION_TIME_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

void CSpinUpdateBase::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTimeStamp( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pRotationSpeed( GetSpinSpeedAttribute(), pParticles );
	CM128AttributeWriteIterator pRot( GetAttributeToSpin(), pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;
	fltx4 fl4Dt = ReplicateX4( pParticles->m_flDt );
	fltx4 fl4ScaleFactor = ReplicateX4( flStrength );

	int nActive = pParticles->m_nActiveParticles;
	for( int i=0; i < nActive; i += 4 )
	{
		fltx4 fl4SimTime = MinSIMD( fl4Dt, SubSIMD( fl4CurTime, *pCreationTimeStamp ) );
		fl4SimTime = MulSIMD( fl4SimTime, fl4ScaleFactor );
		*pRot = MaddSIMD( fl4SimTime, *pRotationSpeed, *pRot );

		++pRot;
		++pRotationSpeed;
		++pCreationTimeStamp;
	}
}


class C_OP_Spin : public CGeneralSpin
{
	DECLARE_PARTICLE_OPERATOR( C_OP_Spin );

	int GetAttributeToSpin( void ) const
	{
		return PARTICLE_ATTRIBUTE_ROTATION;
	}
};

DEFINE_PARTICLE_OPERATOR( C_OP_Spin, "Rotation Spin Roll", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_Spin ) 
	DMXELEMENT_UNPACK_FIELD( "spin_rate_degrees", "0", int, m_nSpinRateDegrees )
	DMXELEMENT_UNPACK_FIELD( "spin_stop_time", "0", float, m_fSpinRateStopTime )
	DMXELEMENT_UNPACK_FIELD( "spin_rate_min", "0", int, m_nSpinRateMinDegrees )
END_PARTICLE_OPERATOR_UNPACK( C_OP_Spin )

class C_OP_SpinUpdate : public CSpinUpdateBase
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SpinUpdate );

	virtual int GetAttributeToSpin( void ) const
	{
		return PARTICLE_ATTRIBUTE_ROTATION;
	}

	virtual int GetSpinSpeedAttribute( void ) const
	{
		return PARTICLE_ATTRIBUTE_ROTATION_SPEED;
	}
};

DEFINE_PARTICLE_OPERATOR( C_OP_SpinUpdate, "Rotation Basic", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SpinUpdate ) 
END_PARTICLE_OPERATOR_UNPACK( C_OP_SpinUpdate )

class C_OP_SpinYaw : public CGeneralSpin
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SpinYaw );

	int GetAttributeToSpin( void ) const
	{
		return PARTICLE_ATTRIBUTE_YAW;
	}
};

DEFINE_PARTICLE_OPERATOR( C_OP_SpinYaw, "Rotation Spin Yaw", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SpinYaw ) 
	DMXELEMENT_UNPACK_FIELD( "yaw_rate_degrees", "0", int, m_nSpinRateDegrees )
	DMXELEMENT_UNPACK_FIELD( "yaw_stop_time", "0", float, m_fSpinRateStopTime )
	DMXELEMENT_UNPACK_FIELD( "yaw_rate_min", "0", int, m_nSpinRateMinDegrees )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SpinYaw )



//-----------------------------------------------------------------------------
// Size changing operator
//-----------------------------------------------------------------------------
class C_OP_InterpolateRadius : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_InterpolateRadius );

	uint32 GetReadInitialAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_RADIUS_MASK;
	}

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_RADIUS_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_flBias = ( m_flBias != 0.0f ) ? m_flBias : 0.5f;
		m_fl4BiasParam = PreCalcBiasParameter( ReplicateX4( m_flBias ) );
	}

	float m_flStartTime;
	float m_flEndTime;
	float m_flStartScale;
	float m_flEndScale;
	bool m_bEaseInAndOut;
	float m_flBias;
	fltx4 m_fl4BiasParam;
};

DEFINE_PARTICLE_OPERATOR( C_OP_InterpolateRadius, "Radius Scale", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_InterpolateRadius ) 
	DMXELEMENT_UNPACK_FIELD( "start_time", "0", float, m_flStartTime )
	DMXELEMENT_UNPACK_FIELD( "end_time", "1", float, m_flEndTime )
	DMXELEMENT_UNPACK_FIELD( "radius_start_scale", "1", float, m_flStartScale )
	DMXELEMENT_UNPACK_FIELD( "radius_end_scale", "1", float, m_flEndScale )
	DMXELEMENT_UNPACK_FIELD( "ease_in_and_out", "0", bool, m_bEaseInAndOut )
	DMXELEMENT_UNPACK_FIELD( "scale_bias", "0.5", float, m_flBias )
END_PARTICLE_OPERATOR_UNPACK( C_OP_InterpolateRadius )

void C_OP_InterpolateRadius::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	if ( m_flEndTime <= m_flStartTime )
		return;
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128AttributeWriteIterator pRadius( PARTICLE_ATTRIBUTE_RADIUS, pParticles );
	CM128InitialAttributeIterator pInitialRadius( PARTICLE_ATTRIBUTE_RADIUS, pParticles );

	fltx4 fl4StartTime = ReplicateX4( m_flStartTime );
	fltx4 fl4EndTime = ReplicateX4( m_flEndTime );
	fltx4 fl4OOTimeWidth = ReciprocalSIMD( SubSIMD( fl4EndTime, fl4StartTime ) );

	fltx4 fl4ScaleWidth = ReplicateX4( m_flEndScale - m_flStartScale );
	fltx4 fl4StartScale = ReplicateX4( m_flStartScale );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nCtr = pParticles->m_nPaddedActiveParticles;

	if ( m_bEaseInAndOut )
	{
		do 
		{
			fltx4 fl4LifeDuration = *pLifeDuration;
			fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );
			fltx4 fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) ); // maybe need accurate div here?
			fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
			fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );
			if ( IsAnyNegative( fl4GoodMask ) )
			{
				fltx4 fl4FadeWindow = MulSIMD( SubSIMD( fl4LifeTime, fl4StartTime ), fl4OOTimeWidth );
				fl4FadeWindow = AddSIMD( fl4StartScale, MulSIMD( SimpleSpline( fl4FadeWindow ), fl4ScaleWidth ) );
				// !!speed!! - can anyone really tell the diff between spline and lerp here?
				*pRadius = MaskedAssign( 
					fl4GoodMask, MulSIMD( *pInitialRadius, fl4FadeWindow ), *pRadius );
			}
			++pCreationTime;
			++pLifeDuration;
			++pRadius;
			++pInitialRadius;
		} while (--nCtr );
	}
	else
	{
		if ( m_flBias == 0.5f )        // no bias case
		{
			do 
			{
				fltx4 fl4LifeDuration = *pLifeDuration;
				fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );
				fltx4 fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) ); // maybe need accurate div here?
				fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
				fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );
				if ( IsAnyNegative( fl4GoodMask ) )
				{
					fltx4 fl4FadeWindow = MulSIMD( SubSIMD( fl4LifeTime, fl4StartTime ), fl4OOTimeWidth );
					fl4FadeWindow = AddSIMD( fl4StartScale, MulSIMD( fl4FadeWindow, fl4ScaleWidth ) );
					*pRadius = MaskedAssign( fl4GoodMask, MulSIMD( *pInitialRadius, fl4FadeWindow ), *pRadius );
				}
				++pCreationTime;
				++pLifeDuration;
				++pRadius;
				++pInitialRadius;
			} while (--nCtr );
		}
		else
		{
			// use rational approximation to bias
			do 
			{
				fltx4 fl4LifeDuration = *pLifeDuration;
				fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );
				fltx4 fl4LifeTime = MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) ); // maybe need accurate div here?
				fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
				fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );
				if ( IsAnyNegative( fl4GoodMask ) )
				{
					fltx4 fl4FadeWindow = MulSIMD( SubSIMD( fl4LifeTime, fl4StartTime ), fl4OOTimeWidth );
#ifdef FP_EXCEPTIONS_ENABLED
					// Wherever fl4GoodMask is zero we need to ensure that fl4FadeWindow is not zero
					// to avoid 0/0 divides in BiasSIMD. Setting those elements to fl4EndTime
					// should do the trick...
					fl4FadeWindow = OrSIMD( AndSIMD( fl4GoodMask, fl4EndTime ), AndNotSIMD( fl4GoodMask, fl4EndTime ) );
#endif
					fl4FadeWindow = AddSIMD( fl4StartScale, MulSIMD( BiasSIMD( fl4FadeWindow, m_fl4BiasParam ), fl4ScaleWidth ) );
					*pRadius = MaskedAssign( 
						fl4GoodMask, 
						MulSIMD( *pInitialRadius, fl4FadeWindow ), *pRadius );
				}
				++pCreationTime;
				++pLifeDuration;
				++pRadius;
				++pInitialRadius;
			} while (--nCtr );
		}
	}
}



//-----------------------------------------------------------------------------
// Color Fade
//-----------------------------------------------------------------------------
class C_OP_ColorInterpolate : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_ColorInterpolate );

	uint32 GetReadInitialAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_TINT_RGB_MASK;
	}

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_TINT_RGB_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_flColorFade[0] = m_ColorFade[0] / 255.0f;
		m_flColorFade[1] = m_ColorFade[1] / 255.0f;
		m_flColorFade[2] = m_ColorFade[2] / 255.0f;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	Color	m_ColorFade;
	float	m_flColorFade[3];
	float	m_flFadeStartTime;
	float	m_flFadeEndTime;
	bool	m_bEaseInOut;
};



void C_OP_ColorInterpolate::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	C4VAttributeWriteIterator pColor( PARTICLE_ATTRIBUTE_TINT_RGB, pParticles );
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	C4VInitialAttributeIterator pInitialColor( PARTICLE_ATTRIBUTE_TINT_RGB, pParticles );
	if ( m_flFadeEndTime == m_flFadeStartTime )
		return;

	fltx4 ooInRange = ReplicateX4( 1.0 / ( m_flFadeEndTime - m_flFadeStartTime ) );

	fltx4 curTime = pParticles->m_fl4CurTime;
	fltx4 lowRange = ReplicateX4( m_flFadeStartTime );

	fltx4 targetR = ReplicateX4( m_flColorFade[0] );
	fltx4 targetG = ReplicateX4( m_flColorFade[1] );
	fltx4 targetB = ReplicateX4( m_flColorFade[2] );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	if ( m_bEaseInOut )
	{
		do 
		{
			fltx4 goodMask = CmpGtSIMD( *pLifeDuration, Four_Zeros );
			if ( IsAnyNegative( goodMask ) )
			{
				fltx4 flLifeTime = DivSIMD( SubSIMD( curTime, *pCreationTime ), *pLifeDuration );
			
				fltx4 T = MulSIMD( SubSIMD( flLifeTime, lowRange ), ooInRange );
				T = MinSIMD( Four_Ones, MaxSIMD( Four_Zeros, T ) );
				T = SimpleSpline( T );
				pColor->x = MaskedAssign( goodMask, AddSIMD( pInitialColor->x, MulSIMD( T, SubSIMD( targetR, pInitialColor->x ) ) ), pColor->x );
				pColor->y = MaskedAssign( goodMask, AddSIMD( pInitialColor->y, MulSIMD( T, SubSIMD( targetG, pInitialColor->y ) ) ), pColor->y );
				pColor->z = MaskedAssign( goodMask, AddSIMD( pInitialColor->z, MulSIMD( T, SubSIMD( targetB, pInitialColor->z ) ) ), pColor->z );
			}
			++pColor;
			++pCreationTime;
			++pLifeDuration;
			++pInitialColor;

		} while( --nCtr );
	}
	else
	{
		do 
		{
			fltx4 goodMask = CmpGtSIMD( *pLifeDuration, Four_Zeros );
			if ( IsAnyNegative( goodMask ) )
			{
				fltx4 flLifeTime = DivSIMD( SubSIMD( curTime, *pCreationTime ), *pLifeDuration );
			
				fltx4 T = MulSIMD( SubSIMD( flLifeTime, lowRange ), ooInRange );
				T = MinSIMD( Four_Ones, MaxSIMD( Four_Zeros, T ) );
			
				pColor->x = MaskedAssign( goodMask, AddSIMD( pInitialColor->x, MulSIMD( T, SubSIMD( targetR, pInitialColor->x ) ) ), pColor->x );
				pColor->y = MaskedAssign( goodMask, AddSIMD( pInitialColor->y, MulSIMD( T, SubSIMD( targetG, pInitialColor->y ) ) ), pColor->y );
				pColor->z = MaskedAssign( goodMask, AddSIMD( pInitialColor->z, MulSIMD( T, SubSIMD( targetB, pInitialColor->z ) ) ), pColor->z );
			}
			++pColor;
			++pCreationTime;
			++pLifeDuration;
			++pInitialColor;

		} while( --nCtr );
	}
}

DEFINE_PARTICLE_OPERATOR( C_OP_ColorInterpolate, "Color Fade", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_ColorInterpolate ) 
	DMXELEMENT_UNPACK_FIELD( "color_fade", "255 255 255 255", Color, m_ColorFade )
	DMXELEMENT_UNPACK_FIELD( "fade_start_time", "0", float, m_flFadeStartTime )
	DMXELEMENT_UNPACK_FIELD( "fade_end_time", "1", float, m_flFadeEndTime )
	DMXELEMENT_UNPACK_FIELD( "ease_in_and_out", "1", bool, m_bEaseInOut )
END_PARTICLE_OPERATOR_UNPACK( C_OP_ColorInterpolate )


//-----------------------------------------------------------------------------
// Position Lock to Control Point
// Locks all particles to the specified control point
// Useful for making particles move with their emitter and so forth
//-----------------------------------------------------------------------------
class C_OP_PositionLock : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_PositionLock );

	struct C_OP_PositionLockContext_t
	{
		Vector m_vPrevPosition;
		matrix3x4_t m_matPrevTransform;
	};
	
	int m_nControlPointNumber;
	Vector m_vPrevPosition;
	float m_flStartTime_min;
	float m_flStartTime_max;
	float m_flStartTime_exp;
	float m_flEndTime_min;
	float m_flEndTime_max;
	float m_flEndTime_exp;
	float m_flRange;
	bool  m_bLockRot;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_PREV_XYZ_MASK | PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK |
			PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return 1ULL << m_nControlPointNumber;
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nControlPointNumber = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nControlPointNumber ) );
	}

	size_t GetRequiredContextBytes( void ) const
	{
		return sizeof( C_OP_PositionLockContext_t );
	}

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		C_OP_PositionLockContext_t *pCtx=reinterpret_cast<C_OP_PositionLockContext_t *>( pContext );
		pCtx->m_vPrevPosition = vec3_origin;
		pParticles->GetControlPointTransformAtTime( m_nControlPointNumber, pParticles->m_flCurTime, &pCtx->m_matPrevTransform );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_PositionLock , "Movement Lock to Control Point", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_PositionLock ) 
	DMXELEMENT_UNPACK_FIELD( "control_point_number", "0", int, m_nControlPointNumber )
	DMXELEMENT_UNPACK_FIELD( "start_fadeout_min", "1", float, m_flStartTime_min )
	DMXELEMENT_UNPACK_FIELD( "start_fadeout_max", "1", float, m_flStartTime_max )
	DMXELEMENT_UNPACK_FIELD( "start_fadeout_exponent", "1", float, m_flStartTime_exp )
	DMXELEMENT_UNPACK_FIELD( "end_fadeout_min", "1", float, m_flEndTime_min )
	DMXELEMENT_UNPACK_FIELD( "end_fadeout_max", "1", float, m_flEndTime_max )
	DMXELEMENT_UNPACK_FIELD( "end_fadeout_exponent", "1", float, m_flEndTime_exp )
	DMXELEMENT_UNPACK_FIELD( "distance fade range", "0", float, m_flRange )
	DMXELEMENT_UNPACK_FIELD( "lock rotation", "0", bool, m_bLockRot )
END_PARTICLE_OPERATOR_UNPACK( C_OP_PositionLock )

#ifdef OLD_NON_SSE_POSLOCK_FOR_TESTING
void C_OP_PositionLock::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	Vector vecControlPoint = pParticles->GetControlPointAtCurrentTime( m_nControlPointNumber );

	// At initialization, set prevposition to the control point to prevent random placements/velocities

	C_OP_PositionLockContext_t *pCtx=reinterpret_cast<C_OP_PositionLockContext_t *>( pContext );

	if ( pCtx->m_vPrevPosition == Vector (0, 0, 0) )

	{
		pCtx->m_vPrevPosition = vecControlPoint;
	}

	// Control point movement delta

	int nRandomOffset = pParticles->OperatorRandomSampleOffset();
	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		Vector vecPrevCPPos = pCtx->m_vPrevPosition;

		const float *pCreationTime;
		const float *pLifeDuration;	
		pCreationTime = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );
		pLifeDuration = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_LIFE_DURATION, i );
		float flLifeTime = *pLifeDuration != 0.0f ? clamp( ( pParticles->m_flCurTime - *pCreationTime ) / ( *pLifeDuration ), 0.0f, 1.0f ) : 0.0f;
		if ( *pCreationTime >= ( pParticles->m_flCurTime - pParticles->m_flDt ) )
		{
			pParticles->GetControlPointAtTime( m_nControlPointNumber, *pCreationTime, &vecPrevCPPos );
		}

		Vector vDelta = vecControlPoint - vecPrevCPPos;
		vDelta *= flStrength;

		// clamp activity to start/end time
		int nParticleId = *pParticles->GetIntAttributePtr( PARTICLE_ATTRIBUTE_PARTICLE_ID, i );
		float flStartTime = pParticles->RandomFloatExp( nParticleId + nRandomOffset + 9, m_flStartTime_min, m_flStartTime_max, m_flStartTime_exp );
		float flEndTime = pParticles->RandomFloatExp( nParticleId + nRandomOffset + 10, m_flEndTime_min, m_flEndTime_max, m_flEndTime_exp );

		// bias attachedness by fadeout
		float flLockScale = SimpleSplineRemapValClamped( flLifeTime, flStartTime, flEndTime, 1.0f, 0.0f );

		float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
		float *xyz_prev = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );

		Vector vecParticlePosition, vecParticlePosition_prev ;
		SetVectorFromAttribute( vecParticlePosition, xyz ); 
		SetVectorFromAttribute( vecParticlePosition_prev, xyz_prev ); 
		float flDampenAmount = 1;
		if ( m_flRange != 0 )
		{
			Vector ofs;
			ofs = (vecParticlePosition + ( vDelta * flLockScale ) ) - vecControlPoint;
			float flDistance = ofs.Length();
			flDampenAmount = SimpleSplineRemapValClamped( flDistance, 0, m_flRange, 1.0f, 0.0f );
			flDampenAmount = Bias( flDampenAmount, .2 );
		}
		Vector vParticleDelta = vDelta * flLockScale * flDampenAmount;


		vecParticlePosition += vParticleDelta;
		vecParticlePosition_prev += vParticleDelta;
		SetVectorAttribute( xyz, vecParticlePosition );
		SetVectorAttribute( xyz_prev, vecParticlePosition_prev );
	}

	// Store off the control point position for the next delta computation
	pCtx->m_vPrevPosition = vecControlPoint;


};

#else
void C_OP_PositionLock::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	Vector vecControlPoint = pParticles->GetControlPointAtCurrentTime( m_nControlPointNumber );

	// At initialization, set prevposition to the control point to prevent random placements/velocities

	C_OP_PositionLockContext_t *pCtx=reinterpret_cast<C_OP_PositionLockContext_t *>( pContext );

	if ( pCtx->m_vPrevPosition == Vector (0, 0, 0) )

	{
		pCtx->m_vPrevPosition = vecControlPoint;
		pParticles->GetControlPointTransformAtTime( m_nControlPointNumber, pParticles->m_flCurTime, &pCtx->m_matPrevTransform );
	}

	Vector vDelta;
	matrix3x4_t matCurrentTransform;
	matrix3x4_t matTransformLock;

	if ( m_bLockRot )
	{
		pParticles->GetControlPointTransformAtTime( m_nControlPointNumber, pParticles->m_flCurTime, &matCurrentTransform );
		matrix3x4_t matPrev;
		//if ( MatricesAreEqual ( matCurrentTransform, pCtx->m_matPrevTransform ) )
		//	return;
		MatrixInvert( pCtx->m_matPrevTransform, matPrev );
		MatrixMultiply( matCurrentTransform, matPrev, matTransformLock);
	}

	int nContext = GetSIMDRandContext();

	// Control point movement delta - not full transform
	vDelta 	= vecControlPoint - pCtx->m_vPrevPosition;
	//if ( vDelta == vec3_origin && !m_bLockRot )
	//	return;

	vDelta *= flStrength;
	FourVectors v4Delta;
	v4Delta.DuplicateVector( vDelta );

	FourVectors v4ControlPoint;
	v4ControlPoint.DuplicateVector( vecControlPoint );
	C4VAttributeWriteIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	C4VAttributeWriteIterator pPrevXYZ( PARTICLE_ATTRIBUTE_PREV_XYZ, pParticles );
	fltx4 fl4_Dt = ReplicateX4( pParticles->m_flDt );

	int nCtr = pParticles->m_nPaddedActiveParticles;
	bool bUseRange = ( m_flRange != 0.0 );
	fltx4 fl4OORange;
	if ( bUseRange )
		fl4OORange = ReplicateX4( 1.0 / m_flRange );

	fltx4 fl4BiasParm = PreCalcBiasParameter( ReplicateX4( 0.2 ) );
	if ( m_flStartTime_min >= 1.0 )							// always locked on
	{
		CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
		do 
		{
			fltx4 fl4ParticleAge = SubSIMD( pParticles->m_fl4CurTime, *pCreationTime);
			fltx4 fl4CreationFrameBias = MinSIMD( fl4ParticleAge, fl4_Dt );
			fl4CreationFrameBias = MulSIMD( DivSIMD( Four_Ones, fl4_Dt ), fl4CreationFrameBias );
			FourVectors v4ScaledDelta = v4Delta;			
			v4ScaledDelta *= fl4CreationFrameBias;

			fltx4 fl4LockStrength = ReplicateX4( flStrength );
			// ok, some of these particles should be moved
			if ( bUseRange )
			{
				FourVectors ofs = *pXYZ;
				ofs += v4ScaledDelta;
				ofs -= v4ControlPoint;
				fltx4 fl4Dist = ofs.length();
				fl4Dist = BiasSIMD( MinSIMD( Four_Ones, MulSIMD( fl4Dist, fl4OORange ) ), fl4BiasParm );
				v4ScaledDelta *= SubSIMD( Four_Ones, fl4Dist );
				fl4LockStrength = SubSIMD( Four_Ones, MulSIMD ( fl4Dist, fl4LockStrength ) );
			}
			if ( m_bLockRot )
			{
				fl4LockStrength = MulSIMD( fl4LockStrength, fl4CreationFrameBias );
				FourVectors fvCurPos = *pXYZ;
				FourVectors fvPrevPos = *pPrevXYZ;
				fvCurPos.TransformBy( matTransformLock );
				fvPrevPos.TransformBy( matTransformLock );
				fvCurPos -= *pXYZ;
				fvCurPos *= fl4LockStrength;
				fvPrevPos -= *pPrevXYZ;
				fvPrevPos *= fl4LockStrength;
				*(pXYZ) += fvCurPos;
				*(pPrevXYZ) += fvPrevPos;
			}
			else
			{
				*(pXYZ) += v4ScaledDelta;
				*(pPrevXYZ) += v4ScaledDelta;
			}
			++pCreationTime;
			++pXYZ;
			++pPrevXYZ;
		} while ( --nCtr );
	}
	else
	{
		CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
		CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
		fltx4 fl4CurTime = pParticles->m_fl4CurTime;
		fltx4 fl4StartRange = ReplicateX4( m_flStartTime_max - m_flStartTime_min );
		fltx4 fl4StartBias = ReplicateX4( m_flStartTime_min );
		fltx4 fl4EndRange = ReplicateX4( m_flEndTime_max - m_flEndTime_min );
		fltx4 fl4EndBias = ReplicateX4( m_flEndTime_min );
		int nSSEStartExponent = m_flStartTime_exp * 4.0;
		int nSSEEndExponent = m_flEndTime_exp * 4.0;
		do 
		{

			fltx4 fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime );
			fltx4 fl4CreationFrameBias = MinSIMD( fl4LifeTime, fl4_Dt );
			fl4CreationFrameBias = MulSIMD( DivSIMD( Four_Ones, fl4_Dt ), fl4CreationFrameBias );

			FourVectors v4ScaledDelta = v4Delta;			
			v4ScaledDelta *= fl4CreationFrameBias;

			fl4LifeTime = MaxSIMD( Four_Zeros, MinSIMD( Four_Ones,
														MulSIMD( fl4LifeTime, ReciprocalEstSIMD( *pLifeDuration ) ) ) );
			fltx4 fl4StartTime = Pow_FixedPoint_Exponent_SIMD( RandSIMD( nContext ), nSSEStartExponent );
			fl4StartTime = AddSIMD( fl4StartBias, MulSIMD( fl4StartTime, fl4StartRange ) );

			fltx4 fl4EndTime = Pow_FixedPoint_Exponent_SIMD( RandSIMD( nContext ), nSSEEndExponent );
			fl4EndTime = AddSIMD( fl4EndBias, MulSIMD( fl4EndTime, fl4EndRange ) );
	   
			// now, determine "lockedness"
			fltx4 fl4LockScale = DivSIMD( SubSIMD( fl4LifeTime, fl4StartTime ), SubSIMD( fl4EndTime, fl4StartTime ) );
			fl4LockScale = SubSIMD( Four_Ones, MaxSIMD( Four_Zeros, MinSIMD( Four_Ones, fl4LockScale ) ) );
			if ( IsAnyNegative( CmpGtSIMD( fl4LockScale, Four_Zeros ) ) )
			{
				//fl4LockScale = MulSIMD( fl4LockScale, fl4CreationFrameBias );
				v4ScaledDelta *= fl4LockScale;
				fltx4 fl4LockStrength = fl4LockScale ;
				// ok, some of these particles should be moved
				if ( bUseRange )
				{
					FourVectors ofs = *pXYZ;
					ofs += v4ScaledDelta;
					ofs -= v4ControlPoint;
					fltx4 fl4Dist = ofs.length();
					fl4Dist = BiasSIMD( MinSIMD( Four_Ones, MulSIMD( fl4Dist, fl4OORange ) ), fl4BiasParm );
					v4ScaledDelta *= SubSIMD( Four_Ones, fl4Dist );
					fl4LockStrength = SubSIMD( Four_Ones, MulSIMD ( fl4Dist, fl4LockStrength ) );
				}
				if ( m_bLockRot )
				{
					fl4LockStrength = MulSIMD( fl4LockStrength, fl4CreationFrameBias );
					FourVectors fvCurPos = *pXYZ;
					FourVectors fvPrevPos = *pPrevXYZ;
					fvCurPos.TransformBy( matTransformLock );
					fvPrevPos.TransformBy( matTransformLock );
					fvCurPos -= *pXYZ;
					fvCurPos *= fl4LockStrength;
					fvPrevPos -= *pPrevXYZ;
					fvPrevPos *= fl4LockStrength;
					*(pXYZ) += fvCurPos;
					*(pPrevXYZ) += fvPrevPos;
				}
				else
				{
					*(pXYZ) += v4ScaledDelta;
					*(pPrevXYZ) += v4ScaledDelta;
				}
			}
			++pCreationTime;
			++pLifeDuration;
			++pXYZ;
			++pPrevXYZ;
		} while ( --nCtr );
	}
	// Store off the control point position for the next delta computation
	pCtx->m_vPrevPosition = vecControlPoint;
	pCtx->m_matPrevTransform = matCurrentTransform;
	ReleaseSIMDRandContext( nContext );
};
#endif




//-----------------------------------------------------------------------------
// Controlpoint Light
// Determines particle color/fakes lighting using the influence of control
// points
//-----------------------------------------------------------------------------
class C_OP_ControlpointLight : public CParticleOperatorInstance
{
	float			m_flScale;
	LightDesc_t		m_LightNode1, m_LightNode2, m_LightNode3, m_LightNode4;
	int				m_nControlPoint1, m_nControlPoint2, m_nControlPoint3, m_nControlPoint4;
	Vector			m_vecCPOffset1, m_vecCPOffset2, m_vecCPOffset3, m_vecCPOffset4;
	float			m_LightFiftyDist1, m_LightZeroDist1, m_LightFiftyDist2, m_LightZeroDist2, 
					m_LightFiftyDist3, m_LightZeroDist3, m_LightFiftyDist4, m_LightZeroDist4;
	Color			m_LightColor1, m_LightColor2, m_LightColor3, m_LightColor4;
	bool			m_bLightType1, m_bLightType2, m_bLightType3, m_bLightType4, m_bLightDynamic1, 
					m_bLightDynamic2, m_bLightDynamic3, m_bLightDynamic4, m_bUseNormal, m_bUseHLambert, 
					m_bLightActive1, m_bLightActive2, m_bLightActive3, m_bLightActive4, 
					m_bClampLowerRange, m_bClampUpperRange;

	DECLARE_PARTICLE_OPERATOR( C_OP_ControlpointLight );

	uint32 GetReadInitialAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_TINT_RGB_MASK;
	}

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_TINT_RGB_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK;
	}
	
	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nControlPoint1 ) | ( 1ULL << m_nControlPoint2 ) | 
			   ( 1ULL << m_nControlPoint3 ) | ( 1ULL << m_nControlPoint4 );
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_LightNode1.m_Color[0] = m_LightColor1[0] / 255.0f;
		m_LightNode1.m_Color[1] = m_LightColor1[1] / 255.0f;
		m_LightNode1.m_Color[2] = m_LightColor1[2] / 255.0f;
		m_LightNode2.m_Color[0] = m_LightColor2[0] / 255.0f;
		m_LightNode2.m_Color[1] = m_LightColor2[1] / 255.0f;
		m_LightNode2.m_Color[2] = m_LightColor2[2] / 255.0f;
		m_LightNode3.m_Color[0] = m_LightColor3[0] / 255.0f;
		m_LightNode3.m_Color[1] = m_LightColor3[1] / 255.0f;
		m_LightNode3.m_Color[2] = m_LightColor3[2] / 255.0f;
		m_LightNode4.m_Color[0] = m_LightColor4[0] / 255.0f;
		m_LightNode4.m_Color[1] = m_LightColor4[1] / 255.0f;
		m_LightNode4.m_Color[2] = m_LightColor4[2] / 255.0f;
		m_LightNode1.m_Range = 0;
		m_LightNode2.m_Range = 0;
		m_LightNode3.m_Range = 0;
		m_LightNode4.m_Range = 0;
		m_LightNode1.m_Falloff=5.0;
		m_LightNode2.m_Falloff=5.0;
		m_LightNode3.m_Falloff=5.0;
		m_LightNode4.m_Falloff=5.0;
		m_LightNode1.m_Attenuation0 = 0;
		m_LightNode1.m_Attenuation1 = 0;
		m_LightNode1.m_Attenuation2 = 1;
		m_LightNode2.m_Attenuation0 = 0;
		m_LightNode2.m_Attenuation1 = 0;
		m_LightNode2.m_Attenuation2 = 1;
		m_LightNode3.m_Attenuation0 = 0;
		m_LightNode3.m_Attenuation1 = 0;
		m_LightNode3.m_Attenuation2 = 1;
		m_LightNode4.m_Attenuation0 = 0;
		m_LightNode4.m_Attenuation1 = 0;
		m_LightNode4.m_Attenuation2 = 1;

		if ( !m_bLightType1 )
		{
			m_LightNode1.m_Type = MATERIAL_LIGHT_POINT;
		}
		else
		{
			m_LightNode1.m_Type = MATERIAL_LIGHT_SPOT;
		}

		if ( !m_bLightType2 )
		{
			m_LightNode2.m_Type = MATERIAL_LIGHT_POINT;
		}
		else
		{
			m_LightNode2.m_Type = MATERIAL_LIGHT_SPOT;
		}

		if ( !m_bLightType3 )
		{
			m_LightNode3.m_Type = MATERIAL_LIGHT_POINT;
		}

		else
		{
			m_LightNode3.m_Type = MATERIAL_LIGHT_SPOT;
		}

		if ( !m_bLightType4 )
		{
			m_LightNode4.m_Type = MATERIAL_LIGHT_POINT;
		}
		else
		{
			m_LightNode4.m_Type = MATERIAL_LIGHT_SPOT;
		}

		if ( !m_bLightDynamic1 && ( m_LightColor1 != Color( 0, 0, 0, 255 ) ) )
		{
			m_bLightActive1 = true;
		}
		else
		{
			m_bLightActive1 = false;
		}
		if ( !m_bLightDynamic2 && ( m_LightColor2 != Color( 0, 0, 0, 255 ) ) )
		{
			m_bLightActive2 = true;
		}
		else
		{
			m_bLightActive2 = false;
		}
		if ( !m_bLightDynamic3 && ( m_LightColor3 != Color( 0, 0, 0, 255 ) ) )
		{
			m_bLightActive3 = true;
		}
		else
		{
			m_bLightActive3 = false;
		}
		if ( !m_bLightDynamic4 && ( m_LightColor4 != Color( 0, 0, 0, 255 ) ) )
		{
			m_bLightActive4 = true;
		}
		else
		{
			m_bLightActive4 = false;
		}
		m_LightNode1.SetupNewStyleAttenuation ( m_LightFiftyDist1, m_LightZeroDist1 );
 		m_LightNode2.SetupNewStyleAttenuation ( m_LightFiftyDist2, m_LightZeroDist2 );
 		m_LightNode3.SetupNewStyleAttenuation ( m_LightFiftyDist3, m_LightZeroDist3 );
 		m_LightNode4.SetupNewStyleAttenuation ( m_LightFiftyDist4, m_LightZeroDist4 );

	}

	void Render( CParticleCollection *pParticles ) const;

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_ControlpointLight, "Color Light from Control Point", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_ControlpointLight )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Control Point",  "0", int, m_nControlPoint1 )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Control Point Offset", "0 0 0", Vector, m_vecCPOffset1 )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Type 0=Point 1=Spot", "0", bool, m_bLightType1 )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Color", "0 0 0 255", Color, m_LightColor1 )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Dynamic Light", "0", bool, m_bLightDynamic1 )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Direction", "0 0 0", Vector, m_LightNode1.m_Direction )
	DMXELEMENT_UNPACK_FIELD( "Light 1 50% Distance", "100", float, m_LightFiftyDist1 )
	DMXELEMENT_UNPACK_FIELD( "Light 1 0% Distance", "200", float, m_LightZeroDist1 )	
	DMXELEMENT_UNPACK_FIELD( "Light 1 Spot Inner Cone", "30.0", float, m_LightNode1.m_Theta )
	DMXELEMENT_UNPACK_FIELD( "Light 1 Spot Outer Cone", "45.0", float, m_LightNode1.m_Phi )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Control Point",  "0", int, m_nControlPoint2 )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Control Point Offset", "0 0 0", Vector, m_vecCPOffset2 )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Type 0=Point 1=Spot", "0", bool, m_bLightType2 )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Color", "0 0 0 255", Color, m_LightColor2 )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Dynamic Light", "0", bool, m_bLightDynamic2 )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Direction", "0 0 0", Vector, m_LightNode2.m_Direction )
	DMXELEMENT_UNPACK_FIELD( "Light 2 50% Distance", "100", float, m_LightFiftyDist2 )
	DMXELEMENT_UNPACK_FIELD( "Light 2 0% Distance", "200", float, m_LightZeroDist2 )	
	DMXELEMENT_UNPACK_FIELD( "Light 2 Spot Inner Cone", "30.0", float, m_LightNode2.m_Theta )
	DMXELEMENT_UNPACK_FIELD( "Light 2 Spot Outer Cone", "45.0", float, m_LightNode2.m_Phi )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Control Point",  "0", int, m_nControlPoint3 )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Control Point Offset", "0 0 0", Vector, m_vecCPOffset3 )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Type 0=Point 1=Spot", "0", bool, m_bLightType3 )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Color", "0 0 0 255", Color, m_LightColor3 )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Dynamic Light", "0", bool, m_bLightDynamic3 )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Direction", "0 0 0", Vector, m_LightNode3.m_Direction )
	DMXELEMENT_UNPACK_FIELD( "Light 3 50% Distance", "100", float, m_LightFiftyDist3 )
	DMXELEMENT_UNPACK_FIELD( "Light 3 0% Distance", "200", float, m_LightZeroDist3 )	
	DMXELEMENT_UNPACK_FIELD( "Light 3 Spot Inner Cone", "30.0", float, m_LightNode3.m_Theta )
	DMXELEMENT_UNPACK_FIELD( "Light 3 Spot Outer Cone", "45.0", float, m_LightNode3.m_Phi )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Control Point",  "0", int, m_nControlPoint4 )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Control Point Offset", "0 0 0", Vector, m_vecCPOffset4 )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Type 0=Point 1=Spot", "0", bool, m_bLightType4 )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Color", "0 0 0 255", Color, m_LightColor4 )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Dynamic Light", "0", bool, m_bLightDynamic4 )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Direction", "0 0 0", Vector, m_LightNode4.m_Direction )
	DMXELEMENT_UNPACK_FIELD( "Light 4 50% Distance", "100", float, m_LightFiftyDist4 )
	DMXELEMENT_UNPACK_FIELD( "Light 4 0% Distance", "200", float, m_LightZeroDist4 )	
	DMXELEMENT_UNPACK_FIELD( "Light 4 Spot Inner Cone", "30.0", float, m_LightNode4.m_Theta )
	DMXELEMENT_UNPACK_FIELD( "Light 4 Spot Outer Cone", "45.0", float, m_LightNode4.m_Phi )
	DMXELEMENT_UNPACK_FIELD( "Initial Color Bias", "0.0", float, m_flScale )
	DMXELEMENT_UNPACK_FIELD( "Clamp Minimum Light Value to Initial Color", "0", bool, m_bClampLowerRange )
	DMXELEMENT_UNPACK_FIELD( "Clamp Maximum Light Value to Initial Color", "0", bool, m_bClampUpperRange )
	DMXELEMENT_UNPACK_FIELD( "Compute Normals From Control Points", "0", bool, m_bUseNormal )
	DMXELEMENT_UNPACK_FIELD( "Half-Lambert Normals", "1", bool, m_bUseHLambert )
END_PARTICLE_OPERATOR_UNPACK( C_OP_ControlpointLight )

void C_OP_ControlpointLight::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	//Set up location of each light - this needs to be done every time as the CP's can move
	Vector vecLocation1, vecLocation2, vecLocation3, vecLocation4;
	vecLocation1 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint1 );
	vecLocation2 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint2 );
	vecLocation3 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint3 );
	vecLocation4 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint4 );


	LightDesc_t LightNode1 = m_LightNode1;
	LightDesc_t LightNode2 = m_LightNode2;
	LightDesc_t LightNode3 = m_LightNode3;
	LightDesc_t LightNode4 = m_LightNode3;

	// Apply any offsets
	LightNode1.m_Position = vecLocation1 + m_vecCPOffset1;
	LightNode2.m_Position = vecLocation2 + m_vecCPOffset2;
	LightNode3.m_Position = vecLocation3 + m_vecCPOffset3;
	LightNode4.m_Position = vecLocation4 + m_vecCPOffset4;


	C4VAttributeIterator pInitialColor( PARTICLE_ATTRIBUTE_TINT_RGB, pParticles );
	C4VAttributeWriteIterator pColor( PARTICLE_ATTRIBUTE_TINT_RGB, pParticles );
	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	

	// Set up lighting conditions and attenuation
	if ( m_bLightDynamic1 )
	{
		// Get the color and luminosity at this position
		Color lc;
		g_pParticleSystemMgr->Query()->GetLightingAtPoint( LightNode1.m_Position, lc );
		LightNode1.m_Color[0] = lc[0] / 255.0f;
		LightNode1.m_Color[1] = lc[1] / 255.0f;
		LightNode1.m_Color[2] = lc[2] / 255.0f;
	}
	if ( m_bLightDynamic2 )
	{
		// Get the color and luminosity at this position
		Color lc;
		g_pParticleSystemMgr->Query()->GetLightingAtPoint( LightNode2.m_Position, lc );
		LightNode2.m_Color[0] = lc[0] / 255.0f;
		LightNode2.m_Color[1] = lc[1] / 255.0f;
		LightNode2.m_Color[2] = lc[2] / 255.0f;
	}
	if ( m_bLightDynamic3 )
	{
		// Get the color and luminosity at this position
		Color lc;
		g_pParticleSystemMgr->Query()->GetLightingAtPoint( LightNode3.m_Position, lc );
		LightNode3.m_Color[0] = lc[0] / 255.0f;
		LightNode3.m_Color[1] = lc[1] / 255.0f;
		LightNode3.m_Color[2] = lc[2] / 255.0f;
	}
	if ( m_bLightDynamic4 )
	{
		// Get the color and luminosity at this position
		Color lc;
		g_pParticleSystemMgr->Query()->GetLightingAtPoint( LightNode4.m_Position, lc );
		LightNode4.m_Color[0] = lc[0] / 255.0f;
		LightNode4.m_Color[1] = lc[1] / 255.0f;
		LightNode4.m_Color[2] = lc[2] / 255.0f;
	}
	LightNode1.RecalculateDerivedValues();
	LightNode2.RecalculateDerivedValues();
	LightNode3.RecalculateDerivedValues();
	LightNode4.RecalculateDerivedValues();
		
	FourVectors vScale;
	vScale.DuplicateVector( Vector(m_flScale, m_flScale, m_flScale) );
		
	if ( m_bUseNormal )
	{
		FourVectors vCPPosition1, vCPPosition2, vCPPosition3, vCPPosition4;
		//vCPPosition1.DuplicateVector( LightNode1.m_Position );
		vCPPosition1.DuplicateVector( vecLocation1 );
		vCPPosition2.DuplicateVector( vecLocation2 );
		vCPPosition3.DuplicateVector( vecLocation3 );
		vCPPosition4.DuplicateVector( vecLocation4 );

		int nCtr = pParticles->m_nPaddedActiveParticles;
		do 
		{
			FourVectors vLighting = vScale;				
			vLighting *= *pInitialColor;
			FourVectors vNormal = *pXYZ;
			vNormal -= vCPPosition1;
			vNormal.VectorNormalizeFast();
			LightNode1.ComputeLightAtPoints( *pXYZ, vNormal, vLighting, m_bUseHLambert );
			vNormal = *pXYZ;
			vNormal -= vCPPosition2;
			vNormal.VectorNormalizeFast();
			LightNode2.ComputeLightAtPoints( *pXYZ, vNormal, vLighting, m_bUseHLambert );
			vNormal = *pXYZ;
			vNormal -= vCPPosition3;
			vNormal.VectorNormalizeFast();
			LightNode3.ComputeLightAtPoints( *pXYZ, vNormal, vLighting, m_bUseHLambert );
			vNormal = *pXYZ;
			vNormal -= vCPPosition4;
			vNormal.VectorNormalizeFast();
			LightNode4.ComputeLightAtPoints( *pXYZ, vNormal, vLighting, m_bUseHLambert );
			
			if ( m_bClampLowerRange	)
			{
				FourVectors vInitialClamp = *pInitialColor;
				vLighting.x = MaxSIMD( vLighting.x, vInitialClamp.x );
				vLighting.y = MaxSIMD( vLighting.y, vInitialClamp.y );
				vLighting.z = MaxSIMD( vLighting.z, vInitialClamp.z );
			}
			else
			{
				vLighting.x = MaxSIMD( vLighting.x, Four_Zeros );
				vLighting.y = MaxSIMD( vLighting.y, Four_Zeros );
				vLighting.z = MaxSIMD( vLighting.z, Four_Zeros );
			}
			if ( m_bClampUpperRange	)
			{
				FourVectors vInitialClamp = *pInitialColor;
				vLighting.x = MinSIMD( vLighting.x, vInitialClamp.x );
				vLighting.y = MinSIMD( vLighting.y, vInitialClamp.y );
				vLighting.z = MinSIMD( vLighting.z, vInitialClamp.z );
			}
			else
			{
				vLighting.x = MinSIMD( vLighting.x, Four_Ones );
				vLighting.y = MinSIMD( vLighting.y, Four_Ones );
				vLighting.z = MinSIMD( vLighting.z, Four_Ones );
			}
			
			*pColor = vLighting;
			
			++pColor;
			++pXYZ;
			++pInitialColor;
		} while (--nCtr);
	}
	else
	{
		int nCtr = pParticles->m_nPaddedActiveParticles;
		do 
		{
			FourVectors vLighting = vScale;				
			vLighting *= *pInitialColor;
			
			LightNode1.ComputeNonincidenceLightAtPoints( *pXYZ, vLighting );
			LightNode2.ComputeNonincidenceLightAtPoints( *pXYZ, vLighting );
			LightNode3.ComputeNonincidenceLightAtPoints( *pXYZ, vLighting );
			LightNode4.ComputeNonincidenceLightAtPoints( *pXYZ, vLighting );
			
			
			if ( m_bClampLowerRange	)
			{
				FourVectors vInitialClamp = *pInitialColor;
				vLighting.x = MaxSIMD( vLighting.x, vInitialClamp.x );
				vLighting.y = MaxSIMD( vLighting.y, vInitialClamp.y );
				vLighting.z = MaxSIMD( vLighting.z, vInitialClamp.z );
			}
			else
			{
				vLighting.x = MaxSIMD( vLighting.x, Four_Zeros );
				vLighting.y = MaxSIMD( vLighting.y, Four_Zeros );
				vLighting.z = MaxSIMD( vLighting.z, Four_Zeros );
			}
			if ( m_bClampUpperRange	)
			{
				FourVectors vInitialClamp = *pInitialColor;
				vLighting.x = MinSIMD( vLighting.x, vInitialClamp.x );
				vLighting.y = MinSIMD( vLighting.y, vInitialClamp.y );
				vLighting.z = MinSIMD( vLighting.z, vInitialClamp.z );
			}
			else
			{
				vLighting.x = MinSIMD( vLighting.x, Four_Ones );
				vLighting.y = MinSIMD( vLighting.y, Four_Ones );
				vLighting.z = MinSIMD( vLighting.z, Four_Ones );
			}
			
			
			*pColor = vLighting;
			
			++pColor;
			++pXYZ;
			++pInitialColor;
		} while (--nCtr);
	}
};


//-----------------------------------------------------------------------------
// Render visualization
//-----------------------------------------------------------------------------
void C_OP_ControlpointLight::Render( CParticleCollection *pParticles ) const
{					   
	Vector vecOrigin1 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint1 );
	vecOrigin1 += m_vecCPOffset1;
	Vector vecOrigin2 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint2 );
	vecOrigin2 += m_vecCPOffset2;
	Vector vecOrigin3 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint3 );
	vecOrigin3 += m_vecCPOffset3;
	Vector vecOrigin4 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint4 );
	vecOrigin4 += m_vecCPOffset4;

	Color LightColor1Outer;
	LightColor1Outer[0] = m_LightColor1[0] / 2.0f;
	LightColor1Outer[1] = m_LightColor1[1] / 2.0f;
	LightColor1Outer[2] = m_LightColor1[2] / 2.0f;
	LightColor1Outer[3] = 255;
	Color LightColor2Outer;
	LightColor2Outer[0] = m_LightColor2[0] / 2.0f;
	LightColor2Outer[1] = m_LightColor2[1] / 2.0f;
	LightColor2Outer[2] = m_LightColor2[2] / 2.0f;
	LightColor2Outer[3] = 255;
	Color LightColor3Outer;
	LightColor3Outer[0] = m_LightColor3[0] / 2.0f;
	LightColor3Outer[1] = m_LightColor3[1] / 2.0f;
	LightColor3Outer[2] = m_LightColor3[2] / 2.0f;
	LightColor3Outer[3] = 255;
	Color LightColor4Outer;
	LightColor4Outer[0] = m_LightColor4[0] / 2.0f;
	LightColor4Outer[1] = m_LightColor4[1] / 2.0f;
	LightColor4Outer[2] = m_LightColor4[2] / 2.0f;
	LightColor4Outer[3] = 255;
	if ( m_bLightActive1 )
	{
		RenderWireframeSphere( vecOrigin1, m_LightFiftyDist1, 16, 8, m_LightColor1, false );
		RenderWireframeSphere( vecOrigin1, m_LightZeroDist1, 16, 8, LightColor1Outer, false );
	}
	if ( m_bLightActive2 )
	{	
		RenderWireframeSphere( vecOrigin2, m_LightFiftyDist2, 16, 8, m_LightColor2, false );
		RenderWireframeSphere( vecOrigin2, m_LightZeroDist2, 16, 8, LightColor2Outer, false );
	}
	if ( m_bLightActive3 )
	{
		RenderWireframeSphere( vecOrigin3, m_LightFiftyDist3, 16, 8, m_LightColor3, false );
		RenderWireframeSphere( vecOrigin3, m_LightZeroDist3, 16, 8, LightColor3Outer, false );
	}
	if ( m_bLightActive4 )
	{
		RenderWireframeSphere( vecOrigin4, m_LightFiftyDist4, 16, 8, m_LightColor4, false );
		RenderWireframeSphere( vecOrigin4, m_LightZeroDist4, 16, 8, LightColor4Outer, false );
	}
	
}



// set child controlpoints - copy the positions of our particles to the control points of a child
class C_OP_SetChildControlPoints : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetChildControlPoints );

	int m_nChildGroupID;
	int m_nFirstControlPoint;
	int m_nNumControlPoints;
	int m_nFirstSourcePoint;

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_SetChildControlPoints, "Set child control points from particle positions", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetChildControlPoints ) 
	DMXELEMENT_UNPACK_FIELD( "Group ID to affect", "0", int, m_nChildGroupID )
	DMXELEMENT_UNPACK_FIELD( "First control point to set", "0", int, m_nFirstControlPoint )
	DMXELEMENT_UNPACK_FIELD( "# of control points to set", "1", int, m_nNumControlPoints )
	DMXELEMENT_UNPACK_FIELD( "first particle to copy", "0", int, m_nFirstSourcePoint )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetChildControlPoints )


void C_OP_SetChildControlPoints::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	int nFirst=max(0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nFirstControlPoint ) );
	int nToSet=min( pParticles->m_nActiveParticles-m_nFirstSourcePoint, m_nNumControlPoints );
	nToSet=min( nToSet, MAX_PARTICLE_CONTROL_POINTS-nFirst );
	if ( nToSet )
	{
		for( CParticleCollection *pChild = pParticles->m_Children.m_pHead; pChild; pChild = pChild->m_pNext )
		{
			if ( pChild->GetGroupID() == m_nChildGroupID )
			{
				for( int p=0; p < nToSet; p++ )
				{
					const float *pXYZ = pParticles->GetFloatAttributePtr( 
						PARTICLE_ATTRIBUTE_XYZ, p + m_nFirstSourcePoint );
					Vector cPnt( pXYZ[0], pXYZ[4], pXYZ[8] );
					pChild->SetControlPoint( p+nFirst, cPnt );
				}
			}
		}
	}
}




//-----------------------------------------------------------------------------
// Set Control Point Positions
//-----------------------------------------------------------------------------
class C_OP_SetControlPointPositions : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetControlPointPositions );

	bool m_bUseWorldLocation;
	int m_nCP1, m_nCP1Parent;
	int m_nCP2, m_nCP2Parent;
	int m_nCP3, m_nCP3Parent;
	int m_nCP4, m_nCP4Parent;
	Vector m_vecCP1Pos, m_vecCP2Pos, m_vecCP3Pos, m_vecCP4Pos;
	int m_nHeadLocation;

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	bool ShouldRunBeforeEmitters( void ) const
	{
		return true;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;


};

DEFINE_PARTICLE_OPERATOR( C_OP_SetControlPointPositions, "Set Control Point Positions", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointPositions )
	DMXELEMENT_UNPACK_FIELD( "First Control Point Number", "1", int, m_nCP1 )
	DMXELEMENT_UNPACK_FIELD( "First Control Point Parent", "0", int, m_nCP1Parent )
	DMXELEMENT_UNPACK_FIELD( "First Control Point Location", "128 0 0", Vector, m_vecCP1Pos )
	DMXELEMENT_UNPACK_FIELD( "Second Control Point Number", "2", int, m_nCP2 )
	DMXELEMENT_UNPACK_FIELD( "Second Control Point Parent", "0", int, m_nCP2Parent )
	DMXELEMENT_UNPACK_FIELD( "Second Control Point Location", "0 128 0", Vector, m_vecCP2Pos )
	DMXELEMENT_UNPACK_FIELD( "Third Control Point Number", "3", int, m_nCP3 )
	DMXELEMENT_UNPACK_FIELD( "Third Control Point Parent", "0", int, m_nCP3Parent )
	DMXELEMENT_UNPACK_FIELD( "Third Control Point Location", "-128 0 0", Vector, m_vecCP3Pos )
	DMXELEMENT_UNPACK_FIELD( "Fourth Control Point Number", "4", int, m_nCP4 )
	DMXELEMENT_UNPACK_FIELD( "Fourth Control Point Parent", "0", int, m_nCP4Parent )
	DMXELEMENT_UNPACK_FIELD( "Fourth Control Point Location", "0 -128 0", Vector, m_vecCP4Pos )
	DMXELEMENT_UNPACK_FIELD( "Set positions in world space", "0", bool, m_bUseWorldLocation )
	DMXELEMENT_UNPACK_FIELD( "Control Point to offset positions from", "0", int, m_nHeadLocation )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointPositions )

void C_OP_SetControlPointPositions::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	if ( !m_bUseWorldLocation )
	{
		Vector vecControlPoint = pParticles->GetControlPointAtCurrentTime( m_nHeadLocation );
		matrix3x4_t mat;
		pParticles->GetControlPointTransformAtTime( m_nHeadLocation, pParticles->m_flCurTime, &mat );
		Vector vecTransformLocal = vec3_origin;

		VectorTransform( m_vecCP1Pos, mat, vecTransformLocal );
		pParticles->SetControlPoint( m_nCP1, vecTransformLocal );
		pParticles->SetControlPointParent( m_nCP1, m_nCP1Parent );

		VectorTransform( m_vecCP2Pos, mat, vecTransformLocal );
		pParticles->SetControlPoint( m_nCP2, vecTransformLocal );
		pParticles->SetControlPointParent( m_nCP2, m_nCP2Parent );

		VectorTransform( m_vecCP3Pos, mat, vecTransformLocal );
		pParticles->SetControlPoint( m_nCP3, vecTransformLocal );
		pParticles->SetControlPointParent( m_nCP3, m_nCP3Parent );

		VectorTransform( m_vecCP4Pos, mat, vecTransformLocal );
		pParticles->SetControlPoint( m_nCP4, vecTransformLocal );
		pParticles->SetControlPointParent( m_nCP4, m_nCP4Parent );
	}
	else
	{
		pParticles->SetControlPoint( m_nCP1, m_vecCP1Pos );
		pParticles->SetControlPointParent( m_nCP1, m_nCP1Parent );
		pParticles->SetControlPoint( m_nCP2, m_vecCP2Pos );
		pParticles->SetControlPointParent( m_nCP2, m_nCP2Parent );
		pParticles->SetControlPoint( m_nCP3, m_vecCP3Pos );
		pParticles->SetControlPointParent( m_nCP3, m_nCP3Parent );
		pParticles->SetControlPoint( m_nCP4, m_vecCP4Pos );
		pParticles->SetControlPointParent( m_nCP4, m_nCP4Parent );
	}
}


//-----------------------------------------------------------------------------
// Dampen Movement Relative to Control Point
// The closer a particle is the the assigned control point, the less
// it can move
//-----------------------------------------------------------------------------
class C_OP_DampenToCP : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_DampenToCP );

	int m_nControlPointNumber;
	float m_flRange, m_flScale;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK |
			PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nControlPointNumber );
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nControlPointNumber = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nControlPointNumber ) );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_DampenToCP , "Movement Dampen Relative to Control Point", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_DampenToCP ) 
DMXELEMENT_UNPACK_FIELD( "control_point_number", "0", int, m_nControlPointNumber )
DMXELEMENT_UNPACK_FIELD( "falloff range", "100", float, m_flRange )
DMXELEMENT_UNPACK_FIELD( "dampen scale", "1", float, m_flScale )
END_PARTICLE_OPERATOR_UNPACK( C_OP_DampenToCP )

void C_OP_DampenToCP::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	if ( m_flRange <= 0.0f )
		return;

	Vector vecControlPoint = pParticles->GetControlPointAtCurrentTime( m_nControlPointNumber );

	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
		float *xyz_prev = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );

		Vector vecParticlePosition, vecParticlePosition_prev, vParticleDelta ;

		SetVectorFromAttribute( vecParticlePosition, xyz ); 
		SetVectorFromAttribute( vecParticlePosition_prev, xyz_prev ); 
		Vector ofs;
		ofs = vecParticlePosition - vecControlPoint;
		float flDistance = ofs.Length();
		float flDampenAmount;
		if ( flDistance > m_flRange )
		{
			continue;
		}
		else
		{
			flDampenAmount = flDistance  / m_flRange;
			flDampenAmount = pow( flDampenAmount, m_flScale);
		}
		
		vParticleDelta = vecParticlePosition - vecParticlePosition_prev;
		Vector vParticleDampened = vParticleDelta * flDampenAmount;
		vecParticlePosition = vecParticlePosition_prev + vParticleDampened;
		Vector vecParticlePositionOrg;
		SetVectorFromAttribute( vecParticlePositionOrg, xyz ); 
		VectorLerp (vecParticlePositionOrg, vecParticlePosition, flStrength, vecParticlePosition );
		SetVectorAttribute( xyz, vecParticlePosition );
	}
};




//-----------------------------------------------------------------------------
// Distance Between CP Operator
//-----------------------------------------------------------------------------
class C_OP_DistanceBetweenCPs : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_DistanceBetweenCPs );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}
	uint32 GetReadInitialAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nStartCP ) | ( 1ULL << m_nEndCP );
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nCollisionGroupNumber = g_pParticleSystemMgr->Query()->GetCollisionGroupFromName( m_CollisionGroupName );
		m_nStartCP = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nStartCP ) );
		m_nEndCP = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nEndCP ) );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int		m_nFieldOutput;
	float	m_flInputMin;
	float	m_flInputMax;
	float	m_flOutputMin;
	float	m_flOutputMax;
	int		m_nStartCP;
	int		m_nEndCP;
	bool	m_bLOS;
	char	m_CollisionGroupName[128];
	int		m_nCollisionGroupNumber;
	float	m_flMaxTraceLength;
	float	m_flLOSScale;
	bool	m_bScaleInitialRange;
};

DEFINE_PARTICLE_OPERATOR( C_OP_DistanceBetweenCPs, "Remap Distance Between Two Control Points to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_DistanceBetweenCPs )
DMXELEMENT_UNPACK_FIELD( "distance minimum","0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "distance maximum","128", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
DMXELEMENT_UNPACK_FIELD( "starting control point","0", int, m_nStartCP )
DMXELEMENT_UNPACK_FIELD( "ending control point","1", int, m_nEndCP )
DMXELEMENT_UNPACK_FIELD( "ensure line of sight","0", bool, m_bLOS )
DMXELEMENT_UNPACK_FIELD_STRING( "LOS collision group", "NONE", m_CollisionGroupName )
DMXELEMENT_UNPACK_FIELD( "Maximum Trace Length", "-1", float, m_flMaxTraceLength )
DMXELEMENT_UNPACK_FIELD( "LOS Failure Scalar", "0", float, m_flLOSScale )
DMXELEMENT_UNPACK_FIELD( "output is scalar of initial random range","0", bool, m_bScaleInitialRange )
END_PARTICLE_OPERATOR_UNPACK( C_OP_DistanceBetweenCPs )

void C_OP_DistanceBetweenCPs::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	// clamp the result to 0 and 1 if it's alpha
	float flMin=m_flOutputMin;
	float flMax=m_flOutputMax;
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = clamp(m_flOutputMin, 0.0f, 1.0f );
		flMax = clamp(m_flOutputMax, 0.0f, 1.0f );
	}
	Vector vecControlPoint1 = pParticles->GetControlPointAtCurrentTime( m_nStartCP );
	Vector vecControlPoint2 = pParticles->GetControlPointAtCurrentTime( m_nEndCP );
	Vector vecDelta = vecControlPoint1 - vecControlPoint2;
	float flDistance = vecDelta.Length();


	if ( m_bLOS )
	{
		Vector vecEndPoint = vecControlPoint2;
		if ( m_flMaxTraceLength != -1.0f && m_flMaxTraceLength < flDistance )
		{
			VectorNormalize(vecEndPoint);
			vecEndPoint *= m_flMaxTraceLength;
			vecEndPoint += vecControlPoint1;
		}
		CBaseTrace tr;
		g_pParticleSystemMgr->Query()->TraceLine( vecControlPoint1, vecEndPoint, MASK_OPAQUE_AND_NPCS, NULL, m_nCollisionGroupNumber, &tr );
		if (tr.fraction != 1.0f)
		{
			flDistance *= tr.fraction * m_flLOSScale;
		}

	}

	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		float flOutput = RemapValClamped( flDistance, m_flInputMin, m_flInputMax, flMin, flMax  );
		if ( m_bScaleInitialRange )
		{
			const float *pInitialOutput = pParticles->GetInitialFloatAttributePtr( m_nFieldOutput, i );
			flOutput = *pInitialOutput * flOutput;
		}
		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );

		*pOutput = Lerp (flStrength, *pOutput, flOutput);
	}
}



//-----------------------------------------------------------------------------
// Distance to CP Operator
//-----------------------------------------------------------------------------
class C_OP_DistanceToCP : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_DistanceToCP );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nStartCP ) | ( 1ULL << m_nEndCP );
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nCollisionGroupNumber = g_pParticleSystemMgr->Query()->GetCollisionGroupFromName( m_CollisionGroupName );
		m_nStartCP = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nStartCP ) );
		m_nEndCP = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nEndCP ) );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int		m_nFieldOutput;
	float	m_flInputMin;
	float	m_flInputMax;
	float	m_flOutputMin;
	float	m_flOutputMax;
	int		m_nStartCP;
	int		m_nEndCP;
	bool	m_bLOS;
	char	m_CollisionGroupName[128];
	int		m_nCollisionGroupNumber;
	float	m_flMaxTraceLength;
	float	m_flLOSScale;
	bool	m_bScaleInitialRange;
	bool	m_bActiveRange;
};

DEFINE_PARTICLE_OPERATOR( C_OP_DistanceToCP, "Remap Distance to Control Point to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_DistanceToCP )
DMXELEMENT_UNPACK_FIELD( "distance minimum","0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "distance maximum","128", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
DMXELEMENT_UNPACK_FIELD( "control point","0", int, m_nStartCP )
DMXELEMENT_UNPACK_FIELD( "ensure line of sight","0", bool, m_bLOS )
DMXELEMENT_UNPACK_FIELD_STRING( "LOS collision group", "NONE", m_CollisionGroupName )
DMXELEMENT_UNPACK_FIELD( "Maximum Trace Length", "-1", float, m_flMaxTraceLength )
DMXELEMENT_UNPACK_FIELD( "LOS Failure Scalar", "0", float, m_flLOSScale )
DMXELEMENT_UNPACK_FIELD( "output is scalar of initial random range","0", bool, m_bScaleInitialRange )
DMXELEMENT_UNPACK_FIELD( "only active within specified distance","0", bool, m_bActiveRange )
END_PARTICLE_OPERATOR_UNPACK( C_OP_DistanceToCP )

void C_OP_DistanceToCP::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	// clamp the result to 0 and 1 if it's alpha
	float flMin=m_flOutputMin;
	float flMax=m_flOutputMax;
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = clamp(m_flOutputMin, 0.0f, 1.0f );
		flMax = clamp(m_flOutputMax, 0.0f, 1.0f );
	}
	Vector vecControlPoint1 = pParticles->GetControlPointAtCurrentTime( m_nStartCP );

	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		Vector vecPosition2;
		const float *pXYZ = pParticles->GetFloatAttributePtr(PARTICLE_ATTRIBUTE_XYZ, i );
		vecPosition2 = Vector(pXYZ[0], pXYZ[4], pXYZ[8]); 
		Vector vecDelta = vecControlPoint1 - vecPosition2;
		float flDistance = vecDelta.Length();
		if ( m_bActiveRange && ( flDistance < m_flInputMin || flDistance > m_flInputMax ) )
		{
			continue;
		}
		if ( m_bLOS )
		{
			Vector vecEndPoint = vecPosition2;
			if ( m_flMaxTraceLength != -1.0f && m_flMaxTraceLength < flDistance )
			{
				VectorNormalize(vecEndPoint);
				vecEndPoint *= m_flMaxTraceLength;
				vecEndPoint += vecControlPoint1;
			}
			CBaseTrace tr;
			g_pParticleSystemMgr->Query()->TraceLine( vecControlPoint1, vecEndPoint, MASK_OPAQUE_AND_NPCS, NULL , m_nCollisionGroupNumber, &tr );
			if (tr.fraction != 1.0f)
			{
				flDistance *= tr.fraction * m_flLOSScale;
			}

		}

		float flOutput = RemapValClamped( flDistance, m_flInputMin, m_flInputMax, flMin, flMax  );
		if ( m_bScaleInitialRange )
		{
			const float *pInitialOutput = pParticles->GetInitialFloatAttributePtr( m_nFieldOutput, i );
			flOutput = *pInitialOutput * flOutput;
		}
		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );

		*pOutput = Lerp (flStrength, *pOutput, flOutput);
			//float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
			//float flOutput = RemapValClamped( flDistance, m_flInputMin, m_flInputMax, flMin, flMax  );
			//*pOutput = Lerp (flStrength, *pOutput, flOutput);
	}
}

//-----------------------------------------------------------------------------
// Assign CP to Player
//-----------------------------------------------------------------------------
class C_OP_SetControlPointToPlayer : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetControlPointToPlayer );

	int m_nCP1;

	Vector m_vecCP1Pos;

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nCP1 = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nCP1 ) );
	}

	bool ShouldRunBeforeEmitters( void ) const
	{
		return true;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;


};

DEFINE_PARTICLE_OPERATOR( C_OP_SetControlPointToPlayer, "Set Control Point To Player", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointToPlayer )
DMXELEMENT_UNPACK_FIELD( "Control Point Number", "1", int, m_nCP1 )
DMXELEMENT_UNPACK_FIELD( "Control Point Offset", "0 0 0", Vector, m_vecCP1Pos )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointToPlayer )

void C_OP_SetControlPointToPlayer::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
		Vector vecClientPos =g_pParticleSystemMgr->Query()->GetLocalPlayerPos();
		pParticles->SetControlPoint( m_nCP1, m_vecCP1Pos + vecClientPos );
		Vector vecForward;
		Vector vecRight;
		Vector vecUp;
		g_pParticleSystemMgr->Query()->GetLocalPlayerEyeVectors( &vecForward, &vecRight, &vecUp);
		pParticles->SetControlPointOrientation( m_nCP1, vecForward, vecRight, vecUp );
}





//-------------------------
// Emits particles from particles
//NOT FINISHED
//-------------------------
class C_OP_PerParticleEmitter : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_PerParticleEmitter );
	
	struct C_OP_PerParticleEmitterContext_t
	{
		float	m_flTotalActualParticlesSoFar;
		int		m_nTotalEmittedSoFar;
		bool	m_bStoppedEmission;
	};

	int m_nChildGroupID;
	bool m_bInheritVelocity;
	float m_flEmitRate;
	float m_flVelocityScale;
	float m_flStartTime;
	float m_flEmissionDuration;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ | PARTICLE_ATTRIBUTE_PREV_XYZ;
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		if ( m_flEmitRate < 0.0f )
		{
			m_flEmitRate = 0.0f;
		}
		if ( m_flEmissionDuration < 0.0f )
		{
			m_flEmissionDuration = 0.0f;
		}
	}

	inline bool IsInfinitelyEmitting() const
	{
		return ( m_flEmissionDuration == 0.0f );
	}

	virtual bool MayCreateMoreParticles( CParticleCollection *pParticles, void *pContext ) const
	{
		C_OP_PerParticleEmitterContext_t *pCtx = reinterpret_cast<C_OP_PerParticleEmitterContext_t *>( pContext );
		if ( pCtx->m_bStoppedEmission )
			return false;

		if ( m_flEmitRate <= 0.0f )
			return false;

		if ( m_flEmissionDuration != 0.0f && ( pParticles->m_flCurTime - pParticles->m_flDt ) > ( m_flStartTime + m_flEmissionDuration ) )
			return false;

		return true;
	}

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		C_OP_PerParticleEmitterContext_t *pCtx=reinterpret_cast<C_OP_PerParticleEmitterContext_t *>( pContext );
		pCtx->m_flTotalActualParticlesSoFar = 0.0f;
		pCtx->m_nTotalEmittedSoFar = 0;
		pCtx->m_bStoppedEmission = false;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_PerParticleEmitter, "Per Particle Emitter", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_PerParticleEmitter ) 
DMXELEMENT_UNPACK_FIELD( "Group ID to affect", "1", int, m_nChildGroupID )
DMXELEMENT_UNPACK_FIELD( "Inherit Velocity", "0", int, m_bInheritVelocity )
DMXELEMENT_UNPACK_FIELD( "Emission Rate", "100", float, m_flEmitRate )
DMXELEMENT_UNPACK_FIELD( "Velocity Scale", "0", int, m_flVelocityScale )
DMXELEMENT_UNPACK_FIELD( "Emission Start Time", "0", float, m_flStartTime )
DMXELEMENT_UNPACK_FIELD( "Emission Duration", "0", float, m_flEmissionDuration )
END_PARTICLE_OPERATOR_UNPACK( C_OP_PerParticleEmitter )


void C_OP_PerParticleEmitter::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	for( CParticleCollection *pChild = pParticles->m_Children.m_pHead; pChild; pChild = pChild->m_pNext )
	{
		if ( pChild->GetGroupID() == m_nChildGroupID )
		{
			for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
			{
				C_OP_PerParticleEmitterContext_t *pCtx=reinterpret_cast<C_OP_PerParticleEmitterContext_t *>( pContext );
				const float *pXYZ = pParticles->GetFloatAttributePtr(
					PARTICLE_ATTRIBUTE_XYZ, i );
				const float *pXYZ_Prev = pParticles->GetFloatAttributePtr( 
					PARTICLE_ATTRIBUTE_PREV_XYZ, i );
				Vector vecParticlePosition, vecParticlePosition_prev, vParticleDelta ;

				vecParticlePosition = Vector ( pXYZ[0], pXYZ[4], pXYZ[8] ); 
				vecParticlePosition_prev = Vector ( pXYZ_Prev[0], pXYZ_Prev[4], pXYZ_Prev[8] );  
				vParticleDelta = vecParticlePosition - vecParticlePosition_prev;
	
				float flEmissionRate = m_flEmitRate * flStrength;

				if ( m_flVelocityScale != 0.0f )
				{
					float flVelocity = vParticleDelta.Length();
					flEmissionRate *= flVelocity * m_flVelocityScale * pParticles->m_flDt;
				}

				if ( flEmissionRate == 0.0f )
					continue;

				if ( !C_OP_PerParticleEmitter::MayCreateMoreParticles( pChild, pContext ) )
					continue;

				Assert( flEmissionRate != 0.0f );

				// determine our previous and current draw times and clamp them to start time and emission duration
				float flPrevDrawTime = pParticles->m_flCurTime - pParticles->m_flDt;
				float flCurrDrawTime = pParticles->m_flCurTime;

				if ( !IsInfinitelyEmitting() )
				{
					if ( flPrevDrawTime < m_flStartTime )
					{
						flPrevDrawTime = m_flStartTime;
					}
					if ( flCurrDrawTime > m_flStartTime + m_flEmissionDuration )
					{
						flCurrDrawTime = m_flStartTime + m_flEmissionDuration;
					}
				}

				float flDeltaTime = flCurrDrawTime - flPrevDrawTime;

				//Calculate emission rate by delta time from last frame to determine number of particles to emit this frame as a fractional float
				float flActualParticlesToEmit = flEmissionRate  * flDeltaTime;
				int nParticlesEmitted = pCtx->m_nTotalEmittedSoFar;
				//Add emitted particle to float counter to allow for fractional emission
				pCtx->m_flTotalActualParticlesSoFar += flActualParticlesToEmit;

				//Floor float accumulated value and subtract whole int emitted so far from the result to determine total whole particles to emit this frame
				int nParticlesToEmit = 	floor ( pCtx->m_flTotalActualParticlesSoFar ) - pCtx->m_nTotalEmittedSoFar;

				//Add emitted particles to running int total.
				pCtx->m_nTotalEmittedSoFar += nParticlesToEmit;


				if ( nParticlesToEmit == 0 )
					continue;

				// We're only allowed to emit so many particles, though..
				// If we run out of room, only emit the last N particles
				int nActualParticlesToEmit = nParticlesToEmit;
				int nAllowedParticlesToEmit = pChild->m_nMaxAllowedParticles - pParticles->m_nActiveParticles;
				if ( nAllowedParticlesToEmit < nParticlesToEmit )
				{
					nActualParticlesToEmit = nAllowedParticlesToEmit;
				}
				if ( nActualParticlesToEmit == 0 )
					continue;

				int nStartParticle = pChild->m_nActiveParticles;
				pChild->SetNActiveParticles( nActualParticlesToEmit + pChild->m_nActiveParticles );


				float flTimeStampStep = ( flDeltaTime ) / ( nActualParticlesToEmit );
				float flTimeStep = flPrevDrawTime + flTimeStampStep;
				Vector vecMoveStampStep = vParticleDelta / nActualParticlesToEmit ;
				Vector vecMoveStep = vecParticlePosition_prev + vecMoveStampStep ;

				if ( nParticlesEmitted != pChild->m_nActiveParticles )
				{

					uint32 nInittedMask = ( PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK | PARTICLE_ATTRIBUTE_CREATION_TIME_MASK );					
					// init newly emitted particles
					pChild->InitializeNewParticles( nParticlesEmitted, pChild->m_nActiveParticles - nParticlesEmitted, nInittedMask );
					//CHECKSYSTEM( this );
				}

				// Set the particle creation time to the exact sub-frame particle emission time
				// !! speed!! do sse init here
				for( int j = nStartParticle; j < nStartParticle + nActualParticlesToEmit; j++ )
				{
					float *pTimeStamp = pChild->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_CREATION_TIME, j );
					flTimeStep = min( flTimeStep, flCurrDrawTime );
					*pTimeStamp = flTimeStep;
					flTimeStep += flTimeStampStep;
					float *pXYZ_Child = pChild->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, j );
					float *pXYZ_Prev_Child = pChild->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, j );
					Vector vecChildXYZ;
					SetVectorFromAttribute ( vecChildXYZ, pXYZ_Child);
					vecChildXYZ = vecMoveStep;
					SetVectorAttribute ( pXYZ_Child, vecChildXYZ);
					vecMoveStep += vecMoveStampStep;
					if ( m_bInheritVelocity )
					{
						*pXYZ_Prev_Child = *pXYZ_Prev; 
					}
					else
					{
						*pXYZ_Prev_Child = *pXYZ_Child;
					}

				}

			}
		}
	}
}


class C_OP_LockToBone : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_LockToBone );

	int m_nControlPointNumber;
	float m_flLifeTimeFadeStart;
	float m_flLifeTimeFadeEnd;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		int ret= PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK |
			PARTICLE_ATTRIBUTE_HITBOX_RELATIVE_XYZ_MASK | PARTICLE_ATTRIBUTE_HITBOX_INDEX_MASK;
		ret |= PARTICLE_ATTRIBUTE_CREATION_TIME_MASK;
		return ret;

	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nControlPointNumber );
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nControlPointNumber = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nControlPointNumber ) );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_LockToBone , "Movement Lock to Bone", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_LockToBone ) 
	DMXELEMENT_UNPACK_FIELD( "control_point_number", "0", int, m_nControlPointNumber )
	DMXELEMENT_UNPACK_FIELD( "lifetime fade start", "0", float, m_flLifeTimeFadeStart )
	DMXELEMENT_UNPACK_FIELD( "lifetime fade end", "0", float, m_flLifeTimeFadeEnd )
END_PARTICLE_OPERATOR_UNPACK( C_OP_LockToBone )

void C_OP_LockToBone::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	pParticles->UpdateHitBoxInfo( m_nControlPointNumber );
	if ( pParticles->m_ControlPointHitBoxes[m_nControlPointNumber].CurAndPrevValid() )
	{
		float flAgeThreshold = m_flLifeTimeFadeEnd;
		if ( flAgeThreshold <= 0.0 )
			flAgeThreshold = 1.0e20;
		float flIScale = 0.0;
		if ( m_flLifeTimeFadeEnd > m_flLifeTimeFadeStart )
			flIScale = 1.0/( m_flLifeTimeFadeEnd - m_flLifeTimeFadeStart );

		for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
		{
			float *pXYZ = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
			float *pPrevXYZ = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );
			const float *pUVW = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_HITBOX_RELATIVE_XYZ, i );
			const int nBoxIndex = *pParticles->GetIntAttributePtr( PARTICLE_ATTRIBUTE_HITBOX_INDEX, i );
			float const *pCreationTime = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );
			
			float flAge = pParticles->m_flCurTime -*pCreationTime;

			if ( flAge < flAgeThreshold )
			{
				if (
					( nBoxIndex < pParticles->m_ControlPointHitBoxes[m_nControlPointNumber].m_nNumHitBoxes ) &&
					( nBoxIndex < pParticles->m_ControlPointHitBoxes[m_nControlPointNumber].m_nNumPrevHitBoxes ) &&
					( nBoxIndex >= 0 )
					)
				{
					Vector vecParticlePosition;
					ModelHitBoxInfo_t const &hb = pParticles->m_ControlPointHitBoxes[m_nControlPointNumber].m_pHitBoxes[ nBoxIndex ];
					vecParticlePosition.x = Lerp( pUVW[0], hb.m_vecBoxMins.x, hb.m_vecBoxMaxes.x );
					vecParticlePosition.y = Lerp( pUVW[4], hb.m_vecBoxMins.y, hb.m_vecBoxMaxes.y );
					vecParticlePosition.z = Lerp( pUVW[8], hb.m_vecBoxMins.z, hb.m_vecBoxMaxes.z );
					Vector vecWorldPosition;
					VectorTransform( vecParticlePosition, hb.m_Transform, vecWorldPosition );
				
					Vector vecPrevParticlePosition;
					ModelHitBoxInfo_t phb = pParticles->m_ControlPointHitBoxes[m_nControlPointNumber].m_pPrevBoxes[ nBoxIndex ];
					vecPrevParticlePosition.x = Lerp( pUVW[0], phb.m_vecBoxMins.x, phb.m_vecBoxMaxes.x );
					vecPrevParticlePosition.y = Lerp( pUVW[4], phb.m_vecBoxMins.y, phb.m_vecBoxMaxes.y );
					vecPrevParticlePosition.z = Lerp( pUVW[8], phb.m_vecBoxMins.z, phb.m_vecBoxMaxes.z );
					Vector vecPrevWorldPosition;
					VectorTransform( vecPrevParticlePosition, phb.m_Transform, vecPrevWorldPosition );
				
					Vector Delta = vecWorldPosition-vecPrevWorldPosition;
				
					if ( flAge > m_flLifeTimeFadeStart )
						Delta *= flStrength * ( 1.0- ( ( flAge - m_flLifeTimeFadeStart ) * flIScale ) );
				
					Vector xyz;
					SetVectorFromAttribute( xyz, pXYZ );
					xyz += Delta;
					SetVectorAttribute( pXYZ, xyz );
				
					Vector prevxyz;
					SetVectorFromAttribute( prevxyz, pPrevXYZ );
					prevxyz += Delta;
					SetVectorAttribute( pPrevXYZ, prevxyz );
				}
			}
		}
	}
};
	
//-----------------------------------------------------------------------------
// Plane Cull Operator - cull particles on the "wrong" side of a plane
//-----------------------------------------------------------------------------
class C_OP_PlaneCull : public CParticleOperatorInstance
{
	int m_nPlaneControlPoint;
	Vector m_vecPlaneDirection;
	float m_flPlaneOffset;

	DECLARE_PARTICLE_OPERATOR( C_OP_PlaneCull );

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nPlaneControlPoint );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_PlaneCull, "Cull when crossing plane", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_PlaneCull ) 
	DMXELEMENT_UNPACK_FIELD( "Control Point for point on plane", "0", int, m_nPlaneControlPoint )
	DMXELEMENT_UNPACK_FIELD( "Cull plane offset", "0", float, m_flPlaneOffset )
	DMXELEMENT_UNPACK_FIELD( "Plane Normal", "0 0 1", Vector, m_vecPlaneDirection )
END_PARTICLE_OPERATOR_UNPACK( C_OP_PlaneCull )

void C_OP_PlaneCull::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	int nLimit = pParticles->m_nPaddedActiveParticles << 2;

	// setup vars
	FourVectors v4N ;
	v4N.DuplicateVector( m_vecPlaneDirection );
	v4N.VectorNormalize();
	FourVectors v4Pnt;
	v4Pnt.DuplicateVector( pParticles->GetControlPointAtCurrentTime( m_nPlaneControlPoint ) );
	FourVectors ofs = v4N;
	ofs *= ReplicateX4( m_flPlaneOffset );
	v4Pnt -= ofs;
	
	for ( int i = 0; i < nLimit; i+= 4 )
	{
		FourVectors f4PlaneRel = (*pXYZ );
		f4PlaneRel -= v4Pnt;
		fltx4 fl4PlaneEq = ( f4PlaneRel * v4N );
		if ( IsAnyNegative( fl4PlaneEq ) )
		{
			// not especially pretty - we need to kill some particles.
			int nMask = TestSignSIMD( fl4PlaneEq );
			if ( nMask & 1 )
				pParticles->KillParticle( i );
			if ( nMask & 2 )
				pParticles->KillParticle( i + 1 );
			if ( nMask & 4 )
				pParticles->KillParticle( i + 2 );
			if ( nMask & 8 )
				pParticles->KillParticle( i + 3 );
			
		}
		++pXYZ;
	}
}

//-----------------------------------------------------------------------------
// Model Cull Operator - cull particles inside or outside of a brush/animated model
//-----------------------------------------------------------------------------
class C_OP_ModelCull : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_ModelCull );

	int m_nControlPointNumber;
	bool m_bBoundBox;
	bool m_bCullOutside;

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK;
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nControlPointNumber = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nControlPointNumber ) );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_ModelCull , "Cull relative to model", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_ModelCull ) 
DMXELEMENT_UNPACK_FIELD( "control_point_number", "0", int, m_nControlPointNumber )
DMXELEMENT_UNPACK_FIELD( "use only bounding box", "0", bool, m_bBoundBox )
DMXELEMENT_UNPACK_FIELD( "cull outside instead of inside", "0", bool, m_bCullOutside )
END_PARTICLE_OPERATOR_UNPACK( C_OP_ModelCull )

void C_OP_ModelCull::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	pParticles->UpdateHitBoxInfo( m_nControlPointNumber );
	if ( pParticles->m_ControlPointHitBoxes[m_nControlPointNumber].CurAndPrevValid() )
	{
		for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
		{
			float *pXYZ = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
			Vector vecParticlePosition;

			SetVectorFromAttribute( vecParticlePosition, pXYZ );

			bool bInside = g_pParticleSystemMgr->Query()->IsPointInControllingObjectHitBox( pParticles, m_nControlPointNumber, vecParticlePosition, m_bBoundBox );
			if ( ( bInside && m_bCullOutside ) || ( !bInside && !m_bCullOutside ))
				continue;

			pParticles->KillParticle(i);
		}
	}
};

//-----------------------------------------------------------------------------
// Assign CP to Center
//-----------------------------------------------------------------------------
class C_OP_SetControlPointToCenter : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetControlPointToCenter );

	int m_nCP1;

	Vector m_vecCP1Pos;

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nCP1 = max( 0, min( MAX_PARTICLE_CONTROL_POINTS-1, m_nCP1 ) );
	}

	bool ShouldRunBeforeEmitters( void ) const
	{
		return true;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;


};

DEFINE_PARTICLE_OPERATOR( C_OP_SetControlPointToCenter, "Set Control Point To Particles' Center", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointToCenter )
DMXELEMENT_UNPACK_FIELD( "Control Point Number to Set", "1", int, m_nCP1 )
DMXELEMENT_UNPACK_FIELD( "Center Offset", "0 0 0", Vector, m_vecCP1Pos )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointToCenter )

void C_OP_SetControlPointToCenter::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{

	Vector vecMinBounds;
	Vector vecMaxBounds;

	pParticles->GetBounds( &vecMinBounds, &vecMaxBounds );

	Vector vecCenter = ( ( vecMinBounds + vecMaxBounds ) / 2 );

	pParticles->SetControlPoint( m_nCP1, m_vecCP1Pos + vecCenter );
}





//-----------------------------------------------------------------------------
// Velocity Match a group of particles
//-----------------------------------------------------------------------------
class C_OP_VelocityMatchingForce : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_VelocityMatchingForce );

	float m_flDirScale;
	float m_flSpdScale;
	int	m_nCPBroadcast;

	struct VelocityMatchingForceContext_t
	{
		Vector	m_vecAvgVelocity;
		float	m_flAvgSpeed;
	};

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		VelocityMatchingForceContext_t *pCtx = reinterpret_cast<VelocityMatchingForceContext_t *>( pContext );
		pCtx->m_vecAvgVelocity = vec3_origin;
		pCtx->m_flAvgSpeed = 0;
	}

	size_t GetRequiredContextBytes( void ) const
	{
		return sizeof( VelocityMatchingForceContext_t );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_VelocityMatchingForce , "Movement Match Particle Velocities", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_VelocityMatchingForce ) 
DMXELEMENT_UNPACK_FIELD( "Direction Matching Strength", "0.25", float, m_flDirScale )
DMXELEMENT_UNPACK_FIELD( "Speed Matching Strength", "0.25", float, m_flSpdScale )
DMXELEMENT_UNPACK_FIELD( "Control Point to Broadcast Speed and Direction To", "-1", int, m_nCPBroadcast )
END_PARTICLE_OPERATOR_UNPACK( C_OP_VelocityMatchingForce )

void C_OP_VelocityMatchingForce::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	VelocityMatchingForceContext_t *pCtx = reinterpret_cast<VelocityMatchingForceContext_t *>( pContext );

	Vector vecVelocityAvg =  vec3_origin;
	float flAvgSpeed = 0;

	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{


		float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
		float *xyz_prev = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );

		Vector vecXYZ;
		Vector vecPXYZ;
		SetVectorFromAttribute( vecXYZ, xyz );
		SetVectorFromAttribute( vecPXYZ, xyz_prev );
		Vector vecVelocityCur = ( ( vecXYZ - vecPXYZ ) / pParticles->m_flDt );
		vecVelocityAvg += vecVelocityCur;
		float flSpeed = vecVelocityCur.Length();
		flAvgSpeed += flSpeed;

		if ( pCtx->m_vecAvgVelocity != vec3_origin )
		{
			Vector vecScaledXYZ;
			VectorNormalizeFast(vecVelocityCur);
			VectorLerp( vecVelocityCur, pCtx->m_vecAvgVelocity, m_flDirScale, vecScaledXYZ );
			VectorNormalizeFast(vecScaledXYZ);
			flSpeed = Lerp ( m_flSpdScale, flSpeed, pCtx->m_flAvgSpeed );
			vecScaledXYZ *= flSpeed;
			vecScaledXYZ = ( ( vecScaledXYZ * pParticles->m_flDt ) + vecPXYZ );
			SetVectorAttribute( xyz, vecScaledXYZ );
		}
	}

	VectorNormalizeFast( vecVelocityAvg );
	pCtx->m_vecAvgVelocity = vecVelocityAvg;
	pCtx->m_flAvgSpeed = ( flAvgSpeed / pParticles->m_nActiveParticles );
	if ( m_nCPBroadcast != -1 )
	{
		pParticles->SetControlPoint( m_nCPBroadcast, Vector ( pCtx->m_flAvgSpeed, pCtx->m_flAvgSpeed, pCtx->m_flAvgSpeed ) );
		pParticles->SetControlPointForwardVector( m_nCPBroadcast, pCtx->m_vecAvgVelocity );
	}
};



//-----------------------------------------------------------------------------
// Orient to heading
//-----------------------------------------------------------------------------
class C_OP_OrientTo2dDirection : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_OrientTo2dDirection );

	float m_flRotOffset;
	float m_flSpinStrength;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ROTATION_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_OrientTo2dDirection , "Rotation Orient to 2D Direction", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_OrientTo2dDirection ) 
DMXELEMENT_UNPACK_FIELD( "Rotation Offset", "0", float, m_flRotOffset )
DMXELEMENT_UNPACK_FIELD( "Spin Strength", "1", float, m_flSpinStrength )
END_PARTICLE_OPERATOR_UNPACK( C_OP_OrientTo2dDirection )

void C_OP_OrientTo2dDirection::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{

	float flRotOffset = m_flRotOffset * ( M_PI / 180.0f );
	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{

		const float *xyz = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_XYZ, i );
		const float *xyz_prev = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_PREV_XYZ, i );
		float *roll = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_ROTATION, i );

		Vector vecXYZ;
		Vector vecPXYZ;
		vecXYZ.x = xyz[0];
		vecXYZ.y = xyz[4];
		vecXYZ.z = xyz[8];
		vecPXYZ.x = xyz_prev[0];
		vecPXYZ.y = xyz_prev[4];
		vecPXYZ.z = xyz_prev[8];
		Vector vecVelocityCur = ( vecXYZ - vecPXYZ );

		vecVelocityCur.z = 0.0f;
		VectorNormalizeFast ( vecVelocityCur );

		float flCurRot = *roll;

		float flVelRot = atan2(vecVelocityCur.y, vecVelocityCur.x ) + M_PI;

		flVelRot += flRotOffset;

		float flRotation = Lerp ( m_flSpinStrength, flCurRot, flVelRot );
		*roll = flRotation;
	}

};



//-----------------------------------------------------------------------------
// Orient relative to CP
//-----------------------------------------------------------------------------
class C_OP_Orient2DRelToCP : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_Orient2DRelToCP );

	float m_flRotOffset;
	float m_flSpinStrength;
	int m_nCP;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ROTATION_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK ;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nCP );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_Orient2DRelToCP , "Rotation Orient Relative to CP", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_Orient2DRelToCP ) 
DMXELEMENT_UNPACK_FIELD( "Rotation Offset", "0", float, m_flRotOffset )
DMXELEMENT_UNPACK_FIELD( "Spin Strength", "1", float, m_flSpinStrength )
DMXELEMENT_UNPACK_FIELD( "Control Point", "0", int, m_nCP )
END_PARTICLE_OPERATOR_UNPACK( C_OP_Orient2DRelToCP )

void C_OP_Orient2DRelToCP::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{

	float flRotOffset = m_flRotOffset * ( M_PI / 180.0f );
	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{

		const float *xyz = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_XYZ, i );
		float *roll = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_ROTATION, i );

		Vector vecXYZ;
		Vector vecCP;
		vecCP = pParticles->GetControlPointAtCurrentTime( m_nCP );
		vecXYZ.x = xyz[0];
		vecXYZ.y = xyz[4];
		vecXYZ.z = xyz[8];

		Vector vecVelocityCur = ( vecXYZ - vecCP );

		vecVelocityCur.z = 0.0f;
		VectorNormalizeFast ( vecVelocityCur );

		float flCurRot = *roll;

		float flVelRot = atan2(vecVelocityCur.y, vecVelocityCur.x ) + M_PI;

		flVelRot += flRotOffset;

		float flRotation = Lerp ( m_flSpinStrength, flCurRot, flVelRot );
		*roll = flRotation;
	}
};


//-----------------------------------------------------------------------------
// Max Velocity - clamps the maximum velocity of a particle
//-----------------------------------------------------------------------------
class C_OP_MaxVelocity : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_MaxVelocity );

	float m_flMaxVelocity;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_MaxVelocity , "Movement Max Velocity", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_MaxVelocity ) 
DMXELEMENT_UNPACK_FIELD( "Maximum Velocity", "0", float, m_flMaxVelocity )
END_PARTICLE_OPERATOR_UNPACK( C_OP_MaxVelocity )

void C_OP_MaxVelocity::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{


	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
		float *xyz_prev = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );

		Vector vecXYZ;
		Vector vecPXYZ;
		SetVectorFromAttribute( vecXYZ, xyz );
		SetVectorFromAttribute( vecPXYZ, xyz_prev );
		Vector vecVelocityCur = ( ( vecXYZ - vecPXYZ ) );
		float flSpeed = vecVelocityCur.Length();
		VectorNormalizeFast( vecVelocityCur );
		float flMaxVelocityNormalized = m_flMaxVelocity * pParticles->m_flDt;
		vecVelocityCur *= min( flSpeed, flMaxVelocityNormalized);
		vecXYZ = vecPXYZ + vecVelocityCur;
		SetVectorAttribute( xyz, vecXYZ );
	}
};

//-----------------------------------------------------------------------------
// Maintain position along a path
//-----------------------------------------------------------------------------
struct SequentialPositionContext_t
{
	int		m_nParticleCount;
	float	m_flStep;
	int		m_nCountAmount;
};

class C_OP_MaintainSequentialPath : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_MaintainSequentialPath );

	float m_fMaxDistance;
	float m_flNumToAssign;
	bool m_bLoop;
	float m_flCohesionStrength;
	struct CPathParameters m_PathParams;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		uint64 nStartMask = ( 1ULL << m_PathParams.m_nStartControlPointNumber ) - 1;
		uint64 nEndMask = ( 1ULL << ( m_PathParams.m_nEndControlPointNumber + 1 ) ) - 1;
		return nEndMask & (~nStartMask);
	}

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		SequentialPositionContext_t *pCtx = reinterpret_cast<SequentialPositionContext_t *>( pContext );
		pCtx->m_nParticleCount = 0;
		if ( m_flNumToAssign > 1.0f )
		{
			pCtx->m_flStep = 1.0f / ( m_flNumToAssign - 1 );
		}
		else
		{
			pCtx->m_flStep = 0.0f;
		}
		pCtx->m_nCountAmount = 1;
	}

	void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_PathParams.ClampControlPointIndices();
	}

	size_t GetRequiredContextBytes( void ) const
	{
		return sizeof( SequentialPositionContext_t );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_MaintainSequentialPath, "Movement Maintain Position Along Path", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_MaintainSequentialPath ) 
	DMXELEMENT_UNPACK_FIELD( "maximum distance", "0", float, m_fMaxDistance )
	DMXELEMENT_UNPACK_FIELD( "bulge", "0", float, m_PathParams.m_flBulge )
	DMXELEMENT_UNPACK_FIELD( "start control point number", "0", int, m_PathParams.m_nStartControlPointNumber )
	DMXELEMENT_UNPACK_FIELD( "end control point number", "0", int, m_PathParams.m_nEndControlPointNumber )
	DMXELEMENT_UNPACK_FIELD( "bulge control 0=random 1=orientation of start pnt 2=orientation of end point", "0", int, m_PathParams.m_nBulgeControl )
	DMXELEMENT_UNPACK_FIELD( "mid point position", "0.5", float, m_PathParams.m_flMidPoint )
	DMXELEMENT_UNPACK_FIELD( "particles to map from start to end", "100", float, m_flNumToAssign )
	DMXELEMENT_UNPACK_FIELD( "restart behavior (0 = bounce, 1 = loop )", "1", bool, m_bLoop )
	DMXELEMENT_UNPACK_FIELD( "cohesion strength", "1", float, m_flCohesionStrength )
END_PARTICLE_OPERATOR_UNPACK( C_OP_MaintainSequentialPath )


void C_OP_MaintainSequentialPath::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	// NOTE: Using C_OP_ContinuousEmitter:: avoids a virtual function call
	SequentialPositionContext_t *pCtx = reinterpret_cast<SequentialPositionContext_t *>( pContext );

	float fl_Cohesion = ( 1 - m_flCohesionStrength );

	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
		float *pxyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );

		Vector StartPnt, MidP, EndPnt;
		pParticles->CalculatePathValues( m_PathParams, pParticles->m_flCurTime, &StartPnt, &MidP, &EndPnt);
		if ( pCtx->m_nParticleCount >= m_flNumToAssign || pCtx->m_nParticleCount < 0 )
		{
			if ( m_bLoop )
			{
				pCtx->m_nParticleCount = 0;
			}
			else
			{
				pCtx->m_nCountAmount *= -1;
				pCtx->m_nParticleCount = min ( pCtx->m_nParticleCount, (int)( m_flNumToAssign - 1) );
				pCtx->m_nParticleCount = max ( pCtx->m_nParticleCount, 1 );
			}
		}

		float t= pCtx->m_nParticleCount * pCtx->m_flStep;


		// form delta terms needed for quadratic bezier
		Vector Delta0=MidP-StartPnt;
		Vector Delta1 = EndPnt-MidP;

		Vector L0 = StartPnt+t*Delta0;
		Vector L1 = MidP+t*Delta1;

		Vector Pnt = L0+(L1-L0)*t;

		// Allow an offset distance and position lerp
		Vector vecXYZ;
		Vector vecPXYZ;

		SetVectorFromAttribute( vecXYZ, xyz );
		SetVectorFromAttribute( vecPXYZ, pxyz );

		vecXYZ -= Pnt;
		vecPXYZ -= Pnt;

		float flXYZOffset = min (vecXYZ.Length(), m_fMaxDistance );
		float flPXYZOffset = min (vecPXYZ.Length(), m_fMaxDistance ); 

		VectorNormalizeFast( vecXYZ );
		vecXYZ *= flXYZOffset * fl_Cohesion;
		VectorNormalizeFast( vecPXYZ );
		vecPXYZ *= flPXYZOffset * fl_Cohesion;

		vecXYZ += Pnt;
		vecPXYZ += Pnt;

		xyz[0] = vecXYZ.x;
		xyz[4] = vecXYZ.y;
		xyz[8] = vecXYZ.z;
		pxyz[0] = vecPXYZ.x;
		pxyz[4] = vecPXYZ.y;
		pxyz[8] = vecPXYZ.z;

		pCtx->m_nParticleCount += pCtx->m_nCountAmount;
	}
}


//-----------------------------------------------------------------------------
// Remap Dot Product to Scalar Operator
//-----------------------------------------------------------------------------
class C_OP_RemapDotProductToScalar : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapDotProductToScalar );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nInputCP1 ) | ( 1ULL << m_nInputCP2 );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int		m_nInputCP1;
	int		m_nInputCP2;
	int		m_nFieldOutput;
	float	m_flInputMin;
	float	m_flInputMax;
	float	m_flOutputMin;
	float	m_flOutputMax;
	bool	m_bUseParticleVelocity;
	bool	m_bScaleInitialRange;
	bool	m_bActiveRange;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RemapDotProductToScalar, "Remap Dot Product to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapDotProductToScalar )
	DMXELEMENT_UNPACK_FIELD( "use particle velocity for first input", "0", bool, m_bUseParticleVelocity )
	DMXELEMENT_UNPACK_FIELD( "first input control point", "0", int, m_nInputCP1 )
	DMXELEMENT_UNPACK_FIELD( "second input control point", "0", int, m_nInputCP2 )
	DMXELEMENT_UNPACK_FIELD( "input minimum (-1 to 1)","0", float, m_flInputMin )
	DMXELEMENT_UNPACK_FIELD( "input maximum (-1 to 1)","1", float, m_flInputMax )
	DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
	DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
	DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
	DMXELEMENT_UNPACK_FIELD( "output is scalar of initial random range","0", bool, m_bScaleInitialRange )
	DMXELEMENT_UNPACK_FIELD( "only active within specified input range","0", bool, m_bActiveRange )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapDotProductToScalar )

void C_OP_RemapDotProductToScalar::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	// clamp the result to 0 and 1 if it's alpha
	float flMin=m_flOutputMin;
	float flMax=m_flOutputMax;
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = clamp(m_flOutputMin, 0.0f, 1.0f );
		flMax = clamp(m_flOutputMax, 0.0f, 1.0f );
	}

	Vector	vecInput1;
	Vector	vecInput2;

	CParticleSIMDTransformation pXForm1;
	CParticleSIMDTransformation pXForm2;
	pParticles->GetControlPointTransformAtTime( m_nInputCP1, pParticles->m_flCurTime, &pXForm1 );
	pParticles->GetControlPointTransformAtTime( m_nInputCP2, pParticles->m_flCurTime, &pXForm2 );

	vecInput1 = pXForm1.m_v4Fwd.Vec( 0 );
	vecInput2 = pXForm2.m_v4Fwd.Vec( 0 );

	float flInput = DotProduct( vecInput1, vecInput2 );

	// only use within start/end time frame and, if set, active input range
	if ( ( m_bActiveRange && !m_bUseParticleVelocity && ( flInput < m_flInputMin || flInput > m_flInputMax ) ) )
		return;

	// FIXME: SSE-ize
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		if ( m_bUseParticleVelocity )
		{
			const float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
			const float *pxyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );
			Vector vecXYZ;
			Vector vecPXYZ;

			vecXYZ.x = xyz[0];
			vecXYZ.y = xyz[4];
			vecXYZ.z = xyz[8];
			vecPXYZ.x = pxyz[0];
			vecPXYZ.y = pxyz[4];
			vecPXYZ.z = pxyz[8];
			
			vecInput1 = vecXYZ - vecPXYZ;
			VectorNormalizeFast( vecInput1 );

			float flInputDot = DotProduct( vecInput1, vecInput2 );

			// only use within start/end time frame and, if set, active input range
			if ( ( m_bActiveRange && (flInputDot < m_flInputMin || flInputDot > m_flInputMax ) ) )
				continue;
		}

		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
		float flOutput = RemapValClamped( flInput, m_flInputMin, m_flInputMax, flMin, flMax  );
		if ( m_bScaleInitialRange )
		{
			flOutput *= *pOutput;
		}
		if ( ATTRIBUTES_WHICH_ARE_INTS & ( 1 << m_nFieldOutput ) )
		{
			*pOutput = int ( flOutput );
		}
		else
		{
			*pOutput = flOutput;
		}
	}
}



//-----------------------------------------------------------------------------
// Remap CP to Scalar Operator
//-----------------------------------------------------------------------------
class C_OP_RemapCPtoScalar : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapCPtoScalar );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return 1ULL << m_nCPInput;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nField = int (clamp (m_nField, 0, 2));
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int		m_nCPInput;                                                             
	int		m_nFieldOutput;
	int		m_nField;
	float	m_flInputMin;
	float	m_flInputMax;
	float	m_flOutputMin;
	float	m_flOutputMax;
	float	m_flStartTime;
	float	m_flEndTime;
	bool	m_bScaleInitialRange;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RemapCPtoScalar, "Remap Control Point to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapCPtoScalar )
DMXELEMENT_UNPACK_FIELD( "emitter lifetime start time (seconds)", "-1", float, m_flStartTime )
DMXELEMENT_UNPACK_FIELD( "emitter lifetime end time (seconds)", "-1", float, m_flEndTime )
DMXELEMENT_UNPACK_FIELD( "input control point number", "0", int, m_nCPInput )
DMXELEMENT_UNPACK_FIELD( "input minimum","0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "input maximum","1", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD( "input field 0-2 X/Y/Z","0", int, m_nField )
DMXELEMENT_UNPACK_FIELD_USERDATA( "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
DMXELEMENT_UNPACK_FIELD( "output is scalar of initial random range","0", bool, m_bScaleInitialRange )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapCPtoScalar )

void C_OP_RemapCPtoScalar::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	const float *pCreationTime;
	// clamp the result to 0 and 1 if it's alpha
	float flMin=m_flOutputMin;
	float flMax=m_flOutputMax;
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = clamp(m_flOutputMin, 0.0f, 1.0f );
		flMax = clamp(m_flOutputMax, 0.0f, 1.0f );
	}
	Vector vecControlPoint = pParticles->GetControlPointAtCurrentTime( m_nCPInput );

	float flInput = vecControlPoint[m_nField];

	// FIXME: SSE-ize
	for( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		pCreationTime = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );
		// using raw creation time to map to emitter lifespan
		float flLifeTime = *pCreationTime;  

		// only use within start/end time frame
		if ( ( ( flLifeTime < m_flStartTime ) || ( flLifeTime >= m_flEndTime ) ) && ( ( m_flStartTime != -1.0f) && ( m_flEndTime != -1.0f) ) )
			continue;


		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
		float flOutput = RemapValClamped( flInput, m_flInputMin, m_flInputMax, flMin, flMax  );
		if ( m_bScaleInitialRange )
		{
			flOutput = *pOutput * flOutput;
		}
		if ( ATTRIBUTES_WHICH_ARE_INTS & ( 1 << m_nFieldOutput ) )
		{
			*pOutput = int ( flOutput );
		}
		else
		{
			*pOutput = flOutput;
		}
	}
}

//-----------------------------------------------------------------------------
// Rotate Particle around axis
//-----------------------------------------------------------------------------
class C_OP_MovementRotateParticleAroundAxis : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_MovementRotateParticleAroundAxis );

	Vector m_vecRotAxis;
	float m_flRotRate;
	int m_nCP;
	bool m_bLocalSpace;

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK ;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return 1ULL << m_nCP;
	}

	void InitParams( CParticleSystemDefinition *pDef )
	{
		VectorNormalize( m_vecRotAxis );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

};

DEFINE_PARTICLE_OPERATOR( C_OP_MovementRotateParticleAroundAxis , "Movement Rotate Particle Around Axis", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_MovementRotateParticleAroundAxis ) 
DMXELEMENT_UNPACK_FIELD( "Rotation Axis", "0 0 1", Vector, m_vecRotAxis )
DMXELEMENT_UNPACK_FIELD( "Rotation Rate", "180", float, m_flRotRate )
DMXELEMENT_UNPACK_FIELD( "Control Point", "0", int, m_nCP )
DMXELEMENT_UNPACK_FIELD( "Use Local Space", "0", bool, m_bLocalSpace )
END_PARTICLE_OPERATOR_UNPACK( C_OP_MovementRotateParticleAroundAxis )

void C_OP_MovementRotateParticleAroundAxis::Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	float flRotRate = m_flRotRate * pParticles->m_flDt;

	matrix3x4_t matRot;

	Vector vecRotAxis = m_vecRotAxis;

	if ( m_bLocalSpace )
	{
		matrix3x4_t matLocalCP;
		pParticles->GetControlPointTransformAtCurrentTime( m_nCP, &matLocalCP );
		VectorRotate( m_vecRotAxis, matLocalCP, vecRotAxis );
	}

	MatrixBuildRotationAboutAxis ( vecRotAxis, flRotRate, matRot );

	Vector vecCPPos = pParticles->GetControlPointAtCurrentTime( m_nCP );

	FourVectors fvCPPos;
	fvCPPos.DuplicateVector( vecCPPos );

	fltx4 fl4Strength = ReplicateX4( flStrength );

	C4VAttributeWriteIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	C4VAttributeWriteIterator pPrevXYZ( PARTICLE_ATTRIBUTE_PREV_XYZ, pParticles );

	int nCtr = pParticles->m_nPaddedActiveParticles;
	do 
	{
		FourVectors fvCurPos = *pXYZ;
		fvCurPos -= fvCPPos;
		FourVectors fvPrevPos = *pPrevXYZ;
		fvPrevPos -= fvCPPos;

		fvCurPos.RotateBy( matRot );
		fvPrevPos.RotateBy( matRot );

		fvCurPos += fvCPPos;
		fvCurPos -= *pXYZ;
		fvCurPos *= fl4Strength;
		*pXYZ += fvCurPos;
		fvPrevPos += fvCPPos;
		fvPrevPos -= *pPrevXYZ;
		fvPrevPos *= fl4Strength;
		*pPrevXYZ += fvPrevPos;

		++pXYZ;
		++pPrevXYZ;
	} while ( --nCtr );

};

//-----------------------------------------------------------------------------
// Remap Speed to CP Operator  
//-----------------------------------------------------------------------------
class C_OP_RemapSpeedtoCP : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapSpeedtoCP );

	uint32 GetWrittenAttributes( void ) const
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const
	{
		return 0;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nInControlPointNumber ) | ( 1ULL << m_nOutControlPointNumber );
	}

	bool ShouldRunBeforeEmitters( void ) const
	{
		return true;
	}
	virtual void InitParams(CParticleSystemDefinition *pDef )
	{
		// Safety for bogus input->output feedback loop
		if ( m_nInControlPointNumber == m_nOutControlPointNumber )
			m_nOutControlPointNumber = -1;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const;

	int		m_nInControlPointNumber;
	int		m_nOutControlPointNumber;
	int		m_nField;
	float	m_flInputMin;
	float	m_flInputMax;
	float	m_flOutputMin;
	float	m_flOutputMax;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RemapSpeedtoCP, "Remap CP Speed to CP", OPERATOR_GENERIC );
BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapSpeedtoCP )
DMXELEMENT_UNPACK_FIELD( "input control point", "0", int, m_nInControlPointNumber )
DMXELEMENT_UNPACK_FIELD( "input minimum","0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "input maximum","1", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD( "output control point", "-1", int, m_nOutControlPointNumber )
DMXELEMENT_UNPACK_FIELD( "Output field 0-2 X/Y/Z","0", int, m_nField )
DMXELEMENT_UNPACK_FIELD( "output minimum","0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum","1", float, m_flOutputMax )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapSpeedtoCP );

void C_OP_RemapSpeedtoCP::Operate( CParticleCollection *pParticles, float flStrength,  void *pContext ) const
{
	if ( m_nOutControlPointNumber >= 0 )
	{
		Vector vecPrevPos;
		pParticles->GetControlPointAtPrevTime( m_nInControlPointNumber, &vecPrevPos );
		Vector vecDelta;
		vecDelta = pParticles->GetControlPointAtCurrentTime( m_nInControlPointNumber ) - vecPrevPos;
		float flSpeed = vecDelta.Length() / pParticles->m_flPreviousDt;
		float flOutput = RemapValClamped( flSpeed, m_flInputMin, m_flInputMax, m_flOutputMin, m_flOutputMax  );

		Vector vecControlPoint = pParticles->GetControlPointAtCurrentTime( m_nOutControlPointNumber );
		vecControlPoint[m_nField] = flOutput;
		pParticles->SetControlPoint( m_nOutControlPointNumber, vecControlPoint );
	}
}

//-----------------------------------------------------------------------------
//
// Portal 2 operators
//
// Ported from the Portal 2 / CS:GO particle library (the CS:GO tree forked from
// the Portal 2 branch). DMX field names and defaults match those operators so
// retail Portal 2 PCFs unpack as authored.
//
//-----------------------------------------------------------------------------
static FORCEINLINE fltx4 P2LerpSIMD( const fltx4 &percent, const fltx4 &a, const fltx4 &b )
{
	return AddSIMD( a, MulSIMD( SubSIMD( b, a ), percent ) );
}

// Remap val from clamped range between a and b to new range between c and d
static FORCEINLINE fltx4 P2RemapValClampedSIMD(
    const fltx4 &val, const fltx4 &a, const fltx4 &b, const fltx4 &c, const fltx4 &d )
{
	fltx4 range = MaskedAssign( CmpEqSIMD( a, b ), Four_Ones, SubSIMD( b, a ) );
	fltx4 cVal = MaxSIMD( Four_Zeros, MinSIMD( Four_Ones, DivSIMD( SubSIMD( val, a ), range ) ) );
	return P2LerpSIMD( cVal, c, d );
}

// Clamp range for ramp/oscillate operators, chosen at init so the per-particle
// loop needn't branch
static void P2ScalarClampRange( int nField, fltx4 *pMin, fltx4 *pMax )
{
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << nField ) )
	{
		*pMin = Four_Zeros;
		*pMax = Four_Ones;
	}
	else if ( ATTRIBUTES_WHICH_ARE_SIZE & ( 1 << nField ) )
	{
		*pMin = Four_Zeros;
		*pMax = Four_FLT_MAX;
	}
	else
	{
		*pMin = Four_Negative_FLT_MAX;
		*pMax = Four_FLT_MAX;
	}
}

// Kills the particles of a group of four whose mask lane is set
static FORCEINLINE void P2KillMaskedParticles(
    CParticleCollection *pParticles, int nFirst, const fltx4 &fl4KillMask )
{
	if ( IsAnyNegative( fl4KillMask ) )
	{
		int nMask = TestSignSIMD( fl4KillMask );
		if ( nMask & 1 )
			pParticles->KillParticle( nFirst );
		if ( nMask & 2 )
			pParticles->KillParticle( nFirst + 1 );
		if ( nMask & 4 )
			pParticles->KillParticle( nFirst + 2 );
		if ( nMask & 8 )
			pParticles->KillParticle( nFirst + 3 );
	}
}

//-----------------------------------------------------------------------------
// Fade In Operator - fast version
//-----------------------------------------------------------------------------
class C_OP_FadeInSimple : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_FadeInSimple );

	uint32 GetWrittenAttributes( void ) const { return PARTICLE_ATTRIBUTE_ALPHA_MASK; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const { return PARTICLE_ATTRIBUTE_ALPHA_MASK; }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_flFadeInTime;
};

DEFINE_PARTICLE_OPERATOR( C_OP_FadeInSimple, "Alpha Fade In Simple", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_FadeInSimple )
DMXELEMENT_UNPACK_FIELD( "proportional fade in time", ".25", float, m_flFadeInTime )
END_PARTICLE_OPERATOR_UNPACK( C_OP_FadeInSimple )

void C_OP_FadeInSimple::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128InitialAttributeIterator pInitialAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeWriteIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );

	fltx4 CurTime = pParticles->m_fl4CurTime;

	int nCtr = pParticles->m_nPaddedActiveParticles;

	fltx4 fl4FadeInTime = ReplicateX4( m_flFadeInTime );

	do
	{
		// Find our life percentage
		fltx4 fl4LifeTime = SubSIMD( CurTime, *pCreationTime );

		fl4LifeTime = MaxSIMD( Four_Zeros,
		    MinSIMD( Four_Ones, MulSIMD( fl4LifeTime, ReciprocalEstSIMD( *pLifeDuration ) ) ) );

		fltx4 ApplyMask = CmpGtSIMD( fl4FadeInTime, fl4LifeTime );
		if ( IsAnyNegative( ApplyMask ) )
		{
			// Fading in
			fltx4 NewAlpha = SimpleSplineRemapValWithDeltasClamped( fl4LifeTime, Four_Zeros,
			    fl4FadeInTime, ReciprocalEstSIMD( fl4FadeInTime ), Four_Zeros, *pInitialAlpha );
			*( pAlpha ) = MaskedAssign( ApplyMask, NewAlpha, *( pAlpha ) );
		}
		++pCreationTime;
		++pLifeDuration;
		++pInitialAlpha;
		++pAlpha;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Fade Out Operator - fast version
//-----------------------------------------------------------------------------
class C_OP_FadeOutSimple : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_FadeOutSimple );

	uint32 GetWrittenAttributes( void ) const { return PARTICLE_ATTRIBUTE_ALPHA_MASK; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const { return PARTICLE_ATTRIBUTE_ALPHA_MASK; }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_flFadeOutTime;
};

DEFINE_PARTICLE_OPERATOR( C_OP_FadeOutSimple, "Alpha Fade Out Simple", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_FadeOutSimple )
DMXELEMENT_UNPACK_FIELD( "proportional fade out time", ".25", float, m_flFadeOutTime )
END_PARTICLE_OPERATOR_UNPACK( C_OP_FadeOutSimple )

void C_OP_FadeOutSimple::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128InitialAttributeIterator pInitialAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeWriteIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nCtr = pParticles->m_nPaddedActiveParticles;

	fltx4 fl4FadeOutTime = ReplicateX4( 1.0f - m_flFadeOutTime );
	fltx4 fl4Fadespan = ReplicateX4( m_flFadeOutTime );

	do
	{
		// Find our life percentage
		fltx4 fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime );
		fltx4 fl4LifeDuration = *pLifeDuration;

		fl4LifeTime = MulSIMD( fl4LifeTime, ReciprocalEstSIMD( fl4LifeDuration ) );

		fltx4 ApplyMask = CmpLtSIMD( fl4FadeOutTime, fl4LifeTime );
		if ( IsAnyNegative( ApplyMask ) )
		{
			// Fading out
			fltx4 fl4Frac =
			    MulSIMD( SubSIMD( fl4LifeTime, fl4FadeOutTime ), ReciprocalEstSIMD( fl4Fadespan ) );
			fl4Frac = MinSIMD( Four_Ones, MaxSIMD( Four_Zeros, fl4Frac ) );
			fl4Frac = SimpleSpline( fl4Frac );
			fl4Frac = SubSIMD( Four_Ones, fl4Frac );
			fltx4 NewAlpha = MulSIMD( *pInitialAlpha, fl4Frac );

			*pAlpha = MaskedAssign( ApplyMask, MinSIMD( NewAlpha, *pAlpha ), *pAlpha );
		}
		++pCreationTime;
		++pLifeDuration;
		++pInitialAlpha;
		++pAlpha;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Clamp Scalar Operator
//-----------------------------------------------------------------------------
class C_OP_ClampScalar : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_ClampScalar );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const { return 0; }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nFieldOutput;
	float m_flOutputMin;
	float m_flOutputMax;
};

DEFINE_PARTICLE_OPERATOR( C_OP_ClampScalar, "Clamp Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_ClampScalar )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum", "0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum", "1", float, m_flOutputMax )
END_PARTICLE_OPERATOR_UNPACK( C_OP_ClampScalar )

void C_OP_ClampScalar::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
		float flOutput = clamp( *pOutput, m_flOutputMin, m_flOutputMax );
		*pOutput = Lerp( flStrength, *pOutput, flOutput );
	}
}

//-----------------------------------------------------------------------------
// Oscillating Scalar Simple operator - fast, with few options
//-----------------------------------------------------------------------------
class C_OP_OscillateScalarSimple : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_OscillateScalarSimple );

	float m_Rate;
	float m_Frequency;
	int m_nField;
	float m_flOscMult;
	float m_flOscAdd;
	fltx4 m_fl4MinCmp, m_fl4MaxCmp;

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nField; }

	uint32 GetReadAttributes( void ) const { return 0; }

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		P2ScalarClampRange( m_nField, &m_fl4MinCmp, &m_fl4MaxCmp );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_OscillateScalarSimple, "Oscillate Scalar Simple", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_OscillateScalarSimple )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "oscillation field", "7", int, m_nField, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "oscillation rate", "0", float, m_Rate )
DMXELEMENT_UNPACK_FIELD( "oscillation frequency", "1", float, m_Frequency )
DMXELEMENT_UNPACK_FIELD( "oscillation multiplier", "2", float, m_flOscMult )
DMXELEMENT_UNPACK_FIELD( "oscillation start phase", ".5", float, m_flOscAdd )
END_PARTICLE_OPERATOR_UNPACK( C_OP_OscillateScalarSimple )

void C_OP_OscillateScalarSimple::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeWriteIterator pOscField( m_nField, pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;
	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );
	fltx4 fl4CosFactorMultiplier = ReplicateX4( m_flOscMult );
	fltx4 fl4CosFactorAdd = ReplicateX4( m_flOscAdd );
	fltx4 fl4CosFactor = AddSIMD( MulSIMD( fl4CosFactorMultiplier, fl4CurTime ), fl4CosFactorAdd );
	fltx4 fl4Frequency = ReplicateX4( m_Frequency );
	fltx4 fl4Rate = ReplicateX4( m_Rate );
	fltx4 fl4OscMultiplier = MulSIMD( fl4Rate, fl4ScaleFactor );
	fltx4 fl4Cos = MulSIMD( fl4CosFactor, fl4Frequency );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	fltx4 fl4OscillateAmt = MulSIMD( fl4OscMultiplier, SinEst01SIMD( fl4Cos ) );

	do
	{
		fltx4 fl4OscVal = AddSIMD( *pOscField, fl4OscillateAmt );
		*pOscField = MaxSIMD( MinSIMD( fl4OscVal, m_fl4MaxCmp ), m_fl4MinCmp );

		++pOscField;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Remap Difference Between Sequential Particles Operator
//-----------------------------------------------------------------------------
class C_OP_DifferencePreviousParticle : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_DifferencePreviousParticle );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const { return 1 << m_nFieldInput; }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nFieldInput;
	int m_nFieldOutput;
	float m_flInputMin;
	float m_flInputMax;
	float m_flOutputMin;
	float m_flOutputMax;
	bool m_bScaleInitialRange;
	bool m_bActiveRange;
	bool m_bSetPreviousParticle;
};

DEFINE_PARTICLE_OPERATOR( C_OP_DifferencePreviousParticle,
    "Remap Difference of Sequential Particle Vector to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_DifferencePreviousParticle )
DMXELEMENT_UNPACK_FIELD( "difference minimum", "0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "difference maximum", "128", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "input field", "0", int, m_nFieldInput, "intchoice particlefield_vector" )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum", "0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum", "1", float, m_flOutputMax )
DMXELEMENT_UNPACK_FIELD(
    "output is scalar of initial random range", "0", bool, m_bScaleInitialRange )
DMXELEMENT_UNPACK_FIELD( "only active within specified difference", "0", bool, m_bActiveRange )
DMXELEMENT_UNPACK_FIELD( "also set ouput to previous particle", "0", bool, m_bSetPreviousParticle )
END_PARTICLE_OPERATOR_UNPACK( C_OP_DifferencePreviousParticle )

void C_OP_DifferencePreviousParticle::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	// clamp the result to 0 and 1 if it's alpha
	float flMin = m_flOutputMin;
	float flMax = m_flOutputMax;
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = clamp( m_flOutputMin, 0.0f, 1.0f );
		flMax = clamp( m_flOutputMax, 0.0f, 1.0f );
	}
	Vector vecPreviousVal = vec3_invalid;
	int nPreviousParticleNumber = 0;
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		const float *pInput = pParticles->GetFloatAttributePtr( m_nFieldInput, i );
		if ( vecPreviousVal != vec3_invalid )
		{
			Vector vecPosition2 = Vector( pInput[0], pInput[4], pInput[8] );
			float flDistance = vecPreviousVal.DistTo( vecPosition2 );
			if ( m_bActiveRange && ( flDistance < m_flInputMin || flDistance > m_flInputMax ) )
			{
				continue;
			}

			float flOutput =
			    RemapValClamped( flDistance, m_flInputMin, m_flInputMax, flMin, flMax );
			float flOutput2 = flOutput;
			if ( m_bScaleInitialRange )
			{
				const float *pInitialOutput = pParticles->GetFloatAttributePtr( m_nFieldOutput, i );
				flOutput = *pInitialOutput * flOutput;
			}
			float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
			float *pOutput2 =
			    pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, nPreviousParticleNumber );

			*pOutput = flOutput;
			if ( m_bSetPreviousParticle )
				*pOutput2 *= flOutput2;
		}
		SetVectorFromAttribute( vecPreviousVal, pInput );
		nPreviousParticleNumber = i;
	}
}

//-----------------------------------------------------------------------------
// Lerp Initial Scalar Operator
//-----------------------------------------------------------------------------
class C_OP_LerpScalar : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_LerpScalar );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const { return 1 << m_nFieldOutput; }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nFieldOutput;
	float m_flOutput;
	float m_flStartTime;
	float m_flEndTime;
};

DEFINE_PARTICLE_OPERATOR( C_OP_LerpScalar, "Lerp Initial Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_LerpScalar )
DMXELEMENT_UNPACK_FIELD( "start time", "0", float, m_flStartTime )
DMXELEMENT_UNPACK_FIELD( "end time", "1", float, m_flEndTime )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "value to lerp to", "1", float, m_flOutput )
END_PARTICLE_OPERATOR_UNPACK( C_OP_LerpScalar )

void C_OP_LerpScalar::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		const float *pCt = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );
		const float *pLifespan =
		    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_LIFE_DURATION, i );
		float flAge = ( pParticles->m_flCurTime - *pCt ) / ( *pLifespan + FLT_EPSILON );
		if ( flAge < m_flStartTime || flAge > m_flEndTime )
			continue;
		const float *pInput = pParticles->GetInitialFloatAttributePtr( m_nFieldOutput, i );
		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
		float flOutput = RemapValClamped( flAge, m_flStartTime, m_flEndTime, *pInput, m_flOutput );
		*pOutput = Lerp( flStrength, *pOutput, flOutput );
	}
}

//-----------------------------------------------------------------------------
// Remap Speed Operator
//-----------------------------------------------------------------------------
#define MIN_PARTICLE_SPEED 0.001

class C_OP_RemapSpeed : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapSpeed );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	uint32 GetReadInitialAttributes( void ) const { return 1 << m_nFieldOutput; }

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_flInputMin = MAX( MIN_PARTICLE_SPEED, m_flInputMin );
		m_flInputMax = MAX( MIN_PARTICLE_SPEED, m_flInputMax );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nFieldOutput;
	float m_flInputMin;
	float m_flInputMax;
	float m_flOutputMin;
	float m_flOutputMax;
	bool m_bScaleInitialRange;
	bool m_bScaleCurrent;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RemapSpeed, "Remap Speed to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapSpeed )
DMXELEMENT_UNPACK_FIELD( "input minimum", "0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "input maximum", "1", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum", "0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum", "1", float, m_flOutputMax )
DMXELEMENT_UNPACK_FIELD(
    "output is scalar of initial random range", "0", bool, m_bScaleInitialRange )
DMXELEMENT_UNPACK_FIELD( "output is scalar of current value", "0", bool, m_bScaleCurrent )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapSpeed )

void C_OP_RemapSpeed::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	// clamp the result to 0 and 1 if it's alpha
	fltx4 flMin = ReplicateX4( m_flOutputMin );
	fltx4 flMax = ReplicateX4( m_flOutputMax );
	if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
	{
		flMin = ReplicateX4( clamp( m_flOutputMin, 0.0f, 1.0f ) );
		flMax = ReplicateX4( clamp( m_flOutputMax, 0.0f, 1.0f ) );
	}

	fltx4 fl4Dt = ReplicateX4( pParticles->m_flDt );
	fltx4 fl4InputMin = ReplicateX4( m_flInputMin );
	fltx4 fl4InputMax = ReplicateX4( m_flInputMax );
	fltx4 fl4Strength = ReplicateX4( flStrength );
	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	C4VAttributeIterator pPrevXYZ( PARTICLE_ATTRIBUTE_PREV_XYZ, pParticles );
	CM128AttributeWriteIterator pOutput( m_nFieldOutput, pParticles );
	CM128InitialAttributeIterator pInitialOutput( m_nFieldOutput, pParticles );

	for ( int i = 0; i < pParticles->m_nPaddedActiveParticles; i++ )
	{
		FourVectors v4Delta = *pXYZ;
		v4Delta -= *pPrevXYZ;
		fltx4 fl4Speed = DivSIMD( v4Delta.length(), fl4Dt );
		fltx4 fl4Output = P2RemapValClampedSIMD( fl4Speed, fl4InputMin, fl4InputMax, flMin, flMax );
		if ( m_bScaleInitialRange )
		{
			fl4Output = MulSIMD( *pInitialOutput, fl4Output );
		}
		if ( m_bScaleCurrent )
		{
			fl4Output = MulSIMD( *pOutput, fl4Output );
		}

		*pOutput = P2LerpSIMD( fl4Strength, *pOutput, fl4Output );

		++pXYZ;
		++pPrevXYZ;
		++pOutput;
		++pInitialOutput;
	}
}

//-----------------------------------------------------------------------------
// Ramp Scalar Linear - changes a scalar value at a set rate
//-----------------------------------------------------------------------------
class C_OP_RampScalarLinear : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RampScalarLinear );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nField; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK |
		       PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_bUsesStartEnd = !( m_flStartTime_min == 0 && m_flStartTime_max == 0 &&
		                     m_flEndTime_min == 1 && m_flEndTime_max == 1 && m_bProportionalOp );
		P2ScalarClampRange( m_nField, &m_fl4MinCmp, &m_fl4MaxCmp );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_RateMin;
	float m_RateMax;
	float m_flStartTime_min;
	float m_flStartTime_max;
	float m_flEndTime_min;
	float m_flEndTime_max;
	fltx4 m_fl4MinCmp;
	fltx4 m_fl4MaxCmp;
	int m_nField;
	bool m_bProportionalOp;
	bool m_bUsesStartEnd;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RampScalarLinear, "Ramp Scalar Linear Random", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarLinear )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "ramp field", "3", int, m_nField, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "ramp rate min", "0", float, m_RateMin )
DMXELEMENT_UNPACK_FIELD( "ramp rate max", "0", float, m_RateMax )
DMXELEMENT_UNPACK_FIELD( "start time min", "0", float, m_flStartTime_min )
DMXELEMENT_UNPACK_FIELD( "start time max", "0", float, m_flStartTime_max )
DMXELEMENT_UNPACK_FIELD( "end time min", "1", float, m_flEndTime_min )
DMXELEMENT_UNPACK_FIELD( "end time max", "1", float, m_flEndTime_max )
DMXELEMENT_UNPACK_FIELD( "start/end proportional", "1", bool, m_bProportionalOp )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarLinear )

void C_OP_RampScalarLinear::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	C4IAttributeIterator pParticleId( PARTICLE_ATTRIBUTE_PARTICLE_ID, pParticles );
	CM128AttributeWriteIterator pRampField( m_nField, pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nRandomOffset = pParticles->OperatorRandomSampleOffset();

	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );

	fltx4 fl4StartTimeMin = ReplicateX4( m_flStartTime_min );
	fltx4 fl4StartTimeWidth = ReplicateX4( m_flStartTime_max - m_flStartTime_min );
	fltx4 fl4EndTimeMin = ReplicateX4( m_flEndTime_min );
	fltx4 fl4EndTimeWidth = ReplicateX4( m_flEndTime_max - m_flEndTime_min );

	fltx4 fl4RateMin = ReplicateX4( m_RateMin );
	fltx4 fl4RateWidth = ReplicateX4( m_RateMax - m_RateMin );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	do
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );
		if ( m_bUsesStartEnd )
		{
			fltx4 fl4LifeTime;
			if ( m_bProportionalOp )
			{
				fl4LifeTime = MulSIMD(
				    SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) );
			}
			else
			{
				fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime );
			}

			fltx4 fl4StartTime = pParticles->RandomFloat( *pParticleId, nRandomOffset + 11 );
			fl4StartTime = AddSIMD( fl4StartTimeMin, MulSIMD( fl4StartTimeWidth, fl4StartTime ) );
			fltx4 fl4EndTime = pParticles->RandomFloat( *pParticleId, nRandomOffset + 12 );
			fl4EndTime = AddSIMD( fl4EndTimeMin, MulSIMD( fl4EndTimeWidth, fl4EndTime ) );
			fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
			fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );
		}

		if ( IsAnyNegative( fl4GoodMask ) )
		{
			fltx4 fl4Rate = AddSIMD( fl4RateMin,
			    MulSIMD( fl4RateWidth, pParticles->RandomFloat( *pParticleId, nRandomOffset ) ) );

			fltx4 fl4RampVal = AddSIMD( *pRampField, MulSIMD( fl4Rate, fl4ScaleFactor ) );

			*pRampField = MaskedAssign( fl4GoodMask,
			    MaxSIMD( MinSIMD( fl4RampVal, m_fl4MaxCmp ), m_fl4MinCmp ), *pRampField );
		}
		++pCreationTime;
		++pLifeDuration;
		++pRampField;
		++pParticleId;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Ramp Scalar Spline - ease in/out a scalar value over a curve with definable bias
//-----------------------------------------------------------------------------
class C_OP_RampScalarSpline : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RampScalarSpline );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nField; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK |
		       PARTICLE_ATTRIBUTE_PARTICLE_ID_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		P2ScalarClampRange( m_nField, &m_fl4MinCmp, &m_fl4MaxCmp );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_RateMin;
	float m_RateMax;
	float m_flStartTime_min;
	float m_flStartTime_max;
	float m_flEndTime_min;
	float m_flEndTime_max;
	float m_flBias;
	fltx4 m_fl4MinCmp;
	fltx4 m_fl4MaxCmp;
	int m_nField;
	bool m_bProportionalOp;
	bool m_bEaseOut;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RampScalarSpline, "Ramp Scalar Spline Random", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarSpline )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "ramp field", "3", int, m_nField, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "ramp rate min", "0", float, m_RateMin )
DMXELEMENT_UNPACK_FIELD( "ramp rate max", "0", float, m_RateMax )
DMXELEMENT_UNPACK_FIELD( "start time min", "0", float, m_flStartTime_min )
DMXELEMENT_UNPACK_FIELD( "start time max", "0", float, m_flStartTime_max )
DMXELEMENT_UNPACK_FIELD( "end time min", "1", float, m_flEndTime_min )
DMXELEMENT_UNPACK_FIELD( "end time max", "1", float, m_flEndTime_max )
DMXELEMENT_UNPACK_FIELD( "start/end proportional", "1", bool, m_bProportionalOp )
DMXELEMENT_UNPACK_FIELD( "ease out", "0", bool, m_bEaseOut )
DMXELEMENT_UNPACK_FIELD( "bias", ".5", float, m_flBias )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarSpline )

void C_OP_RampScalarSpline::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	C4IAttributeIterator pParticleId( PARTICLE_ATTRIBUTE_PARTICLE_ID, pParticles );
	CM128AttributeWriteIterator pRampField( m_nField, pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	int nRandomOffset = pParticles->OperatorRandomSampleOffset();

	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );

	fltx4 fl4StartTimeMin = ReplicateX4( m_flStartTime_min );
	fltx4 fl4StartTimeWidth = ReplicateX4( m_flStartTime_max - m_flStartTime_min );
	fltx4 fl4EndTimeMin = ReplicateX4( m_flEndTime_min );
	fltx4 fl4EndTimeWidth = ReplicateX4( m_flEndTime_max - m_flEndTime_min );

	fltx4 fl4RateMin = ReplicateX4( m_RateMin );
	fltx4 fl4RateWidth = ReplicateX4( m_RateMax - m_RateMin );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	fltx4 fl4Bias = PreCalcBiasParameter( ReplicateX4( m_flBias ) );

	do
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );

		fltx4 fl4LifeTime;

		fltx4 fl4StartTime = pParticles->RandomFloat( *pParticleId, nRandomOffset + 11 );
		fl4StartTime = AddSIMD( fl4StartTimeMin, MulSIMD( fl4StartTimeWidth, fl4StartTime ) );
		fltx4 fl4EndTime = pParticles->RandomFloat( *pParticleId, nRandomOffset + 12 );
		fl4EndTime = AddSIMD( fl4EndTimeMin, MulSIMD( fl4EndTimeWidth, fl4EndTime ) );

		if ( m_bProportionalOp )
		{
			fl4LifeTime = MulSIMD(
			    SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) );
		}
		else
		{
			fl4LifeTime = SubSIMD( fl4CurTime, *pCreationTime );
		}

		fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );

		if ( IsAnyNegative( fl4GoodMask ) )
		{
			fltx4 fl4Rate = AddSIMD( fl4RateMin,
			    MulSIMD( fl4RateWidth, pParticles->RandomFloat( *pParticleId, nRandomOffset ) ) );

			fltx4 ooInRange =
			    DivSIMD( Four_Ones, AddSIMD( Four_Epsilons, SubSIMD( fl4EndTime, fl4StartTime ) ) );
			fltx4 fl4Spline = MulSIMD( SubSIMD( fl4LifeTime, fl4StartTime ), ooInRange );

			fl4Spline = MinSIMD( Four_Ones, MaxSIMD( Four_Zeros, fl4Spline ) );
			if ( m_bEaseOut )
			{
				fltx4 fl4EaseOutMask = CmpGtSIMD( fl4Spline, Four_PointFives );
				fl4Spline =
				    MaskedAssign( fl4EaseOutMask, SubSIMD( Four_Ones, fl4Spline ), fl4Spline );
				fl4Spline = MulSIMD( Four_Twos, fl4Spline );
			}

			fl4Spline = BiasSIMD( fl4Spline, fl4Bias );

			fltx4 fl4RampVal =
			    AddSIMD( *pRampField, MulSIMD( fl4Rate, MulSIMD( fl4Spline, fl4ScaleFactor ) ) );

			*pRampField = MaskedAssign( fl4GoodMask,
			    MaxSIMD( MinSIMD( fl4RampVal, m_fl4MaxCmp ), m_fl4MinCmp ), *pRampField );
		}
		++pCreationTime;
		++pLifeDuration;
		++pRampField;
		++pParticleId;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Ramp Scalar Linear Simple - linear ramp of scalar value - fast version
//-----------------------------------------------------------------------------
class C_OP_RampScalarLinearSimple : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RampScalarLinearSimple );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nField; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		P2ScalarClampRange( m_nField, &m_fl4MinCmp, &m_fl4MaxCmp );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_Rate;
	float m_flStartTime;
	float m_flEndTime;
	fltx4 m_fl4MinCmp;
	fltx4 m_fl4MaxCmp;
	int m_nField;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_RampScalarLinearSimple, "Ramp Scalar Linear Simple", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarLinearSimple )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "ramp field", "3", int, m_nField, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "ramp rate", "0", float, m_Rate )
DMXELEMENT_UNPACK_FIELD( "start time", "0", float, m_flStartTime )
DMXELEMENT_UNPACK_FIELD( "end time", "1", float, m_flEndTime )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarLinearSimple )

void C_OP_RampScalarLinearSimple::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128AttributeWriteIterator pRampField( m_nField, pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );

	fltx4 fl4StartTime = ReplicateX4( m_flStartTime );
	fltx4 fl4EndTime = ReplicateX4( m_flEndTime );
	fltx4 fl4Rate = ReplicateX4( m_Rate );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	do
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );

		fltx4 fl4LifeTime =
		    MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );

		if ( IsAnyNegative( fl4GoodMask ) )
		{
			fltx4 fl4RampVal = AddSIMD( *pRampField, MulSIMD( fl4Rate, fl4ScaleFactor ) );

			*pRampField = MaskedAssign( fl4GoodMask,
			    MaxSIMD( MinSIMD( fl4RampVal, m_fl4MaxCmp ), m_fl4MinCmp ), *pRampField );
		}
		++pCreationTime;
		++pLifeDuration;
		++pRampField;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Ramp Scalar Spline Simple - ease in/out a scalar value - fast version
//-----------------------------------------------------------------------------
class C_OP_RampScalarSplineSimple : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RampScalarSplineSimple );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nField; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK | PARTICLE_ATTRIBUTE_LIFE_DURATION_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		P2ScalarClampRange( m_nField, &m_fl4MinCmp, &m_fl4MaxCmp );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_Rate;
	float m_flStartTime;
	float m_flEndTime;
	fltx4 m_fl4MinCmp;
	fltx4 m_fl4MaxCmp;
	int m_nField;
	bool m_bEaseOut;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_RampScalarSplineSimple, "Ramp Scalar Spline Simple", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarSplineSimple )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "ramp field", "3", int, m_nField, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "ramp rate", "0", float, m_Rate )
DMXELEMENT_UNPACK_FIELD( "start time", "0", float, m_flStartTime )
DMXELEMENT_UNPACK_FIELD( "end time", "1", float, m_flEndTime )
DMXELEMENT_UNPACK_FIELD( "ease out", "0", bool, m_bEaseOut )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RampScalarSplineSimple )

void C_OP_RampScalarSplineSimple::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	CM128AttributeIterator pCreationTime( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pLifeDuration( PARTICLE_ATTRIBUTE_LIFE_DURATION, pParticles );
	CM128AttributeWriteIterator pRampField( m_nField, pParticles );

	fltx4 fl4CurTime = pParticles->m_fl4CurTime;

	fltx4 fl4ScaleFactor = ReplicateX4( flStrength * pParticles->m_flDt );

	fltx4 fl4StartTime = ReplicateX4( m_flStartTime );
	fltx4 fl4EndTime = ReplicateX4( m_flEndTime );
	fltx4 fl4Rate = ReplicateX4( m_Rate );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	do
	{
		fltx4 fl4LifeDuration = *pLifeDuration;
		fltx4 fl4GoodMask = CmpGtSIMD( fl4LifeDuration, Four_Zeros );

		fltx4 fl4LifeTime =
		    MulSIMD( SubSIMD( fl4CurTime, *pCreationTime ), ReciprocalEstSIMD( fl4LifeDuration ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpGeSIMD( fl4LifeTime, fl4StartTime ) );
		fl4GoodMask = AndSIMD( fl4GoodMask, CmpLtSIMD( fl4LifeTime, fl4EndTime ) );

		if ( IsAnyNegative( fl4GoodMask ) )
		{
			fltx4 ooInRange =
			    DivSIMD( Four_Ones, AddSIMD( Four_Epsilons, SubSIMD( fl4EndTime, fl4StartTime ) ) );
			fltx4 fl4Spline = MulSIMD( SubSIMD( fl4LifeTime, fl4StartTime ), ooInRange );
			fl4Spline = MinSIMD( Four_Ones, MaxSIMD( Four_Zeros, fl4Spline ) );

			if ( m_bEaseOut )
			{
				fltx4 fl4EaseOutMask = CmpGtSIMD( fl4Spline, Four_PointFives );
				fl4Spline =
				    MaskedAssign( fl4EaseOutMask, SubSIMD( Four_Ones, fl4Spline ), fl4Spline );
				fl4Spline = MulSIMD( Four_Twos, fl4Spline );
			}

			fl4Spline = SimpleSpline( fl4Spline );

			fltx4 fl4RampVal =
			    AddSIMD( *pRampField, MulSIMD( fl4Rate, MulSIMD( fl4Spline, fl4ScaleFactor ) ) );

			*pRampField = MaskedAssign( fl4GoodMask,
			    MaxSIMD( MinSIMD( fl4RampVal, m_fl4MaxCmp ), m_fl4MinCmp ), *pRampField );
		}
		++pCreationTime;
		++pLifeDuration;
		++pRampField;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Lifespan Minimum Alpha Decay Operator (kills particles if they cross alpha boundary)
//-----------------------------------------------------------------------------
class C_OP_AlphaDecay : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_AlphaDecay );

	float m_flMinAlpha;

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_ALPHA_MASK | PARTICLE_ATTRIBUTE_ALPHA2_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_flMinAlpha = MAX( 0, m_flMinAlpha );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_AlphaDecay, "Lifespan Minimum Alpha Decay", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_AlphaDecay )
DMXELEMENT_UNPACK_FIELD( "minimum alpha", "0", float, m_flMinAlpha )
END_PARTICLE_OPERATOR_UNPACK( C_OP_AlphaDecay )

void C_OP_AlphaDecay::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	fltx4 fl4MinAlpha = ReplicateX4( m_flMinAlpha + FLT_EPSILON );

	CM128AttributeIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeIterator pAlpha2( PARTICLE_ATTRIBUTE_ALPHA2, pParticles );

	int nLimit = pParticles->m_nPaddedActiveParticles << 2;

	for ( int i = 0; i < nLimit; i += 4 )
	{
		fltx4 fl4KillMask = CmpLeSIMD( MulSIMD( *pAlpha, *pAlpha2 ), fl4MinAlpha );
		P2KillMaskedParticles( pParticles, i, fl4KillMask );
		++pAlpha;
		++pAlpha2;
	}
}

//-----------------------------------------------------------------------------
// Lifespan Minimum Radius Decay Operator (kills particles if they cross radius boundary)
//-----------------------------------------------------------------------------
class C_OP_RadiusDecay : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RadiusDecay );

	float m_flMinRadius;

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const { return PARTICLE_ATTRIBUTE_RADIUS_MASK; }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RadiusDecay, "Lifespan Minimum Radius Decay", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RadiusDecay )
DMXELEMENT_UNPACK_FIELD( "minimum radius", "1", float, m_flMinRadius )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RadiusDecay )

void C_OP_RadiusDecay::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	fltx4 fl4MinRadius = ReplicateX4( m_flMinRadius );

	CM128AttributeIterator pRadius( PARTICLE_ATTRIBUTE_RADIUS, pParticles );

	int nLimit = pParticles->m_nPaddedActiveParticles << 2;

	for ( int i = 0; i < nLimit; i += 4 )
	{
		fltx4 fl4KillMask = CmpLeSIMD( *pRadius, fl4MinRadius );
		P2KillMaskedParticles( pParticles, i, fl4KillMask );
		++pRadius;
	}
}

//-----------------------------------------------------------------------------
// Distance Cull Operator - cull particles on the "wrong" side of a sphere
//-----------------------------------------------------------------------------
class C_OP_DistanceCull : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_DistanceCull );

	int m_nControlPoint;
	Vector m_vecPointOffset;
	float m_flDistance;
	bool m_bCullInside;

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const { return PARTICLE_ATTRIBUTE_XYZ_MASK; }

	virtual uint64 GetReadControlPointMask() const { return ( 1ULL << m_nControlPoint ); }

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nControlPoint = MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nControlPoint ) );
	}

	void Render( CParticleCollection *pParticles ) const;

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_DistanceCull, "Cull when crossing sphere", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_DistanceCull )
DMXELEMENT_UNPACK_FIELD( "Control Point", "0", int, m_nControlPoint )
DMXELEMENT_UNPACK_FIELD( "Cull Distance", "0", float, m_flDistance )
DMXELEMENT_UNPACK_FIELD( "Control Point offset", "0 0 0", Vector, m_vecPointOffset )
DMXELEMENT_UNPACK_FIELD( "Cull inside instead of outside", "0", bool, m_bCullInside )
END_PARTICLE_OPERATOR_UNPACK( C_OP_DistanceCull )

void C_OP_DistanceCull::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	int nLimit = pParticles->m_nPaddedActiveParticles << 2;

	FourVectors v4Offset;
	v4Offset.DuplicateVector( m_vecPointOffset );
	FourVectors v4CullPosition;
	v4CullPosition.DuplicateVector( pParticles->GetControlPointAtCurrentTime( m_nControlPoint ) );
	v4CullPosition += v4Offset;
	fltx4 fl4CullDistance = ReplicateX4( m_flDistance );

	for ( int i = 0; i < nLimit; i += 4 )
	{
		FourVectors f4ParticlePos = *pXYZ;
		f4ParticlePos -= v4CullPosition;
		fltx4 fl4DistanceTest = f4ParticlePos.length();
		fltx4 fl4CullMask;
		if ( m_bCullInside )
			fl4CullMask = CmpLtSIMD( fl4DistanceTest, fl4CullDistance );
		else
			fl4CullMask = CmpGtSIMD( fl4DistanceTest, fl4CullDistance );

		P2KillMaskedParticles( pParticles, i, fl4CullMask );
		++pXYZ;
	}
}

void C_OP_DistanceCull::Render( CParticleCollection *pParticles ) const
{
	Vector vecOrigin1 = pParticles->GetControlPointAtCurrentTime( m_nControlPoint );
	vecOrigin1 += m_vecPointOffset;

	RenderWireframeSphere( vecOrigin1, m_flDistance, 16, 8, Color( 255, 255, 255, 255 ), false );
}

//-----------------------------------------------------------------------------
// Inherit Attribute From Parent Particle
//-----------------------------------------------------------------------------
class C_OP_InheritFromParentParticles : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_InheritFromParentParticles );

	struct ParentParticlesContext_t
	{
		int m_nCurrentParentParticle;
	};

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const { return 0; }

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		ParentParticlesContext_t *pCtx = reinterpret_cast<ParentParticlesContext_t *>( pContext );
		pCtx->m_nCurrentParentParticle = 0;
	}

	size_t GetRequiredContextBytes( void ) const { return sizeof( ParentParticlesContext_t ); }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	float m_flScale;
	int m_nFieldOutput;
	int m_nIncrement;
	bool m_bRandomDistribution;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_InheritFromParentParticles, "Inherit Attribute From Parent Particle", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_InheritFromParentParticles )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "Inherited Field", "3", int, m_nFieldOutput, "intchoice particlefield" )
DMXELEMENT_UNPACK_FIELD( "Scale", "1", float, m_flScale )
DMXELEMENT_UNPACK_FIELD( "Random Parent Particle Distribution", "0", bool, m_bRandomDistribution )
DMXELEMENT_UNPACK_FIELD( "Particle Increment Amount", "1", int, m_nIncrement )
END_PARTICLE_OPERATOR_UNPACK( C_OP_InheritFromParentParticles )

void C_OP_InheritFromParentParticles::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	if ( !pParticles->m_pParent )
	{
		return;
	}
	ParentParticlesContext_t *pCtx = reinterpret_cast<ParentParticlesContext_t *>( pContext );
	int nActiveParticles = pParticles->m_pParent->m_nActiveParticles;

	if ( nActiveParticles == 0 )
	{
		return;
	}

	nActiveParticles = MAX( 0, nActiveParticles - 1 );

	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		if ( m_bRandomDistribution )
		{
			pCtx->m_nCurrentParentParticle = pParticles->RandomInt( 0, nActiveParticles );
		}
		else if ( pCtx->m_nCurrentParentParticle > nActiveParticles )
		{
			pCtx->m_nCurrentParentParticle = 0;
		}
		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );
		const float *pParentValue = pParticles->m_pParent->GetFloatAttributePtr(
		    m_nFieldOutput, pCtx->m_nCurrentParentParticle );

		if ( ATTRIBUTES_WHICH_ARE_VEC3S_MASK & ( 1 << m_nFieldOutput ) )
		{
			Vector vecParentValue;
			SetVectorFromAttribute( vecParentValue, pParentValue );
			vecParentValue *= m_flScale;

			// Clamp to 0-1 if color
			if ( ATTRIBUTES_WHICH_ARE_COLOR_AND_OPACITY & ( 1 << m_nFieldOutput ) )
			{
				vecParentValue = vecParentValue.Min( Vector( 1, 1, 1 ) );
				vecParentValue = vecParentValue.Max( Vector( 0, 0, 0 ) );
			}
			SetVectorAttribute( pOutput, vecParentValue );
		}
		else
		{
			float flOutput = *pParentValue * m_flScale;
			if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
			{
				flOutput = clamp( flOutput, 0.0f, 1.0f );
			}
			*pOutput = flOutput;
		}

		pCtx->m_nCurrentParentParticle += m_nIncrement;
	}
}

//-----------------------------------------------------------------------------
// Remap Control Point to Vector Operator
//-----------------------------------------------------------------------------
class C_OP_RemapCPtoVector : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapCPtoVector );

	uint32 GetWrittenAttributes( void ) const
	{
		return 1 << m_nFieldOutput | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const { return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK; }

	uint32 GetReadInitialAttributes( void ) const
	{
		if ( m_bScaleInitialRange )
			return 1 << m_nFieldOutput;
		else
			return 0;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		uint64 nMask = ( 1ULL << m_nCPInput );
		if ( m_nLocalSpaceCP != -1 )
		{
			nMask |= ( 1ULL << m_nLocalSpaceCP );
		}
		return nMask;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nCPInput;
	int m_nFieldOutput;
	int m_nLocalSpaceCP;
	Vector m_vInputMin;
	Vector m_vInputMax;
	Vector m_vOutputMin;
	Vector m_vOutputMax;
	float m_flStartTime;
	float m_flEndTime;
	bool m_bScaleInitialRange;
	bool m_bScaleCurrent;
	bool m_bOffset;
	bool m_bAccelerate;
};

DEFINE_PARTICLE_OPERATOR( C_OP_RemapCPtoVector, "Remap Control Point to Vector", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapCPtoVector )
DMXELEMENT_UNPACK_FIELD( "emitter lifetime start time (seconds)", "-1", float, m_flStartTime )
DMXELEMENT_UNPACK_FIELD( "emitter lifetime end time (seconds)", "-1", float, m_flEndTime )
DMXELEMENT_UNPACK_FIELD( "input control point number", "0", int, m_nCPInput )
DMXELEMENT_UNPACK_FIELD( "input minimum", "0 0 0", Vector, m_vInputMin )
DMXELEMENT_UNPACK_FIELD( "input maximum", "0 0 0", Vector, m_vInputMax )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "0", int, m_nFieldOutput, "intchoice particlefield_vector" )
DMXELEMENT_UNPACK_FIELD( "output minimum", "0 0 0", Vector, m_vOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum", "0 0 0", Vector, m_vOutputMax )
DMXELEMENT_UNPACK_FIELD(
    "output is scalar of initial random range", "0", bool, m_bScaleInitialRange )
DMXELEMENT_UNPACK_FIELD( "output is scalar of current value", "0", bool, m_bScaleCurrent )
DMXELEMENT_UNPACK_FIELD( "offset position", "0", bool, m_bOffset )
DMXELEMENT_UNPACK_FIELD( "accelerate position", "0", bool, m_bAccelerate )
DMXELEMENT_UNPACK_FIELD( "local space CP", "-1", int, m_nLocalSpaceCP )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapCPtoVector )

void C_OP_RemapCPtoVector::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	Vector vecControlPoint;
	pParticles->GetControlPointAtTime( m_nCPInput, pParticles->m_flCurTime, &vecControlPoint );
	Vector vOutputMinLocal = m_vOutputMin;
	Vector vOutputMaxLocal = m_vOutputMax;
	if ( m_nLocalSpaceCP != -1 )
	{
		matrix3x4_t mat;
		pParticles->GetControlPointTransformAtTime(
		    m_nLocalSpaceCP, pParticles->m_flCurTime, &mat );
		Vector vecTransformLocal = vec3_origin;
		VectorRotate( vOutputMinLocal, mat, vecTransformLocal );
		vOutputMinLocal = vecTransformLocal;
		VectorRotate( vOutputMaxLocal, mat, vecTransformLocal );
		vOutputMaxLocal = vecTransformLocal;
	}

	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		const float *pCreationTime =
		    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );
		// using raw creation time to map to emitter lifespan
		float flLifeTime = *pCreationTime;

		// only use within start/end time frame
		if ( ( ( flLifeTime < m_flStartTime ) || ( flLifeTime >= m_flEndTime ) ) &&
		     ( ( m_flStartTime != -1.0f ) && ( m_flEndTime != -1.0f ) ) )
			continue;

		float *pOutput = pParticles->GetFloatAttributePtrForWrite( m_nFieldOutput, i );

		Vector vOutput;
		vOutput.x = RemapValClamped(
		    vecControlPoint.x, m_vInputMin.x, m_vInputMax.x, vOutputMinLocal.x, vOutputMaxLocal.x );
		vOutput.y = RemapValClamped(
		    vecControlPoint.y, m_vInputMin.y, m_vInputMax.y, vOutputMinLocal.y, vOutputMaxLocal.y );
		vOutput.z = RemapValClamped(
		    vecControlPoint.z, m_vInputMin.z, m_vInputMax.z, vOutputMinLocal.z, vOutputMaxLocal.z );

		if ( m_bScaleInitialRange )
		{
			Vector vOrgValue;
			const float *pInput = pParticles->GetInitialFloatAttributePtr( m_nFieldOutput, i );
			SetVectorFromAttribute( vOrgValue, pInput );
			vOutput *= vOrgValue;
		}
		if ( m_bScaleCurrent )
		{
			vOutput *= *pOutput;
		}
		if ( ATTRIBUTES_WHICH_ARE_COLOR_AND_OPACITY & ( 1 << m_nFieldOutput ) )
		{
			pOutput[0] = MAX( 0.0f, MIN( vOutput.x, 1.0f ) );
			pOutput[4] = MAX( 0.0f, MIN( vOutput.y, 1.0f ) );
			pOutput[8] = MAX( 0.0f, MIN( vOutput.z, 1.0f ) );
		}
		else
		{
			float *pXYZ_Prev =
			    pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );
			Vector vXYZPrev;
			if ( m_bAccelerate )
			{
				if ( m_bOffset )
				{
					Vector vOrgValue;
					SetVectorFromAttribute( vOrgValue, pOutput );
					SetVectorFromAttribute( vXYZPrev, pXYZ_Prev );
					vOutput += vOrgValue;
					vXYZPrev += vOutput;
					vOutput += vOutput * pParticles->m_flDt;
					SetVectorAttribute( pOutput, vOutput );
					SetVectorAttribute( pXYZ_Prev, vXYZPrev );
				}
				else
				{
					vOutput *= pParticles->m_flDt;
					SetVectorAttribute( pOutput, vOutput );
				}
			}
			else
			{
				vXYZPrev = vOutput;
				if ( m_bOffset )
				{
					Vector vOrgValue;
					SetVectorFromAttribute( vOrgValue, pOutput );
					SetVectorFromAttribute( vXYZPrev, pXYZ_Prev );
					vOutput += vOrgValue;
					vXYZPrev += vOutput;
				}
				SetVectorAttribute( pOutput, vOutput );
				SetVectorAttribute( pXYZ_Prev, vXYZPrev );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Remap Velocity to Vector Operator
//-----------------------------------------------------------------------------
class C_OP_RemapVelocityToVector : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapVelocityToVector );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nFieldOutput;
	float m_flScale;
	bool m_bNormalize;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_RemapVelocityToVector, "Remap Velocity to Vector", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapVelocityToVector )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "0", int, m_nFieldOutput, "intchoice particlefield_vector" )
DMXELEMENT_UNPACK_FIELD( "normalize", "0", bool, m_bNormalize )
DMXELEMENT_UNPACK_FIELD( "scale factor", "1", float, m_flScale )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapVelocityToVector )

void C_OP_RemapVelocityToVector::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C4VAttributeIterator prev_xyz( PARTICLE_ATTRIBUTE_PREV_XYZ, pParticles );
	C4VAttributeIterator xyz( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	C4VAttributeWriteIterator pOutField( m_nFieldOutput, pParticles );
	int nCtr = pParticles->m_nPaddedActiveParticles;
	if ( m_bNormalize )
	{
		fltx4 fl4Scale = ReplicateX4( m_flScale );
		do
		{
			FourVectors v4Vel = *xyz;
			v4Vel -= *prev_xyz;
			v4Vel.VectorNormalize();
			v4Vel *= fl4Scale;
			*pOutField = v4Vel;
			++pOutField;
			++xyz;
			++prev_xyz;
		} while ( --nCtr );
	}
	else
	{
		fltx4 fl4Scale =
		    ReplicateX4( m_flScale * 1.0 / ( MAX( 1.0e-20, pParticles->m_flPreviousDt ) ) );
		do
		{
			FourVectors v4Vel = *xyz;
			v4Vel -= *prev_xyz;
			v4Vel *= fl4Scale;
			*pOutField = v4Vel;
			++pOutField;
			++xyz;
			++prev_xyz;
		} while ( --nCtr );
	}
}

//-----------------------------------------------------------------------------
// Remap Direction to CP to Vector Operator
//-----------------------------------------------------------------------------
class C_OP_RemapDirectionToCPToVector : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RemapDirectionToCPToVector );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	virtual uint64 GetReadControlPointMask() const { return ( 1ULL << m_nCP ); }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nCP;
	int m_nFieldOutput;
	float m_flScale;
	float m_flOffsetRot;
	Vector m_vecOffsetAxis;
	bool m_bNormalize;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_RemapDirectionToCPToVector, "Remap Direction to CP to Vector", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RemapDirectionToCPToVector )
DMXELEMENT_UNPACK_FIELD( "control point", "0", int, m_nCP )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "0", int, m_nFieldOutput, "intchoice particlefield_vector" )
DMXELEMENT_UNPACK_FIELD( "normalize", "0", bool, m_bNormalize )
DMXELEMENT_UNPACK_FIELD( "offset axis", "0 0 0", Vector, m_vecOffsetAxis )
DMXELEMENT_UNPACK_FIELD( "offset rotation", "0", float, m_flOffsetRot )
DMXELEMENT_UNPACK_FIELD( "scale factor", "1", float, m_flScale )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RemapDirectionToCPToVector )

void C_OP_RemapDirectionToCPToVector::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C4VAttributeIterator xyz( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	C4VAttributeWriteIterator pOutField( m_nFieldOutput, pParticles );
	int nCtr = pParticles->m_nPaddedActiveParticles;
	FourVectors v4CPPosition;
	FourVectors v4Offset;
	v4Offset.DuplicateVector( vec3_origin );

	matrix3x4_t matRot;
	MatrixBuildRotationAboutAxis( m_vecOffsetAxis, m_flOffsetRot, matRot );
	v4CPPosition.DuplicateVector( pParticles->GetControlPointAtCurrentTime( m_nCP ) );

	if ( m_bNormalize )
	{
		fltx4 fl4Scale = ReplicateX4( m_flScale );
		do
		{
			FourVectors v4Vel = *xyz;
			v4Vel -= v4CPPosition;
			v4Vel.RotateBy( matRot );
			v4Vel.VectorNormalize();
			v4Vel *= fl4Scale;
			*pOutField = v4Vel;
			++pOutField;
			++xyz;
		} while ( --nCtr );
	}
	else
	{
		fltx4 fl4Scale =
		    ReplicateX4( m_flScale * 1.0 / ( MAX( 1.0e-20, pParticles->m_flPreviousDt ) ) );
		do
		{
			FourVectors v4Vel = *xyz;
			v4Vel -= v4CPPosition;
			v4Vel += v4Offset;
			v4Vel.RotateBy( matRot );
			v4Vel *= fl4Scale;
			*pOutField = v4Vel;
			++pOutField;
			++xyz;
		} while ( --nCtr );
	}
}

//-----------------------------------------------------------------------------
// Remap Percentage Between Two Control Points to Scalar Operator
//-----------------------------------------------------------------------------
class C_OP_PercentageBetweenCPs : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_PercentageBetweenCPs );

	uint32 GetWrittenAttributes( void ) const { return 1 << m_nFieldOutput; }

	uint32 GetReadAttributes( void ) const { return PARTICLE_ATTRIBUTE_XYZ_MASK; }

	uint32 GetReadInitialAttributes( void ) const { return 1 << m_nFieldOutput; }

	virtual uint64 GetReadControlPointMask() const
	{
		return ( 1ULL << m_nStartCP ) | ( 1ULL << m_nEndCP );
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nStartCP = MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nStartCP ) );
		m_nEndCP = MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nEndCP ) );
		if ( ATTRIBUTES_WHICH_ARE_0_TO_1 & ( 1 << m_nFieldOutput ) )
		{
			m_flOutputMin = clamp( m_flOutputMin, 0.0f, 1.0f );
			m_flOutputMax = clamp( m_flOutputMax, 0.0f, 1.0f );
		}
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;

	int m_nFieldOutput;
	float m_flInputMin;
	float m_flInputMax;
	float m_flOutputMin;
	float m_flOutputMax;
	int m_nStartCP;
	int m_nEndCP;
	bool m_bScaleInitialRange;
	bool m_bScaleCurrent;
	bool m_bActiveRange;
	bool m_bRadialCheck;
};

DEFINE_PARTICLE_OPERATOR( C_OP_PercentageBetweenCPs,
    "Remap Percentage Between Two Control Points to Scalar", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_PercentageBetweenCPs )
DMXELEMENT_UNPACK_FIELD( "percentage minimum", "0", float, m_flInputMin )
DMXELEMENT_UNPACK_FIELD( "percentage maximum", "1", float, m_flInputMax )
DMXELEMENT_UNPACK_FIELD_USERDATA(
    "output field", "3", int, m_nFieldOutput, "intchoice particlefield_scalar" )
DMXELEMENT_UNPACK_FIELD( "output minimum", "0", float, m_flOutputMin )
DMXELEMENT_UNPACK_FIELD( "output maximum", "1", float, m_flOutputMax )
DMXELEMENT_UNPACK_FIELD( "starting control point", "0", int, m_nStartCP )
DMXELEMENT_UNPACK_FIELD( "ending control point", "1", int, m_nEndCP )
DMXELEMENT_UNPACK_FIELD(
    "output is scalar of initial random range", "0", bool, m_bScaleInitialRange )
DMXELEMENT_UNPACK_FIELD( "output is scalar of current value", "0", bool, m_bScaleCurrent )
DMXELEMENT_UNPACK_FIELD( "only active within input range", "0", bool, m_bActiveRange )
DMXELEMENT_UNPACK_FIELD( "treat distance between points as radius", "1", bool, m_bRadialCheck )
END_PARTICLE_OPERATOR_UNPACK( C_OP_PercentageBetweenCPs )

void C_OP_PercentageBetweenCPs::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	Vector vecControlPoint1 = pParticles->GetControlPointAtCurrentTime( m_nStartCP );
	Vector vecControlPoint2 = pParticles->GetControlPointAtCurrentTime( m_nEndCP );

	C4VAttributeIterator xyz( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	CM128AttributeWriteIterator pOutField( m_nFieldOutput, pParticles );
	CM128InitialAttributeIterator pInitialValue( m_nFieldOutput, pParticles );

	FourVectors fvControlPoint1;
	FourVectors fvControlpoint2;
	fvControlPoint1.DuplicateVector( vecControlPoint1 );
	fvControlpoint2.DuplicateVector( vecControlPoint2 );
	FourVectors fvDelta = fvControlPoint1;
	fvDelta -= fvControlpoint2;
	fltx4 fl4Distance = fvDelta.length();
	fltx4 fl4InputMin = ReplicateX4( m_flInputMin );
	fltx4 fl4InputMax = ReplicateX4( m_flInputMax );
	fltx4 fl4OutputMin = ReplicateX4( m_flOutputMin );
	fltx4 fl4OutputMax = ReplicateX4( m_flOutputMax );

	int nCtr = pParticles->m_nPaddedActiveParticles;

	do
	{
		fltx4 fl4Perc;
		if ( m_bRadialCheck )
		{
			FourVectors fvParticleDelta;
			fvParticleDelta.DuplicateVector( vecControlPoint1 );
			fvParticleDelta -= *xyz;
			fltx4 fl4ParticleDistance = AddSIMD( fvParticleDelta.length(), Four_Epsilons );
			fl4Perc = DivSIMD( Four_Ones, DivSIMD( fl4Distance, fl4ParticleDistance ) );
		}
		else
		{
			// Parametric position of the closest point on the CP line
			float flT[4];
			for ( int j = 0; j < 4; ++j )
			{
				Vector vecClosest;
				Vector vecParticle(
				    SubFloat( xyz->x, j ), SubFloat( xyz->y, j ), SubFloat( xyz->z, j ) );
				CalcClosestPointOnLine(
				    vecParticle, vecControlPoint1, vecControlPoint2, vecClosest, &flT[j] );
			}
			fl4Perc = LoadUnalignedSIMD( flT );
		}

		fltx4 fl4Output =
		    P2RemapValClampedSIMD( fl4Perc, fl4InputMin, fl4InputMax, fl4OutputMin, fl4OutputMax );

		if ( m_bScaleInitialRange )
		{
			fl4Output = MulSIMD( fl4Output, *pInitialValue );
		}
		if ( m_bScaleCurrent )
		{
			fl4Output = MulSIMD( fl4Output, *pOutField );
		}

		if ( m_bActiveRange )
		{
			fltx4 fl4GoodMask = CmpGeSIMD( fl4Perc, fl4InputMin );
			fl4GoodMask = AndSIMD( fl4GoodMask, CmpLeSIMD( fl4Perc, fl4InputMax ) );
			*pOutField = MaskedAssign( fl4GoodMask, fl4Output, *pOutField );
		}
		else
		{
			*pOutField = fl4Output;
		}

		++pOutField;
		++xyz;
		++pInitialValue;
	} while ( --nCtr );
}

//-----------------------------------------------------------------------------
// Movement Lock to Saved Position Along Path
// Requires "Position Along Path Sequential" with "Save Offset" on the same system
//-----------------------------------------------------------------------------
class C_OP_LockToSavedSequentialPath : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_LockToSavedSequentialPath );

	int m_nPathCount;
	bool m_bCPPairs;
	struct CPathParameters m_PathParams;

	struct LockPathPositionContext_t
	{
		Vector m_vecPrevStartPnt[MAX_PARTICLE_CONTROL_POINTS];
		Vector m_vecPrevMidP[MAX_PARTICLE_CONTROL_POINTS];
		Vector m_vecPrevEndPnt[MAX_PARTICLE_CONTROL_POINTS];
	};

	uint32 GetWrittenAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_HITBOX_INDEX_MASK | PARTICLE_ATTRIBUTE_CREATION_TIME_MASK |
		       PARTICLE_ATTRIBUTE_HITBOX_RELATIVE_XYZ_MASK;
	}

	virtual uint64 GetReadControlPointMask() const
	{
		uint64 nStartMask = ( 1ULL << m_PathParams.m_nStartControlPointNumber ) - 1;
		uint64 nEndMask = m_bCPPairs
		                      ? 0xFFFFFFFFFFFFFFFFULL
		                      : ( 1ULL << ( m_PathParams.m_nEndControlPointNumber + 1 ) ) - 1;
		return nEndMask & ( ~nStartMask );
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_PathParams.ClampControlPointIndices();
		m_nPathCount = MAX( 1, m_bCPPairs ? m_PathParams.m_nEndControlPointNumber -
		                                        m_PathParams.m_nStartControlPointNumber
		                                  : 1 );
	}

	size_t GetRequiredContextBytes( void ) const { return sizeof( LockPathPositionContext_t ); }

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		LockPathPositionContext_t *pCtx = reinterpret_cast<LockPathPositionContext_t *>( pContext );
		for ( int i = 0; i < m_nPathCount; ++i )
		{
			pCtx->m_vecPrevStartPnt[i] = vec3_invalid;
		}
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_LockToSavedSequentialPath,
    "Movement Lock to Saved Position Along Path", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_LockToSavedSequentialPath )
DMXELEMENT_UNPACK_FIELD( "bulge", "0", float, m_PathParams.m_flBulge )
DMXELEMENT_UNPACK_FIELD(
    "start control point number", "0", int, m_PathParams.m_nStartControlPointNumber )
DMXELEMENT_UNPACK_FIELD(
    "end control point number", "1", int, m_PathParams.m_nEndControlPointNumber )
DMXELEMENT_UNPACK_FIELD(
    "bulge control 0=random 1=orientation of start pnt 2=orientation of end point", "0", int,
    m_PathParams.m_nBulgeControl )
DMXELEMENT_UNPACK_FIELD( "mid point position", "0.5", float, m_PathParams.m_flMidPoint )
DMXELEMENT_UNPACK_FIELD(
    "Use sequential CP pairs between start and end point", "0", bool, m_bCPPairs )
END_PARTICLE_OPERATOR_UNPACK( C_OP_LockToSavedSequentialPath )

void C_OP_LockToSavedSequentialPath::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	LockPathPositionContext_t *pCtx = reinterpret_cast<LockPathPositionContext_t *>( pContext );

	Vector Delta0[MAX_PARTICLE_CONTROL_POINTS];
	Vector Delta1[MAX_PARTICLE_CONTROL_POINTS];
	Vector PrevDelta0[MAX_PARTICLE_CONTROL_POINTS];
	Vector PrevDelta1[MAX_PARTICLE_CONTROL_POINTS];
	Vector StartPnt[MAX_PARTICLE_CONTROL_POINTS];
	Vector MidP[MAX_PARTICLE_CONTROL_POINTS];
	Vector EndPnt[MAX_PARTICLE_CONTROL_POINTS];

	for ( int i = 0; i < m_nPathCount; ++i )
	{
		struct CPathParameters CurrentPathParams = m_PathParams;
		if ( m_bCPPairs )
		{
			CurrentPathParams.m_nStartControlPointNumber += i;
			CurrentPathParams.m_nEndControlPointNumber =
			    CurrentPathParams.m_nStartControlPointNumber + 1;
		}

		pParticles->CalculatePathValues(
		    CurrentPathParams, pParticles->m_flCurTime, &StartPnt[i], &MidP[i], &EndPnt[i] );
		// If it's first run, initialize our values
		if ( pCtx->m_vecPrevStartPnt[i] == vec3_invalid )
		{
			pParticles->CalculatePathValues( CurrentPathParams,
			    pParticles->m_flCurTime - pParticles->m_flDt, &pCtx->m_vecPrevStartPnt[i],
			    &pCtx->m_vecPrevMidP[i], &pCtx->m_vecPrevEndPnt[i] );
		}

		// form delta terms needed for quadratic bezier
		Delta0[i] = MidP[i] - StartPnt[i];
		Delta1[i] = EndPnt[i] - MidP[i];
		PrevDelta0[i] = pCtx->m_vecPrevMidP[i] - pCtx->m_vecPrevStartPnt[i];
		PrevDelta1[i] = pCtx->m_vecPrevEndPnt[i] - pCtx->m_vecPrevMidP[i];
	}

	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		float *xyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_XYZ, i );
		float *pxyz = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_PREV_XYZ, i );
		const float *pSavedPos =
		    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_HITBOX_RELATIVE_XYZ, i );
		const float *pCt = pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );

		Vector vecSavedPos;
		SetVectorFromAttribute( vecSavedPos, pSavedPos );

		float flParticleAge = pParticles->m_flCurTime - *pCt;

		float flCreationFrameBias = MIN( flParticleAge, pParticles->m_flDt );
		flCreationFrameBias *= ( 1 / pParticles->m_flDt );

		float t = vecSavedPos.x;
		// The saved start CP selects the path segment; keep it inside the
		// segments computed above
		int nStartCP = int( MAX( 0, vecSavedPos.y - m_PathParams.m_nStartControlPointNumber ) );
		nStartCP = MIN( nStartCP, m_nPathCount - 1 );

		Vector PrevL0 = pCtx->m_vecPrevStartPnt[nStartCP] + t * PrevDelta0[nStartCP];
		Vector PrevL1 = pCtx->m_vecPrevMidP[nStartCP] + t * PrevDelta1[nStartCP];
		Vector PrevPnt = PrevL0 + ( PrevL1 - PrevL0 ) * t;
		Vector L0 = StartPnt[nStartCP] + t * Delta0[nStartCP];
		Vector L1 = MidP[nStartCP] + t * Delta1[nStartCP];
		Vector Pnt = L0 + ( L1 - L0 ) * t;

		Pnt -= PrevPnt;

		VectorLerp( vec3_origin, Pnt, vec_t( flCreationFrameBias ), Pnt );

		Vector vecXYZ;
		Vector vecPXYZ;

		SetVectorFromAttribute( vecXYZ, xyz );
		SetVectorFromAttribute( vecPXYZ, pxyz );

		vecXYZ += Pnt;
		vecPXYZ += Pnt;

		xyz[0] = vecXYZ.x;
		xyz[4] = vecXYZ.y;
		xyz[8] = vecXYZ.z;
		pxyz[0] = vecPXYZ.x;
		pxyz[4] = vecPXYZ.y;
		pxyz[8] = vecPXYZ.z;
	}

	for ( int i = 0; i < m_nPathCount; ++i )
	{
		pCtx->m_vecPrevStartPnt[i] = StartPnt[i];
		pCtx->m_vecPrevMidP[i] = MidP[i];
		pCtx->m_vecPrevEndPnt[i] = EndPnt[i];
	}
}

//-----------------------------------------------------------------------------
// Normal Lock to Control Point
// Rotates particle normals with their control point
//-----------------------------------------------------------------------------
class C_OP_NormalLock : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_NormalLock );

	struct C_OP_NormalLockContext_t
	{
		matrix3x4_t m_matPrevTransform;
	};

	int m_nControlPointNumber;

	uint32 GetWrittenAttributes( void ) const { return PARTICLE_ATTRIBUTE_NORMAL_MASK; }

	uint32 GetReadAttributes( void ) const { return PARTICLE_ATTRIBUTE_CREATION_TIME_MASK; }

	virtual uint64 GetReadControlPointMask() const { return 1ULL << m_nControlPointNumber; }

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nControlPointNumber =
		    MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nControlPointNumber ) );
	}

	size_t GetRequiredContextBytes( void ) const { return sizeof( C_OP_NormalLockContext_t ); }

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		C_OP_NormalLockContext_t *pCtx = reinterpret_cast<C_OP_NormalLockContext_t *>( pContext );
		pParticles->GetControlPointTransformAtTime(
		    m_nControlPointNumber, pParticles->m_flCurTime, &pCtx->m_matPrevTransform );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_NormalLock, "Normal Lock to Control Point", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_NormalLock )
DMXELEMENT_UNPACK_FIELD( "control_point_number", "0", int, m_nControlPointNumber )
END_PARTICLE_OPERATOR_UNPACK( C_OP_NormalLock )

void C_OP_NormalLock::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C_OP_NormalLockContext_t *pCtx = reinterpret_cast<C_OP_NormalLockContext_t *>( pContext );

	matrix3x4_t matCurrentTransform;
	matrix3x4_t matTransformLock;

	pParticles->GetControlPointTransformAtTime(
	    m_nControlPointNumber, pParticles->m_flCurTime, &matCurrentTransform );
	matrix3x4_t matPrev;
	MatrixInvert( pCtx->m_matPrevTransform, matPrev );
	MatrixMultiply( matCurrentTransform, matPrev, matTransformLock );

	for ( int i = 0; i < pParticles->m_nActiveParticles; ++i )
	{
		const float *pCreationTime =
		    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_CREATION_TIME, i );

		float flParticleAge = pParticles->m_flCurTime - *pCreationTime;

		float flCreationFrameBias = MIN( flParticleAge, pParticles->m_flDt );
		flCreationFrameBias *= ( 1 / pParticles->m_flDt );

		float *normal = pParticles->GetFloatAttributePtrForWrite( PARTICLE_ATTRIBUTE_NORMAL, i );

		Vector vecNormalOld, vecNormalNew;
		SetVectorFromAttribute( vecNormalOld, normal );

		VectorRotate( vecNormalOld, matTransformLock, vecNormalNew );

		VectorLerp( vecNormalOld, vecNormalNew, vec_t( flCreationFrameBias ), vecNormalNew );

		SetVectorAttribute( normal, vecNormalNew );
	}

	// Store off the control point transform for the next delta computation
	pCtx->m_matPrevTransform = matCurrentTransform;
}

//-----------------------------------------------------------------------------
// Restart after Randomized Duration
//-----------------------------------------------------------------------------
class C_OP_RestartAfterDuration : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RestartAfterDuration );

	float m_flDurationMin;
	float m_flDurationMax;
	int m_nCP;
	int m_nCPField;
	int m_nChildGroupID;
	bool m_bOnlyChildren;

	struct RestartAfterDurationContext_t
	{
		float m_flLastRestart;
		float m_flRestartDuration;
	};

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const { return 0; }

	virtual uint64 GetReadControlPointMask() const
	{
		return ( m_nCP >= 0 ) ? ( 1ULL << m_nCP ) : 0;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nCP = clamp( m_nCP, -1, MAX_PARTICLE_CONTROL_POINTS - 1 );
		m_nCPField = clamp( m_nCPField, 0, 2 );
	}

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		RestartAfterDurationContext_t *pCtx =
		    reinterpret_cast<RestartAfterDurationContext_t *>( pContext );
		pCtx->m_flRestartDuration = pParticles->RandomFloat( m_flDurationMin, m_flDurationMax );
		pCtx->m_flLastRestart = 0;
	}

	size_t GetRequiredContextBytes( void ) const { return sizeof( RestartAfterDurationContext_t ); }

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_RestartAfterDuration, "Restart Effect after Duration", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_RestartAfterDuration )
DMXELEMENT_UNPACK_FIELD( "Minimum Restart Time", "0", float, m_flDurationMin )
DMXELEMENT_UNPACK_FIELD( "Maximum Restart Time", "1", float, m_flDurationMax )
DMXELEMENT_UNPACK_FIELD( "Control Point to Scale Duration", "-1", int, m_nCP )
DMXELEMENT_UNPACK_FIELD( "Control Point Field X/Y/Z", "0", int, m_nCPField )
DMXELEMENT_UNPACK_FIELD( "Only Restart Children", "0", bool, m_bOnlyChildren )
DMXELEMENT_UNPACK_FIELD( "Child Group ID", "0", int, m_nChildGroupID )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RestartAfterDuration )

void C_OP_RestartAfterDuration::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	if ( !pParticles->m_bEmissionStopped )
	{
		RestartAfterDurationContext_t *pCtx =
		    reinterpret_cast<RestartAfterDurationContext_t *>( pContext );
		float flDuration = pCtx->m_flRestartDuration;
		if ( m_nCP > -1 )
		{
			flDuration *= pParticles->GetControlPointAtCurrentTime( m_nCP )[m_nCPField];
		}
		if ( pParticles->m_flCurTime > flDuration + pCtx->m_flLastRestart )
		{
			if ( m_bOnlyChildren )
			{
				for ( CParticleCollection *pChild = pParticles->m_Children.m_pHead; pChild;
				    pChild = pChild->m_pNext )
				{
					if ( pChild->GetGroupID() == m_nChildGroupID )
						pChild->Restart();
				}
			}
			else
			{
				pParticles->Restart();
			}
			pCtx->m_flLastRestart = pParticles->m_flCurTime;
			pCtx->m_flRestartDuration = pParticles->RandomFloat( m_flDurationMin, m_flDurationMax );
		}
	}
}

//-----------------------------------------------------------------------------
// Set Control Point Rotation
//-----------------------------------------------------------------------------
class C_OP_SetControlPointRotation : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetControlPointRotation );

	Vector m_vecRotAxis;
	float m_flRotRate;
	int m_nCP;
	int m_nLocalCP;

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const { return 0; }

	virtual uint64 GetReadControlPointMask() const
	{
		uint64 nMask = ( 1ULL << m_nCP );
		if ( m_nLocalCP != -1 )
		{
			nMask |= ( 1ULL << m_nLocalCP );
		}
		return nMask;
	}

	bool ShouldRunBeforeEmitters( void ) const { return true; }

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		VectorNormalize( m_vecRotAxis );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_SetControlPointRotation, "Set Control Point Rotation", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointRotation )
DMXELEMENT_UNPACK_FIELD( "Rotation Axis", "0 0 1", Vector, m_vecRotAxis )
DMXELEMENT_UNPACK_FIELD( "Rotation Rate", "180", float, m_flRotRate )
DMXELEMENT_UNPACK_FIELD( "Control Point", "0", int, m_nCP )
DMXELEMENT_UNPACK_FIELD( "Local Space Control Point", "-1", int, m_nLocalCP )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointRotation )

void C_OP_SetControlPointRotation::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	float flRotRate = m_flRotRate * pParticles->m_flDt;

	matrix3x4_t matCP, matRot, matFinal;

	Vector vecRotAxis = m_vecRotAxis;

	if ( m_nLocalCP > -1 )
	{
		matrix3x4_t matLocalCP;
		pParticles->GetControlPointTransformAtCurrentTime( m_nLocalCP, &matLocalCP );
		VectorRotate( m_vecRotAxis, matLocalCP, vecRotAxis );
	}

	pParticles->GetControlPointTransformAtCurrentTime( m_nCP, &matCP );

	MatrixBuildRotationAboutAxis( vecRotAxis, flRotRate, matRot );

	MatrixMultiply( matCP, matRot, matFinal );

	Quaternion quatRot;
	MatrixQuaternion( matFinal, quatRot );
	pParticles->SetControlPointOrientation( m_nCP, quatRot );
}

//-----------------------------------------------------------------------------
// Set Control Point to Impact Point
//-----------------------------------------------------------------------------
class C_OP_SetControlPointToImpactPoint : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetControlPointToImpactPoint );

	int m_nCPOut;
	int m_nCPIn;
	int m_nCollisionGroupNumber;
	float m_flUpdateRate;
	float m_flTraceLength;
	float m_flOffset;
	Vector m_vecTraceDir;
	char m_CollisionGroupName[128];

	struct C_OP_SetCPToImpactPointContext_t
	{
		float m_flNextUpdateTime;
	};

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const { return 0; }

	virtual uint64 GetReadControlPointMask() const { return 1ULL << m_nCPIn; }

	bool ShouldRunBeforeEmitters( void ) const { return true; }

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nCollisionGroupNumber =
		    g_pParticleSystemMgr->Query()->GetCollisionGroupFromName( m_CollisionGroupName );
		m_nCPIn = MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nCPIn ) );
		m_nCPOut = MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nCPOut ) );
	}

	size_t GetRequiredContextBytes( void ) const
	{
		return sizeof( C_OP_SetCPToImpactPointContext_t );
	}

	virtual void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const
	{
		C_OP_SetCPToImpactPointContext_t *pCtx =
		    reinterpret_cast<C_OP_SetCPToImpactPointContext_t *>( pContext );
		pCtx->m_flNextUpdateTime = 0.0 - m_flUpdateRate;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR(
    C_OP_SetControlPointToImpactPoint, "Set Control Point to Impact Point", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointToImpactPoint )
DMXELEMENT_UNPACK_FIELD( "Control Point to Set", "1", int, m_nCPOut )
DMXELEMENT_UNPACK_FIELD( "Control Point to Trace From", "1", int, m_nCPIn )
DMXELEMENT_UNPACK_FIELD( "Trace Direction Override", "0 0 0", Vector, m_vecTraceDir )
DMXELEMENT_UNPACK_FIELD( "Trace Update Rate", "0.5", float, m_flUpdateRate )
DMXELEMENT_UNPACK_FIELD( "Max Trace Length", "1024", float, m_flTraceLength )
DMXELEMENT_UNPACK_FIELD( "Offset End Point Amount", "0", float, m_flOffset )
DMXELEMENT_UNPACK_FIELD_STRING( "trace collision group", "NONE", m_CollisionGroupName )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointToImpactPoint )

void C_OP_SetControlPointToImpactPoint::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	C_OP_SetCPToImpactPointContext_t *pCtx =
	    reinterpret_cast<C_OP_SetCPToImpactPointContext_t *>( pContext );

	if ( pCtx->m_flNextUpdateTime <= pParticles->m_flCurTime )
	{
		Vector vecTraceDir = m_vecTraceDir;
		if ( m_vecTraceDir == vec3_origin )
		{
			Vector vecRight, vecUp;
			pParticles->GetControlPointOrientationAtTime(
			    m_nCPIn, pParticles->m_flCurTime, &vecTraceDir, &vecRight, &vecUp );
		}

		Vector vecStartPnt = pParticles->GetControlPointAtCurrentTime( m_nCPIn );
		Vector vecEndPnt = vecStartPnt + ( vecTraceDir * m_flTraceLength );

		CBaseTrace tr;
		g_pParticleSystemMgr->Query()->TraceLine(
		    vecStartPnt, vecEndPnt, MASK_ALL, NULL, m_nCollisionGroupNumber, &tr );

		Vector vecForward, vecRight, vecUp;
		vecForward = tr.plane.normal;
		VectorVectors( vecForward, vecRight, vecUp );
		Vector vecPos = tr.endpos + ( vecTraceDir * -m_flOffset );
		pParticles->SetControlPoint( m_nCPOut, vecPos );
		pParticles->SetControlPointOrientation( m_nCPOut, vecForward, vecRight, vecUp );
		pCtx->m_flNextUpdateTime = pParticles->m_flCurTime + m_flUpdateRate;
	}
}

//-----------------------------------------------------------------------------
// Set control points from particle positions - copies the positions of our
// particles to our own control points
//-----------------------------------------------------------------------------
class C_OP_SetControlPointsToParticle : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetControlPointsToParticle );

	int m_nFirstControlPoint;
	int m_nNumControlPoints;
	int m_nFirstSourcePoint;
	bool m_bSetOrientation;

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_SetControlPointsToParticle,
    "Set control points from particle positions", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointsToParticle )
DMXELEMENT_UNPACK_FIELD( "First control point to set", "0", int, m_nFirstControlPoint )
DMXELEMENT_UNPACK_FIELD( "# of control points to set", "1", int, m_nNumControlPoints )
DMXELEMENT_UNPACK_FIELD( "first particle to copy", "0", int, m_nFirstSourcePoint )
DMXELEMENT_UNPACK_FIELD( "set orientation", "0", bool, m_bSetOrientation )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetControlPointsToParticle )

void C_OP_SetControlPointsToParticle::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	int nFirst = MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nFirstControlPoint ) );
	int nToSet = MIN( pParticles->m_nActiveParticles - m_nFirstSourcePoint, m_nNumControlPoints );
	nToSet = MIN( nToSet, MAX_PARTICLE_CONTROL_POINTS - nFirst );
	for ( int p = 0; p < nToSet; p++ )
	{
		const float *pXYZ =
		    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_XYZ, p + m_nFirstSourcePoint );
		Vector cPnt( pXYZ[0], pXYZ[4], pXYZ[8] );
		pParticles->SetControlPoint( p + nFirst, cPnt );

		if ( m_bSetOrientation )
		{
			const float *pXYZ_Prev = pParticles->GetFloatAttributePtr(
			    PARTICLE_ATTRIBUTE_PREV_XYZ, p + m_nFirstSourcePoint );
			Vector vecXYZ, vecXYZPrev;
			SetVectorFromAttribute( vecXYZ, pXYZ );
			SetVectorFromAttribute( vecXYZPrev, pXYZ_Prev );
			Vector vecFwd = vecXYZ - vecXYZPrev;
			vecFwd.NormalizeInPlace();
			Vector vecRight, vecUp;
			VectorVectors( vecFwd, vecRight, vecUp );
			pParticles->SetControlPointOrientation( p + nFirst, vecFwd, vecRight, vecUp );
		}
	}
}

//-----------------------------------------------------------------------------
// Set per child control point from particle positions - copies the position
// of each particle to one control point of one child
//-----------------------------------------------------------------------------
class C_OP_SetPerChildControlPoint : public CParticleOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_SetPerChildControlPoint );

	int m_nChildGroupID;
	int m_nFirstControlPoint;
	int m_nNumControlPoints;
	int m_nFirstSourcePoint;
	bool m_bSetOrientation;

	uint32 GetWrittenAttributes( void ) const { return 0; }

	uint32 GetReadAttributes( void ) const
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_PREV_XYZ_MASK;
	}

	virtual void InitParams( CParticleSystemDefinition *pDef, CDmxElement *pElement )
	{
		m_nFirstControlPoint =
		    MAX( 0, MIN( MAX_PARTICLE_CONTROL_POINTS - 1, m_nFirstControlPoint ) );
	}

	virtual void Operate( CParticleCollection *pParticles, float flStrength, void *pContext ) const;
};

DEFINE_PARTICLE_OPERATOR( C_OP_SetPerChildControlPoint,
    "Set per child control point from particle positions", OPERATOR_GENERIC );

BEGIN_PARTICLE_OPERATOR_UNPACK( C_OP_SetPerChildControlPoint )
DMXELEMENT_UNPACK_FIELD( "Group ID to affect", "0", int, m_nChildGroupID )
DMXELEMENT_UNPACK_FIELD( "control point to set", "0", int, m_nFirstControlPoint )
DMXELEMENT_UNPACK_FIELD( "# of children to set", "1", int, m_nNumControlPoints )
DMXELEMENT_UNPACK_FIELD( "first particle to copy", "0", int, m_nFirstSourcePoint )
DMXELEMENT_UNPACK_FIELD( "set orientation", "0", bool, m_bSetOrientation )
END_PARTICLE_OPERATOR_UNPACK( C_OP_SetPerChildControlPoint )

void C_OP_SetPerChildControlPoint::Operate(
    CParticleCollection *pParticles, float flStrength, void *pContext ) const
{
	int nToSet =
	    MIN( m_nNumControlPoints, MIN( pParticles->m_nActiveParticles - m_nFirstSourcePoint,
	                                  pParticles->m_Children.Count() ) );
	if ( nToSet <= 0 )
		return;

	int nCurrentPoint = m_nFirstSourcePoint;

	for ( CParticleCollection *pChild = pParticles->m_Children.m_pHead; pChild;
	    pChild = pChild->m_pNext )
	{
		if ( pChild->GetGroupID() == m_nChildGroupID && nToSet )
		{
			const float *pXYZ =
			    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_XYZ, nCurrentPoint );
			Vector cPnt( pXYZ[0], pXYZ[4], pXYZ[8] );
			pChild->SetControlPoint( m_nFirstControlPoint, cPnt );

			if ( m_bSetOrientation )
			{
				const float *pXYZ_Prev =
				    pParticles->GetFloatAttributePtr( PARTICLE_ATTRIBUTE_PREV_XYZ, nCurrentPoint );
				Vector vecXYZ, vecXYZPrev;
				SetVectorFromAttribute( vecXYZ, pXYZ );
				SetVectorFromAttribute( vecXYZPrev, pXYZ_Prev );
				Vector vecFwd = vecXYZ - vecXYZPrev;
				vecFwd.NormalizeInPlace();
				Vector vecRight, vecUp;
				VectorVectors( vecFwd, vecRight, vecUp );
				pChild->SetControlPointOrientation( m_nFirstControlPoint, vecFwd, vecRight, vecUp );
			}
			nToSet--;

			nCurrentPoint++;
		}
	}
}

void AddBuiltInParticleOperators( void )
{
	// Portal 2 operators
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_FadeInSimple );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_FadeOutSimple );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_ClampScalar );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_OscillateScalarSimple );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_DifferencePreviousParticle );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_LerpScalar );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapSpeed );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RampScalarLinear );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RampScalarSpline );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RampScalarLinearSimple );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RampScalarSplineSimple );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_AlphaDecay );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RadiusDecay );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_DistanceCull );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_InheritFromParentParticles );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapCPtoVector );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapVelocityToVector );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapDirectionToCPToVector );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_PercentageBetweenCPs );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_LockToSavedSequentialPath );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_NormalLock );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RestartAfterDuration );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetControlPointRotation );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetControlPointToImpactPoint );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetControlPointsToParticle );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetPerChildControlPoint );

	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_BasicMovement );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_Decay );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_VelocityDecay );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_FadeAndKill );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_FadeIn );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_FadeOut );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_Spin );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SpinUpdate );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SpinYaw );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_OrientTo2dDirection );	
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_Orient2DRelToCP );	
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_InterpolateRadius );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_ColorInterpolate );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_OscillateScalar );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_OscillateVector );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_DampenToCP );	
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_PositionLock );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_LockToBone );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_DistanceBetweenCPs );		
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_DistanceToCP );		
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetControlPointToPlayer );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetControlPointToCenter );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetChildControlPoints );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_SetControlPointPositions );	
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_PlaneCull );	
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_ModelCull );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_Cull );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_ControlpointLight ); 	
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapScalar );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_Noise );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_VectorNoise );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_VelocityMatchingForce );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_MaxVelocity );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_MaintainSequentialPath );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapDotProductToScalar );
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapCPtoScalar );		
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_MovementRotateParticleAroundAxis );		
	REGISTER_PARTICLE_OPERATOR( FUNCTION_OPERATOR, C_OP_RemapSpeedtoCP );		
}

