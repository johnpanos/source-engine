//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: C++11-compatible host frame graph (RFC 0003 "Tick and render-frame
//          composition", R10). The host describes one frame as an ordered
//          list of legacy phases; each becomes a legacy node on a single
//          sequence lane of a FrameCoordinator graph, which is validated,
//          sealed and executed serially on the calling thread by the
//          deterministic executor. Implementation and graph storage remain
//          C++20.
//
//=============================================================================//

#ifndef JOBSYSTEM_SERIAL_FRAME_GRAPH_H
#define JOBSYSTEM_SERIAL_FRAME_GRAPH_H

namespace jobsystem
{

// One ordered legacy phase. run returns false to end the frame early: every
// later phase is canceled and never runs. run must return normally; it may
// not throw or longjmp past the graph (callers that use longjmp for error
// exits trap it inside run and return false).
struct FramePhaseDesc
{
	const char *name;                        // stable, non-empty; part of the graph shape
	bool ( *run )( void *context, int arg ); // required
	void *context;                           // read on every run
	int arg;                                 // read on every run
};

struct SerialFrameRun
{
	bool valid;        // false: invalid input or graph; no phase ran
	bool stopped;      // a phase returned false
	unsigned executed; // phases whose run() was called
};

// Phases run on the calling thread, exactly once each and exactly in array
// order, until one returns false. The sealed graph is reused while the shape
// (count, names and run functions) is unchanged; contexts and args are read
// per run. One thread uses an instance at a time; runs do not nest.
class SerialFrameGraph
{
public:
	SerialFrameGraph();
	~SerialFrameGraph();

	SerialFrameRun Run( const FramePhaseDesc *phases, unsigned count );

	// Diagnostics: number of graph (re)builds since construction.
	unsigned SealCount() const;

private:
	SerialFrameGraph( const SerialFrameGraph & );
	SerialFrameGraph &operator=( const SerialFrameGraph & );

	struct Impl;
	Impl *m_impl;
};

} // namespace jobsystem

#endif // JOBSYSTEM_SERIAL_FRAME_GRAPH_H
