//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: An in-memory IMapCodec for suites that need open/save without a
//			file format (RFC 0002 test fakes implement the same ports as the
//			production providers). Encode stores a copy of the document and
//			returns a token; Decode returns that copy re-stamped with the
//			requested serial. Decoding unknown text fails like a parse error,
//			and 'failEncode' injects an encode failure.
//
//=============================================================================//

#ifndef HAMMERTEST_FAKE_MAP_CODEC_H
#define HAMMERTEST_FAKE_MAP_CODEC_H

#include "hammer/ports/map_codec.h"

#include <map>
#include <string>

namespace hammertest
{

class FakeMapCodec final : public hammer::ports::IMapCodec
{
public:
	bool failEncode = false;

	std::string_view FormatName() const override { return "fake"; }

	foundation::Expected<hammer::ports::DecodedMap, hammer::ports::CodecError> Decode(
	    std::string_view text, std::uint32_t serial ) const override
	{
		const auto it = m_saved.find( std::string( text ) );
		if ( it == m_saved.end() )
		{
			return foundation::MakeUnexpected(
			    hammer::ports::CodecError{ "not a saved document", 1 } );
		}
		const hammer::scene::MapDocument &source = it->second;
		hammer::ports::DecodedMap out{ hammer::scene::MapDocument( serial ), {} };
		auto restamp = [serial]( hammer::scene::ObjectId id )
		{
			if ( id.IsValid() )
			{
				id.value =
				    ( static_cast<std::uint64_t>( serial ) << 32 ) | ( id.value & 0xffffffffu );
			}
			return id;
		};
		for ( auto [id, solid] : source.Solids() )
		{
			solid.id = restamp( solid.id );
			solid.owner = restamp( solid.owner );
			solid.group = restamp( solid.group );
			out.document.Put( solid );
		}
		for ( auto [id, entity] : source.Entities() )
		{
			entity.id = restamp( entity.id );
			entity.group = restamp( entity.group );
			out.document.Put( entity );
		}
		for ( auto [id, group] : source.Groups() )
		{
			group.id = restamp( group.id );
			group.group = restamp( group.group );
			out.document.Put( group );
		}
		out.document.MutableSettings() = source.Settings();
		return out;
	}

	foundation::Expected<std::string, hammer::ports::CodecError> Encode(
	    const hammer::scene::MapDocument &document ) const override
	{
		if ( failEncode )
		{
			return foundation::MakeUnexpected(
			    hammer::ports::CodecError{ "injected encode failure", 0 } );
		}
		const std::string token = "fake-document-" + std::to_string( m_saved.size() + 1 );
		m_saved.emplace( token, document );
		return token;
	}

private:
	mutable std::map<std::string, hammer::scene::MapDocument> m_saved;
};

} // namespace hammertest

#endif // HAMMERTEST_FAKE_MAP_CODEC_H
