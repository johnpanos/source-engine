//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Live shadow verifier for the previous-frame bone-setup cohort
//          (RFC 0003 R10/R21). See bone_setup_shadow_verify.h.
//
//          What one item (SetupBones( NULL, -1, -1, t ) on an unparented
//          entity) writes, and so what a snapshot must cover so the serial
//          rerun starts where the pooled item started and the pooled state
//          can be put back afterwards:
//
//          - the bone cache (m_CachedBoneData), the bone accessor's readable
//            and writable masks, the previous/accumulated masks, the last
//            setup time and the model bone counter;
//          - m_nSequence (reset when out of range) and the sequence
//            transitioner's queue and parity;
//          - attachments (SetupBones_AttachmentHelper -> PutAttachment, which
//            also reads the previous value for the origin velocity);
//          - m_pRagdollInfo->m_bActive (UnragdollBlend);
//          - jiggle-bone state (BuildTransformations);
//          - overlay layers, their event cycles and their interpolation
//            history (C_BaseAnimatingOverlay::AccumulateLayers ->
//            CheckForLayerChanges, BlendWeight);
//          - flex weights (C_BaseFlex::BuildTransformations -> SetFlexWeight).
//
//          Items the verifier does not rerun, counted by reason:
//
//          - parented: the item does no work (SetupBonesOnBaseAnimatingAtTime
//            skips entities with a move parent);
//          - ik: the entity has (or the pooled item allocated) an IK context.
//            Its targets carry private members and padding that cannot be
//            captured byte-exactly from outside CIKContext;
//          - jiggle-alloc / bone-merge-alloc: the pooled item allocated (or
//            released) jiggle state or a bone-merge cache. With the object
//            in place the rerun is exact: the bone-merge cache is a memo
//            keyed on the followed entity and both model headers;
//          - renderfx: kRenderFxDistort/Hologram draw from the global random
//            stream in ApplyBoneMatrixTransform, which a rerun would advance;
//          - mouth: ControlMouth writes a pose parameter and its
//            interpolation history;
//          - layout: bone, attachment or layer counts changed.
//
//          Effects outside the snapshot, and why they do not matter here:
//          the model cache lock (bracketed like a runner); EFL_SETTING_UP_BONES
//          (added and removed within the call; compared, not restored);
//          CalculateIKLocks' partition suppression and abs-recompute stack
//          (IK items are skipped); lazily created studio headers and abs
//          transforms (caches the pooled item already filled);
//          CheckForLayerChanges' UpdateVisibility (idempotent); nested
//          SetupBones on other entities through attachments (their cache is
//          valid for this frame, so the call only ORs an already set mask).
//          The pose debugger (ent_posedebug) keeps per-model state and is not
//          supported while verifying.
//
//=============================================================================//

#include "cbase.h"
#include "bone_setup_shadow_verify.h"
#include "c_baseanimating.h"
#include "c_baseanimatingoverlay.h"
#include "c_baseflex.h"
#include "bone_setup.h"
#include "jigglebones.h"
#include "interpolatedvar.h"
#include "jobsystem/batch_shadow_verify.h"
#include "tier0/platform.h"

#include <float.h>
#include <math.h>

#include <memory>
#include <vector>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_bone_setup_verify( "cl_bone_setup_verify", "0", 0,
    "Shadow-verify previous-frame bone setup (RFC 0003): after each pooled batch, rerun its "
    "items serially from their captured state and byte-compare with the pooled outputs. "
    "The frame keeps the pooled outputs. Report: cl_bone_setup_verify_report.",
    true, 0, true, 1 );
static ConVar cl_bone_setup_verify_fault( "cl_bone_setup_verify_fault", "0", 0,
    "Negative control for cl_bone_setup_verify: 1 nudges the first unparented item's first "
    "bone by one ulp inside the pooled batch, 2 drops that item. Only while verifying.",
    true, 0, true, 2 );

bool CBoneSetupShadowVerifier::s_bObserveItems = false;

namespace
{

enum BoneShadowSkip_t
{
	BONE_SHADOW_VERIFIABLE = 0,
	BONE_SHADOW_SKIP_PARENTED,
	BONE_SHADOW_SKIP_IK,
	BONE_SHADOW_SKIP_JIGGLE_ALLOC,
	BONE_SHADOW_SKIP_BONE_MERGE_ALLOC,
	BONE_SHADOW_SKIP_RENDERFX,
	BONE_SHADOW_SKIP_MOUTH,
	BONE_SHADOW_SKIP_LAYOUT,
	BONE_SHADOW_SKIP_COUNT
};

const char *const s_pSkipNames[BONE_SHADOW_SKIP_COUNT] = { "verifiable", "parented", "ik",
    "jiggle-alloc", "bone-merge-alloc", "renderfx", "mouth", "layout" };

// C_AnimationLayer without its trailing padding, so equal layers compare equal.
struct PackedLayer_t
{
	int nSequence;
	float flPrevCycle;
	float flWeight;
	int nOrder;
	float flPlaybackRate;
	float flCycle;
	float flLayerAnimtime;
	float flLayerFadeOuttime;
	float flBlendIn;
	float flBlendOut;
	int bClientBlend;
};

PackedLayer_t PackLayer( const C_AnimationLayer &layer )
{
	PackedLayer_t packed;
	packed.nSequence = layer.m_nSequence;
	packed.flPrevCycle = layer.m_flPrevCycle;
	packed.flWeight = layer.m_flWeight;
	packed.nOrder = layer.m_nOrder;
	packed.flPlaybackRate = layer.m_flPlaybackRate;
	packed.flCycle = layer.m_flCycle;
	packed.flLayerAnimtime = layer.m_flLayerAnimtime;
	packed.flLayerFadeOuttime = layer.m_flLayerFadeOuttime;
	packed.flBlendIn = layer.m_flBlendIn;
	packed.flBlendOut = layer.m_flBlendOut;
	packed.bClientBlend = layer.m_bClientBlend ? 1 : 0;
	return packed;
}

void AppendLayers( jobsystem::ShadowRecord &record, const char *pName,
    const C_AnimationLayer *pLayers, int nLayers, std::vector<PackedLayer_t> &scratch )
{
	scratch.resize( nLayers );
	for ( int i = 0; i < nLayers; ++i )
		scratch[i] = PackLayer( pLayers[i] );
	record.Append( pName, nLayers ? &scratch[0] : NULL, nLayers * sizeof( PackedLayer_t ) );
}

typedef CInterpolatedVar<C_AnimationLayer> LayerHistory_t;

struct BoneShadowSnapshot_t
{
	// Compared: the item's state, packed without padding.
	jobsystem::ShadowRecord record;

	// Restore-only typed copies and identities (not compared).
	const CIKContext *pIk;
	const CJiggleBones *pJiggle;
	const CBoneMergeCache *pBoneMerge;
	int nBones;
	int nAttachments;
	int nLayers;
	int nLayerHistories; // may be fewer than nLayers (SetNumAnimOverlays)
	std::vector<C_AnimationLayer> transitions;
	std::vector<C_AnimationLayer> layers;
	std::vector<std::unique_ptr<LayerHistory_t>> layerHistory;
	std::vector<JiggleData> jiggle;
};

struct BoneShadowTotals_t
{
	unsigned batches;
	unsigned items;
	unsigned verified;
	unsigned mismatches;
	unsigned restoreMismatches;
	unsigned faultBatches;
	unsigned multiThreadBatches; // pooled items ran on two or more threads
	unsigned offHostItems;       // pooled items that ran off the gathering thread
	unsigned skipped[BONE_SHADOW_SKIP_COUNT];
	char szFirstSkipped[BONE_SHADOW_SKIP_COUNT][96]; // first entity per reason
	char szVerifiedKinds[8][96];                     // distinct verified class/model
	unsigned nVerifiedKinds;
	double flGatherSeconds;
	double flVerifySeconds;
};

BoneShadowTotals_t s_totals;
const unsigned MAX_REPORTED_MISMATCHES = 16;
unsigned s_nReportedMismatches;

} // namespace

// The cohort seen by jobsystem::BatchShadowVerifier. A friend of
// C_BaseAnimating (bone caches and accessors are protected).
class CBoneSetupShadowCohort
{
public:
	typedef BoneShadowSnapshot_t Snapshot;

	std::vector<C_BaseAnimating *> m_items;
	CBoneSetupShadowVerifier::ItemFn m_pfnItem;
	float m_flTime;
	CBoneSetupShadowVerifier::RunnerFn m_pfnRunnerBegin;
	CBoneSetupShadowVerifier::RunnerFn m_pfnRunnerEnd;

	void Capture( unsigned iItem, Snapshot &out )
	{
		C_BaseAnimating *pEntity = m_items[iItem];
		jobsystem::ShadowRecord &record = out.record;
		record.Clear();

		out.nBones = pEntity->m_CachedBoneData.Count();
		record.Append(
		    "bones", pEntity->m_CachedBoneData.Base(), out.nBones * sizeof( matrix3x4_t ) );
		record.AppendValue( "readableBones", pEntity->m_BoneAccessor.GetReadableBones() );
		record.AppendValue( "writableBones", pEntity->m_BoneAccessor.GetWritableBones() );
		record.AppendValue( "prevBoneMask", pEntity->m_iPrevBoneMask );
		record.AppendValue( "accumulatedBoneMask", pEntity->m_iAccumulatedBoneMask );
		record.AppendValue( "lastBoneSetupTime", pEntity->m_flLastBoneSetupTime );
		record.AppendValue( "modelBoneCounter", pEntity->m_iMostRecentModelBoneCounter );
		record.AppendValue( "sequence", pEntity->m_nSequence );
		record.AppendValue( "prevNewSequenceParity", pEntity->m_nPrevNewSequenceParity );
		record.AppendValue( "eflags", pEntity->GetEFlags() );

		out.nAttachments = pEntity->m_Attachments.Count();
		record.Append( "attachments", pEntity->m_Attachments.Base(),
		    out.nAttachments * sizeof( CAttachmentData ) );

		const CUtlVector<C_AnimationLayer> &queue =
		    pEntity->m_SequenceTransitioner.m_animationQueue;
		out.transitions.assign( queue.Base(), queue.Base() + queue.Count() );
		AppendLayers( record, "transitions", queue.Base(), queue.Count(), m_scratch );

		const bool bRagdollActive = pEntity->m_pRagdollInfo && pEntity->m_pRagdollInfo->m_bActive;
		record.AppendValue( "ragdollActive", bRagdollActive );

		out.pIk = pEntity->m_pIk;
		out.pJiggle = pEntity->m_pJiggleBones;
		record.AppendValue( "hasIk", pEntity->m_pIk != NULL );
		record.AppendValue( "hasJiggle", pEntity->m_pJiggleBones != NULL );
		out.pBoneMerge = pEntity->m_pBoneMergeCache;
		record.AppendValue( "hasBoneMerge", pEntity->m_pBoneMergeCache != NULL );
		out.jiggle.clear();
		if ( pEntity->m_pJiggleBones )
		{
			const CUtlLinkedList<JiggleData> &list = pEntity->m_pJiggleBones->m_jiggleBoneState;
			for ( int i = list.Head(); i != list.InvalidIndex(); i = list.Next( i ) )
				out.jiggle.push_back( list[i] );
		}
		record.Append( "jiggle", out.jiggle.empty() ? NULL : &out.jiggle[0],
		    out.jiggle.size() * sizeof( JiggleData ) );

		out.nLayers = 0;
		out.nLayerHistories = 0;
		C_BaseAnimatingOverlay *pOverlay = dynamic_cast<C_BaseAnimatingOverlay *>( pEntity );
		if ( pOverlay )
		{
			out.nLayers = pOverlay->m_AnimOverlay.Count();
			out.layers.assign(
			    pOverlay->m_AnimOverlay.Base(), pOverlay->m_AnimOverlay.Base() + out.nLayers );
			AppendLayers(
			    record, "layers", pOverlay->m_AnimOverlay.Base(), out.nLayers, m_scratch );
			record.Append( "layerEventCycles", pOverlay->m_flOverlayPrevEventCycle,
			    sizeof( pOverlay->m_flOverlayPrevEventCycle ) );
			CaptureLayerHistory( pOverlay, out );
		}

		C_BaseFlex *pFlex = dynamic_cast<C_BaseFlex *>( pEntity );
		if ( pFlex )
			record.Append( "flexWeights", pFlex->m_flexWeight, sizeof( pFlex->m_flexWeight ) );
	}

	void Restore( unsigned iItem, const Snapshot &in )
	{
		C_BaseAnimating *pEntity = m_items[iItem];
		const jobsystem::ShadowRecord &record = in.record;

		// SkipReason guarantees the counts match the snapshot.
		record.Read( "bones", pEntity->m_CachedBoneData.Base(), in.nBones * sizeof( matrix3x4_t ) );
		int nMask = 0;
		record.Read( "readableBones", &nMask, sizeof( nMask ) );
		pEntity->m_BoneAccessor.SetReadableBones( nMask );
		record.Read( "writableBones", &nMask, sizeof( nMask ) );
		pEntity->m_BoneAccessor.SetWritableBones( nMask );
		record.Read(
		    "prevBoneMask", &pEntity->m_iPrevBoneMask, sizeof( pEntity->m_iPrevBoneMask ) );
		record.Read( "accumulatedBoneMask", &pEntity->m_iAccumulatedBoneMask,
		    sizeof( pEntity->m_iAccumulatedBoneMask ) );
		record.Read( "lastBoneSetupTime", &pEntity->m_flLastBoneSetupTime,
		    sizeof( pEntity->m_flLastBoneSetupTime ) );
		record.Read( "modelBoneCounter", &pEntity->m_iMostRecentModelBoneCounter,
		    sizeof( pEntity->m_iMostRecentModelBoneCounter ) );
		record.Read( "sequence", &pEntity->m_nSequence, sizeof( pEntity->m_nSequence ) );
		record.Read( "prevNewSequenceParity", &pEntity->m_nPrevNewSequenceParity,
		    sizeof( pEntity->m_nPrevNewSequenceParity ) );
		record.Read( "attachments", pEntity->m_Attachments.Base(),
		    in.nAttachments * sizeof( CAttachmentData ) );

		CUtlVector<C_AnimationLayer> &queue = pEntity->m_SequenceTransitioner.m_animationQueue;
		queue.SetCount( (int)in.transitions.size() );
		for ( int i = 0; i < queue.Count(); ++i )
			queue[i] = in.transitions[i];

		if ( pEntity->m_pRagdollInfo )
			record.Read( "ragdollActive", &pEntity->m_pRagdollInfo->m_bActive, sizeof( bool ) );

		// Verifiable items keep their IK and jiggle objects through the pooled
		// item (SkipReason). A different object here was allocated by a
		// diverging rerun: release it and put the snapshot's back.
		if ( pEntity->m_pIk != in.pIk )
		{
			delete pEntity->m_pIk;
			pEntity->m_pIk = const_cast<CIKContext *>( in.pIk );
		}
		if ( pEntity->m_pJiggleBones != in.pJiggle )
		{
			delete pEntity->m_pJiggleBones;
			pEntity->m_pJiggleBones = const_cast<CJiggleBones *>( in.pJiggle );
		}
		if ( pEntity->m_pBoneMergeCache != in.pBoneMerge && !in.pBoneMerge )
		{
			delete pEntity->m_pBoneMergeCache;
			pEntity->m_pBoneMergeCache = NULL;
		}
		if ( pEntity->m_pJiggleBones )
		{
			CUtlLinkedList<JiggleData> &list = pEntity->m_pJiggleBones->m_jiggleBoneState;
			list.RemoveAll();
			for ( size_t i = 0; i < in.jiggle.size(); ++i )
				list.AddToTail( in.jiggle[i] );
		}

		C_BaseAnimatingOverlay *pOverlay = dynamic_cast<C_BaseAnimatingOverlay *>( pEntity );
		if ( pOverlay )
		{
			for ( int i = 0; i < in.nLayers; ++i )
				pOverlay->m_AnimOverlay[i] = in.layers[i];
			for ( int i = 0; i < in.nLayerHistories; ++i )
				pOverlay->m_iv_AnimOverlay[i].Copy( in.layerHistory[i].get() );
			record.Read( "layerEventCycles", pOverlay->m_flOverlayPrevEventCycle,
			    sizeof( pOverlay->m_flOverlayPrevEventCycle ) );
		}

		C_BaseFlex *pFlex = dynamic_cast<C_BaseFlex *>( pEntity );
		if ( pFlex )
			record.Read( "flexWeights", pFlex->m_flexWeight, sizeof( pFlex->m_flexWeight ) );
	}

	// Called once per item per comparison; counts the reason.
	int SkipReason( unsigned iItem, const Snapshot &pre )
	{
		const int nReason = ClassifyItem( m_items[iItem], pre );
		char szKind[96];
		DescribeItem( m_items[iItem], szKind, sizeof( szKind ) );
		if ( nReason != BONE_SHADOW_VERIFIABLE )
		{
			if ( !s_totals.skipped[nReason]++ )
				V_strncpy( s_totals.szFirstSkipped[nReason], szKind, sizeof( szKind ) );
			return nReason;
		}
		for ( unsigned i = 0; i < s_totals.nVerifiedKinds; ++i )
		{
			if ( !V_strcmp( s_totals.szVerifiedKinds[i], szKind ) )
				return nReason;
		}
		if ( s_totals.nVerifiedKinds < ARRAYSIZE( s_totals.szVerifiedKinds ) )
			V_strncpy(
			    s_totals.szVerifiedKinds[s_totals.nVerifiedKinds++], szKind, sizeof( szKind ) );
		return nReason;
	}

	void RerunBegin() { m_pfnRunnerBegin(); }
	void Rerun( unsigned iItem ) { m_pfnItem( m_items[iItem], m_flTime ); }
	void RerunEnd() { m_pfnRunnerEnd(); }

	void OnMismatch( unsigned iItem, const Snapshot &pooled, const Snapshot &serial,
	    const jobsystem::ShadowDivergence &divergence )
	{
		if ( s_nReportedMismatches >= MAX_REPORTED_MISMATCHES )
			return;
		++s_nReportedMismatches;
		C_BaseAnimating *pEntity = m_items[iItem];
		const model_t *pModel = pEntity->GetModel();
		char szDetail[160] = "";
		if ( divergence.kind == jobsystem::ShadowDivergence::BYTES && divergence.pField &&
		     !V_strcmp( divergence.pField, "bones" ) )
		{
			const size_t iBone = divergence.nByteOffset / sizeof( matrix3x4_t );
			const size_t iFloat =
			    ( divergence.nByteOffset % sizeof( matrix3x4_t ) ) / sizeof( float );
			const size_t iField = pooled.record.Find( "bones" );
			float flPooled = 0.0f;
			float flSerial = 0.0f;
			V_memcpy( &flPooled,
			    pooled.record.FieldData( iField ) + iBone * sizeof( matrix3x4_t ) +
			        iFloat * sizeof( float ),
			    sizeof( float ) );
			V_memcpy( &flSerial,
			    serial.record.FieldData( iField ) + iBone * sizeof( matrix3x4_t ) +
			        iFloat * sizeof( float ),
			    sizeof( float ) );
			V_snprintf( szDetail, sizeof( szDetail ), " bone %u [%u][%u] pooled %.9g serial %.9g",
			    (unsigned)iBone, (unsigned)( iFloat / 4 ), (unsigned)( iFloat % 4 ), flPooled,
			    flSerial );
		}
		Warning( "bone_setup_verify: MISMATCH frame %d item %u ent %d %s (%s): %s %s byte %u%s\n",
		    gpGlobals->framecount, iItem, pEntity->entindex(), pEntity->GetClassname(),
		    pModel ? modelinfo->GetModelName( pModel ) : "no model",
		    divergence.kind == jobsystem::ShadowDivergence::LAYOUT ? "layout at" : "field",
		    divergence.pField ? divergence.pField : "(end)", (unsigned)divergence.nByteOffset,
		    szDetail );
	}

	void OnRestoreMismatch( unsigned iItem, const jobsystem::ShadowDivergence &divergence )
	{
		Warning( "bone_setup_verify: RESTORE MISMATCH frame %d item %u ent %d: field %s byte %u\n",
		    gpGlobals->framecount, iItem, m_items[iItem]->entindex(),
		    divergence.pField ? divergence.pField : "(end)", (unsigned)divergence.nByteOffset );
	}

private:
	static void DescribeItem( C_BaseAnimating *pEntity, char *pszOut, int nOut )
	{
		const model_t *pModel = pEntity->GetModel();
		V_snprintf( pszOut, nOut, "%s %s", pEntity->GetClassname(),
		    pModel ? modelinfo->GetModelName( pModel ) : "no-model" );
	}

	static int ClassifyItem( C_BaseAnimating *pEntity, const Snapshot &pre )
	{
		if ( pEntity->GetMoveParent() )
			return BONE_SHADOW_SKIP_PARENTED;
		if ( pre.pIk || pEntity->m_pIk )
			return BONE_SHADOW_SKIP_IK;
		if ( pre.pJiggle != pEntity->m_pJiggleBones )
			return BONE_SHADOW_SKIP_JIGGLE_ALLOC;
		if ( pre.pBoneMerge != pEntity->m_pBoneMergeCache )
			return BONE_SHADOW_SKIP_BONE_MERGE_ALLOC;
		if ( pEntity->m_nRenderFX == kRenderFxDistort || pEntity->m_nRenderFX == kRenderFxHologram )
			return BONE_SHADOW_SKIP_RENDERFX;
		if ( pEntity->MouthInfo().NeedsEnvelope() )
			return BONE_SHADOW_SKIP_MOUTH;
		C_BaseAnimatingOverlay *pOverlay = dynamic_cast<C_BaseAnimatingOverlay *>( pEntity );
		const int nLayers = pOverlay ? pOverlay->m_AnimOverlay.Count() : 0;
		const int nHistories = pOverlay ? pOverlay->m_iv_AnimOverlay.Count() : 0;
		if ( pre.nBones != pEntity->m_CachedBoneData.Count() ||
		     pre.nAttachments != pEntity->m_Attachments.Count() || pre.nLayers != nLayers ||
		     pre.nLayerHistories != nHistories )
		{
			return BONE_SHADOW_SKIP_LAYOUT;
		}
		return BONE_SHADOW_VERIFIABLE;
	}

	void CaptureLayerHistory( C_BaseAnimatingOverlay *pOverlay, Snapshot &out )
	{
		// Compared: every history entry's time and packed layer; restored: a
		// private copy of each interpolated variable.
		const int nHistories = pOverlay->m_iv_AnimOverlay.Count();
		out.nLayerHistories = nHistories;
		if ( (int)out.layerHistory.size() < nHistories )
			out.layerHistory.resize( nHistories );
		m_history.clear();
		for ( int iLayer = 0; iLayer < nHistories; ++iLayer )
		{
			LayerHistory_t &history = pOverlay->m_iv_AnimOverlay[iLayer];
			if ( !out.layerHistory[iLayer] )
				out.layerHistory[iLayer].reset( new LayerHistory_t( history.GetDebugName() ) );
			out.layerHistory[iLayer]->Copy( &history );
			int nEntries = 0;
			for ( int i = history.GetHead(); history.IsValidIndex( i ); i = history.GetNext( i ) )
				++nEntries;
			AppendBytes( &nEntries, sizeof( nEntries ) );
			for ( int i = history.GetHead(); history.IsValidIndex( i ); i = history.GetNext( i ) )
			{
				float flChangeTime = 0.0f;
				const C_AnimationLayer *pValue = history.GetHistoryValue( i, flChangeTime );
				AppendBytes( &flChangeTime, sizeof( flChangeTime ) );
				const PackedLayer_t packed = PackLayer( *pValue );
				AppendBytes( &packed, sizeof( packed ) );
			}
		}
		out.record.Append(
		    "layerHistory", m_history.empty() ? NULL : &m_history[0], m_history.size() );
	}

	void AppendBytes( const void *pData, size_t nBytes )
	{
		const unsigned char *pBytes = static_cast<const unsigned char *>( pData );
		m_history.insert( m_history.end(), pBytes, pBytes + nBytes );
	}

	std::vector<PackedLayer_t> m_scratch;
	std::vector<unsigned char> m_history;
};

namespace
{
CBoneSetupShadowCohort s_cohort;
jobsystem::BatchShadowVerifier<CBoneSetupShadowCohort> s_verifier;
bool s_bVerifyingBatch;
int s_nFaultMode;
C_BaseAnimating *s_pFaultTarget;
ThreadId_t s_hostThread;
// The thread each pooled item ran on (0: not run). Each slot is written by
// the one runner that runs its item and read after the batch's join.
std::vector<ThreadId_t> s_itemThreads;
} // namespace

bool CBoneSetupShadowVerifier::BeginBatch( C_BaseAnimating *const *ppItems, int nItems )
{
	s_bVerifyingBatch = cl_bone_setup_verify.GetBool() && nItems > 0;
	s_bObserveItems = s_bVerifyingBatch;
	s_pFaultTarget = NULL;
	if ( !s_bVerifyingBatch )
		return false;

	const double flStart = Plat_FloatTime();
	s_cohort.m_items.assign( ppItems, ppItems + nItems );
	s_verifier.Gather( s_cohort, (unsigned)nItems );
	s_hostThread = ThreadGetCurrentId();
	s_itemThreads.assign( nItems, 0 );

	s_nFaultMode = cl_bone_setup_verify_fault.GetInt();
	if ( s_nFaultMode )
	{
		for ( int i = 0; i < nItems; ++i )
		{
			if ( !ppItems[i]->GetMoveParent() && ppItems[i]->m_CachedBoneData.Count() > 0 )
			{
				s_pFaultTarget = ppItems[i];
				break;
			}
		}
	}
	s_totals.flGatherSeconds += Plat_FloatTime() - flStart;
	return true;
}

void CBoneSetupShadowVerifier::RunObservedItem(
    C_BaseAnimating *pEntity, float flTime, ItemFn pfnItem )
{
	const std::vector<C_BaseAnimating *> &items = s_cohort.m_items;
	for ( size_t i = 0; i < items.size(); ++i )
	{
		if ( items[i] == pEntity )
		{
			s_itemThreads[i] = ThreadGetCurrentId();
			break;
		}
	}
	if ( pEntity != s_pFaultTarget )
	{
		pfnItem( pEntity, flTime );
		return;
	}
	// Seeded fault (negative control): drop the item, or nudge its first bone.
	if ( s_nFaultMode == 2 )
		return;
	pfnItem( pEntity, flTime );
	if ( pEntity->m_CachedBoneData.Count() > 0 )
	{
		float &flValue = pEntity->m_CachedBoneData[0][0][3];
		flValue = nextafterf( flValue, FLT_MAX );
	}
}

void CBoneSetupShadowVerifier::EndBatch(
    ItemFn pfnItem, float flTime, RunnerFn pfnRunnerBegin, RunnerFn pfnRunnerEnd )
{
	if ( !s_bVerifyingBatch )
		return;
	s_bVerifyingBatch = false;
	s_bObserveItems = false;

	const double flStart = Plat_FloatTime();
	ThreadId_t firstThread = 0;
	bool bMultiThread = false;
	for ( size_t i = 0; i < s_itemThreads.size(); ++i )
	{
		const ThreadId_t thread = s_itemThreads[i];
		if ( !thread )
			continue;
		if ( thread != s_hostThread )
			++s_totals.offHostItems;
		if ( !firstThread )
			firstThread = thread;
		else if ( thread != firstThread )
			bMultiThread = true;
	}
	if ( bMultiThread )
		++s_totals.multiThreadBatches;

	s_cohort.m_pfnItem = pfnItem;
	s_cohort.m_flTime = flTime;
	s_cohort.m_pfnRunnerBegin = pfnRunnerBegin;
	s_cohort.m_pfnRunnerEnd = pfnRunnerEnd;
	const jobsystem::ShadowBatchCounts counts = s_verifier.CompareWithSerialRerun( s_cohort );

	++s_totals.batches;
	s_totals.items += counts.items;
	s_totals.verified += counts.verified;
	s_totals.mismatches += counts.mismatches;
	s_totals.restoreMismatches += counts.restoreMismatches;
	if ( s_pFaultTarget )
		++s_totals.faultBatches;
	s_pFaultTarget = NULL;
	s_totals.flVerifySeconds += Plat_FloatTime() - flStart;
}

void CBoneSetupShadowVerifier::Report()
{
	unsigned nSkipped = 0;
	for ( int i = 1; i < BONE_SHADOW_SKIP_COUNT; ++i )
		nSkipped += s_totals.skipped[i];
	Msg( "bone_setup_verify: batches %u items %u verified %u skipped %u mismatches %u "
	     "restore_mismatches %u fault_batches %u gather_ms %.3f verify_ms %.3f\n",
	    s_totals.batches, s_totals.items, s_totals.verified, nSkipped, s_totals.mismatches,
	    s_totals.restoreMismatches, s_totals.faultBatches, s_totals.flGatherSeconds * 1000.0,
	    s_totals.flVerifySeconds * 1000.0 );
	Msg( "bone_setup_verify: multi_thread_batches %u off_host_items %u\n",
	    s_totals.multiThreadBatches, s_totals.offHostItems );
	Msg( "bone_setup_verify: skipped" );
	for ( int i = 1; i < BONE_SHADOW_SKIP_COUNT; ++i )
		Msg( " %s %u", s_pSkipNames[i], s_totals.skipped[i] );
	Msg( "\n" );
	for ( unsigned i = 0; i < s_totals.nVerifiedKinds; ++i )
		Msg( "bone_setup_verify: verified kind: %s\n", s_totals.szVerifiedKinds[i] );
	for ( int i = 1; i < BONE_SHADOW_SKIP_COUNT; ++i )
	{
		if ( s_totals.skipped[i] )
			Msg( "bone_setup_verify: first %s: %s\n", s_pSkipNames[i], s_totals.szFirstSkipped[i] );
	}
}

void CBoneSetupShadowVerifier::ReportIfUsed()
{
	if ( s_totals.batches )
		Report();
}

CON_COMMAND( cl_bone_setup_verify_report, "Print the bone-setup shadow verifier's counts." )
{
	CBoneSetupShadowVerifier::Report();
}
