//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Live shadow verification of a pooled batch against a serial rerun
//          (RFC 0003 R10/R21: pooled outputs must equal serial outputs).
//
//          A cohort's items run as a pooled batch between a host gather and a
//          host commit. With verification on, the gather captures every
//          item's state (the pre-state); after the batch joins, the commit
//
//            1. asks the cohort which items it can verify (pre and pooled
//               state known; nothing the rerun would change is outside the
//               cohort's snapshot);
//            2. captures each verifiable item's pooled state;
//            3. restores every verifiable item to its pre-state;
//            4. reruns them serially, in item order, on the host thread;
//            5. captures the serial state and byte-compares it with the
//               pooled state;
//            6. restores the pooled state and captures once more: the frame
//               continues with exactly what the pooled batch produced, and a
//               restore that is not byte-exact is itself reported.
//
//          Restoring every item before rerunning any keeps serial semantics
//          when one item reads another's state (the serial rerun sees what a
//          legacy serial loop would have seen). Items the cohort cannot verify
//          are neither restored nor rerun, and are counted by reason.
//
//          The comparison is exact (bytes), because pooled and serial run the
//          same code on the same inputs. Nothing here starts threads, and
//          everything runs on the thread that calls Gather and CompareWithSerialRerun.
//
//          Header-only and dialect-neutral (C++11 and later): the client and
//          the standalone fixture (unittests/jobsystemtest/shadowverifytest.cpp)
//          compile the same code.
//
//=============================================================================//
#ifndef JOBSYSTEM_BATCH_SHADOW_VERIFY_H
#define JOBSYSTEM_BATCH_SHADOW_VERIFY_H

#include <stddef.h>
#include <string.h>

#include <vector>

namespace jobsystem
{

// One item's state as an ordered list of named byte fields. Field names must
// outlive the record (string literals).
class ShadowRecord
{
public:
	void Clear()
	{
		m_bytes.clear();
		m_fields.clear();
	}

	void Append( const char *pName, const void *pData, size_t nBytes )
	{
		Field field;
		field.pName = pName;
		field.nOffset = m_bytes.size();
		field.nBytes = nBytes;
		m_fields.push_back( field );
		if ( nBytes )
		{
			const unsigned char *pBytes = static_cast<const unsigned char *>( pData );
			m_bytes.insert( m_bytes.end(), pBytes, pBytes + nBytes );
		}
	}

	// For trivially copyable values only.
	template <typename T> void AppendValue( const char *pName, const T &value )
	{
		Append( pName, &value, sizeof( T ) );
	}

	size_t FieldCount() const { return m_fields.size(); }
	const char *FieldName( size_t iField ) const { return m_fields[iField].pName; }
	size_t FieldSize( size_t iField ) const { return m_fields[iField].nBytes; }
	const unsigned char *FieldData( size_t iField ) const
	{
		return m_fields[iField].nBytes ? &m_bytes[m_fields[iField].nOffset] : NULL;
	}

	// Index of the named field, or FieldCount() when absent.
	size_t Find( const char *pName ) const
	{
		for ( size_t i = 0; i < m_fields.size(); ++i )
		{
			if ( strcmp( m_fields[i].pName, pName ) == 0 )
				return i;
		}
		return m_fields.size();
	}

	// Copies the named field into pDest; false (and no write) when the field
	// is absent or its size differs.
	bool Read( const char *pName, void *pDest, size_t nBytes ) const
	{
		const size_t iField = Find( pName );
		if ( iField == m_fields.size() || m_fields[iField].nBytes != nBytes )
			return false;
		if ( nBytes )
			memcpy( pDest, FieldData( iField ), nBytes );
		return true;
	}

private:
	struct Field
	{
		const char *pName;
		size_t nOffset;
		size_t nBytes;
	};
	std::vector<unsigned char> m_bytes;
	std::vector<Field> m_fields;
};

struct ShadowDivergence
{
	enum Kind
	{
		SAME,
		LAYOUT, // field count, name or size differs
		BYTES   // same layout, a byte differs
	};
	Kind kind;
	size_t iField;      // first differing field (LAYOUT: first position that differs)
	const char *pField; // its name in the reference record, or NULL past its end
	size_t nByteOffset; // BYTES: first differing byte within the field
};

// First difference between two records, in field order.
inline ShadowDivergence FirstShadowDivergence(
    const ShadowRecord &reference, const ShadowRecord &candidate )
{
	ShadowDivergence result;
	result.kind = ShadowDivergence::SAME;
	result.iField = 0;
	result.pField = NULL;
	result.nByteOffset = 0;
	const size_t nReference = reference.FieldCount();
	const size_t nCandidate = candidate.FieldCount();
	const size_t nCommon = nReference < nCandidate ? nReference : nCandidate;
	for ( size_t i = 0; i < nCommon; ++i )
	{
		if ( strcmp( reference.FieldName( i ), candidate.FieldName( i ) ) != 0 ||
		     reference.FieldSize( i ) != candidate.FieldSize( i ) )
		{
			result.kind = ShadowDivergence::LAYOUT;
			result.iField = i;
			result.pField = reference.FieldName( i );
			return result;
		}
		const unsigned char *pA = reference.FieldData( i );
		const unsigned char *pB = candidate.FieldData( i );
		for ( size_t b = 0; b < reference.FieldSize( i ); ++b )
		{
			if ( pA[b] != pB[b] )
			{
				result.kind = ShadowDivergence::BYTES;
				result.iField = i;
				result.pField = reference.FieldName( i );
				result.nByteOffset = b;
				return result;
			}
		}
	}
	if ( nReference != nCandidate )
	{
		result.kind = ShadowDivergence::LAYOUT;
		result.iField = nCommon;
		result.pField = nCommon < nReference ? reference.FieldName( nCommon ) : NULL;
	}
	return result;
}

// Counts for one CompareWithSerialRerun call; the caller accumulates them.
struct ShadowBatchCounts
{
	unsigned items;             // items in the batch
	unsigned verified;          // rerun and compared
	unsigned skipped;           // not verifiable (see the cohort's reasons)
	unsigned mismatches;        // serial state differs from pooled state
	unsigned restoreMismatches; // pooled state not restored byte-exactly
};

// Cohort requirements (all calls on the verifying thread, between the batch's
// join and its commit):
//
//   typedef ... Snapshot;  // has a member `ShadowRecord record` for comparison,
//                          // plus any private side state the cohort needs to
//                          // restore (e.g. private copies of history buffers)
//   void Capture( unsigned iItem, Snapshot &out );     // reads item state only
//   void Restore( unsigned iItem, const Snapshot &in ); // item state := snapshot
//   int  SkipReason( unsigned iItem, const Snapshot &pre ); // 0 = verifiable
//   void RerunBegin();                     // brackets the serial rerun
//   void Rerun( unsigned iItem );          // the batch's item work, serially
//   void RerunEnd();
//   void OnMismatch( unsigned iItem, const Snapshot &pooled, const Snapshot &serial,
//                    const ShadowDivergence &divergence );
//   void OnRestoreMismatch( unsigned iItem, const ShadowDivergence &divergence );
//
// Capture of the same state must give identical bytes (no padding or
// pointer identities in the record).
template <class Cohort> class BatchShadowVerifier
{
public:
	typedef typename Cohort::Snapshot Snapshot;

	// Gather: before the batch runs. Captures every item's pre-state.
	void Gather( Cohort &cohort, unsigned nItems )
	{
		m_nItems = nItems;
		Resize( m_pre, nItems );
		for ( unsigned i = 0; i < nItems; ++i )
			cohort.Capture( i, m_pre[i] );
	}

	unsigned GatheredItems() const { return m_nItems; }

	// Commit: after the batch joined. Leaves every item in its pooled state.
	ShadowBatchCounts CompareWithSerialRerun( Cohort &cohort )
	{
		ShadowBatchCounts counts;
		counts.items = m_nItems;
		counts.verified = 0;
		counts.skipped = 0;
		counts.mismatches = 0;
		counts.restoreMismatches = 0;

		m_verifiable.assign( m_nItems, 0 );
		Resize( m_pooled, m_nItems );
		Resize( m_serial, m_nItems );
		for ( unsigned i = 0; i < m_nItems; ++i )
		{
			if ( cohort.SkipReason( i, m_pre[i] ) == 0 )
				m_verifiable[i] = 1;
			else
				++counts.skipped;
		}
		for ( unsigned i = 0; i < m_nItems; ++i )
		{
			if ( m_verifiable[i] )
				cohort.Capture( i, m_pooled[i] );
		}
		for ( unsigned i = 0; i < m_nItems; ++i )
		{
			if ( m_verifiable[i] )
				cohort.Restore( i, m_pre[i] );
		}
		cohort.RerunBegin();
		for ( unsigned i = 0; i < m_nItems; ++i )
		{
			if ( m_verifiable[i] )
				cohort.Rerun( i );
		}
		cohort.RerunEnd();
		for ( unsigned i = 0; i < m_nItems; ++i )
		{
			if ( !m_verifiable[i] )
				continue;
			++counts.verified;
			cohort.Capture( i, m_serial[i] );
			const ShadowDivergence divergence =
			    FirstShadowDivergence( m_pooled[i].record, m_serial[i].record );
			if ( divergence.kind != ShadowDivergence::SAME )
			{
				++counts.mismatches;
				cohort.OnMismatch( i, m_pooled[i], m_serial[i], divergence );
			}
		}
		for ( unsigned i = 0; i < m_nItems; ++i )
		{
			if ( !m_verifiable[i] )
				continue;
			cohort.Restore( i, m_pooled[i] );
			// m_serial[i] is no longer needed: reuse it for the check.
			cohort.Capture( i, m_serial[i] );
			const ShadowDivergence divergence =
			    FirstShadowDivergence( m_pooled[i].record, m_serial[i].record );
			if ( divergence.kind != ShadowDivergence::SAME )
			{
				++counts.restoreMismatches;
				cohort.OnRestoreMismatch( i, divergence );
			}
		}
		m_nItems = 0;
		return counts;
	}

private:
	static void Resize( std::vector<Snapshot> &snapshots, unsigned nItems )
	{
		if ( snapshots.size() < nItems )
			snapshots.resize( nItems );
	}

	unsigned m_nItems = 0;
	std::vector<Snapshot> m_pre;
	std::vector<Snapshot> m_pooled;
	std::vector<Snapshot> m_serial;
	std::vector<unsigned char> m_verifiable;
};

} // namespace jobsystem

#endif // JOBSYSTEM_BATCH_SHADOW_VERIFY_H
