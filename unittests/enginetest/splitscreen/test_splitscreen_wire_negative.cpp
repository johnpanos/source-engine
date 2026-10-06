//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for engine.splitscreen-wire.v1. Each deliberately
//			bad codec must be caught by the shared suite; a defect that passes
//			is a failure of the suite.
//
//=============================================================================//

#include "splitscreen_wire_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

using namespace splitscreenwire;

namespace
{
// Defect: slot written in 2 bits.
bool WideSlotWrite( bf_write &out, const NetSplitScreenUser &m )
{
	if ( m.slot < 0 || m.slot >= kMaxLocalPlayers )
		return false;
	out.WriteUBitLong( m.slot, 2 );
	return true;
}
// Defect: no range check on the write side.
bool UncheckedSvcWrite( bf_write &out, const SvcSplitScreen &m )
{
	out.WriteUBitLong( m.action, 1 );
	out.WriteUBitLong( m.slot, kSlotBits );
	out.WriteUBitLong( m.entityIndex, kEntityIndexBits );
	return true;
}
// Defect: the reader accepts an entity index of zero.
bool ZeroAcceptingSvcRead( bf_read &in, SvcSplitScreen &m )
{
	m.action = (int)in.ReadUBitLong( 1 );
	m.slot = (int)in.ReadUBitLong( kSlotBits );
	m.entityIndex = (int)in.ReadUBitLong( kEntityIndexBits );
	return !in.IsOverflowed();
}
// Defect: the connect reader trusts the wire count and writes a partial result.
bool PartialConnectRead( bf_read &in, ClcSplitPlayerConnect &m )
{
	m.count = (int)in.ReadUBitLong( kConnectCountBits );
	for ( int i = 0; i < m.count && i < kMaxConnectRecords; ++i )
	{
		in.ReadString( m.records[i].first, kStringBytes );
		in.ReadString( m.records[i].second, kStringBytes );
	}
	return true;
}
} // namespace

int main()
{
	struct Case
	{
		const char *name;
		enginetest::WireCodec codec;
	} cases[4];
	for ( int i = 0; i < 4; ++i )
		cases[i].codec = enginetest::RealCodec();
	cases[0].name = "wide-slot";
	cases[0].codec.writeUser = WideSlotWrite;
	cases[1].name = "unchecked-svc-write";
	cases[1].codec.writeSvc = UncheckedSvcWrite;
	cases[2].name = "zero-entity-accepted";
	cases[2].codec.readSvc = ZeroAcceptingSvcRead;
	cases[3].name = "partial-connect-read";
	cases[3].codec.readConnect = PartialConnectRead;

	int checks = 0, failures = 0;
	for ( int i = 0; i < 4; ++i )
	{
		const enginetest::WireReport r = enginetest::RunSplitScreenWireConformance( cases[i].codec );
		++checks;
		if ( r.failures == 0 )
		{
			++failures;
			std::printf( "FAIL: defect '%s' was not detected\n", cases[i].name );
		}
		else
		{
			std::printf( "ok: defect '%s' detected (%d failing checks)\n", cases[i].name, r.failures );
		}
	}
	// The unmodified codec must still pass, so detection means something.
	++checks;
	if ( enginetest::RunSplitScreenWireConformance( enginetest::RealCodec() ).failures != 0 )
		++failures;
	return testing::ReportConformance( checks, failures );
}
