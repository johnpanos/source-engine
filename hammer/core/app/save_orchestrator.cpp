//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of transactional save orchestration (RFC 0002). No
//			tier0/MFC/PCH dependencies; headless-core profile.
//
//=============================================================================//

#include "hammer/app/save_orchestrator.h"

namespace hammer::app
{

SaveStatus SaveDocument( ports::IFileStore &store, const std::string &path,
    const std::string &content, const std::string &tempSuffix )
{
	const std::string tempPath = path + tempSuffix;

	// 1) Write to a temporary sibling. If this fails, the target is untouched.
	if ( !store.Write( tempPath, content ) )
	{
		store.Remove( tempPath ); // best-effort cleanup of any partial temp
		return SaveStatus::kTempWriteFailed;
	}

	// 2) Atomically replace the target with the temp. If the rename fails, the
	//    prior target is still intact; drop the temp.
	if ( !store.Rename( tempPath, path ) )
	{
		store.Remove( tempPath );
		return SaveStatus::kCommitFailed;
	}

	return SaveStatus::kOk;
}

} // namespace hammer::app
