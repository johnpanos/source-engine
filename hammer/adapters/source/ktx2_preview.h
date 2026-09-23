//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: KTX2 preview adapter for the headless Hammer material catalog.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_SOURCE_KTX2_PREVIEW_H
#define HAMMER_ADAPTERS_SOURCE_KTX2_PREVIEW_H

#include "hammer/formats/material_catalog.h"

namespace hammer::adapters::source
{

// Uses the same container reader as the native runtime. An already packaged
// RGBA8/BGRA8 2D image can be displayed by the GTK texture browser. Other
// formats fail explicitly until a preview provider can decode them.
std::optional<hammer::formats::VtfImage> DecodeKtx2Preview(
    const std::string &bytes, std::string &error );

} // namespace hammer::adapters::source

#endif // HAMMER_ADAPTERS_SOURCE_KTX2_PREVIEW_H
