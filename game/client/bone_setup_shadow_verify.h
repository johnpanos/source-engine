//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Live shadow verifier for the previous-frame bone-setup cohort
//          (C_BaseAnimating::ThreadedBoneSetup*, RFC 0003 R10/R21).
//
//          Off unless cl_bone_setup_verify is 1. When on, the cohort's gather
//          captures each item's bone-setup state, and its commit reruns the
//          items serially from that state and byte-compares the result with
//          what the pooled batch produced, then puts the pooled state back
//          (public/jobsystem/batch_shadow_verify.h owns that protocol). The
//          frame uses the pooled outputs either way.
//
//          All calls except ObservesItems/RunObservedItem are on the thread
//          that runs the cohort's gather and commit. No thread is started.
//
//=============================================================================//
#ifndef BONE_SETUP_SHADOW_VERIFY_H
#define BONE_SETUP_SHADOW_VERIFY_H

class C_BaseAnimating;

class CBoneSetupShadowVerifier
{
public:
	typedef void ( *ItemFn )( C_BaseAnimating *pEntity, float flTime );
	typedef void ( *RunnerFn )();

	// Gather, before the batch is dispatched. Decides (from the ConVar) whether
	// this batch is verified and, if so, captures every item's pre-state and
	// picks the seeded fault's target. Returns whether the batch is verified.
	static bool BeginBatch( C_BaseAnimating *const *ppItems, int nItems );

	// Whether the current batch's items run through RunObservedItem. Written
	// by BeginBatch before dispatch and by EndBatch after the join; false
	// whenever the verifier is off.
	static bool ObservesItems() { return s_bObserveItems; }

	// Runs one pooled item (any thread): notes the thread it ran on and
	// applies the seeded fault (cl_bone_setup_verify_fault) to its target.
	static void RunObservedItem( C_BaseAnimating *pEntity, float flTime, ItemFn pfnItem );

	// Commit, after the batch joined and before the item list is released.
	// pfnItem is the batch's item work, rerun serially between runner
	// brackets. Leaves every item as the pooled batch left it.
	static void EndBatch(
	    ItemFn pfnItem, float flTime, RunnerFn pfnRunnerBegin, RunnerFn pfnRunnerEnd );

	// Prints the cumulative counts (cl_bone_setup_verify_report).
	static void Report();

	// Prints the counts at client shutdown if any batch was verified.
	static void ReportIfUsed();

private:
	static bool s_bObserveItems;
};

#endif // BONE_SETUP_SHADOW_VERIFY_H
