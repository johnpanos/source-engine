#ifndef JOB_SCHEDULER_H
#define JOB_SCHEDULER_H

#include "jobsystem/parallel_batch.h"
#include "jobsystem/worker_backend.h"
#include "tier0/dbg.h"

// Assuming a hypothetical Box3D task interface similar to Box2D 3.0:
// typedef void b3TaskCallback(int32_t startIndex, int32_t endIndex, uint32_t threadContext, void* taskContext);

class CJobSystemBox3DTaskScheduler
{
public:
	CJobSystemBox3DTaskScheduler( jobsystem::IWorkerBackend *pBackend ) 
		: m_pBackend( pBackend )
	{
	}

	struct BatchContext {
		void (*taskFunc)(int, int, unsigned int, void*);
		void* taskContext;
	};

	// This method mimics the Box3D enqueue callback signature.
	void* EnqueueTask( void (*taskFunc)(int, int, unsigned int, void*), int itemCount, int minRange, void* taskContext )
	{

		BatchContext* pContext = new BatchContext;
		pContext->taskFunc = taskFunc;
		pContext->taskContext = taskContext;

		jobsystem::BatchDesc desc;
		desc.name = "Box3D_ParallelTask";
		desc.context = pContext;
		desc.count = itemCount;
		desc.process = []( void *ctx, unsigned index ) {
			BatchContext* pBatchCtx = static_cast<BatchContext*>(ctx);
			// Pass index as startIndex, index+1 as endIndex, threadContext as 0 (or actual thread ID)
			pBatchCtx->taskFunc( index, index + 1, 0, pBatchCtx->taskContext );
		};
		desc.maxParticipants = 0; // Use all available

		// Execute inline (blocks until the batch completes)
		jobsystem::ExecuteParallelBatch( desc, m_pBackend, jobsystem::BatchMode::Parallel );
		
		return pContext; // Return context as the "task handle"
	}

	void FinishTask( void* taskHandle )
	{
		// ExecuteParallelBatch is already synchronous, so the task is guaranteed complete here.
		BatchContext* pContext = static_cast<BatchContext*>(taskHandle);
		delete pContext;
	}

private:
	jobsystem::IWorkerBackend *m_pBackend;
};

#endif // JOB_SCHEDULER_H
