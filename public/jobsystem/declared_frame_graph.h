//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: C++11-compatible frame graph whose ordering comes from declared
//          resource access (RFC 0003 "Resource declarations", R20/R21).
//
//          A frame region is described as nodes in legacy order. Host nodes
//          run on the calling thread, always in array order. Batch nodes
//          process independent items on bounded runners. A node that
//          conflicts with an earlier node (a shared domain, one of them
//          writing) runs after it; nothing else orders a batch, so a batch may
//          run at the same time as host nodes it does not conflict with.
//          Implementation and graph storage remain C++20.
//
//=============================================================================//

#ifndef JOBSYSTEM_DECLARED_FRAME_GRAPH_H
#define JOBSYSTEM_DECLARED_FRAME_GRAPH_H

namespace jobsystem
{

class IWorkerBackend;

// A domain is a caller-defined resource id. Two accesses conflict when they
// name the same domain and at least one writes. FRAME_DOMAIN_ALL conflicts
// with every domain: an unaudited legacy node declares it.
struct FrameAccess
{
	unsigned domain;
	bool write;
};

enum
{
	FRAME_DOMAIN_ALL = 0xFFFFFFFFu
};

enum FrameNodeKind
{
	FRAME_NODE_HOST,  // run(context) on the calling thread
	FRAME_NODE_BATCH, // items [0, count(context)) on bounded runners
};

// Callbacks may not throw or longjmp past the graph, and may not block on the
// backend's pool. Batch items must be independent: process(context, i) for
// different i may run at the same time. count is read once per run, by the
// first runner (on any thread), after every node the batch depends on and
// before any item. begin/end (optional) bracket each participating runner on
// its own thread. The pooled executor runs in waves: a batch overlaps the host
// nodes that become ready together with it.
struct FrameNodeDesc
{
	const char *name; // stable, non-empty
	FrameNodeKind kind;
	void *context; // read on every run
	void ( *run )( void *context );
	unsigned ( *count )( void *context );
	void ( *process )( void *context, unsigned index );
	void ( *begin )( void *context );
	void ( *end )( void *context );
	const FrameAccess *access; // borrowed for the call
	unsigned accessCount;
};

enum FrameGraphMode
{
	FRAME_GRAPH_SERIAL, // one thread, array order, batch items ascending
	FRAME_GRAPH_POOLED, // host nodes on the caller, batch runners on the backend
};

struct DeclaredFrameRun
{
	bool valid;              // false: invalid input or graph; no node ran
	unsigned hostNodesRun;   // host nodes whose run() was called
	unsigned batchItemsRun;  // batch items processed
	unsigned batchRunners;   // runners per batch in this run's graph
};

// The sealed graph is reused while the shape (node names, kinds, callbacks,
// declarations, mode and runner count) is unchanged. One thread uses an
// instance at a time; runs do not nest.
class DeclaredFrameGraph
{
public:
	DeclaredFrameGraph();
	~DeclaredFrameGraph();

	DeclaredFrameRun Run( const FrameNodeDesc *nodes, unsigned count, IWorkerBackend *backend,
	    FrameGraphMode mode );

	// For the last sealed shape: nodes a and b are unordered by the declared
	// edges, so a pooled run may execute them at the same time.
	bool MayOverlap( unsigned a, unsigned b ) const;
	unsigned OverlappingPairs() const;

	// Diagnostics: number of graph (re)builds since construction.
	unsigned SealCount() const;

private:
	DeclaredFrameGraph( const DeclaredFrameGraph & );
	DeclaredFrameGraph &operator=( const DeclaredFrameGraph & );

	struct Impl;
	Impl *m_impl;
};

} // namespace jobsystem

#endif // JOBSYSTEM_DECLARED_FRAME_GRAPH_H
