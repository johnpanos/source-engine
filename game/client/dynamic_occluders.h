//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frame's moving objects as occluders (render.dynamic-
//          occlusion.v1, dynamic_occluders.cpp).
//
//=============================================================================//
#ifndef DYNAMIC_OCCLUDERS_H
#define DYNAMIC_OCCLUDERS_H
#ifdef _WIN32
#pragma once
#endif

// Whether moving objects block light through the engine (r_dynamic_occlusion
// and an engine that takes them): their blob shadows are then not drawn.
bool DynamicOccluders_Active();

#endif // DYNAMIC_OCCLUDERS_H
