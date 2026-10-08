//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.composition's world (RFC 0016 K5): temporal inputs and reconstruction.
//
//=============================================================================//

#include "core_world_internal.h"

namespace render::composition
{

bool CoreWorld::CaptureTemporalInputs( const char *prefix, bool afterReset )
{
	if ( !m_TemporalEnabled || !prefix || !*prefix || std::strlen( prefix ) > 1024 )
		return false;
	std::lock_guard<std::mutex> lock( m_TemporalLock );
	if ( !m_TemporalCapturePrefix.empty() )
		return false;
	m_TemporalCapturePrefix = prefix;
	m_TemporalCaptureAfterReset = afterReset;
	m_TemporalCaptureGeneration = m_TemporalGeneration;
	return true;
}

bool CoreWorld::ReconstructTemporal( int x, int y, int rw, int rh, int ow, int oh, float dt )
{
	if ( !m_TemporalEnabled || rw <= 0 || rh <= 0 || ow <= 0 || oh <= 0 || dt <= 0 )
		return false;
	auto *slots = m_Frontend.CorePassSlots();
	if ( !slots )
		return false;
	TemporalRequest request;
	request.x = x;
	request.y = y;
	request.render = { std::uint32_t( rw ), std::uint32_t( rh ) };
	request.output = { std::uint32_t( ow ), std::uint32_t( oh ) };
	request.jitterX = m_JitterX;
	request.jitterY = m_JitterY;
	request.deltaMilliseconds = dt;
	request.generation = m_TemporalGeneration;
	const std::uint32_t tag = 0x88000000u | ( ++m_TemporalSerial & 0x00ffffffu );
	{
		std::lock_guard<std::mutex> lock( m_TemporalLock );
		if ( m_TemporalRequests.size() >= 8 )
			return false;
		if ( !m_TemporalCaptureAfterReset || m_TemporalCaptureGeneration != m_TemporalGeneration )
		{
			request.capturePrefix = std::move( m_TemporalCapturePrefix );
			m_TemporalCapturePrefix.clear();
			m_TemporalCaptureAfterReset = false;
		}
		if ( !request.capturePrefix.empty() )
		{
			auto camera = m_PendingCameras.find( m_TemporalView );
			auto previous = m_PreviousCameras.find( m_TemporalView );
			request.cameraValid =
			    camera != m_PendingCameras.end() && previous != m_PreviousCameras.end();
			if ( request.cameraValid )
			{
				request.currentToClip = camera->second.toClip;
				request.previousToClip = previous->second.toClip;
			}
			// Capture only submitted topology, on demand. Unselected body/LOD
			// vertices retain bind-pose positions and are not motion evidence.
			for ( const auto &[key, pose] : m_PendingPoses )
			{
				const auto old = m_PreviousPoses.find( key );
				const auto currentCamera = m_PendingCameras.find( key.first );
				const auto previousCamera = m_PreviousCameras.find( key.first );
				if ( old == m_PreviousPoses.end() || currentCamera == m_PendingCameras.end() ||
				     previousCamera == m_PreviousCameras.end() || old->second.model != pose.model ||
				     old->second.body != pose.body || old->second.lod != pose.lod ||
				     old->second.vertices.size() != pose.vertices.size() )
					continue;
				if ( request.objects.size() == 128 )
				{
					++request.omittedObjects;
					continue;
				}
				TemporalObjectSample sample;
				sample.identity = key.second;
				sample.view = key.first;
				sample.model = pose.model;
				sample.currentToClip = currentCamera->second.toClip;
				sample.previousToClip = previousCamera->second.toClip;
				const auto &viewport = currentCamera->second.viewport;
				sample.viewport = { viewport.x, viewport.y, viewport.width, viewport.height,
				    viewport.minDepth, viewport.maxDepth };
				bool first = true;
				const auto selected =
				    m_ModelPoseSources[pose.model].SelectedSurfaces( pose.body, pose.lod );
				const auto &mesh = m_StaticMeshes[pose.model];
				// The pose's own level: its surfaces and its indices are that
				// level's, from zero.
				const pass::world::WorldData::StaticMeshLod *level =
				    pose.lod < mesh.lodCount() ? &mesh.lods[pose.lod] : nullptr;
				if ( !level || !level->indices )
					continue;
				for ( std::size_t surface = 0; surface < mesh.surfaces.size(); ++surface )
				{
					const auto material = mesh.skinMaterials.empty()
					                          ? mesh.surfaces[surface].material
					                      : pose.skin < mesh.skinMaterials.size() &&
					                              surface < mesh.skinMaterials[pose.skin].size()
					                          ? mesh.skinMaterials[pose.skin][surface]
					                          : ~0u;
					sample.surfaceMaterials.push_back( material < m_StaticMaterials.size()
					                                       ? m_StaticMaterials[material].name
					                                       : "" );
				}
				std::size_t triangleCount = 0;
				for ( auto surfaceId : selected )
					triangleCount += mesh.surfaces[surfaceId].indexCount / 3;
				const std::size_t stride =
				    std::max<std::size_t>( 1, ( triangleCount + 1023 ) / 1024 );
				std::size_t triangle = 0;
				for ( auto surfaceId : selected )
				{
					const auto &surface = mesh.surfaces[surfaceId];
					for ( auto index = surface.firstIndex;
					    index + 2 < surface.firstIndex + surface.indexCount;
					    index += 3, ++triangle )
					{
						if ( sample.triangles.size() == 1024 ||
						     ( triangle % stride && surface.indexCount > 192 ) )
							continue;
						std::array<float, 18> probe;
						for ( unsigned corner = 0; corner < 3; ++corner )
						{
							const auto vertex = ( *level->indices )[index + corner];
							std::copy_n(
							    pose.vertices[vertex].position, 3, probe.begin() + corner * 3 );
							std::copy_n( old->second.vertices[vertex].position, 3,
							    probe.begin() + 9 + corner * 3 );
						}
						sample.triangles.push_back( probe );
						sample.triangleSurfaces.push_back( surfaceId );
					}
					for ( auto index = surface.firstIndex;
					    index < surface.firstIndex + surface.indexCount; ++index )
					{
						const auto vertex = ( *level->indices )[index];
						for ( unsigned axis = 0; axis < 3; ++axis )
						{
							const float current = pose.vertices[vertex].position[axis];
							const float offset =
							    old->second.vertices[vertex].position[axis] - current;
							if ( first )
							{
								sample.bounds[axis] = sample.bounds[axis + 3] = current;
								sample.previousOffset[axis] = offset;
							}
							sample.bounds[axis] = std::min( sample.bounds[axis], current );
							sample.bounds[axis + 3] = std::max( sample.bounds[axis + 3], current );
							sample.translationError = std::max( sample.translationError,
							    std::abs( offset - sample.previousOffset[axis] ) );
						}
						first = false;
					}
				}
				if ( !first )
					request.objects.push_back( std::move( sample ) );
			}
		}
		m_TemporalRequests.emplace( tag, request );
	}
	slots->MarkSlot( tag );
	return true;
}

void CoreWorld::ResetTemporalHistory()
{
	++m_TemporalGeneration;
	m_PreviousCameras.clear();
	m_PendingCameras.clear();
	m_PreviousPoses.clear();
	m_PendingPoses.clear();
}

void CoreWorld::CommitTemporalFrame( bool submitted )
{
	if ( submitted )
	{
		m_PreviousCameras = std::move( m_PendingCameras );
		m_PreviousPoses = std::move( m_PendingPoses );
		m_PendingCameras.clear();
		m_PendingPoses.clear();
	}
	else
		ResetTemporalHistory();
}

} // namespace render::composition
