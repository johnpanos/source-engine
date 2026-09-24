//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: dxsupport.cfg as KeyValues for the shader APIs: reading it (with a
//          mod's dxsupport_override.cfg), describing its groups to the
//          render.dxsupport-policy.v1 owner, and applying the groups that
//          policy selects. Shared by the D3D9 and native Vulkan device managers.
//
//===========================================================================//

#ifndef DXSUPPORT_KEYVALUES_H
#define DXSUPPORT_KEYVALUES_H

#include "render/dxsupport_policy.h"

#include <vector>

class KeyValues;
class IFileSystem;

namespace dxsupport
{
// Loads pConfigFile from EXECUTABLE_PATH and merges pOverrideFile from GAME
// when the game has its own search path. The caller owns the result
// (deleteThis). NULL when the file is missing or -ignoredxsupportcfg is set.
KeyValues *ReadConfig(
    IFileSystem *pFileSystem, const char *pConfigFile, const char *pOverrideFile );

// The facts the policy reads from each top-level group, in file order, and
// the groups themselves at the same indices.
void DescribeGroups( KeyValues *pConfig, std::vector<render::DxSupportGroupFacts> *pFacts,
    std::vector<KeyValues *> *pGroups );

// The card group for a device, NULL when none (render::FindCardGroup).
KeyValues *FindCardGroup( KeyValues *pConfig, int nVendorID, int nDeviceID );

// The groups whose hardware caps apply, in order (render::ResolveDeviceGroupOrder).
std::vector<KeyValues *> DeviceGroups(
    KeyValues *pConfig, int nDxLevel, int nVendorID, int nDeviceID );

// The host's CPU speed and vendor and its system RAM, as the D3D9 device has
// always measured them for dxsupport.cfg (RAM through MEMORYSTATUS: at least
// 128 MB, and at most 4095 MB outside Windows).
void FillHostFacts( render::DxSupportQuery *pQuery );

// Copies the recommended configuration into pConfiguration: every selected
// group's values in policy order, the picmip floor, then ConVar.mat_dxlevel.
// -debugdxsupport prints each group and the result.
void ApplyRecommendedConfig(
    KeyValues *pConfig, const render::DxSupportQuery &query, KeyValues *pConfiguration );
} // namespace dxsupport

#endif // DXSUPPORT_KEYVALUES_H
