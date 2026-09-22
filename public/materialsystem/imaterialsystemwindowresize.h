//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Optional nonblocking native-window resize contract.
//
//=============================================================================//

#ifndef IMATERIALSYSTEMWINDOWRESIZE_H
#define IMATERIALSYSTEMWINDOWRESIZE_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

#define MATERIALSYSTEM_WINDOW_RESIZE_INTERFACE_VERSION "VMaterialSystemWindowResize001"

struct MaterialWindowResizeRequest_t
{
	uint64 m_nSerial;
	uint32 m_nDrawableWidth;
	uint32 m_nDrawableHeight;
};

struct MaterialWindowResizeStatus_t
{
	uint64 m_nRequestedSerial;
	uint64 m_nCompletedSerial;
	uint32 m_nDrawableWidth;
	uint32 m_nDrawableHeight;
};

abstract_class IMaterialSystemWindowResize
{
public:
	// Publishes a latest-value request into the current render command queue.
	// This method never waits for the render worker or GPU.
	virtual bool RequestWindowResize( const MaterialWindowResizeRequest_t &request ) = 0;

	// Reads completion published by the render worker. This method never waits.
	virtual MaterialWindowResizeStatus_t GetWindowResizeStatus() const = 0;

protected:
	virtual ~IMaterialSystemWindowResize() {}
};

#endif // IMATERIALSYSTEMWINDOWRESIZE_H
