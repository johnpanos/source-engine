//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Job-system compatibility names for the engine-wide strict C++20
//          recoverable-result vocabulary.
//
//=============================================================================//

#ifndef JOBSYSTEM_EXPECTED_H
#define JOBSYSTEM_EXPECTED_H

#include "foundation/expected.h"

namespace jobsystem
{

using foundation::Expected;
using foundation::MakeUnexpected;
using foundation::Unexpected;

} // namespace jobsystem

#endif // JOBSYSTEM_EXPECTED_H
