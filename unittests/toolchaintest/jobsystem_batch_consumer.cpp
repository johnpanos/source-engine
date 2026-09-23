//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frozen external consumer of the jobsystem C++11 facade
//          (toolchain.abi.v1). Built in the fixture-only legacy-cxx11 dialect,
//          as a prebuilt mod or plugin would be, and linked with the real
//          jobsystem sources built as C++20. It implements IWorkerBackend (a
//          vtable called from C++20) and drives ExecuteParallelBatch (C++20
//          called from C++11), passing std::function and BatchDesc across.
//
//=============================================================================//

#include "abi_checks.h"

#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#if __cplusplus != 201103L
#error "the frozen consumer must be compiled as C++11"
#endif

int toolchaintest::g_checks = 0;
int toolchaintest::g_failures = 0;

namespace
{

class ThreadBackend : public jobsystem::IWorkerBackend
{
public:
	explicit ThreadBackend( int workers ) : m_workers( workers ), m_calls( 0 ) {}

	virtual void ParallelFor( int n, const std::function<void( int )> &body )
	{
		++m_calls;
		std::vector<std::thread> threads;
		for ( int i = 0; i < n; ++i )
			threads.push_back( std::thread( [&body, i]() { body( i ); } ) );
		for ( size_t i = 0; i < threads.size(); ++i )
			threads[i].join();
	}

	virtual int WorkerCount() const { return m_workers; }

	int Calls() const { return m_calls; }

private:
	int m_workers;
	int m_calls;
};

struct BatchState
{
	std::vector<unsigned> out;
	std::atomic<int> processed;
	std::atomic<int> begins;
	std::atomic<int> ends;
	std::string tag; // a dual-ABI std::string owned entirely on this side
};

void Process( void *context, unsigned index )
{
	BatchState &state = *static_cast<BatchState *>( context );
	state.out[index] = index * index + static_cast<unsigned>( state.tag.size() );
	++state.processed;
}

void Begin( void *context ) { ++static_cast<BatchState *>( context )->begins; }
void End( void *context ) { ++static_cast<BatchState *>( context )->ends; }

void RunBatch( jobsystem::BatchMode mode, ThreadBackend *backend )
{
	const unsigned count = 1000;
	BatchState state;
	state.out.assign( count, 0u );
	state.processed = 0;
	state.begins = 0;
	state.ends = 0;
	state.tag = "frozen-consumer";

	jobsystem::BatchDesc desc;
	desc.name = "toolchain.abi";
	desc.context = &state;
	desc.count = count;
	desc.process = &Process;
	desc.begin = &Begin;
	desc.end = &End;
	desc.maxParticipants = 4;

	ABI_CHECK( jobsystem::ExecuteParallelBatch( desc, backend, mode ) );
	ABI_CHECK( state.processed == static_cast<int>( count ) );
	bool all = true;
	for ( unsigned i = 0; i < count; ++i )
		all = all && state.out[i] == i * i + static_cast<unsigned>( state.tag.size() );
	ABI_CHECK( all );
	ABI_CHECK( state.begins >= 1 && state.begins == state.ends );
	ABI_CHECK( state.begins <= 4 );
}

} // namespace

int main()
{
	toolchaintest::CheckBoundaryLayouts();

	ThreadBackend backend( 3 );
	RunBatch( jobsystem::BatchMode::Serial, &backend );
	RunBatch( jobsystem::BatchMode::Parallel, &backend );
	ABI_CHECK( backend.Calls() >= 1 ); // the C++20 side dispatched through our vtable

	// Invalid input fails before any callback runs, on either side of the ABI.
	jobsystem::BatchDesc invalid;
	invalid.count = 5;
	ABI_CHECK( !jobsystem::ExecuteParallelBatch( invalid, &backend, jobsystem::BatchMode::Parallel ) );

	return toolchaintest::Finish();
}
