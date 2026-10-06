//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared suite for engine.splitscreen-wire.v1. It runs against a codec
//			table so the real codecs and deliberately bad ones use the same checks.
//
//=============================================================================//

#ifndef SPLITSCREEN_WIRE_CONFORMANCE_H
#define SPLITSCREEN_WIRE_CONFORMANCE_H

#include "common/splitscreen_wire.h"

#include <cstring>

namespace enginetest
{
using namespace splitscreenwire;

struct WireCodec
{
	bool ( *writeUser )( bf_write &, const NetSplitScreenUser & );
	bool ( *readUser )( bf_read &, NetSplitScreenUser & );
	bool ( *writeSvc )( bf_write &, const SvcSplitScreen & );
	bool ( *readSvc )( bf_read &, SvcSplitScreen & );
	bool ( *writeConnect )( bf_write &, const ClcSplitPlayerConnect & );
	bool ( *readConnect )( bf_read &, ClcSplitPlayerConnect & );
};

inline WireCodec RealCodec()
{
	WireCodec c = { WriteNetSplitScreenUser, ReadNetSplitScreenUser, WriteSvcSplitScreen,
		ReadSvcSplitScreen, WriteClcSplitPlayerConnect, ReadClcSplitPlayerConnect };
	return c;
}

struct WireReport
{
	int checks;
	int failures;
	const char *firstFailure;
	int firstFailureLine;
};

#define WIRE_CHECK( report, cond )                                                                 \
	do                                                                                             \
	{                                                                                              \
		++( report ).checks;                                                                       \
		if ( !( cond ) )                                                                           \
		{                                                                                          \
			if ( ( report ).failures++ == 0 )                                                      \
			{                                                                                      \
				( report ).firstFailure = #cond;                                                   \
				( report ).firstFailureLine = __LINE__;                                            \
			}                                                                                      \
		}                                                                                          \
	} while ( 0 )

inline WireReport RunSplitScreenWireConformance( const WireCodec &c )
{
	WireReport r = { 0, 0, "", 0 };
	unsigned char buf[2048];

	// net_SplitScreenUser: both slots round-trip in exactly one bit; slot 2 is refused.
	for ( int slot = 0; slot < kMaxLocalPlayers; ++slot )
	{
		bf_write w( buf, sizeof( buf ) );
		NetSplitScreenUser in = { slot }, out = { -1 };
		WIRE_CHECK( r, c.writeUser( w, in ) );
		WIRE_CHECK( r, w.GetNumBitsWritten() == kSlotBits );
		bf_read rd( buf, sizeof( buf ), w.GetNumBitsWritten() );
		WIRE_CHECK( r, c.readUser( rd, out ) && out.slot == slot );
	}
	{
		bf_write w( buf, sizeof( buf ) );
		NetSplitScreenUser bad = { kMaxLocalPlayers }, neg = { -1 };
		WIRE_CHECK( r, !c.writeUser( w, bad ) && !c.writeUser( w, neg ) );
		WIRE_CHECK( r, w.GetNumBitsWritten() == 0 );
		bf_read empty( buf, 0, 0 );
		NetSplitScreenUser out = { 7 };
		WIRE_CHECK( r, !c.readUser( empty, out ) && out.slot == 7 );
	}

	// svc_SplitScreen: 1 + 1 + 11 bits, add and remove, edge entity indices.
	const int entities[] = { 1, 2, 63, 1024, ( 1 << kEntityIndexBits ) - 1 };
	for ( int action = 0; action < 2; ++action )
		for ( int slot = 0; slot < kMaxLocalPlayers; ++slot )
			for ( size_t e = 0; e < sizeof( entities ) / sizeof( entities[0] ); ++e )
			{
				bf_write w( buf, sizeof( buf ) );
				SvcSplitScreen in = { action, slot, entities[e] }, out = { -1, -1, -1 };
				WIRE_CHECK( r, c.writeSvc( w, in ) );
				WIRE_CHECK( r, w.GetNumBitsWritten() == 1 + kSlotBits + kEntityIndexBits );
				bf_read rd( buf, sizeof( buf ), w.GetNumBitsWritten() );
				WIRE_CHECK( r, c.readSvc( rd, out ) && out.action == action && out.slot == slot
				                   && out.entityIndex == entities[e] );
			}
	{
		bf_write w( buf, sizeof( buf ) );
		SvcSplitScreen badAction = { 2, 0, 5 }, badSlot = { 0, 2, 5 }, zero = { 0, 0, 0 },
		               huge = { 0, 0, 1 << kEntityIndexBits };
		WIRE_CHECK( r, !c.writeSvc( w, badAction ) && !c.writeSvc( w, badSlot ) );
		WIRE_CHECK( r, !c.writeSvc( w, zero ) && !c.writeSvc( w, huge ) );
		WIRE_CHECK( r, w.GetNumBitsWritten() == 0 );
		// Truncated: only 5 of 13 bits present.
		unsigned char two[2] = { 0xff, 0xff };
		bf_read cut( two, sizeof( two ), 5 );
		SvcSplitScreen out = { 9, 9, 9 };
		WIRE_CHECK( r, !c.readSvc( cut, out ) && out.entityIndex == 9 );
		// Entity index 0 on the wire is refused.
		bf_write z( buf, sizeof( buf ) );
		z.WriteUBitLong( 0, 1 );
		z.WriteUBitLong( 1, kSlotBits );
		z.WriteUBitLong( 0, kEntityIndexBits );
		bf_read zr( buf, sizeof( buf ), z.GetNumBitsWritten() );
		WIRE_CHECK( r, !c.readSvc( zr, out ) );
	}

	// clc_SplitPlayerConnect: one and two players, strings preserved, bounds enforced.
	for ( int count = 1; count <= kMaxConnectRecords; ++count )
	{
		ClcSplitPlayerConnect in;
		std::memset( &in, 0, sizeof( in ) );
		in.count = count;
		for ( int i = 0; i < count; ++i )
		{
			std::strcpy( in.records[i].first, i == 0 ? "player one" : "player two" );
			std::strcpy( in.records[i].second, "ignored" );
		}
		bf_write w( buf, sizeof( buf ) );
		WIRE_CHECK( r, c.writeConnect( w, in ) );
		bf_read rd( buf, sizeof( buf ), w.GetNumBitsWritten() );
		ClcSplitPlayerConnect out;
		std::memset( &out, 0, sizeof( out ) );
		bool ok = c.readConnect( rd, out );
		WIRE_CHECK( r, ok && out.count == count );
		for ( int i = 0; ok && i < count; ++i )
			WIRE_CHECK( r, std::strcmp( out.records[i].first, in.records[i].first ) == 0
			                   && std::strcmp( out.records[i].second, "ignored" ) == 0 );
	}
	{
		ClcSplitPlayerConnect in;
		std::memset( &in, 0, sizeof( in ) );
		bf_write w( buf, sizeof( buf ) );
		in.count = 0;
		WIRE_CHECK( r, !c.writeConnect( w, in ) );
		in.count = kMaxConnectRecords + 1;
		WIRE_CHECK( r, !c.writeConnect( w, in ) );
		in.count = 1;
		std::memset( in.records[0].first, 'x', kStringBytes ); // no terminator
		WIRE_CHECK( r, !c.writeConnect( w, in ) );
		WIRE_CHECK( r, w.GetNumBitsWritten() == 0 );
		// Wire count of 3 (and 0) is refused, as is a truncated record.
		for ( int bad = 0; bad <= 3; bad += 3 )
		{
			bf_write bw( buf, sizeof( buf ) );
			bw.WriteUBitLong( bad, kConnectCountBits );
			bw.WriteString( "a" );
			bw.WriteString( "b" );
			bf_read br( buf, sizeof( buf ), bw.GetNumBitsWritten() );
			ClcSplitPlayerConnect out;
			out.count = 99;
			WIRE_CHECK( r, !c.readConnect( br, out ) && out.count == 99 );
		}
		bf_write tw( buf, sizeof( buf ) );
		tw.WriteUBitLong( 2, kConnectCountBits );
		tw.WriteString( "only one" );
		tw.WriteString( "pair" );
		bf_read tr( buf, sizeof( buf ), tw.GetNumBitsWritten() );
		ClcSplitPlayerConnect out;
		out.count = 99;
		WIRE_CHECK( r, !c.readConnect( tr, out ) && out.count == 99 );
	}

	// Message ids are distinct and fit the type field.
	WIRE_CHECK( r, kClcSplitPlayerConnect != kSvcSplitScreen && kSvcSplitScreen != kNetSplitScreenUser
	                   && kClcSplitPlayerConnect != kNetSplitScreenUser );
	WIRE_CHECK( r, kSvcSplitScreen < 64 && kNetSplitScreenUser < 64 && kClcSplitPlayerConnect < 64 );
	return r;
}
} // namespace enginetest

#endif // SPLITSCREEN_WIRE_CONFORMANCE_H
