//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy thread-pool bridge for the dependency-aware job system
//          (RFC 0003, Phase C / R20 "controlled legacy bridge").
//
//          Exposes the real engine CThreadPool as a jobsystem::IWorkerBackend so
//          job graphs execute on the same worker pool the engine already uses
//          for ParallelProcess. This header is intentionally C++11-clean and
//          does not pull in jobthread.h or the C++20 job-graph headers, so any
//          engine target can request a backend without a standard-version or
//          heavy-include cost.
//
//=============================================================================//

#ifndef VSTDLIB_JOBGRAPH_POOL_BRIDGE_H
#define VSTDLIB_JOBGRAPH_POOL_BRIDGE_H

#ifdef _WIN32
#pragma once
#endif

#include "vstdlib/vstdlib.h"

namespace jobsystem { class IWorkerBackend; }

// Creates a worker backend over a private CThreadPool started with nThreads
// worker threads. nThreads <= 0 yields an inline (serial) backend. The caller
// owns the returned pointer and must free it with DestroyThreadPoolWorkerBackend,
// which stops and destroys the underlying pool.
VSTDLIB_INTERFACE jobsystem::IWorkerBackend *CreateThreadPoolWorkerBackend( int nThreads );
VSTDLIB_INTERFACE void DestroyThreadPoolWorkerBackend( jobsystem::IWorkerBackend *pBackend );

#endif // VSTDLIB_JOBGRAPH_POOL_BRIDGE_H
