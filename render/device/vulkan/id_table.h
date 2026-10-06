// The Vulkan adapter's resource records by id (RFC 0016 K1): a slot table in
// place of a node-based hash map. An id is [kind:8][generation:24][slot+1:32],
// so a lookup is a shift, an index and a compare; records sit contiguously in
// fixed pages that never move, so a record's address stays valid while it
// lives (LiveBuffer and the others hand out pointers). Ids stay unique across
// kinds (the translator tracks buffers and textures in one map) and a freed
// slot's next id has a new generation, so a released id never names a new
// record (until a slot is reused 2^24 times).
#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace render::device::vulkan
{

template <class T>
class IdTable
{
public:
	using Entry = std::pair<std::uint64_t, T>;

	explicit IdTable( std::uint8_t kind ) : m_Kind( kind ) {}
	IdTable( const IdTable & ) = delete;
	IdTable &operator=( const IdTable & ) = delete;

	template <class Table, class Value>
	class Iterator
	{
	public:
		Iterator( Table *table, std::uint32_t slot ) : m_Table( table ), m_Slot( slot ) { Skip(); }
		Value &operator*() const { return m_Table->SlotAt( m_Slot ).entry; }
		Value *operator->() const { return &m_Table->SlotAt( m_Slot ).entry; }
		Iterator &operator++()
		{
			++m_Slot;
			Skip();
			return *this;
		}
		bool operator==( const Iterator &other ) const { return m_Slot == other.m_Slot; }
		bool operator!=( const Iterator &other ) const { return m_Slot != other.m_Slot; }

	private:
		friend class IdTable;
		void Skip()
		{
			while ( m_Slot < m_Table->m_Count && !m_Table->SlotAt( m_Slot ).live )
				++m_Slot;
		}
		Table *m_Table;
		std::uint32_t m_Slot;
	};
	using iterator = Iterator<IdTable, Entry>;
	using const_iterator = Iterator<const IdTable, const Entry>;

	// The id the next emplace takes (the caller mints it, then emplaces it).
	std::uint64_t NextId() const
	{
		const std::uint32_t slot = m_Free.empty() ? m_Count : m_Free.back();
		const std::uint32_t generation = slot < m_Count ? SlotAt( slot ).generation : 0;
		return Compose( slot, generation );
	}

	std::pair<iterator, bool> emplace( std::uint64_t id, T &&value )
	{
		std::uint32_t slot = 0;
		if ( !Decode( id, slot ) || ( slot < m_Count && SlotAt( slot ).live ) ||
		     id != NextId() )
		{
			assert( !"IdTable: emplace of an id NextId did not give" );
			return { end(), false };
		}
		if ( slot == m_Count )
		{
			if ( ( m_Count & kPageMask ) == 0 )
				m_Pages.push_back( std::make_unique<Page>() );
			++m_Count;
		}
		else
			m_Free.pop_back();
		Slot &s = SlotAt( slot );
		s.entry.first = id;
		s.entry.second = std::move( value );
		s.live = true;
		++m_Size;
		return { iterator( this, slot ), true };
	}

	std::pair<iterator, bool> emplace( std::uint64_t id, const T &value )
	{
		return emplace( id, T( value ) );
	}

	iterator find( std::uint64_t id ) { return iterator( this, Find( id ) ); }
	const_iterator find( std::uint64_t id ) const { return const_iterator( this, Find( id ) ); }
	T &at( std::uint64_t id )
	{
		const std::uint32_t slot = Find( id );
		assert( slot != m_Count );
		return SlotAt( slot ).entry.second;
	}

	iterator erase( iterator it )
	{
		Release( it.m_Slot );
		++it;
		return it;
	}
	std::size_t erase( std::uint64_t id )
	{
		const std::uint32_t slot = Find( id );
		if ( slot == m_Count )
			return 0;
		Release( slot );
		return 1;
	}

	// Frees every record; ids issued before stay invalid (generations advance).
	void clear()
	{
		for ( std::uint32_t slot = 0; slot < m_Count; ++slot )
		{
			if ( SlotAt( slot ).live )
				Release( slot );
		}
	}

	std::size_t size() const { return m_Size; }
	bool empty() const { return m_Size == 0; }
	iterator begin() { return iterator( this, 0 ); }
	iterator end() { return iterator( this, m_Count ); }
	const_iterator begin() const { return const_iterator( this, 0 ); }
	const_iterator end() const { return const_iterator( this, m_Count ); }

private:
	static constexpr std::uint32_t kPageBits = 8;
	static constexpr std::uint32_t kPageMask = ( 1u << kPageBits ) - 1;
	static constexpr std::uint32_t kGenerationMask = ( 1u << 24 ) - 1;

	struct Slot
	{
		Entry entry{};
		std::uint32_t generation = 0;
		bool live = false;
	};
	struct Page
	{
		Slot slots[1u << kPageBits];
	};

	Slot &SlotAt( std::uint32_t slot ) { return m_Pages[slot >> kPageBits]->slots[slot & kPageMask]; }
	const Slot &SlotAt( std::uint32_t slot ) const
	{
		return m_Pages[slot >> kPageBits]->slots[slot & kPageMask];
	}

	std::uint64_t Compose( std::uint32_t slot, std::uint32_t generation ) const
	{
		return ( std::uint64_t( m_Kind ) << 56 ) |
		       ( std::uint64_t( generation & kGenerationMask ) << 32 ) |
		       ( std::uint64_t( slot ) + 1 );
	}
	bool Decode( std::uint64_t id, std::uint32_t &slot ) const
	{
		const std::uint32_t low = std::uint32_t( id );
		if ( low == 0 || ( id >> 56 ) != m_Kind )
			return false;
		slot = low - 1;
		return true;
	}
	// The live slot id names, or m_Count.
	std::uint32_t Find( std::uint64_t id ) const
	{
		std::uint32_t slot = 0;
		if ( !Decode( id, slot ) || slot >= m_Count )
			return m_Count;
		const Slot &s = SlotAt( slot );
		return s.live && s.entry.first == id ? slot : m_Count;
	}
	void Release( std::uint32_t slot )
	{
		Slot &s = SlotAt( slot );
		s.entry.second = T{};
		s.live = false;
		s.generation = ( s.generation + 1 ) & kGenerationMask;
		--m_Size;
		m_Free.push_back( slot );
	}

	std::uint8_t m_Kind;
	std::vector<std::unique_ptr<Page>> m_Pages;
	std::vector<std::uint32_t> m_Free;
	std::uint32_t m_Count = 0; // slots ever used
	std::size_t m_Size = 0;
};

} // namespace render::device::vulkan
