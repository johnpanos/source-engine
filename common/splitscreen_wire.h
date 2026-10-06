//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Wire payloads of the three local split-screen net messages
//			(engine.splitscreen-wire.v1). Retail Portal 2's bit layouts
//			(RFC/portal2-splitscreen-retail-engine.md), carried under ids that
//			this fork allocates for itself: retail's type numbers collide with
//			this protocol's, and one-player peers must keep working, so a peer
//			uses these only after both ends advertise the extension.
//
//			Pure bit-level codecs: no engine state, no handlers. Every Read
//			validates and returns false without consuming a partial result;
//			every Write refuses out-of-range input before writing a bit.
//
//=============================================================================//

#ifndef SPLITSCREEN_WIRE_H
#define SPLITSCREEN_WIRE_H

#include "tier1/bitbuf.h"

namespace splitscreenwire
{
// Extension message ids. All fit NETMSG_TYPE_BITS (6) and are free in this
// protocol (SVC_LASTMSG 33, CLC_LASTMSG 17, net_* 0..6).
enum MessageType
{
	kClcSplitPlayerConnect = 18, // client -> server, reliable
	kSvcSplitScreen = 34,        // server -> client
	kNetSplitScreenUser = 35     // either direction: selects the active slot
};

enum Limits
{
	kMaxLocalPlayers = 2, // slots 0..1 (retail's maximum)
	kSlotBits = 1,
	kEntityIndexBits = 11, // MAX_EDICT_BITS
	kConnectCountBits = 8,
	kStringBytes = 0x104, // retail record string size (name / userinfo)
	kMaxConnectRecords = kMaxLocalPlayers
};

enum SvcAction
{
	kActionAdd = 0,
	kActionRemove = 1
};

struct NetSplitScreenUser
{
	int slot;
};

struct SvcSplitScreen
{
	int action; // SvcAction
	int slot;
	int entityIndex; // 1-based client entity index
};

// One connecting player: the retail record is two strings; the first is the
// player's name/userinfo, the second is carried and ignored by the receiver.
struct ConnectRecord
{
	char first[kStringBytes];
	char second[kStringBytes];
};

struct ClcSplitPlayerConnect
{
	int count;
	ConnectRecord records[kMaxConnectRecords];
};

inline bool WriteNetSplitScreenUser( bf_write &out, const NetSplitScreenUser &m )
{
	if ( m.slot < 0 || m.slot >= kMaxLocalPlayers )
		return false;
	out.WriteUBitLong( m.slot, kSlotBits );
	return !out.IsOverflowed();
}

inline bool ReadNetSplitScreenUser( bf_read &in, NetSplitScreenUser &m )
{
	const int slot = (int)in.ReadUBitLong( kSlotBits );
	if ( in.IsOverflowed() || slot >= kMaxLocalPlayers )
		return false;
	m.slot = slot;
	return true;
}

inline bool WriteSvcSplitScreen( bf_write &out, const SvcSplitScreen &m )
{
	if ( ( m.action != kActionAdd && m.action != kActionRemove ) || m.slot < 0 ||
	     m.slot >= kMaxLocalPlayers || m.entityIndex < 1 ||
	     m.entityIndex >= ( 1 << kEntityIndexBits ) )
		return false;
	out.WriteUBitLong( m.action, 1 );
	out.WriteUBitLong( m.slot, kSlotBits );
	out.WriteUBitLong( m.entityIndex, kEntityIndexBits );
	return !out.IsOverflowed();
}

inline bool ReadSvcSplitScreen( bf_read &in, SvcSplitScreen &m )
{
	const int action = (int)in.ReadUBitLong( 1 );
	const int slot = (int)in.ReadUBitLong( kSlotBits );
	const int entity = (int)in.ReadUBitLong( kEntityIndexBits );
	if ( in.IsOverflowed() || slot >= kMaxLocalPlayers || entity < 1 )
		return false;
	m.action = action;
	m.slot = slot;
	m.entityIndex = entity;
	return true;
}

inline bool WriteClcSplitPlayerConnect( bf_write &out, const ClcSplitPlayerConnect &m )
{
	if ( m.count < 1 || m.count > kMaxConnectRecords )
		return false;
	for ( int i = 0; i < m.count; ++i )
	{
		if ( !memchr( m.records[i].first, 0, kStringBytes ) ||
		     !memchr( m.records[i].second, 0, kStringBytes ) )
			return false;
	}
	out.WriteUBitLong( m.count, kConnectCountBits );
	for ( int i = 0; i < m.count; ++i )
	{
		out.WriteString( m.records[i].first );
		out.WriteString( m.records[i].second );
	}
	return !out.IsOverflowed();
}

inline bool ReadClcSplitPlayerConnect( bf_read &in, ClcSplitPlayerConnect &m )
{
	const int count = (int)in.ReadUBitLong( kConnectCountBits );
	if ( in.IsOverflowed() || count < 1 || count > kMaxConnectRecords )
		return false;
	ClcSplitPlayerConnect parsed;
	parsed.count = count;
	for ( int i = 0; i < count; ++i )
	{
		bool ok = in.ReadString( parsed.records[i].first, kStringBytes );
		ok = in.ReadString( parsed.records[i].second, kStringBytes ) && ok;
		if ( !ok || in.IsOverflowed() )
			return false;
	}
	m = parsed;
	return true;
}
} // namespace splitscreenwire

#endif // SPLITSCREEN_WIRE_H
