//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Transactional save orchestration (RFC 0002, hammer.app). Writes to a
//			temporary file, then atomically replaces the target, so a failed save
//			preserves the prior file. This centralizes the save policy the RFC
//			requires instead of scattering write-then-hope logic. Depends only on
//			the hammer.ports file-store contract; no MFC/tier0/PCH.
//
//=============================================================================//

#ifndef HAMMER_APP_SAVE_ORCHESTRATOR_H
#define HAMMER_APP_SAVE_ORCHESTRATOR_H

#include "hammer/ports/file_store.h"

#include <string>

namespace hammer::app
{

enum class SaveStatus
{
	kOk,
	kTempWriteFailed, // could not write the temporary file; target untouched
	kCommitFailed,    // temp written but atomic replace failed; target untouched
};

// Saves 'content' to 'path' via a temporary file and an atomic rename. On any
// failure the prior file at 'path' is left intact and the temporary is cleaned
// up on a best-effort basis. 'tempSuffix' names the sibling temp file.
SaveStatus SaveDocument( ports::IFileStore &store, const std::string &path,
    const std::string &content, const std::string &tempSuffix = ".savetmp" );

} // namespace hammer::app

#endif // HAMMER_APP_SAVE_ORCHESTRATOR_H
