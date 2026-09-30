//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Runtime switching between indirect-light producers
//          (render.indirect-switching, RFC 0011). One switcher per map owns
//          the active producer, a pending one while it seeds, the fade
//          between their volumes, and ended producers until their tickets
//          complete.
//
//          Switching A -> B:
//          1. B's seed is A's newest published volume (or the baked one).
//          2. B.Begin( scene, seed ). On failure A stays active and the error
//             is returned; nothing was mutated.
//          3. B is scheduled while A's last volume stays the published one; A
//             stops scheduling.
//          4. When B first publishes, consumers fade from A's last volume to
//             B's over fadeFrames frames. Both are resident only then.
//          5. A.End(). A is destroyed once the completion serial passes both
//             its ticket and the last frame that sampled its volume.
//
//          Device loss republishes the baked volume at once (no fade: the
//          device's work is gone) and re-establishes the previous selection
//          through steps 1-5 after recovery. Backgrounding stops scheduling;
//          producers that need device features re-seed from the baked volume
//          on resume. A map change ends every producer.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_SWITCHER_H
#define RENDER_INDIRECT_SWITCHER_H

#include "render/indirect_light.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

namespace indirect_light
{
// The composition root's producers: those the profile, device and map offer.
class IProducerCatalog
{
public:
	virtual ~IProducerCatalog() = default;
	[[nodiscard]] virtual bool Offers( ProducerKind kind, const IndirectScene &scene ) const = 0;
	[[nodiscard]] virtual std::unique_ptr<IProducer> Create( ProducerKind kind ) = 0;
};

// What consumers sample in one frame. `generation` changes whenever the
// volume does (a new epoch, a fade step, a republication), so the host
// uploads only then.
struct FrameVolume
{
	std::shared_ptr<const Volume> volume;
	uint64_t generation = 0;
	ProducerKind producer = ProducerKind::Baked;
	// The world's policy this frame: the active producer's, or during a fade
	// the stronger of the two, so a change fades out as its volume does.
	indirect_policy::Policy policy = indirect_policy::Policy::Baked;
	bool fading = false;
	float weight = 1.0f;
	// The probes whose tiles changed from generation `changedSince`'s volume
	// (the one before): a consumer holding that generation updates only
	// these. Null: the whole volume.
	ChangedProbes changed = nullptr;
	uint64_t changedSince = 0;
};

class Switcher
{
public:
	Switcher( IProducerCatalog &catalog, IResourceTracker &resources, uint32_t fadeFrames = 30 )
	    : m_catalog( catalog ), m_resources( resources ), m_fadeFrames( std::max( 1u, fadeFrames ) )
	{
	}
	~Switcher() { EndMap(); }
	Switcher( const Switcher & ) = delete;
	Switcher &operator=( const Switcher & ) = delete;

	// Starts a map with the Baked producer, then selects `requested`. An
	// unoffered request is reported (the returned error) and Baked stays.
	foundation::Expected<void, IndirectError> BeginMap(
	    const IndirectScene &scene, ProducerKind requested )
	{
		EndMap();
		m_scene = scene;
		m_scene.policy = indirect_policy::Policy::Baked;
		auto baked = m_catalog.Create( ProducerKind::Baked );
		if ( !baked )
			return foundation::MakeUnexpected( IndirectError::Unavailable );
		const PublishedVolume none;
		const auto begun = baked->Begin( m_scene, none, m_resources );
		if ( !begun )
			return foundation::MakeUnexpected( begun.Error() );
		m_active = { ProducerKind::Baked, std::move( baked ), indirect_policy::Policy::Baked };
		m_selected = ProducerKind::Baked;
		m_activeEpoch = m_active.producer->Published()->epoch;
		Publish( m_active.producer->Published()->volume, false, 1.0f );
		if ( requested != ProducerKind::Baked )
			return Select( requested );
		return {};
	}

	void EndMap()
	{
		const uint64_t after = m_lastFrame;
		for ( Slot *slot : { &m_active, &m_pending, &m_fading } )
		{
			if ( slot->producer )
				Retire( std::move( slot->producer ), after );
			*slot = Slot();
		}
		m_fromVolume.reset();
		m_fade = 0;
	}

	// Steps 1-2. On failure the active producer stays and nothing changes.
	foundation::Expected<void, IndirectError> Select( ProducerKind kind )
	{
		if ( !m_active.producer )
			return foundation::MakeUnexpected( IndirectError::Unavailable );
		if ( kind == m_active.kind && !m_pending.producer )
			return {};
		if ( !m_catalog.Offers( kind, m_scene ) )
			return foundation::MakeUnexpected( IndirectError::Unavailable );
		auto producer = m_catalog.Create( kind );
		if ( !producer )
			return foundation::MakeUnexpected( IndirectError::Unavailable );
		IndirectScene scene = m_scene;
		scene.policy = PolicyFor( producer->Caps() );
		PublishedVolume seed{ 0, m_frame.volume };
		const auto begun = producer->Begin( scene, seed, m_resources );
		if ( !begun )
			return foundation::MakeUnexpected( begun.Error() );
		// A previous pending producer is superseded.
		if ( m_pending.producer )
			Retire( std::move( m_pending.producer ), m_lastFrame );
		m_pending = { kind, std::move( producer ), scene.policy };
		m_selected = kind;
		return {};
	}

	// Once per frame, before its render work: schedules, fades, retires.
	FrameVolume Frame( FrameWork &work, const light_set::Snapshot &lights )
	{
		m_lastFrame = work.frameSerial;
		if ( !m_background )
		{
			if ( m_pending.producer )
				m_pending.producer->Schedule( work, lights );
			else if ( m_active.producer )
				m_active.producer->Schedule( work, lights );
		}
		// Steps 3-4: the pending producer's first publication starts the fade.
		// The old producer stops scheduling but stays until the fade ends: a
		// device producer's output may be what the fade samples.
		if ( m_pending.producer && !m_fade && m_pending.producer->Published() )
		{
			if ( m_fading.producer )
				Retire( std::move( m_fading.producer ), work.frameSerial );
			m_fromVolume = m_frame.volume;
			m_fadeProbes.reset();
			m_fade = 1;
			m_fading = std::move( m_active );
			m_active = std::move( m_pending );
			m_pending = Slot();
		}
		if ( m_fade )
		{
			const auto newest = m_active.producer->Published();
			const float weight = float( m_fade ) / float( m_fadeFrames );
			const uint64_t since = m_frame.generation;
			// Only the probes where the two volumes differ change: they are
			// found once, then grown by each later publication's changed
			// probes, and each fade frame blends and names only them.
			std::shared_ptr<const Volume> blended;
			const bool sparse = newest && m_fromVolume &&
			                    m_fromVolume->SameTopology( *newest->volume ) &&
			                    FadeProbes( *newest );
			if ( sparse )
				blended = weight >= 1.0f ? newest->volume
				                         : BlendProbes( *m_fromVolume, *newest->volume, weight,
				                               *m_fadeProbes );
			else if ( newest && m_fromVolume )
				blended = Blend( *m_fromVolume, *newest->volume, weight );
			if ( !blended )
				blended = newest ? newest->volume : m_fromVolume;
			Publish( blended, m_fade < m_fadeFrames, weight );
			if ( sparse )
			{
				m_frame.changed = m_fadeProbes;
				m_frame.changedSince = since;
			}
			if ( ++m_fade > m_fadeFrames )
			{
				// Step 5: this frame is the last that samples the old volume.
				m_fade = 0;
				m_fromVolume.reset();
				m_fadeProbes.reset();
				Retire( std::move( m_fading.producer ), work.frameSerial );
				m_fading = Slot();
				m_activeEpoch = newest ? newest->epoch : 0;
				// The last fade frame held the producer's volume itself: its
				// next publication's changed probes carry over.
				m_frameIsActive = sparse;
			}
		}
		else if ( m_active.producer )
		{
			const auto newest = m_active.producer->Published();
			if ( newest && newest->epoch != m_activeEpoch )
			{
				// The producer's next publication after the one this frame
				// held, with no fade between: its changed probes carry over.
				const bool next = m_frameIsActive && newest->epoch == m_activeEpoch + 1;
				m_activeEpoch = newest->epoch;
				const uint64_t since = m_frame.generation;
				Publish( newest->volume, false, 1.0f );
				m_frameIsActive = true;
				if ( next && newest->changed )
				{
					m_frame.changed = newest->changed;
					m_frame.changedSince = since;
				}
			}
		}
		// Step 5: ended producers go once their tickets complete.
		const uint64_t completed = m_resources.CompletedSerial();
		m_retired.erase( std::remove_if( m_retired.begin(), m_retired.end(),
		                     [&]( const Retired &retired )
		                     {
			                     return completed >= retired.after;
		                     } ),
		    m_retired.end() );
		m_frame.producer = m_active.kind;
		m_frame.policy =
		    m_fading.producer ? std::max( m_active.policy, m_fading.policy ) : m_active.policy;
		TrackResidency();
		return m_frame;
	}

	void DeviceLost()
	{
		// The device's work is gone: republish the baked volume at once and
		// begin again after recovery.
		m_lostSelection = m_selected;
		IndirectScene scene = m_scene;
		EndMap();
		m_retired.clear(); // their GPU resources went with the device
		(void)BeginMap( scene, ProducerKind::Baked );
	}

	foundation::Expected<void, IndirectError> DeviceRecovered()
	{
		const ProducerKind previous = m_lostSelection.value_or( ProducerKind::Baked );
		m_lostSelection.reset();
		// The baked volume is republished for the recreated device first.
		Publish( m_frame.volume, false, 1.0f );
		return previous == ProducerKind::Baked ? foundation::Expected<void, IndirectError>{}
		                                       : Select( previous );
	}

	void Background() { m_background = true; }

	void Resume()
	{
		m_background = false;
		// A producer needing device features released its transient resources:
		// it re-seeds from the baked volume.
		if ( m_active.producer && m_active.producer->Caps().requiredFeatures != 0 )
		{
			const ProducerKind kind = m_active.kind;
			m_frame.volume = m_scene.baked;
			Publish( m_scene.baked, false, 1.0f );
			Retire( std::move( m_active.producer ), m_lastFrame );
			m_active = Slot();
			IndirectScene scene = m_scene;
			(void)BeginMap( scene, kind );
		}
	}

	[[nodiscard]] ProducerKind Active() const { return m_active.kind; }
	[[nodiscard]] std::optional<ProducerKind> Pending() const
	{
		return m_pending.producer ? std::optional<ProducerKind>( m_pending.kind ) : std::nullopt;
	}
	[[nodiscard]] ProducerKind Selected() const { return m_selected; }
	[[nodiscard]] size_t RetiredCount() const { return m_retired.size(); }
	// The volumes the switcher holds, and the most it held at once.
	[[nodiscard]] size_t ResidentBytes() const { return m_resident; }
	[[nodiscard]] size_t PeakResidentBytes() const { return m_peakResident; }

private:
	struct Slot
	{
		ProducerKind kind = ProducerKind::Baked;
		std::unique_ptr<IProducer> producer;
		indirect_policy::Policy policy = indirect_policy::Policy::Baked;
	};
	struct Retired
	{
		std::unique_ptr<IProducer> producer;
		uint64_t after;
	};

	static indirect_policy::Policy PolicyFor( const ProducerCaps &caps )
	{
		// The producer's strongest policy: RuntimeIndirect, then the delta.
		for ( indirect_policy::Policy policy :
		    { indirect_policy::Policy::RuntimeIndirect, indirect_policy::Policy::BakedPlusDelta } )
			if ( caps.policies & PolicyBit( policy ) )
				return policy;
		return indirect_policy::Policy::Baked;
	}

	// The fade's probes for the active producer's `newest` publication: where
	// the fade's source and it differ (a superset: each later publication's
	// changed probes are added). False when they cannot be known.
	bool FadeProbes( const PublishedVolume &newest )
	{
		if ( m_fadeProbes && newest.epoch == m_fadeEpoch )
			return true;
		if ( m_fadeProbes && newest.changed && newest.epoch == m_fadeEpoch + 1 )
		{
			std::vector<uint32_t> grown = *m_fadeProbes;
			grown.insert( grown.end(), newest.changed->begin(), newest.changed->end() );
			std::sort( grown.begin(), grown.end() );
			grown.erase( std::unique( grown.begin(), grown.end() ), grown.end() );
			m_fadeProbes = std::make_shared<const std::vector<uint32_t>>( std::move( grown ) );
		}
		else
		{
			m_fadeProbes = std::make_shared<const std::vector<uint32_t>>(
			    DifferingProbes( *m_fromVolume, *newest.volume ) );
		}
		m_fadeEpoch = newest.epoch;
		return true;
	}

	void Publish( std::shared_ptr<const Volume> volume, bool fading, float weight )
	{
		m_frame.volume = std::move( volume );
		m_frame.fading = fading;
		m_frame.weight = weight;
		m_frame.changed.reset();
		m_frame.changedSince = 0;
		m_frameIsActive = false;
		++m_frame.generation;
	}

	void Retire( std::unique_ptr<IProducer> producer, uint64_t lastSampled )
	{
		if ( !producer )
			return;
		const RetireTicket ticket = producer->End();
		m_retired.push_back(
		    { std::move( producer ), std::max( ticket.afterSerial, lastSampled ) } );
	}

	void TrackResidency()
	{
		std::vector<const Volume *> held;
		const auto add = [&]( const std::shared_ptr<const Volume> &volume )
		{
			if ( volume && std::find( held.begin(), held.end(), volume.get() ) == held.end() )
				held.push_back( volume.get() );
		};
		add( m_frame.volume );
		add( m_fromVolume );
		for ( const Slot *slot : { &m_active, &m_pending, &m_fading } )
			if ( slot->producer )
				if ( const auto published = slot->producer->Published() )
					add( published->volume );
		m_resident = 0;
		for ( const Volume *volume : held )
			m_resident += volume->bytes.size();
		m_peakResident = std::max( m_peakResident, m_resident );
	}

	IProducerCatalog &m_catalog;
	IResourceTracker &m_resources;
	uint32_t m_fadeFrames;
	IndirectScene m_scene;
	Slot m_active;
	Slot m_pending;
	Slot m_fading; // the old producer during a fade
	ProducerKind m_selected = ProducerKind::Baked;
	std::optional<ProducerKind> m_lostSelection;
	std::vector<Retired> m_retired;
	std::shared_ptr<const Volume> m_fromVolume;
	uint32_t m_fade = 0;
	uint64_t m_activeEpoch = 0;
	// The frame's volume is the active producer's publication m_activeEpoch.
	bool m_frameIsActive = false;
	// The fade's changing probes, and the publication they were found for.
	ChangedProbes m_fadeProbes;
	uint64_t m_fadeEpoch = 0;
	uint64_t m_lastFrame = 0;
	bool m_background = false;
	FrameVolume m_frame;
	size_t m_resident = 0;
	size_t m_peakResident = 0;
};

} // namespace indirect_light

#endif // RENDER_INDIRECT_SWITCHER_H
